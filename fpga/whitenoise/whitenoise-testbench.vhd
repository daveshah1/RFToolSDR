library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use STD.textio.ALL;

entity wn_testbench is
end wn_testbench;

architecture Behavioral of wn_testbench is

  signal clock : std_logic := '0';
  signal i_val, q_val : std_logic_vector(11 downto 0);

  file outfile : text;
begin

  i_gen : entity work.whitenoise_gen
    generic map(
      int_width => 32,
      ext_width => 12,
      num_lfsrs => 20,
      lfsr_poly => x"80000057")
    port map(
      clock => clock,
      reset => '0',
      enable => '1',
      data => i_val);

  q_gen : entity work.whitenoise_gen
    generic map(
      int_width => 32,
      ext_width => 12,
      num_lfsrs => 20,
      lfsr_poly => x"80000EA6")
    port map(
      clock => clock,
      reset => '0',
      enable => '1',
      data => q_val);

  -- q_gen : entity work.galois_lfsr
  --   generic map(
  --     size => 12,
  --     polynomial => x"829",
  --     init => x"001")
  --   port map(
  --     clock => clock,
  --     reset => '0',
  --     enable => '1',
  --     data => q_val);

  process
    variable i_tmp, q_tmp : integer;
    variable oline : line;
  begin

    file_open(outfile, "output.csv", write_mode);
    write(oline, string'("t, i, q,"));
    writeline(outfile, oline);
    for i in 1 to 65536 loop
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
