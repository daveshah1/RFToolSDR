

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity rx_only_top is
  port(
    sys_clock_in : in std_logic;
    
    --AD9361 interface
    ad_clock : in std_logic;
    
    ad_data_clock_p : in std_logic;
    ad_data_clock_n : in std_logic;
    ad_fb_clock_p : out std_logic;
    ad_fb_clock_n : out std_logic;
  
    ad_rx_frame_p : in std_logic;
    ad_rx_frame_n : in std_logic;
    ad_tx_frame_p : out std_logic;
    ad_tx_frame_n : out std_logic;
  
    ad_p0_d : out std_logic_vector(11 downto 0);
    ad_p1_d : in std_logic_vector(11 downto 0);
    
    ad_spi_clk : out std_logic;
    ad_spi_csn : out std_logic;
    ad_spi_mosi : out std_logic;
    ad_spi_miso : in std_logic;
    
    ad_txnrx : out std_logic;
    ad_enable : out std_logic;
    ad_reset : out std_logic;
    
    --FT601 interface
    ftdi_clock : in std_logic;
    ftdi_txe_n : in std_logic;
    ftdi_rxf_n : in std_logic;
    ftdi_oe_n : out std_logic;
    ftdi_wr_n : out std_logic;
    ftdi_rd_n : out std_logic;
    ftdi_be : inout std_logic_vector(3 downto 0);
    ftdi_data : inout std_logic_vector(31 downto 0);
    
    --Misc I/O
    led_link : out std_logic;
    pa_en : out std_logic;
     
    aux_sda : inout std_logic;
    aux_scl : inout std_logic;
    aux_io0 : inout std_logic;
    aux_io1 : inout std_logic;
    
    ref_term_en : out std_logic;
    ref_ac : in std_logic;
    ref_dc : in std_logic;
    
    --HyperRAM interface
    ram_clk_p : out std_logic;
    ram_clk_n : out std_logic;
    ram_csn : out std_logic;
    ram_rstn : out std_logic;
    ram_dq : inout std_logic_vector(7 downto 0);
    ram_rwds : inout std_logic
    );
end rx_only_top;

architecture Behavioral of rx_only_top is
  COMPONENT command_fifo
    PORT (
      wr_clk : IN STD_LOGIC;
      rd_clk : IN STD_LOGIC;
      din : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
      wr_en : IN STD_LOGIC;
      rd_en : IN STD_LOGIC;
      dout : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
      full : OUT STD_LOGIC;
      empty : OUT STD_LOGIC;
      prog_full : OUT STD_LOGIC;
      prog_empty : OUT STD_LOGIC
    );
  END COMPONENT;
  COMPONENT iq_sample_fifo
    PORT (
      wr_clk : IN STD_LOGIC;
      rd_clk : IN STD_LOGIC;
      din : IN STD_LOGIC_VECTOR(23 DOWNTO 0);
      wr_en : IN STD_LOGIC;
      rd_en : IN STD_LOGIC;
      dout : OUT STD_LOGIC_VECTOR(23 DOWNTO 0);
      full : OUT STD_LOGIC;
      empty : OUT STD_LOGIC
    );
  END COMPONENT;
  COMPONENT response_fifo
    PORT (
      wr_clk : IN STD_LOGIC;
      rd_clk : IN STD_LOGIC;
      din : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
      wr_en : IN STD_LOGIC;
      rd_en : IN STD_LOGIC;
      dout : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
      full : OUT STD_LOGIC;
      empty : OUT STD_LOGIC;
      prog_empty : OUT STD_LOGIC
    );
  END COMPONENT;
  COMPONENT rx_packet_fifo
    PORT (
      wr_clk : IN STD_LOGIC;
      rd_clk : IN STD_LOGIC;
      din : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
      wr_en : IN STD_LOGIC;
      rd_en : IN STD_LOGIC;
      dout : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
      full : OUT STD_LOGIC;
      empty : OUT STD_LOGIC;
      prog_empty : OUT STD_LOGIC
    );
  END COMPONENT;
  
  component sys_pll
  port
   (
    system_clock          : out    std_logic;
    clock_in           : in     std_logic
   );
  end component;
  
  signal reset, enable : std_logic;
  
  signal pc_clock, system_clock, radio_clock : std_logic;
  
  signal ad_sck_int, ad_mosi_int, ad_csn_int : std_logic;
  
  signal cmd_fifo_d : std_logic_vector(31 downto 0);
  signal cmd_fifo_wren, cmd_fifo_rden, cmd_fifo_empty, cmd_fifo_prog_empty, cmd_fifo_prog_full : std_logic;
  signal cmd_fifo_q : std_logic_vector(7 downto 0);
  
  signal resp_fifo_d : std_logic_vector(7 downto 0);
  signal resp_fifo_wren, resp_fifo_rden, resp_fifo_prog_empty : std_logic;
  signal resp_fifo_q : std_logic_vector(31 downto 0);
  
  signal iq_fifo_d : std_logic_vector(23 downto 0);
  signal iq_fifo_wren, iq_fifo_rden, iq_fifo_empty : std_logic;
  signal iq_fifo_q : std_logic_vector(23 downto 0);
  
  signal rxp_fifo_d : std_logic_vector(31 downto 0);
  signal rxp_fifo_wren, rxp_fifo_rden, rxp_fifo_full, rxp_fifo_prog_empty : std_logic;
  signal rxp_fifo_q : std_logic_vector(31 downto 0);
  
  signal streaming_mode : std_logic;
  signal control_sig : std_logic_vector(7 downto 0);
  
  signal spi_rden, spi_wren, spi_start_xfer, spi_end_xfer, spi_done : std_logic;
  signal spi_d, spi_q : std_logic_vector(7 downto 0);
  
