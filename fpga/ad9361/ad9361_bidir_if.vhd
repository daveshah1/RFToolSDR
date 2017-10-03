library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

library UNISIM;
use UNISIM.VComponents.all;

--AD9361 1Rx1Tx LVDS DDR interface
--Copyright (C) 2016-17 David Shah
--Licensed under the MIT License

entity ad9361_sdr_bidir_if is
  port(
    --AD9361 side
    data_clock_p : in std_logic;
    data_clock_n : in std_logic;
    fb_clock_p : out std_logic;
    fb_clock_n : out std_logic;

    rx_frame_p : in std_logic;
    rx_frame_n : in std_logic;
    tx_frame_p : out std_logic;
    tx_frame_n : out std_logic;

    p0_d : out std_logic_vector(11 downto 0);
    p1_d : in std_logic_vector(11 downto 0);

    --User side
    user_clock : out std_logic;
    rx_data_frame : out std_logic;
    rx_data_i : out std_logic_vector(11 downto 0);
    rx_data_q : out std_logic_vector(11 downto 0);

    tx_data_i : in std_logic_vector(11 downto 0);
    tx_data_q : in std_logic_vector(11 downto 0);
    tx_data_req : out std_logic);
end ad9361_sdr_bidir_if;

--The following pairs are swapped (ie inverted) on the PCB
--RX_FRAME; TX_FRAME, TX_D0, FB_CLK, RX_D5, RX_D3, RX_D4
--TODO: change this setting to be a VHDL generic

architecture Behavioral of ad9361_sdr_bidir_if is
  signal clock_se, clock_buf, fb_clock_se : std_logic;
  signal input_data_se : std_logic_vector(5 downto 0);
  signal input_data_sdr_rp, input_data_sdr_fp, input_data_sdr_r, input_data_sdr_f : std_logic_vector(5 downto 0);
  signal output_data_se : std_logic_vector(5 downto 0);
  signal output_data_sdr_rp, output_data_sdr_fp, output_data_sdr_r, output_data_sdr_f : std_logic_vector(5 downto 0);

  signal rx_data_lat_i, tx_data_lat_i : std_logic_vector(11 downto 0);
  signal rx_data_lat_q, tx_data_lat_q : std_logic_vector(11 downto 0);
  signal rx_frame_se_pre, rx_frame_se, tx_frame_se, tx_frame_se_pre, tx_frame_se_inv : std_logic;
