#pragma once
#include "DSPCore.hpp"
using namespace std;

namespace DSP {

// Downconvert a given signal; putting Fc-Bw..Fc+Bw into a 2xBW sample rate
// signal
template <typename T>
inline void Downconvert(const T *in, size_t n, T *out, size_t m, double Fs_in,
                        double Fc, double Bw) {
  T *mixed = new T[n];
  for (int i = 0; i < n; i++) {
    T lo =
        T(1.0 * exp(complex<double>(0, 1) * ((i / Fs_in) * (-2.0 * pi * Fc))));
    mixed[i] = lo * in[i];
  }
  Decimate(mixed, n, out, m, Fs_in, 2 * Bw);
  delete[] mixed;
}

// Envelope detection (without filtering) e.g. for AM
template <typename T>
inline void EnvelopeDetect(const complex<T> *in, size_t n, T *out) {
  transform(in, in + n, out, [](complex<T> x) -> T { return abs(x); });
};

// Complete AM demodulation
template <typename T>
inline void AMDemodulate(const complex<T> *in, size_t n, T *out, size_t m,
                         double Fs_in, double Fs_out) {
  T *envelope = new T[n];
  EnvelopeDetect(in, n, envelope);
  RemoveDC(envelope, n, envelope);
  Resample(envelope, n, out, m, Fs_in, Fs_out);
  delete[] envelope;
};

template <typename T> T anglediff(T a, T b) {
  T x = fmod((a - b) + pi, 2 * pi);
  if (x < 0)
    x += 2 * pi;
  return x - pi;
}

// Complete FM demodulation
template <typename T>
inline void FMDemodulate(const complex<T> *in, size_t n, T *out, size_t m,
                         double Fs_in, double Fs_out, double delta_F = 2.5e3) {
  T phi_last = 0;
  T *demod = new T[n];
  for (int i = 0; i < n; i++) {
    T phi = atan2(imag(in[i]), real(in[i]));
    if (isnan(phi))
      phi = phi_last;
    demod[i] = (anglediff(phi, phi_last) * Fs_in) / delta_F;
  }
  RemoveDC(demod, n, demod);
  Resample(demod, n, out, m, Fs_in, Fs_out);

  delete[] demod;
}
};
