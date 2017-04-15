library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

--Simple Generic Galois LFSR
--Copyright (C) 2017 David Shah
--Licensed under the MIT License

entity galois_lfsr is
  generic(
    size : natural := 32;
    polynomial : std_logic_vector := x"80000057";
    init : std_logic_vector := x"2b8e9b90"
  );
  port(
    clock : in std_logic;
    enable : in std_logic;
    reset : in std_logic; --active high sync reset
    data : out std_logic_vector(size-1 downto 0)
  );
end galois_lfsr;

architecture Behavioral of galois_lfsr is

  signal reg_d : std_logic_vector(size-1 downto 0);
  signal reg_q : std_logic_vector(size-1 downto 0) := init;

begin

  reg_d <= (("0" & reg_q(size-1 downto 1)) xor polynomial) when reg_q(0) = '1' else
           ("0" & reg_q(size-1 downto 1));

  process(clock)
  begin
    if rising_edge(clock) then
      if reset = '1' then
        reg_q <= init;
      else
        if enable = '1' then
          reg_q <= reg_d;
        end if;
      end if;
    end if;
  end process;

  data <= reg_q;
end Behavioral;
