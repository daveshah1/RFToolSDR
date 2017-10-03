#include "librftool.h"
#include "rftool/RFThread.hpp"
using namespace std;

struct RFToolDev {
  RFThread thrd;
};

extern "C" {

HRFTool rftool_begin() {
  HRFTool dev = new RFToolDev();
  dev->thrd.start();
  return dev;
}

void rftool_end(HRFTool dev) { dev->thrd.stop(); }

void rftool_set_center_freq(HRFTool dev, uint64_t freq) {
  dev->thrd.setCenterFreq(freq);
}

uint64_t rftool_get_center_freq(HRFTool dev) {
  return dev->thrd.getCenterFreq();
}

void rftool_set_bandwidth(HRFTool dev, uint32_t bw) {
  dev->thrd.setBandwidth(bw);
}

uint32_t rftool_get_bandwidth(HRFTool dev) {
  return dev->thrd.getCurrentSampleRate();
}

void rftool_set_gain(HRFTool dev, int gain) { dev->thrd.setGain(gain); }

int rftool_get_gain(HRFTool dev) { return dev->thrd.getGain(); }

void rftool_set_agc_enabled(HRFTool dev, bool en) {
  dev->thrd.setAgcEnable(en);
}

bool rftool_get_agc_enabled(HRFTool dev) { return dev->thrd.getAgcEnable(); }

void rftool_set_input(HRFTool dev, int inp) { dev->thrd.setInputPort(inp); }

int rftool_get_input(HRFTool dev) { return dev->thrd.getInputPort(); }

void rftool_get_samples(HRFTool dev, float _Complex *buf, int n) {
  return dev->thrd.getSamples(buf, n);
}

extern int rftool_get_new_samples(HRFTool dev, double _Complex *buf, int n) {
  return dev->thrd.getRecentSamples(buf, n);
}
}
