#pragma once
#include <cmath>
#include <complex.h>
#include <gtkmm.h>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

class FFTRenderer {
public:
  double centerFrequency = 2.4e9;
  double bandwidth = 28e6;
  double sampleRate = 36e6;
  double zoom = 1;
  double offset = 0;
  int viewWidth = 1000;

  double minAmplitude = -180;
  double maxAmplitude = -20;
  int viewHeight = 500;

  bool peakScale = false;

  inline double GetStartFrequencyOff() const {
    return offset - (GetFrequencySpan() / 2);
  };
  inline double GetFrequencySpan() const { return bandwidth / zoom; };
  inline double GetEndFrequencyOff() const {
    return offset + (GetFrequencySpan() / 2);
  };

  const double min_db = -240; // minimum to use if ampltitude==0

  // Apply peak detection to fit FFT to view window; and convert to a power in
  // dB
  // fftData must have 0Hz in the centre
  void FitFFTToView(double maxValue, double refLevel,
                    const _Complex float *fftData, int fftLen,
                    std::vector<double> &out);

  // Render FFT to the cairo context
  void RenderToContext(std::vector<double> points,
                       const Cairo::RefPtr<Cairo::Context> &cr);
};
