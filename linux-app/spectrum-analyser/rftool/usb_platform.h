#pragma once
#include <stdint.h>
// Custom platform functions, in addition to the standard AD ones
void enter_rx_streaming_mode();
void leave_rx_streaming_mode();
void siggen_config_write(uint8_t address, uint32_t data);
void set_txrx_en(bool txrx, bool en);
struct iq_sample {
  int16_t i, q;
};

int rx_get_data(struct iq_sample *buf);
