#include "FFTRenderer.hpp"
#include <iostream>
void FFTRenderer::FitFFTToView(double maxValue, double refLevel,
                               const _Complex float *fftData, int fftLen,
                               std::vector<double> &out) {
  out.resize(viewWidth);
  double binWidth = sampleRate / fftLen;
  double currFreq = GetStartFrequencyOff();
  // Hz per pixel
  double HzPerPixel = GetFrequencySpan() / viewWidth;
  // Number of bins per pixel
  int binsPerPixel = (int)((HzPerPixel / binWidth) + 0.5);
  for (int i = 0; i < viewWidth; i++) {
    double peak = 0;
    double avg = 0;
    double value = 0;
    int startBin = (int)((currFreq / binWidth) + (fftLen / 2) + 0.5);
    for (int bin = startBin; bin <= (startBin + binsPerPixel); bin++) {
      if ((bin >= 0) && (bin < fftLen)) {
        double power = ((creal(fftData[bin]) * creal(fftData[bin])) +
                        (cimag(fftData[bin]) * cimag(fftData[bin]))) /
                       (double(fftLen) * double(fftLen));
        if (power > peak)
          peak = power;
        avg += power;
      }
    }
    avg /= binsPerPixel;
    value = peakScale ? peak : avg;
    if (value == 0)
      out[i] = min_db;
    else
      out[i] = refLevel + 10 * std::log10(value / (maxValue * maxValue));
    currFreq += HzPerPixel;
  };
};

void FFTRenderer::RenderToContext(std::vector<double> points,
                                  const Cairo::RefPtr<Cairo::Context> &cr) {
  double viewScale = viewHeight / (maxAmplitude - minAmplitude);
  // Fill background black
  cr->rectangle(0, 0, viewWidth, viewHeight);
  cr->set_source_rgb(0.0, 0.0, 0.0);
  cr->fill();

  cr->begin_new_path();

  for (int i = 0; i < points.size(); i++) {
    int ypos = viewHeight - int(((points[i] - minAmplitude) * viewScale) + 0.5);
    if (ypos >= viewHeight)
      ypos = viewHeight - 1;
    if (ypos < 0)
      ypos = 0;
    cr->line_to(i, ypos);
  };

  cr->set_line_width(1.0);
  cr->set_source_rgb(0.0, 1.0, 1.0);
  cr->stroke();
}
