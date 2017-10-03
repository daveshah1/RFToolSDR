library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

--PC control interface handler
--Copyright (C) 2016 David Shah
--Licensed under the MIT License

--This interfaces using small FIFOs (mostly for clock domain crossing)
--with the FTDI side USB interface

--It parses the simple command protocol; handling switches between control
--and IQ streaming mode; and controls the SPI interface based on received commands

--Protocol information
--Command format -- from PC to SDR
--All commands are 32 bytes long for simplicity
--Start: 4-byte magic header ASCII "CMND"
--Followed by 1-byte command type
--Followed by 27 bytes of payload, with 0x00 for padding at end if needed

--Response format -- from SDR to PC
--Also always 32 bytes long
--Start: 4-byte magic "RESP"
--Followed by 1-byte command type, same as in command
--Followed by 27 bytes of payload

--Supported commands:

entity pc_control_handler is
  port(
    clock : in std_logic; --clock for all interfaces (sourced from FPGA oscillator)
    enable : in std_logic; --clock enable
    reset : in std_logic; --active high synchronous reset

    --USB command FIFO
    cmd_fifo_q : in std_logic_vector(7 downto 0);
    cmd_fifo_rden : out std_logic;
    cmd_fifo_empty : in std_logic;
    cmd_fifo_prog_empty : in std_logic; --asserted when less than a full command in the fifo

    --USB command response FIFO
    resp_fifo_d : out std_logic_vector(7 downto 0);
    resp_fifo_wren : out std_logic;

    --SPI controller interface
    spi_rden : in std_logic;
    spi_din : out std_logic_vector(7 downto 0);
    spi_wren : in std_logic;
    spi_q : in std_logic_vector(7 downto 0);
    spi_start_xfer : out std_logic;
    spi_end_xfer : out std_logic;
    spi_done : in std_logic;

    --Signal generator interface
    siggen_address : out std_logic_vector(7 downto 0);
    siggen_data : out std_logic_vector(31 downto 0);
    siggen_wren : out std_logic;

    --Control outputs
    streaming_mode : out std_logic; --Asserted when in streaming mode
    control_signals : out std_logic_vector(7 downto 0) --General purpose control outputs
  );
end pc_control_handler;

architecture Behavioral of pc_control_handler is

  --Command and response header "magic bytes"
  type magic_t is array(0 to 3) of std_logic_vector(7 downto 0);
  constant command_magic : magic_t := (x"43", x"4d", x"4e", x"44");
  constant response_magic : magic_t := (x"52", x"45", x"53", x"50");

  --Command types
  subtype cmd_t is std_logic_vector(7 downto 0);

  --SPI transfer: followed by 1-byte length and up to 26-bytes data
  --Response payload lines up with command payload; first two SPI data bytes are echoed back and remainder are SPI read data
  constant cmd_spi_transfer : cmd_t := x"10";

  --Set control signals: followed by 1-byte to set control outputs to; remainder padding
  constant cmd_set_ctrl_sig : cmd_t := x"20";

  --Enter streaming mode: this does not produce a response as the SDR to PC stream is immediately switched to IQ mode
  constant cmd_enter_stream_mode : cmd_t := x"30";

  --Leave streaming mode
  constant cmd_leave_stream_mode : cmd_t := x"31";

  --Signal generator write: this does not produce as a response as it is often used within streaming mode
  constant cmd_siggen_write : cmd_t := x"40";

  constant command_length : natural := 32;
  constant response_length : natural := 32;

  signal command_bytes_read : natural range 0 to command_length := 0;
  signal response_bytes_written : natural range 0 to response_length := 0;

  signal spi_bytes_transferred : natural range 0 to 26 := 0;
  signal spi_transfer_len : natural range 0 to 26 := 0;

  signal streaming_mode_reg : std_logic := '0';
  signal control_signals_reg : std_logic_vector(7 downto 0) := x"00";

  signal current_state : natural range 0 to 15;

  signal current_command : std_logic_vector(7 downto 0);

  signal cmd_rden_int, resp_wren_int : std_logic;
  signal spi_cmd_rden_int, spi_resp_wren_int : std_logic;
  signal spi_resp_d : std_logic_vector(7 downto 0);

  signal spi_rden_last : std_logic := '0';

  signal siggen_address_reg : std_logic_vector(7 downto 0);
  signal siggen_data_reg : std_logic_vector(31 downto 0);
  signal siggen_byte_count : natural range 0 to 5;
