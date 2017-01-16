#pragma once
#include <cstdint>
#include <vector>
using namespace std;
#include "ftd3xx/ftd3xx.h"

namespace RFTool {

class USBInterface {
public:
  USBInterface();  // Constructor opens FTDI device
  ~USBInterface(); // Destructor closes FTDI device

  /* Write a 32-byte command to the device; and store the 32-byte response in a
 buffer if received and requested. Returns false on failure*/
  bool exec_command(uint8_t *command, bool get_response,
                    uint8_t *response = nullptr);

  /*Perform an SPI transaction with the AD9361. Returns false on failure*/
  bool spi_transfer(bool is_read, int len, uint16_t addr, uint8_t *buf);

  /* Set the FPGA control signals register */
  bool set_fpga_ctrl(uint8_t ctrl_val);
  // Build a 32-byte command from command type and payload
  void build_command(uint8_t cmd_type, const vector<uint8_t> &payload,
                     uint8_t *buffer);
  /* Try and receive an rx data packet, copying the raw packet into a buffer
   and returning false on failure*/
  bool get_rx_packet(uint8_t *buffer);

  // Flush buffers
  void flush();

private:
  // Print a 32-byte response to cout in hex for debug purposes
  void print_response(uint8_t *resp);
  // These functions are base code from the FTDI supplied demo
  void get_version();
  void get_vid_pid();
  void turn_off_all_pipes();
  bool get_device_lists(int timeout_ms);
  bool set_ft600_channel_config(FT_60XCONFIGURATION *cfg,
                                CONFIGURATION_FIFO_CLK clock, bool is_600_mode);
  bool set_channel_config(bool is_600_mode, CONFIGURATION_FIFO_CLK clock);
  void turn_off_thread_safe();
  void get_queue_status();
  FT_HANDLE handle;
  bool rev_a_chip;

  bool fifo_600mode;
  uint8_t in_ch_cnt;
  uint8_t out_ch_cnt;

  const int command_len = 32;
  const int response_len = 32;
  const int rx_packet_len = 4096;
  const int timeout = 1000;
};

// Swap the word endianness of a byte array
inline void swap_endian(uint8_t *buffer, int len) {
  for (int i = 0; i < len; i += 4) {
    swap(buffer[i + 3], buffer[i]);
    swap(buffer[i + 2], buffer[i + 1]);
  }
};
};