begin
  clkibuf : IBUFDS
    generic map(
      IBUF_LOW_PWR => FALSE,
      IOSTANDARD => "DEFAULT")
    port map(
      O => clock_se,
      I => data_clock_p,
      IB => data_clock_n
    );
  clkbufio : BUFR
    generic map(
      BUFR_DIVIDE => "BYPASS",
      SIM_DEVICE => "7SERIES")
    port map(
      O => clock_buf,
      CE => '1',
      CLR => '0',
      I => clock_se);

  fbclkdrv : ODDR
    generic map(
      DDR_CLK_EDGE => "OPPOSITE_EDGE",
      INIT => '0',
      SRTYPE => "SYNC")
    port map(
      Q => fb_clock_se,
      C => clock_buf,
      CE => '1',
      D1 => '0',
      D2 => '1',
      R => '0',
      S => '0');

  clkobuf : OBUFDS
    generic map(
      IOSTANDARD => "DEFAULT",
      SLEW => "SLOW")
    port map(
      O => fb_clock_n,
      OB => fb_clock_p,
      I => fb_clock_se);

  gen_io : for i in 0 to 5 generate
    gen_inv_i : if (i = 3) or (i = 4) or (i = 5) generate
      rxibuf : IBUFDS
        generic map(
          IBUF_LOW_PWR => FALSE,
          IOSTANDARD => "DEFAULT")
        port map(
          O => input_data_se(i),
          I => p1_d((i*2)),
          IB => p1_d((i*2)+1));
      input_data_sdr_rp(i) <= not input_data_sdr_r(i);
      input_data_sdr_fp(i) <= not input_data_sdr_f(i);
    end generate;

    gen_true_i : if (i = 0) or (i = 1) or (i = 2) generate
      rxibuf : IBUFDS
        generic map(
          IBUF_LOW_PWR => FALSE,
          IOSTANDARD => "DEFAULT")
        port map(
          O => input_data_se(i),
          I => p1_d((i*2)+1),
          IB => p1_d(i*2));
      input_data_sdr_rp(i) <= input_data_sdr_r(i);
      input_data_sdr_fp(i) <= input_data_sdr_f(i);
    end generate;

    rxiddr : IDDR
      generic map(
        DDR_CLK_EDGE => "OPPOSITE_EDGE",
        INIT_Q1 => '0',
        INIT_Q2 => '0',
        SRTYPE => "SYNC")
      port map(
        Q1 => input_data_sdr_r(i),
        Q2 => input_data_sdr_f(i),
        C => clock_buf,
        CE => '1',
        D => input_data_se(i),
        R => '0',
        S => '0');

    txoddr : ODDR
      generic map(
        DDR_CLK_EDGE => "OPPOSITE_EDGE",
        INIT => '0',
        SRTYPE => "SYNC")
      port map(
        Q => output_data_se(i),
        C => clock_buf,
        CE => '1',
        D1 => output_data_sdr_rp(i),
        D2 => output_data_sdr_fp(i),
        R => '0',
        S => '0');

    gen_inv_o : if (i = 0) generate
      txobuf : OBUFDS
        generic map(
          IOSTANDARD => "DEFAULT",
          SLEW => "SLOW")
        port map(
          O => p0_d((i*2)),
          OB => p0_d((i*2)+1),
          I => output_data_se(i));
      output_data_sdr_rp(i) <= not output_data_sdr_r(i);
      output_data_sdr_fp(i) <= not output_data_sdr_f(i);
    end generate;

    gen_true_o : if (i /= 0) generate
      txobuf : OBUFDS
        generic map(
          IOSTANDARD => "DEFAULT",
          SLEW => "SLOW")
        port map(
          O => p0_d((i*2)+1),
          OB => p0_d(i*2),
          I => output_data_se(i));
      output_data_sdr_rp(i) <= output_data_sdr_r(i);
      output_data_sdr_fp(i) <= output_data_sdr_f(i);
    end generate;

  end generate;

  rxfbuf : IBUFDS
    generic map(
      IBUF_LOW_PWR => FALSE,
      IOSTANDARD => "DEFAULT")
    port map(
      O => rx_frame_se_pre,
      I => rx_frame_n,
      IB => rx_frame_p
    );

  --ODDR for symmetrical latency between data and output
  txfoddr : ODDR
    generic map(
      DDR_CLK_EDGE => "OPPOSITE_EDGE",
      INIT => '0',
      SRTYPE => "SYNC")
    port map(
      Q => tx_frame_se_pre,
      C => clock_buf,
      CE => '1',
      D1 => tx_frame_se_inv,
      D2 => tx_frame_se_inv,
      R => '0',
      S => '0');

  txfbuf : OBUFDS
    generic map(
      IOSTANDARD => "DEFAULT",
      SLEW => "SLOW")
    port map(
      O => tx_frame_n,
      OB => tx_frame_p,
      I => tx_frame_se_pre);

  rx_frame_se <= not rx_frame_se_pre;
  tx_frame_se_inv <= not tx_frame_se;

  process(clock_buf)
  begin
    if falling_edge(clock_buf) then
      if rx_frame_se = '1' then
        rx_data_lat_i(5 downto 0) <= input_data_sdr_fp;
        rx_data_lat_q(5 downto 0) <= input_data_sdr_rp;
        rx_data_frame <= '0';
      else
        rx_data_i <= rx_data_lat_i;
        rx_data_q <= rx_data_lat_q;
        rx_data_frame <= '1';
        rx_data_lat_i(11 downto 6) <= input_data_sdr_fp;
        rx_data_lat_q(11 downto 6) <= input_data_sdr_rp;
      end if;
    end if;
  end process;

  process(clock_buf)
  begin
    if rising_edge(clock_buf) then
      if tx_frame_se = '0' then
        output_data_sdr_f <= tx_data_lat_i(11 downto 6);
        output_data_sdr_r <= tx_data_lat_q(11 downto 6);

        tx_data_req <= '1';
        tx_frame_se <= '1';
      else
        output_data_sdr_f <= tx_data_lat_i(5 downto 0);
        output_data_sdr_r <= tx_data_lat_q(5 downto 0);

        tx_data_lat_i <= tx_data_i;
        tx_data_lat_q <= tx_data_q;

        tx_data_req <= '0';
        tx_frame_se <= '0';
      end if;
    end if;
  end process;

  user_clock <= clock_buf;


end Behavioral;
