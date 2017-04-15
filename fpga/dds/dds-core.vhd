library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

--Direct Digital Synthesis Core
--Copyright (C) 2017 David Shah
--Licensed under the MIT License

entity dds_core is
  generic (
    N : natural := 24; --frequency word size
    M : natural := 12  --phase out size (M <= N)
  );
  port (
    clock : in std_logic; --control and synthesis clock
    reset : in std_logic; --sync reset
    enable : in std_logic;
    freq_val : in std_logic_vector(N-1 downto 0); --frequency setting word
    phase_val : in std_logic_vector(N-1 downto 0); --phase setting word
    phase_load : in std_logic; --update phase
    phase_out : out std_logic_vector(M-1 downto 0) --phase output
  );
end dds_core;

architecture Behavioral of dds_core is

  signal freq_reg : unsigned(N-1 downto 0) := (others => '0');
  signal phase_acc : unsigned(N-1 downto 0) := (others => '0');
  signal phase_out_reg : std_logic_vector(M-1 downto 0) := (others => '0');
begin
  process(clock)
  begin
    if rising_edge(clock) then
      if reset = '1' then
        freq_reg <= (others => '0');
        phase_acc <= (others => '0');
        phase_out_reg <= (others => '0');
      elsif enable = '1' then
        freq_reg <= unsigned(freq_val);
        if phase_load = '1' then
          phase_acc <= unsigned(phase_val);
        else
          phase_acc <= phase_acc + freq_reg;
        end if;
        phase_out_reg <= std_logic_vector(phase_acc(N-1 downto (N-M)));
      end if;
    end if;
  end process;
  phase_out <= phase_out_reg;
end Behavioral;
