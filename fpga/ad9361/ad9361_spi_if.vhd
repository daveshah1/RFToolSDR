library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

--Simple SPI Interface
--Copyright (C) 2016 David Shah
--Licensed under the MIT License

entity ad9361_spi_if is
  generic(
    clock_div : natural := 8; --clock to spi_clock division factor; must be >= 4; also must be even for even duty cycle out
    pre_delay : natural := 5; --number of input clock cycles between asserting CS and first SCK cycle; must be >= 1
    post_delay : natural := 5 --number of input clock cycle between last SCK cycle and deasserting CS; must be >= 1
  );
  port(
    clock : in std_logic; --clock for FPGA side interface; divided by at least two for SPI
    reset : in std_logic; --active high sync reset
    enable : in std_logic; --active high enable
    --Command interface
    data_rden : out std_logic; --this is asserted one cycle before data_in is read
    data_in : in std_logic_vector(7 downto 0);

    data_wren : out std_logic; --this is asserted when data_out is new and valid
    data_out : out std_logic_vector(7 downto 0);
    --Handshaking
    start_xfer : in std_logic; --assert for one clock cycle to start a transfer
    end_xfer : in std_logic; --assert this when requested instead of setting data_in to end transfer
    ready : out std_logic; --asserted while no transfer running i.e. transfer completed

    --SPI port
    spi_clock : out std_logic;
    spi_cs_n : out std_logic;
    spi_mosi : out std_logic;
    spi_miso : in std_logic);
end ad9361_spi_if;

architecture Behavioral of ad9361_spi_if is

  function max(a, b: natural) return natural is
  begin
    if a > b then return a;
             else return b;
    end if;
  end max;

  signal time_in_sck : natural range 0 to clock_div - 1 := 0;
  constant half_period : natural := clock_div / 2;
  constant sck_end : natural := clock_div - 1;
  signal sck_pre : std_logic;
  signal current_state : natural range 0 to 3 := 0;

  signal delay_counter : natural range 0 to max(pre_delay, post_delay) - 1 := 0;
  signal bit_counter : natural range 0 to 7;

  signal output_shiftreg : std_logic_vector(7 downto 0);
  signal input_shiftreg : std_logic_vector(7 downto 0);

begin


  --Control state machine
  process(clock)
  begin
    if rising_edge(clock) then
      if reset = '1' then
        current_state <= 0;
        delay_counter <= 0;
      elsif enable = '1' then
        case current_state is
          when 0 => --wait for start
            if start_xfer = '1' then --setup at start of transfer
              current_state <= 1;
              delay_counter <= 0;
            end if;
          when 1 => --assert CS; pre-data delay
            if delay_counter >= pre_delay - 1 then
              delay_counter <= 0;
              current_state <= 2;
            else
              delay_counter <= delay_counter + 1;
            end if;
          when 2 => --actual SPI transfer
            if bit_counter = 7 and time_in_sck = 0 and end_xfer = '1' then
              current_state <= 3;
            end if;
          when 3 => --end of transfer
            if delay_counter >= post_delay - 1 then
              delay_counter <= 0;
              current_state <= 0;
            else
              delay_counter <= delay_counter + 1;
            end if;
          when others =>
            current_state <= 0;
        end case;
      end if;
    end if;
  end process;

  --Counters
  process(clock)
  begin
    if rising_edge(clock) then
      if reset = '1' then
        time_in_sck <= 0;
        bit_counter <= 7;
      elsif enable = '1' then
        if current_state = 2 then
          if time_in_sck = sck_end then
            time_in_sck <= 0;
            if bit_counter = 0 then
              bit_counter <= 7;
            else
              bit_counter <= bit_counter - 1;
            end if;
          else
            time_in_sck <= time_in_sck + 1;
          end if;
        else
          time_in_sck <= 0;
          bit_counter <= 7;
        end if;
      end if;
    end if;
  end process;

  --Shift registers
  process(clock)
  begin
    if rising_edge(clock) then
      if reset = '1' then
        input_shiftreg <= (others => '0');
        output_shiftreg <= (others => '0');
        spi_clock <= '0';
      elsif enable = '1' then
        if current_state = 2 then
          if not (bit_counter = 7 and time_in_sck = 0 and end_xfer = '1') then
            spi_clock <= sck_pre;
          end if;
          if time_in_sck = sck_end then
            output_shiftreg(7 downto 1) <= output_shiftreg(6 downto 0);
            output_shiftreg(0) <= '0';
          elsif time_in_sck = 0 and bit_counter = 7 then
            output_shiftreg <= data_in;
          end if;

          if bit_counter <= 7 and time_in_sck = half_period then
            input_shiftreg(7 downto 1) <= input_shiftreg(6 downto 0);
            input_shiftreg(0) <= spi_miso;
          end if;
        else
          spi_clock <= '0';
        end if;
      end if;
    end if;
  end process;

  ready <= '1' when current_state = 0 else '0';
  spi_cs_n <= '1' when current_state = 0 else '0';
  spi_mosi <= output_shiftreg(7) when current_state = 2 else '0';
  sck_pre <= '1' when (current_state = 2) and (time_in_sck < half_period) else '0';

  data_rden <= '1' when (current_state = 1 and delay_counter = pre_delay - 1) or
                        (current_state = 2 and time_in_sck = sck_end and bit_counter = 0)
                   else '0';

  data_out <= input_shiftreg;
  data_wren <= '1' when bit_counter = 0 and time_in_sck = sck_end else '0';
end architecture;
