#include "DemodThread.hpp"
#include "../rftool/RFThread.hpp"
#undef min
#undef max
#include "DSPCore.hpp"
#include "DSPDemodulate.hpp"
#include <alsa/asoundlib.h>
#include <iostream>
using namespace std;

void DemodThread::start(RFThread *_rf) {
  rf = _rf;
  stopDemod = false;
  demod_thread = thread([this]() { this->demod_main(); });
}

void DemodThread::stop() {
  stopDemod = true;
  demod_thread.join();
}

void DemodThread::demod_main() {
  int err;
  snd_pcm_t *pcm_handle;
  snd_pcm_hw_params_t *params;

  if ((err = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK,
                          SND_PCM_NONBLOCK)) < 0) {
    cerr << "*** Failed to open audio device ***" << endl;
    cerr << "Demodulation has been disabled" << endl;
    return;
  }

  snd_pcm_hw_params_alloca(&params);
  snd_pcm_hw_params_any(pcm_handle, params);
  snd_pcm_hw_params_set_access(pcm_handle, params,
                               SND_PCM_ACCESS_RW_INTERLEAVED);
  snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
  snd_pcm_hw_params_set_channels(pcm_handle, params, 1);
  snd_pcm_hw_params_set_rate_near(pcm_handle, params, &audio_fs, 0);
  snd_pcm_uframes_t min_buf_size = 96000;
  snd_pcm_hw_params_set_buffer_size_near(pcm_handle, params, &min_buf_size);
  cout << "actual buf size = " << min_buf_size << endl;
  snd_pcm_hw_params(pcm_handle, params);
  chrono::high_resolution_clock::time_point
      curloop = chrono::high_resolution_clock::now(),
      lastloop = curloop - interval;
  size_t af_size = 96000;
  int16_t *afbuf = new int16_t[af_size];
  size_t rf_size = 3000000;
  float _Complex *rfbuf = new float _Complex[rf_size];

  while (!stopDemod) {
    lastloop = curloop;
    curloop = chrono::high_resolution_clock::now();
    double delta =
        double(chrono::duration_cast<chrono::microseconds>(curloop - lastloop)
                   .count()) *
        1e-6;
    delta = min(delta, max(interval.count() * 1.1e-3, 500.0e-3));
    size_t af_count = 0;
    if (demodMode != "OFF") {
      size_t rf_count =
          min(size_t(rf->getCurrentSampleRate() * delta * 1), rf_size);
      rf->getSamples(rfbuf, rf_count);
      for (int i = 0; i < rf_count; i++)
        rfbuf[i] /= 2048.0;
      af_count =
          do_demod(rfbuf, rf_count, rf->getCurrentSampleRate(), afbuf, af_size);
      // cout << af_count << endl;

    } else {
      af_count = min(size_t(delta * audio_fs), af_size);
      fill(afbuf, afbuf + af_count, 0);
    }
    err = snd_pcm_writei(pcm_handle, afbuf, af_count);
    if (err < 0) {
      snd_pcm_prepare(pcm_handle);
      err = snd_pcm_writei(pcm_handle, afbuf, af_count);
      if (err < 0)
        cerr << "audio write failed: " << snd_strerror(err) << endl;
    }
    this_thread::sleep_until(curloop + interval);
  }

  snd_pcm_drain(pcm_handle);
  snd_pcm_close(pcm_handle);

  delete[] afbuf;
  delete[] rfbuf;
}

size_t DemodThread::do_demod(float _Complex *rfbuf, size_t n, double rf_fs,
                             int16_t *audio, size_t m_max) {
  size_t if_size = size_t(n * (ifBandwidth / rf_fs));
  complex<float> *if_buf = new complex<float>[if_size];
  DSP::Downconvert(reinterpret_cast<complex<float> *>(rfbuf), n, if_buf,
                   if_size, rf_fs, demodOffset, ifBandwidth / 2.0);

  double real_af_bw = min(afBandwidth, ifBandwidth / 2.0);

  size_t af_size = size_t(if_size * ((2.0 * real_af_bw) / ifBandwidth));
  float *af_buf = new float[af_size];
  float demod_scale = 1;
  if (demodMode == "AM") {
    DSP::AMDemodulate(if_buf, if_size, af_buf, af_size, ifBandwidth,
                      real_af_bw * 2.0);
    demod_scale = 1e3;
  } else if (demodMode == "NFM") {
    DSP::FMDemodulate(if_buf, if_size, af_buf, af_size, ifBandwidth,
                      real_af_bw * 2.0, 2.5e3);
  } else if (demodMode == "WFM") {
    DSP::FMDemodulate(if_buf, if_size, af_buf, af_size, ifBandwidth,
                      real_af_bw * 2.0, 75e3);
    demod_scale = 1e2;
  } else if (demodMode == "RAW") {
    float *real_buf = new float[if_size];
    for (int i = 0; i < if_size; i++)
      real_buf[i] = real(if_buf[i]);
    DSP::Resample(real_buf, if_size, af_buf, af_size, ifBandwidth,
                  real_af_bw * 2.0);
    demod_scale = 1e3;
    delete[] real_buf;
  } else {
    fill(af_buf, af_buf + af_size, 0);
  }

  size_t result_size =
      min(size_t(af_size * (double(audio_fs) / (2.0 * real_af_bw))), m_max);

  float *resamp_buf = new float[result_size];
  DSP::Resample(af_buf, af_size, resamp_buf, result_size, 2.0 * real_af_bw,
                audio_fs);

  float gain = 32767.0 * pow(10, afGain / 20.0) * demod_scale;
  for (int i = 0; i < result_size; i++) {
    float scaled = resamp_buf[i] * gain;
    if (scaled <= -32768) {
      audio[i] = -32768;
    } else if (scaled >= 32767) {
      audio[i] = 32767;
    } else {
      audio[i] = int16_t(scaled);
    }
  }
  delete[] if_buf;
  delete[] af_buf;
  delete[] resamp_buf;
  return result_size;
};
