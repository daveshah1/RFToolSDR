#pragma once
#include <atomic>
#include <ccomplex>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>
#undef complex
#include <complex>
class RFThread;

using namespace std;

class DemodThread {
public:
  // Start and stop the demodulation  thread
  void start(RFThread *_rf);
  void stop();

  double demodOffset = 0;
  string demodMode = "OFF";
  double ifBandwidth = 10e3;
  double afBandwidth = 5e3;
  double afGain = -60;

private:
  RFThread *rf = nullptr;
  thread demod_thread;
  atomic<bool> stopDemod{false};

  chrono::milliseconds interval = chrono::milliseconds(500);
  unsigned int audio_fs = 48e3;
  void demod_main();
  size_t do_demod(float _Complex *rfbuf, size_t n, double rf_fs, int16_t *audio,
                  size_t m_max);
};
