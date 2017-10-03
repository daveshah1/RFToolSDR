// Copyright 1986-2016 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2016.3 (lin64) Build 1682563 Mon Oct 10 19:07:26 MDT 2016
// Date        : Thu Dec 22 13:02:56 2016
// Host        : david-desktop-arch running 64-bit unknown
// Command     : write_verilog -force -mode synth_stub
//               /home/dave/misc-projects/rftool-fpga/projects/rx_only/rx_only.srcs/sources_1/ip/ila_2/ila_2_stub.v
// Design      : ila_2
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7a50tftg256-2
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* x_core_info = "ila,Vivado 2016.3" *)
module ila_2(clk, probe0, probe1)
/* synthesis syn_black_box black_box_pad_pin="clk,probe0[11:0],probe1[11:0]" */;
  input clk;
  input [11:0]probe0;
  input [11:0]probe1;
endmodule
