#pragma once
#include <stdint.h>
// Custom platform functions, in addition to the standard AD ones
void enter_rx_streaming_mode();
void leave_rx_streaming_mode();

struct iq_sample {
  int16_t i, q;
};

int rx_get_data(struct iq_sample *buf);
