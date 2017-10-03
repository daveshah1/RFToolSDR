library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.MATH_REAL.ALL;

--Parameterisable Gaussian White Noise Generator
--Copyright (C) 2017 David Shah
--Licensed under the MIT License

--This uses a number of LFSR pseudorandom number generators in combination with
--central limit theorem to generate gaussian noise for test purposes

entity whitenoise_gen is
  generic(
    int_width : natural := 32; --width of LFSRs and interal calcs
    ext_width : natural := 12; --width of external value output
    num_lfsrs : natural := 20;  --number of parallel LFSRs
    lfsr_poly : std_logic_vector := x"80000057" --LFSR polynomial
  );
  port(
    clock : in std_logic;
    enable : in std_logic;
    reset : in std_logic;
    data : out std_logic_vector(ext_width - 1 downto 0)
  );
end whitenoise_gen;

architecture Behavioral of whitenoise_gen is

  type lfsr_value_t is array(0 to num_lfsrs - 1) of std_logic_vector(int_width - 1 downto 0);

  signal lfsr_q : lfsr_value_t;

  constant sum_width : natural := natural(log2(real(num_lfsrs))) + int_width;

  signal sum_d : std_logic_vector(sum_width - 1 downto 0) := (others => '0');
  signal sum_q : std_logic_vector(sum_width - 1 downto 0);

begin

  lfsr_gen: for i in 0 to num_lfsrs - 1 generate
    lfsr_i : entity work.galois_lfsr
      generic map(
        size => int_width,
        polynomial => lfsr_poly,
        init => std_logic_vector(to_unsigned( i  + 1, int_width)))
      port map(
        clock => clock,
        enable => enable,
        reset => reset,
        data => lfsr_q(i));
  end generate;

  process(lfsr_q)
    variable sum : signed(sum_width - 1 downto 0);
  begin
    sum := (others => '0');
    for i in 0 to num_lfsrs - 1 loop
      sum := sum + signed(lfsr_q(i));
    end loop;
    sum_d <= std_logic_vector(sum);
  end process;

  process(clock)
  begin
    if rising_edge(clock) then
      if reset = '1' then
        sum_q <= (others => '0');
      elsif enable = '1' then
        sum_q <= sum_d;
      end if;
    end if;
  end process;

  data <= sum_q(int_width - 1 downto (int_width - ext_width));
end Behavioral;
