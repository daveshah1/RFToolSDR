library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

library UNISIM;
use UNISIM.VComponents.all;

--FT60x 245-style FIFO interface
--Copyright (C) 2016 David Shah
--Licensed under the MIT License

entity ft60x_fifo_if is
  port(
    reset : in std_logic; --Active high, async
    enable : in std_logic; --Active high

    ftdi_clock : in std_logic;
    ftdi_txe_n : in std_logic;
    ftdi_rxf_n : in std_logic;
    ftdi_oe_n : out std_logic;
    ftdi_wr_n : out std_logic;
    ftdi_rd_n : out std_logic;
    ftdi_be : inout std_logic_vector(3 downto 0);
    ftdi_data : inout std_logic_vector(31 downto 0);

    --Buffered FTDI clock out to drive FIFOs, etc
    clock_out : out std_logic;

    --Command FIFO write port
    command_fifo_d : out std_logic_vector(31 downto 0);
    command_fifo_wren : out std_logic;
    command_fifo_prog_full : in std_logic; --asserted when less than full command's space in the FIFO

    --Response FIFO read port
    resp_fifo_q : in std_logic_vector(31 downto 0);
    resp_fifo_rden : out std_logic;
    resp_fifo_prog_empty : in std_logic; --asserted when response FIFO contains less than a full response

    --Receive data FIFO read port
    rx_fifo_q : in std_logic_vector(31 downto 0);
    rx_fifo_rden : out std_logic;
    rx_fifo_prog_empty : in std_logic; --asserted when rx data FIFO contains less than a full RX packet

    --Misc signals
    streaming_mode : in std_logic --asserted when in streaming mode (i.e. pulling data from RX FIFO)
  );
end ft60x_fifo_if;

architecture Behavioral of ft60x_fifo_if is

  function max(a, b: natural) return natural is
  begin
    if a > b then return a;
             else return b;
    end if;
  end max;

  constant command_len : natural := 32/4;
  constant response_len : natural := 32/4;
  constant rx_packet_len : natural := 4096/4;

  signal clock_buf : std_logic; --FTDI clock after input buffer

  signal current_state : natural range 0 to 7 := 0;
  signal word_count : natural range 0 to max(rx_packet_len, max(response_len, command_len)) := 0;

  signal streaming_mode_lat : std_logic;
begin
  clkbuf : IBUFG
    generic map(
      IBUF_LOW_PWR => FALSE,
      IOSTANDARD => "DEFAULT")
    port map(
      O => clock_buf,
      I => ftdi_clock);

  clock_out <= clock_buf;

  process(clock_buf, reset)
  begin
    if reset = '1' then
      current_state <= 0;
    elsif rising_edge(clock_buf) then
      if enable = '1' then
        case current_state is
          when 0 => --waiting for event
            if ftdi_rxf_n = '0' then
              current_state <= 1;
            elsif ftdi_txe_n = '0' and resp_fifo_prog_empty = '0' and streaming_mode = '0' then
              current_state <= 4;
            elsif ftdi_txe_n = '0' and rx_fifo_prog_empty = '0' and streaming_mode = '1' then
              current_state <= 6;
            end if;
          when 1 => --read out command start
            current_state <= 2;
          when 2 => --read out command start 2
            current_state <= 3;
          when 3 => --read out command
            if word_count = command_len - 1 then
              current_state <= 0;
            end if;
          when 4 => --write response start
            current_state <= 5;
          when 5 => --write response
            if word_count = response_len - 1 then
              current_state <= 0;
            end if;
          when 6 => --write rx data start
            current_state <= 7;
          when 7 => --write rx data
            if word_count = rx_packet_len - 1 then
              current_state <= 0;
            end if;
          when others =>
            current_state <= 0;
        end case;
      end if;
    end if;
  end process;


  ftdi_wr_n <= '0' when (current_state = 5) or (current_state = 7) else '1';
  ftdi_rd_n <= '0' when (current_state = 2) or (current_state = 3) else '1';
  ftdi_oe_n <= '0' when (current_state = 1) or (current_state = 2) or (current_state = 3) else '1';

  command_fifo_wren <= '1' when ((current_state = 2) or (current_state = 3)) and (ftdi_be = "1111")  and (ftdi_rxf_n = '0') else '0';
  resp_fifo_rden <= '1' when (current_state = 4) or ((current_state = 5) and (word_count < (response_len - 1))) else '0';
  rx_fifo_rden <= '1' when (current_state = 6) or ((current_state = 7) and (word_count < (rx_packet_len - 1))) else '0';

  command_fifo_d <= ftdi_data when (current_state = 2) or (current_state = 3) else (others => '0');

  ftdi_data <= resp_fifo_q when current_state = 5 else
               rx_fifo_q when current_state = 7 else
               (others => 'Z');

  ftdi_be <= "1111" when (current_state = 5) or (current_state = 7) else "ZZZZ";

  process(clock_buf, reset)
  begin
    if reset = '1' then
      word_count <= 0;
      streaming_mode_lat <= '0';
    elsif rising_edge(clock_buf) then
      if enable = '1' then
        streaming_mode_lat <= streaming_mode;
        if current_state = 0 then
          word_count <= 0;
        elsif (current_state = 3) or (current_state = 5) or (current_state = 7) then
          word_count <= word_count + 1;
        end if;
      end if;
    end if;
  end process;

end Behavioral;
