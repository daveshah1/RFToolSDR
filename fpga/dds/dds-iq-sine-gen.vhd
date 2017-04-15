library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

--IQ DDS generator
--Copyright (C) 2017 David Shah
--Licensed under the MIT License

entity dds_iq_sine_gen is
  port(
    clock : in std_logic;
    reset : in std_logic;
    enable : in std_logic;
    frequency : in std_logic_vector(23 downto 0); --frequency setting word
    global_phase : in std_logic_vector(23 downto 0); --global phase setting
    global_phase_load : in std_logic; --assert to update phase
    q_phase_offset : in std_logic_vector(11 downto 0); --phase shift between I and Q
    i_amplitude : in std_logic_vector(7 downto 0); --I and Q amplitude scaling
    q_amplitude : in std_logic_vector(7 downto 0);

    i_out : out std_logic_vector(11 downto 0);
    q_out : out std_logic_vector(11 downto 0)
  );
end dds_iq_sine_gen;

architecture Behavioral of dds_iq_sine_gen is

  signal dds_phase : std_logic_vector(11 downto 0);
  signal i_phase_d, q_phase_d, i_phase_q, q_phase_q : std_logic_vector(11 downto 0);
  signal i_sine, q_sine : std_logic_vector(11 downto 0);
  signal i_scaled_d, q_scaled_d, i_scaled_q, q_scaled_q : std_logic_vector(19 downto 0);

begin
  dds : entity work.dds_core
    generic map(
      N => 24,
      M => 12)
    port map(
      clock => clock,
      reset => reset,
      enable => enable,
      freq_val => frequency,
      phase_val => global_phase,
      phase_load => global_phase_load,
      phase_out => dds_phase);

  i_sine_tbl : entity work.dds_sine_table
    port map(
      clock => clock,
      address => i_phase_q,
      data => i_sine);

  q_sine_tbl : entity work.dds_sine_table
    port map(
      clock => clock,
      address => q_phase_q,
      data => q_sine);

  i_phase_d <= dds_phase;
  q_phase_d <= std_logic_vector(unsigned(dds_phase) + unsigned(q_phase_offset));

  i_scaled_d <= std_logic_vector(resize(signed(i_sine) * signed("0" & i_amplitude), 20));
  q_scaled_d <= std_logic_vector(resize(signed(q_sine) * signed("0" & q_amplitude), 20));

  i_out <= i_scaled_q(19 downto 8);
  q_out <= q_scaled_q(19 downto 8);

  process(clock)
  begin
    if rising_edge(clock) then
      if reset = '1' then
        i_phase_q <= (others => '0');
        q_phase_q <= (others => '0');
        i_scaled_q <= (others => '0');
        q_scaled_q <= (others => '0');
      elsif enable = '1' then
        i_phase_q <= i_phase_d;
        q_phase_q <= q_phase_d;
        i_scaled_q <= i_scaled_d;
        q_scaled_q <= q_scaled_d;
      end if;
    end if;
  end process;
end Behavioral;