begin

  --Main state machine
  process(clock)
  begin
    if rising_edge(clock) then
      if reset = '1' then
        current_state <= 0;
        streaming_mode_reg <= '0';
        control_signals_reg <= x"00";
      elsif enable = '1' then
        case current_state is
          when 0 => --waiting for start
            if cmd_fifo_prog_empty = '0' then
              current_state <= 1;
            end if;

          when 1 => --checking for command header magic
            if command_bytes_read >= 1 then
              if cmd_fifo_q /= command_magic(command_bytes_read - 1) then --header magic mismatch
                current_state <= 15;
              elsif command_bytes_read = 4 then --all 4 magics match
                current_state <= 2;
              end if;
            end if;

          when 2 => --read command type
            current_command <= cmd_fifo_q;
            if cmd_fifo_q = cmd_enter_stream_mode or cmd_fifo_q = cmd_siggen_write then --this command has no response
              current_state <= 4;
            else
              current_state <= 3;
            end if;

          when 3 => --write out response start
            if response_bytes_written = 4 then
              current_state <= 4;
            end if;
          when 4 => --read first payload byte
            current_state <= 5;
          when 5 => --process command
            case current_command is
              when cmd_spi_transfer =>
                spi_transfer_len <= to_integer(unsigned(cmd_fifo_q));
                current_state <= 6;
              when cmd_enter_stream_mode =>
                streaming_mode_reg <= '1';
                current_state <= 13;
              when cmd_leave_stream_mode =>
                streaming_mode_reg <= '0';
                current_state <= 13;
              when cmd_set_ctrl_sig =>
                control_signals_reg <= cmd_fifo_q;
                current_state <= 13;
              when cmd_siggen_write =>
                siggen_address_reg <= cmd_fifo_q;
                current_state <= 9;
                siggen_byte_count <= 0;
              when others => --unknown/unsupported command: end without error for now
                current_state <= 13;
            end case;

          when 6 => --SPI transfer start
            current_state <= 7;
          when 7 => --SPI transfer in progress
            if spi_done = '1' then
              current_state <= 13;
            end if;

          when 9 => --siggen write data xfer
            if siggen_byte_count > 4 then
              current_state <= 10;
            else
              siggen_data_reg <= cmd_fifo_q & siggen_data_reg(31 downto 8);
              siggen_byte_count <= siggen_byte_count + 1;
            end if;
          when 10 => --siggen write out
            current_state <= 13;

          when 13 => --end of command: read out any remaining payload bytes
            if command_bytes_read >= command_length then
              if current_command = cmd_enter_stream_mode or current_command = cmd_siggen_write then
                current_state <= 0;
              else
                current_state <= 14;
              end if;
            end if;
          when 14 => --end of command: pad response with zeros
            if response_bytes_written >= response_length then
              current_state <= 0;
            end if;
          when 15 => --'drain' command FIFO in case of a header error
            if cmd_fifo_empty = '1' then
              current_state <= 0;
            end if;

          when others =>
            current_state <= 0;
        end case;
      end if;
    end if;
  end process;

  --Command and response length counters
  process(clock)
  begin
    if rising_edge(clock) then
      if reset = '1' then
        command_bytes_read <= 0;
        response_bytes_written <= 0;
      elsif enable = '1' then
        if current_state = 0 then
          command_bytes_read <= 0;
          response_bytes_written <= 0;
        else
          if cmd_rden_int = '1' then
            if command_bytes_read < command_length then
              command_bytes_read <= command_bytes_read + 1;
            end if;
          end if;

          if resp_wren_int = '1' then
            if response_bytes_written < response_length then
              response_bytes_written <= response_bytes_written + 1;
            end if;
          end if;
        end if;
      end if;
    end if;
  end process;

  cmd_rden_int <= '1' when (current_state = 1) or (current_state = 4) or
                           ((current_state = 13) and (command_bytes_read < command_length)) or
                           ((current_state = 15) and (cmd_fifo_empty = '0')) or
                           (((current_state = 6) or (current_state = 7)) and spi_cmd_rden_int = '1') or
                           (current_state = 9)
                            else '0';

  resp_wren_int <= '1' when (current_state = 3) or (((current_command /= cmd_enter_stream_mode) and (current_command /= cmd_siggen_write))
                            and ((current_state = 5) or ((current_state = 7) and (spi_resp_wren_int = '1'))
                            or ((current_state = 14) and (response_bytes_written < response_length)))) else '0';

  resp_fifo_d <= response_magic(response_bytes_written) when ((current_state = 3) and (response_bytes_written < 4)) else
                 current_command when ((current_state = 3) and (response_bytes_written = 4)) else
                 cmd_fifo_q when (current_state = 5) else
                 spi_resp_d when (current_state = 7) else
                 x"00";

  spi_cmd_rden_int <= spi_rden;
  spi_din <= cmd_fifo_q;
  spi_start_xfer <= '1' when (current_state = 6) else '0';

  cmd_fifo_rden <= cmd_rden_int;
  resp_fifo_wren <= resp_wren_int;

  --Echo back header for first two bytes; then return SPI transfer result for subsequent bytes
  spi_resp_wren_int <= '1' when ((spi_bytes_transferred < 2) and (spi_rden_last = '1')) or
                                ((spi_bytes_transferred >= 2) and (spi_wren = '1')) else '0';
  spi_resp_d <= cmd_fifo_q when spi_bytes_transferred < 2 else spi_q;

  spi_end_xfer <= '1' when spi_bytes_transferred >= spi_transfer_len else '0';

  process(clock)
  begin
    if rising_edge(clock) then
      if reset = '1' then
        spi_bytes_transferred <= 0;
        spi_rden_last <= '0';
      elsif enable = '1' then
        spi_rden_last <= spi_rden;
        if current_state = 7 then
          if spi_wren = '1' then
            spi_bytes_transferred <= spi_bytes_transferred + 1;
          end if;
        else
          spi_bytes_transferred <= 0;
        end if;
      end if;
    end if;
  end process;

  siggen_address <= siggen_address_reg;
  siggen_data <= siggen_data_reg;
  siggen_wren <= '1' when current_state = 10 else '0';

  streaming_mode <= streaming_mode_reg;
  control_signals <= control_signals_reg;
  --TODO: assign control signals based on current state
end Behavioral;
