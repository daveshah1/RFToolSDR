library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

--Signal Generator Top Level
--Copyright (C) 2017 David Shah
--Licensed under the MIT License

--Register map

--Register 0: general config
--  1..0: mode
--          0 = off, 01 = sine, 10 = noise, 11 = DC
--Register 1: sine config amp/freq
--  31..24: amplitude
--  23..0: frequency (DDS, 1LSB = Fsamp / 2^24)
--Register 2: sine config IQ
--  11..0: IQ phase offset
--All other registers and bits RFU

entity siggen_top is
  port(
    user_clock : in std_logic; --Configuration port clock
    reset : in std_logic; --System reset

    --Configuration port
    address : in std_logic_vector(7 downto 0);
    data : in std_logic_vector(31 downto 0);
    write_enable : in std_logic;

    --Radio interface
    radio_clock : in std_logic;
    radio_req : in std_logic;
    radio_i : out std_logic_vector(11 downto 0);
    radio_q : out std_logic_vector(11 downto 0)
  );
end siggen_top;

architecture Behavioral of siggen_top is
  constant addr_config : std_logic_vector(7 downto 0) := x"00";
  constant addr_sine_amp_freq : std_logic_vector(7 downto 0) := x"01";
  constant addr_sine_iq_offset : std_logic_vector(7 downto 0) := x"02";

  signal reg_config_pre, reg_sine_amp_freq_pre, reg_sine_iq_offset_pre : std_logic_vector(31 downto 0) := (others => '0');
  signal reg_config, reg_sine_amp_freq, reg_sine_iq_offset : std_logic_vector(31 downto 0) := (others => '0');

  signal signal_mode : std_logic_vector(1 downto 0);

  signal sine_i, sine_q, noise_i, noise_q, dc_i, dc_q : std_logic_vector(11 downto 0);
begin

  process(user_clock, reset)
  begin
    if reset = '1' then
      reg_config_pre <= (others => '0');
      reg_sine_amp_freq_pre <= (others => '0');
      reg_sine_iq_offset_pre <= (others => '0');
    elsif rising_edge(user_clock) then
      if write_enable = '1' then
        case address is
          when addr_config =>
            reg_config_pre <= data;
          when addr_sine_amp_freq =>
            reg_sine_amp_freq_pre <= data;
          when addr_sine_iq_offset =>
            reg_sine_iq_offset_pre <= data;
          when others =>
            --nothing
        end case;
      end if;
    end if;
  end process;

  --Synchronise registers to the radio clock domain in which they are used
  process(radio_clock, reset)
  begin
    if reset = '1' then
      reg_config <= (others => '0');
      reg_sine_amp_freq <= (others => '0');
      reg_sine_iq_offset <= (others => '0');
    elsif rising_edge(radio_clock) then
      reg_config <= reg_config_pre;
      reg_sine_amp_freq <= reg_sine_amp_freq_pre;
      reg_sine_iq_offset <= reg_sine_iq_offset_pre;
    end if;
  end process;
  signal_mode <= reg_config(1 downto 0);

  sine_gen : entity work.dds_iq_sine_gen
    port map(
      clock => radio_clock,
      reset => reset,
      enable => radio_req,
      frequency => reg_sine_amp_freq(23 downto 0),
      global_phase => x"000000",
      global_phase_load => '0',
      q_phase_offset => reg_sine_iq_offset(11 downto 0),
      i_amplitude => reg_sine_amp_freq(31 downto 24),
      q_amplitude => reg_sine_amp_freq(31 downto 24),

      i_out => sine_i,
      q_out => sine_q);

  i_noise_gen : entity work.whitenoise_gen
    generic map(
      int_width => 32,
      ext_width => 12,
      num_lfsrs => 20,
      lfsr_poly => x"80000057")
    port map(
      clock => radio_clock,
      reset => reset,
      enable => radio_req,
      data => noise_i);

  q_noise_gen : entity work.whitenoise_gen
    generic map(
      int_width => 32,
      ext_width => 12,
      num_lfsrs => 20,
      lfsr_poly => x"80000EA6")
    port map(
      clock => radio_clock,
      reset => reset,
      enable => radio_req,
      data => noise_q);

  --Convert amplitude to signed, and extend LSB
  dc_i <= "0" & reg_sine_amp_freq(31 downto 24) & reg_sine_amp_freq(24) & reg_sine_amp_freq(24) & reg_sine_amp_freq(24);
  dc_q <= x"000";

  radio_i <= x"000"   when signal_mode = "00" else
             sine_i   when signal_mode = "01" else
             noise_i  when signal_mode = "10" else
             dc_i;

  radio_q <= x"000"   when signal_mode = "00" else
            sine_q   when signal_mode = "01" else
            noise_q  when signal_mode = "10" else
            dc_q;
end architecture;
