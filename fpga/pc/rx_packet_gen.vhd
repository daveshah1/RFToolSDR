library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

--RX I/Q data packetiser
--Copyright (C) 2016 David Shah
--Licensed under the MIT License

entity rx_packet_gen is
  port(
    clock : in std_logic;
    enable : in std_logic;
    reset : in std_logic;

    --IQ samples FIFO interface
    iq_fifo_q : in std_logic_vector(23 downto 0);
    iq_fifo_empty : in std_logic;
    iq_fifo_rden : out std_logic;

    --Rx packet FIFO interface
    packet_fifo_d : out std_logic_vector(31 downto 0);
    packet_fifo_wren : out std_logic;
    packet_fifo_full : in std_logic;

    --System signals
    streaming_mode : in std_logic
  );
end rx_packet_gen;

architecture Behavioral of rx_packet_gen is
  constant packet_len : natural := 256; --packet length in words
  constant samples_per_packet : natural := 255;  --number of samples in each packet

  constant rx_packet_magic : std_logic_vector(31 downto 0) := x"51495852"; --"RXIQ" (little endian) in ASCII

  signal packet_word_count : natural range 0 to packet_len - 1 := 0;
  signal in_packet : std_logic := '0';
  signal fifo_empty_last : std_logic;
begin
  iq_fifo_rden <= not iq_fifo_empty;

  packet_fifo_wren <= (not fifo_empty_last) and (not packet_fifo_full);
  --Use a simple encoding that ensures alignment can always be determined
  --This is not very efficient and a packet based approach with a header would be much better
  packet_fifo_d <= "11" & iq_fifo_q(23 downto 18) & "10" & iq_fifo_q(17 downto 12) & "01" & iq_fifo_q(11 downto 6) & "00" & iq_fifo_q(5 downto 0);
  process(clock)
  begin
    if rising_edge(clock) then
      fifo_empty_last <= iq_fifo_empty;
    end if;
  end process;

end Behavioral;
