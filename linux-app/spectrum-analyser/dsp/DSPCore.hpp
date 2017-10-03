#pragma once
#include <ccomplex>
#undef complex
#include <algorithm>
#include <complex>
#include <cstdint>
#include <iostream>
#include <numeric>

using namespace std;

namespace DSP {

const double pi = 3.14159265359;

// Copy a buffer, typecasting from C to C++
void CBufferToCPP(const _Complex double *in, size_t n, complex<double> *out);

// Convert a buffer from a floating point type to an integer type or vice versa
template <typename Ti, typename To>
inline void Convert(const Ti *in, size_t n, To *out, float scale = 0,
                    float offset = 0) {
  transform(in, in + n, out,
            [scale, offset](Ti x) { return To(x * scale + offset); });
}

// Generate a complex sinusoidal signal with a given frequency (in Hz),
// amplitude and phase (in radians)
template <typename To>
inline void GenerateComplexSine(size_t n, To *out, double freq, double Fs,
                                double amp, double phase = 0) {
  for (size_t i = 0; i < n; i++) {
    out[i] = To(amp * exp(To(0, 1) * ((i / Fs) * (2 * pi * freq) + phase)));
  }
}

// Multiply two signals together
template <typename T>
inline void Multiply(const T *a, const T *b, size_t n, T *out) {
  for (size_t i = 0; i < n; i++) {
    out[i] = a[i] * b[i];
  }
}

// Upsample a signal using the sinc method
template <typename T>
inline void Upsample(const T *in, size_t n, T *out, size_t m, double Fs_in,
                     double Fs_out) {
  /*int a = 3;
  int factor = Fs_out / Fs_in;
  int window_size = 2 * a * factor;
  int offset = a * factor;
  double *window = new double[window_size];

  for (int i = -a; i < a; i++) {
    for (int j = 0; j < factor; j++) {
      int idx = offset + factor * i + j;
      if ((i == 0) && (j == 0)) {
        window[idx] = 1;
      } else {
        double x = i + j * factor;
        window[idx] = (a * sin(pi * x) * sin(pi * x / a)) / (pi * pi * x * x);
      }
    }
  }

  for (size_t i = 0; i < m; i++) {
    T val = 0;
    int i_f = (i / factor) * factor;
    for (int j = i_f - factor * (a - 1); j <= (i_f + factor * a); j++) {
      int k = (j / factor);
      if (k < 0)
        k = 0;
      if (k >= n)
        k = n - 1;
      val += T(in[k] * window[i - j]);
    }
    out[i] = val;
  }

  delete[] window;*/
  for (size_t i = 0; i < m; i++) {
    size_t idx = size_t(i * Fs_in / Fs_out);
    if (idx >= n)
      idx = n - 1;
    out[i] = in[idx];
  }
};

template <typename T> inline T sinc(T x) {
  if (x == 0) {
    return x;
  } else {
    return sin(x) / x;
  }
}

// Generate a FIR low-pass filter with a given cutoff frequency
inline void GenFIRLowPass(size_t n, float *h, double Fc, double Fs) {
  int M = n - 1;
  double wc = 2 * pi * (Fc / Fs);
  for (size_t i = 0; i < n; i++) {
    // Using a hamming window
    h[i] = (0.54 - 0.46 * cos(2 * i * pi / M)) * (wc / pi) *
           sinc(wc * (i - M / 2) / pi);
  }
};

// Decimate a signal, applying a FIR lowpass filter
template <typename T>
inline void Decimate(const T *in, size_t n, T *out, size_t m, double Fs_in,
                     double Fs_out) {
  size_t window_size = max(size_t(2), size_t(2 * (Fs_in / Fs_out)));
  float *window = new float[window_size];
  GenFIRLowPass(window_size, window, Fs_out / 2.0, Fs_in);
  for (size_t i = 0; i < m; i++) {
    size_t k = min(size_t(i * (Fs_in / Fs_out)), n - 1);
    T val = 0;
    for (size_t j = 0; j < window_size; j++) {
      val += T(in[k] * window[j]);
      // val += T(in[k] / float(window_size));
      if (k > 0)
        k--;
    }
    out[i] = val;
  }
  delete[] window;
};

// Call Upsample or Downsample as needed
template <typename T>
inline void Resample(const T *in, size_t n, T *out, size_t m, double Fs_in,
                     double Fs_out) {
  if (Fs_out < Fs_in) {
    Decimate(in, n, out, m, Fs_in, Fs_out);
  } else {
    Upsample(in, n, out, m, Fs_in, Fs_out);
  }
};

// Apply a FIR filter to a signal
template <typename T>
inline void FIRFilter(const T *in, size_t n, T *out, const double *h,
                      size_t m) {
  for (size_t i = 0; i < n; i++) {
    size_t k = i;
    T val = 0;
    for (size_t j = 0; j < m; j++) {
      val += T(in[k] * h[j]);
      if (k > 0)
        k--;
    }
    out[i] = val;
  }
};

// Remove the DC offset from a signal
template <typename T> inline void RemoveDC(const T *in, size_t n, T *out) {
  T avg = accumulate(in, in + n, 0.0) / T(n);
  // cout << avg << endl;
  transform(in, in + n, out, [avg](T x) { return x - avg; });
  /*static T rolling_avg = 0;

  for (int i = 0; i < n; i++) {
    T val = in[i];
    out[i] = val - rolling_avg;
    rolling_avg = 0.995 * rolling_avg + 0.005 * val;
}*/
}
}