--  signal debug_0, debug_1, debug_2 : std_logic_vector(7 downto 0);
  signal debug_0, debug_1, debug_2, debug_3 : std_logic_vector(0 downto 0);

  COMPONENT ila_0 
    PORT (
      clk : IN STD_LOGIC;
      probe0 : IN STD_LOGIC_VECTOR(7 DOWNTO 0); 
      probe1 : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
      probe2 : IN STD_LOGIC_VECTOR(7 DOWNTO 0)
    );
  END COMPONENT  ;
  
  COMPONENT ila_1
    PORT (
      clk : IN STD_LOGIC;
      probe0 : IN STD_LOGIC_VECTOR(0 DOWNTO 0); 
      probe1 : IN STD_LOGIC_VECTOR(0 DOWNTO 0); 
      probe2 : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
      probe3 : IN STD_LOGIC_VECTOR(0 DOWNTO 0)
    );
  END COMPONENT  ;
  
  COMPONENT ila_2
  PORT (
    clk : IN STD_LOGIC;
    probe0 : IN STD_LOGIC_VECTOR(11 DOWNTO 0);
    probe1 : IN STD_LOGIC_VECTOR(11 DOWNTO 0)
  );
  END COMPONENT  ;
  signal rx_data_frame : std_logic;
begin
  reset <= '0';
  enable <= '1';
  
  syspll : sys_pll
   port map ( 
     system_clock => system_clock,
     clock_in => sys_clock_in);
  
  adrx : entity work.ad9361_sdr_rx_if
    port map(
      data_clock_p => ad_data_clock_p,
      data_clock_n => ad_data_clock_n,
      fb_clock_p => ad_fb_clock_p,
      fb_clock_n => ad_fb_clock_n,
      
      rx_frame_p => ad_rx_frame_p,
      rx_frame_n => ad_rx_frame_n,
      tx_frame_p => ad_tx_frame_p,
      tx_frame_n => ad_tx_frame_n,
      
      p0_d => ad_p0_d,
      p1_d => ad_p1_d,
      
      user_clock => radio_clock,
      rx_data_frame => rx_data_frame,
      rx_data_i => iq_fifo_d(11 downto 0),
      rx_data_q => iq_fifo_d(23 downto 12));
 
   iq_fifo_wren <= rx_data_frame;
  
  adspi : entity work.ad9361_spi_if
    generic map(
      clock_div => 8,
      pre_delay => 10,
      post_delay => 10)
    port map(
      clock => system_clock,
      reset => reset,
      enable => enable,
      
      data_rden => spi_rden,
      data_in => spi_d,
      data_wren => spi_wren,
      data_out => spi_q,
      
      start_xfer => spi_start_xfer,
      end_xfer => spi_end_xfer,
      ready => spi_done,
      
      spi_clock => ad_sck_int,
      spi_cs_n => ad_csn_int,
      spi_mosi => ad_mosi_int,
      spi_miso => ad_spi_miso);
  
  ad_spi_clk <= ad_sck_int;
  ad_spi_csn <= ad_Csn_int;
  ad_spi_mosi <= ad_mosi_int;
  rxpack : entity work.rx_packet_gen
    port map(
      clock => system_clock,
      enable => enable,
      reset => reset,
      
      iq_fifo_q => iq_fifo_q,
      iq_fifo_empty => iq_fifo_empty,
      iq_fifo_rden => iq_fifo_rden,
      
      packet_fifo_d => rxp_fifo_d,
      packet_fifo_wren => rxp_fifo_wren,
      packet_fifo_full => rxp_fifo_full,
      
      streaming_mode => streaming_mode);
  
  ctrl : entity work.pc_control_handler
    port map(
      clock => system_clock,
      enable => enable,
      reset => reset,
      
      cmd_fifo_q => cmd_fifo_q,
      cmd_fifo_rden => cmd_fifo_rden,
      cmd_fifo_empty => cmd_fifo_empty,
      cmd_fifo_prog_empty => cmd_fifo_prog_empty,
      
      resp_fifo_d => resp_fifo_d,
      resp_fifo_wren => resp_fifo_wren,
      
      spi_rden => spi_rden,
      spi_din => spi_d,
      spi_wren => spi_wren,
      spi_q => spi_q,
      spi_start_xfer => spi_start_xfer,
      spi_end_xfer => spi_end_xfer,
      spi_done => spi_done,
      
      streaming_mode => streaming_mode,
      control_signals => control_sig);
  
  ftdiif : entity work.ft60x_fifo_if
    port map (
      reset => reset,
      enable => enable,
      
      ftdi_clock => ftdi_clock,
      ftdi_txe_n => ftdi_txe_n,
      ftdi_rxf_n => ftdi_rxf_n,
      ftdi_oe_n => ftdi_oe_n,
      ftdi_wr_n => ftdi_wr_n,
      ftdi_rd_n => ftdi_rd_n,
      ftdi_be => ftdi_be,
      ftdi_data => ftdi_data,
      
      clock_out => pc_clock,
      
      command_fifo_d => cmd_fifo_d,
      command_fifo_wren => cmd_fifo_wren,
      command_fifo_prog_full => cmd_fifo_prog_full,
      
      resp_fifo_q => resp_fifo_q,
      resp_fifo_rden => resp_fifo_rden,
      resp_fifo_prog_empty => resp_fifo_prog_empty,
      
      rx_fifo_q => rxp_fifo_q,
      rx_fifo_rden => rxp_fifo_rden,
      rx_fifo_prog_empty => rxp_fifo_prog_empty,
      
      streaming_mode => streaming_mode);
 
  cmdfifo : command_fifo port map(
    wr_clk => pc_clock,
    rd_clk => system_clock,
    din => cmd_fifo_d,
    wr_en => cmd_fifo_wren,
    rd_en => cmd_fifo_rden,
    dout => cmd_fifo_q,
    full => open,
    empty => cmd_fifo_empty,
    prog_full => cmd_fifo_prog_full,
    prog_empty => cmd_fifo_prog_empty);
    
  iqfifo : iq_sample_fifo port map(
    wr_clk => radio_clock,
    rd_clk => system_clock,
    din => iq_fifo_d,
    wr_en => iq_fifo_wren,
    rd_en => iq_fifo_rden,
    dout => iq_fifo_q,
    full => open,
    empty => iq_fifo_empty);
  
  respfifo : response_fifo port map(
    wr_clk => system_clock,
    rd_clk => pc_clock,
    din => resp_fifo_d,
    wr_en => resp_fifo_wren,
    rd_en => resp_fifo_rden,
    dout => resp_fifo_q,
    full => open,
    empty => open,
    prog_empty => resp_fifo_prog_empty);
    
  rxpfifo : rx_packet_fifo port map(
    wr_clk => system_clock,
    rd_clk => pc_clock,
    din => rxp_fifo_d,
    wr_en => rxp_fifo_wren,
    rd_en => rxp_fifo_rden,
    dout => rxp_fifo_q,
    full => rxp_fifo_full,
    empty => open,
    prog_empty => rxp_fifo_prog_empty);
 
  led_link <= not control_sig(0);
  ad_reset <= not control_sig(1);
  ad_enable <= control_sig(2);
  ad_txnrx <= control_sig(3);
  pa_en <= control_sig(4);
  
  ref_term_en <= '1';
  aux_sda <= 'Z';
  aux_scl <= 'Z';
  aux_io0 <= 'Z';
  aux_io1 <= 'Z';
  ram_clk_p <= '1';
  ram_clk_n <= '0';
  ram_csn <= '1';
  ram_rstn <= '1';
  ram_dq <= (others => 'Z');
  ram_rwds <= 'Z';  
  
  
--  debug_0 <= cmd_fifo_q;
--  debug_1 <= resp_fifo_d;
--  debug_2 <= "0000" & cmd_fifo_prog_empty & cmd_fifo_rden & resp_fifo_prog_empty & resp_fifo_wren;
  
--  debug : ila_0
--  PORT MAP (
--    clk => system_clock,
  
--    probe0 => debug_0, 
--    probe1 => debug_1,
--    probe2 => debug_2
--  );
  debug_0 <= "" & rxp_fifo_rden;
  debug_1 <= "" & streaming_mode;
  debug_2 <= "" & ftdi_rxf_n;
  debug_3 <= "" & ftdi_txe_n;

--  debug1 : ila_1
--    PORT MAP (
--      clk => system_clock,
--      probe0 => debug_0, 
--      probe1 => debug_1, 
--      probe2 => debug_2,
--      probe3 => debug_3
--    );
      
    debug2 : ila_2
      PORT MAP (
        clk => radio_clock,
        probe0 => iq_fifo_d(11 downto 0), 
        probe1 => iq_fifo_d(23 downto 12));
end Behavioral;
