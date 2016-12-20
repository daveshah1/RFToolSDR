# RFTool AD9361 based SDR
RFTool is a standalone AD9361 based SDR with a USB 3 interface and a number of other useful features:

 - AD9361 transceiver IC - dual channel 70MHz-6GHz, up to 54MHz channel bandwidth
 - Xilinx Artix-7 FPGA (XC7A50T-2FTG256I) with 64MBit HyperRAM
 - FT601Q USB 3.0 interface
 - Two LNA inputs (500MHz-3GHz only) and one attenuated input (full AD9361 range)
 - All inputs protected to over +27dBm input
 - Two PA outputs (500MHz-3GHz only) with >20dBm output power and one direct output (full range)
 - Flexible external reference input (switchable termination and coupling)
 - USB powered, no external power needed

Rev 1.0 of the hardware has been built and initial tests completed. FPGA and PC code is still
a work in progress and will be released once more complete.

![Rev 1.0 Hardware](http://ds0.me/sdr/hardware_tiny.jpg)
