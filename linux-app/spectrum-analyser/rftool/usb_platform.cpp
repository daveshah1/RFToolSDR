#include "usb_if.hpp"
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <time.h>
using namespace std;

// AD9361 driver platform shim

static RFTool::USBInterface *usb = nullptr;

extern "C" {
#include "../ad9361/platform/platform.h"
#include "usb_platform.h"

int32_t spi_init(uint32_t device_id, uint8_t clk_pha, uint8_t clk_pol) {
  if (usb == nullptr)
    usb = new RFTool::USBInterface();
  return 0;
};

int32_t spi_read(uint8_t *data, uint8_t bytes_number) {
  throw runtime_error("spi_read not supported, use spi_write_then_read");
};

static const uint8_t cmd_resp_magic[5] = {0x52, 0x45, 0x53, 0x50, 0x10};

int spi_write_then_read(struct spi_device *spi, const unsigned char *txbuf,
                        unsigned n_tx, unsigned char *rxbuf, unsigned n_rx) {
  if (usb == nullptr)
    return -1;
  if (n_tx < 2)
    throw runtime_error("n_tx must be >= 2");
  usb->flush();
  vector<uint8_t> payload;
  payload.push_back(uint8_t(n_tx + n_rx));
  for (int i = 0; i < n_tx; i++)
    payload.push_back(txbuf[i]);
  for (int i = 0; i < n_rx; i++)
    payload.push_back(0);
  uint8_t cmd_buf[32], resp_buf[32];
  usb->build_command(0x10, payload, cmd_buf);
  if (!usb->exec_command(cmd_buf, true, resp_buf))
    return -1;

  RFTool::swap_endian(resp_buf, 32);
  for (int i = 0; i < 5; i++) {
    if (resp_buf[i] != cmd_resp_magic[i])
      cerr << "bad spi resp packet" << endl;
  }
  if (resp_buf[5] != (n_tx + n_rx))
    cerr << "mismatched spi resp packet" << endl;

  for (int i = 0; i < n_rx; i++) {
    rxbuf[i] = resp_buf[6 + n_tx + i];
  }
  return 0;
};

void gpio_init(uint32_t device_id) {
  if (usb == nullptr)
    usb = new RFTool::USBInterface();
};

void gpio_direction(uint8_t pin, uint8_t direction){};

bool gpio_is_valid(int number) { return (number == 0); };

void gpio_set_value(unsigned gpio, int value) {
  // resetn is ctrl sig bit 1 and inverted (so active high)
  if (usb == nullptr)
    return;
  if (gpio == 0) {
    if (value == 0) {
      usb->set_fpga_ctrl(0x02);
    } else {
      usb->set_fpga_ctrl(0x00);
    }
  }
};

void enter_rx_streaming_mode() {
  uint8_t cmd_buf[32], resp_buf[32];
  if (usb == nullptr)
    return;
  usb->build_command(0x30, vector<uint8_t>{}, cmd_buf);
  usb->exec_command(cmd_buf, false, resp_buf);
};

void leave_rx_streaming_mode() {
  uint8_t cmd_buf[32], resp_buf[32];
  if (usb == nullptr)
    return;
  usb->build_command(0x31, vector<uint8_t>{}, cmd_buf);
  usb->flush();
  usb->exec_command(cmd_buf, true, resp_buf);
  usb->flush();
};

static const uint8_t rx_magic[4] = {0x52, 0x58, 0x49, 0x51};
// static const uint8_t rx_magic[4] = {0x51, 0x49, 0x58, 0x52};
const int rx_packet_len = 4096;
static bool first_rx_get = true;
static uint8_t rx_buffer_a[rx_packet_len], rx_buffer_b[rx_packet_len];
static uint8_t *current_packet = rx_buffer_a;
static uint8_t *last_packet = rx_buffer_b;

void rx_purge() { first_rx_get = true; };

int rx_get_data(iq_sample *buf) {
  if (usb == nullptr)
    return 0;
  if (!usb->get_rx_packet(current_packet))
    return 0;
  int out_idx = 0;
  // Workaround for a broken USB interface ATM
  for (int i = 0; i < (rx_packet_len - 4); i++) {

    if (((current_packet[i + 0] & 0xC0) == 0x00) &&
        ((current_packet[i + 1] & 0xC0) == 0x40) &&
        ((current_packet[i + 2] & 0xC0) == 0x80) &&
        ((current_packet[i + 3] & 0xC0) == 0xC0)) {
      // Extract I and Q
      uint16_t i_raw = (uint16_t(current_packet[i + 1] & 0x3F) << 6) |
                       (uint16_t(current_packet[i]) & 0x3F);
      uint16_t q_raw = (uint16_t(current_packet[i + 3] & 0x3F) << 6) |
                       (uint16_t(current_packet[i + 2]) & 0x3F);
      // Sign extend
      buf[out_idx].i = ((i_raw & 0x800) == 0x800) ? (i_raw | 0xF000) : i_raw;
      buf[out_idx].q = ((q_raw & 0x800) == 0x800) ? (q_raw | 0xF000) : q_raw;
      out_idx++;
      i += 3;
    } else {
      // cout << "bad alignment" << endl;
    }
  }

  return out_idx;
}

void udelay(unsigned long usecs) {
  timespec dly;
  dly.tv_sec = 0;
  dly.tv_nsec = usecs * 1000;
  nanosleep(&dly, nullptr);
};

void mdelay(unsigned long msecs) { msleep_interruptible(msecs); }

unsigned long msleep_interruptible(unsigned int msecs) {
  this_thread::sleep_for(chrono::milliseconds(msecs));
  return 0;
};

// AXIADC functions are not yet implemented and dummies to prevent linker errors
void axiadc_init(struct ad9361_rf_phy *phy){};
int axiadc_post_setup(struct ad9361_rf_phy *phy) { return 0; };
unsigned int axiadc_read(struct axiadc_state *st, unsigned long reg) {
  return 0;
};
void axiadc_write(struct axiadc_state *st, unsigned reg, unsigned val){};
int axiadc_set_pnsel(struct axiadc_state *st, int channel,
                     enum adc_pn_sel sel) {
  return 0;
}
void axiadc_idelay_set(struct axiadc_state *st, unsigned lane, unsigned val){};
};
