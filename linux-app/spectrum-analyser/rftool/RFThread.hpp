#pragma once
#include <atomic>
#include <chrono>
#include <complex.h>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>
extern "C" {
#include "../ad9361/ad9361_api.h"
#include "../ad9361/platform/parameters.h"
#include "../ad9361/platform/platform.h"
#include "usb_platform.h"
};

using namespace std;

enum class TxMode { TX_SINE, TX_NOISE };

class RFThread {
public:
  // Start and stop the AD9361 interfacing thread
  void start();
  void stop();
  // Set various parameters
  void setCenterFreq(uint64_t freq); // set rx center freq in Hz
  void setBandwidth(uint32_t bw);    // set rf rx bandwidth in Hz
  void setGain(int rxgain);          // set rx rf gain in dB
  void setAgcEnable(bool agcEn);     // set AGC on or off
  void setInputPort(int port);       // set rx rf input port (0 or 1)
  void setTxEnable(bool en);         // set tx on or off
  void setTxMode(TxMode mode);       // set tx generator mode
  void setTxOffset(int32_t offset);  // set tx generator offset
  void setTxPower(int power);        // set tx output power

  // Copy the n most recent samples into a buffer (C style complex used for
  // compatibility reasons)
  void getSamples(float _Complex *buf, int n);
  // As above but only get new samples, returning the actual number of samples
  // obtained
  int getRecentSamples(double _Complex *buf, int n);

  uint32_t getCurrentSampleRate();
  uint64_t getCenterFreq();
  int getGain();
  bool getAgcEnable();
  int getInputPort();

private:
  thread rf_thread;
  atomic<uint64_t> centerFreq{2450000000UL};
  atomic<bool> centerFreqChanged{true};
  atomic<uint32_t> bandwidth{20000000};
  atomic<bool> bandwidthChanged{true};
  atomic<int> gain{40};
  atomic<bool> gainChanged{true};
  atomic<bool> agcEnabled{false};
  atomic<bool> agcEnabledChanged{true};
  atomic<int> inputPort{0};
  atomic<bool> inputPortChanged;
  atomic<bool> stopRF{false};

  atomic<bool> transmitEnabled{false};
  atomic<bool> txEnableChanged{false};
  atomic<TxMode> transmitSigMode{TxMode::TX_SINE};
  atomic<int32_t> transmitOffset{0};
  atomic<bool> txGenConfigChanged{true};

  atomic<int> txPower{-30};
  atomic<bool> txPowerChanged{true};

  mutex sample_buf_mutex;
  static const size_t sample_buf_size = 33554432;
  iq_sample sample_buf[sample_buf_size];
  int sample_buf_idx = 0;
  int sample_buf_read_offset = 0;

  ad9361_rf_phy *ad9361_phy;
  void thread_main();
  void init_device();

  atomic<uint32_t> currentSampleRate{20000000};

  uint32_t getSampleRateFromBw(uint32_t bw);
  void beginSettingChange();
  void startStreaming();
  void stopStreaming();
  void endSettingChange();

  chrono::high_resolution_clock::time_point last_wrap =
      chrono::high_resolution_clock::now();
};
