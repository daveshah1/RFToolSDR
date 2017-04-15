library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use STD.textio.ALL;

entity dds_testbench is
end dds_testbench;

architecture Behavioral of dds_testbench is

  signal clock : std_logic := '0';
  signal i_val, q_val : std_logic_vector(11 downto 0);
  signal frequency : std_logic_vector(23 downto 0);
  signal q_phase : std_logic_vector(11 downto 0);
  signal i_amp, q_amp : std_logic_vector(7 downto 0);

  file outfile : text;
begin

  frequency <= x"051EB8"; --1MHz with a 50MHz clock
  q_phase <= x"400"; --90 degree offset
  i_amp <= x"FF";
  q_amp <= x"7F";

  gen : entity work.dds_iq_sine_gen
    port map(
      clock => clock,
      reset => '0',
      enable => '1',
      frequency => frequency,
      global_phase => x"000000",
      global_phase_load => '0',
      q_phase_offset => q_phase,
      i_amplitude => i_amp,
      q_amplitude => q_amp,
      i_out => i_val,
      q_out => q_val);

  process
    variable i_tmp, q_tmp : integer;
    variable oline : line;
  begin

    file_open(outfile, "output.csv", write_mode);
    write(oline, string'("t, i, q,"));
    writeline(outfile, oline);
    for i in 1 to 1000 loop
      wait for 10 ns;
      clock <= '1';
      wait for 10 ns;
      clock <= '0';

      i_tmp := to_integer(signed(i_val));
      q_tmp := to_integer(signed(q_val));

      write(oline, (now / 1 ns), left, 4);
      write(oline, string'(", "));
      write(oline, i_tmp, left, 5);
      write(oline, string'(", "));
      write(oline, q_tmp, left, 5);
      write(oline, string'(", "));

      writeline(outfile, oline);
    end loop;

    file_close(outfile);
    wait;
  end process;
end Behavioral;
