#include <iomanip>

/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.cpp
 * Copyright (C) 2016 David Shah <dave@ds0.me>
 *
 */

#include "FFTRenderer.hpp"
#include "rftool/RFThread.hpp"
#include <ccomplex>
#include <iostream>
extern "C" {
#define complex
#ifdef CUDA_FFT
#include <cufftw.h>
#else
#include <fftw3.h>
#endif
}
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>

#include <gtkmm.h>

#include <sstream>

#include <algorithm>
#include <thread>

using namespace std;

/* For testing propose use the local (not installed) ui file */
/* #define UI_FILE PACKAGE_DATA_DIR"/ui/sa_demo.ui" */
#define UI_FILE "sa_demo.ui"

RFThread *rft;
FFTRenderer *fftr;

mutex fft_lock;
vector<double> fft_points;
Glib::RefPtr<Gtk::Adjustment> dispMaxAmp, dispMinAmp, rxDispZoom, rxOffset,
    rxCenterFreq, rxInputGain;
Gtk::ComboBox *fftLengthSel, *rxBwSel, *rxInputSel;
Gtk::CheckButton *rxAgcEnable;
Gtk::DrawingArea *fft_area, *waterfallArea;
Gtk::SpinButton *rxCenterFreqSpinner;
Gtk::Button *autoscaleAmp;
Gtk::Scale *rxGainSet;
Gtk::Label *rxResolution, *cursorFreq;
ulong last_fftLength = -1;
fftw_plan fftplan = nullptr;

const size_t max_fftLength = 16777216;
_Complex double *x_buf, *y_buf;
atomic<bool> draw_done{false};
atomic<ulong> fftLength{524288};
double *fft_window;
// Waterfall display points
const int waterfall_wmax = 3840, waterfall_hmax = 2160;
float waterfall_points[waterfall_hmax][waterfall_wmax] = {{0.0}};
int waterfall_yptr = 0;

void update_fft() {
  while (true) {

    rft->getSamples(x_buf, fftLength);

    if ((fftplan == nullptr) || (fftLength != last_fftLength)) {
        //Build/rebuild window
        const double a0 = 0.42, a1 = 0.5, a2 = 0.08, pi = 3.1415;
        for(int ii = 0; ii < fftLength; ii++) {
            fft_window[ii] = a0 - a1 * cos((2 * pi * ii) / (fftLength - 1)) + a2 * cos((4 * pi * ii) / (fftLength - 1));
        }
    }

    for (int ii = 0; ii < fftLength; ii++) {
      // Multiplying by -1^ii puts centre at origin
      if ((ii % 2) == 1) {
        x_buf[ii] *= -1;
      }
      x_buf[ii] *= fft_window[ii];
    }

    if ((fftplan == nullptr) || (fftLength != last_fftLength)) {
      if (fftplan != nullptr)
        fftw_destroy_plan(fftplan);
      fftplan =
          fftw_plan_dft_1d(int(fftLength), x_buf, y_buf, -1, FFTW_ESTIMATE);
    }
    {
      lock_guard<mutex> fft_lock_acquire(fft_lock);
      fftw_execute(fftplan);
    }
    last_fftLength = fftLength;
    this_thread::sleep_for(chrono::milliseconds(10));
  }
}

bool redraw_fft() {
  fft_area->queue_draw();
  return true;
}

bool fftClickHandler(GdkEventButton *event) {
  if (event->button == 3) {
    rxDispZoom->set_value(1);
    rxOffset->set_value(0);
  } else if (event->button == 1) {
    double clickFreq =
        fftr->GetStartFrequencyOff() +
        (event->x / double(fftr->viewWidth)) * fftr->GetFrequencySpan();
    rxOffset->set_value(clickFreq / 1.0e6);
    rxDispZoom->set_value(fftr->zoom * 4);
  }
  return true;
}

bool fftMouseMoveHandler(GdkEventMotion *event) {
  double ptrFreq =
      fftr->GetStartFrequencyOff() +
      (event->x / double(fftr->viewWidth)) * fftr->GetFrequencySpan();
  stringstream temp;
  temp << "Cursor = ";
  temp << fixed << setprecision(3)
       << (ptrFreq / 1.0e6) + rxCenterFreq->get_value();
  temp << "MHz";
  cursorFreq->set_text(temp.str());
  return false;
}

void centerFreqChanged() {
  rft->setCenterFreq(uint64_t(rxCenterFreq->get_value() * 1e6));
}

void inputGainChanged() { rft->setGain(uint64_t(rxInputGain->get_value())); }

void inputSelChanged() {
  int input_sel;
  rxInputSel->get_active()->get_value(1, input_sel);
  rft->setInputPort(input_sel);
};

void rxBandwidthChanged() {
  double bw;
  rxBwSel->get_active()->get_value(1, bw);
  rft->setBandwidth(bw);
};

void rxAgcChanged() { rft->setAgcEnable(rxAgcEnable->get_active()); }

void autoscale_vert() {
  double minamp = *min_element(fft_points.begin(), fft_points.end());
  double maxamp = *max_element(fft_points.begin(), fft_points.end());
  dispMinAmp->set_value(floor(minamp - 1));
  dispMaxAmp->set_value(ceil(maxamp + 20));
}

bool on_fft_draw(const Cairo::RefPtr<Cairo::Context> &cr) {
  {
    lock_guard<mutex> fft_lock_acquire(fft_lock);

    Gtk::Allocation allocation = fft_area->get_allocation();
    fftr->viewWidth = allocation.get_width();
    fftr->viewHeight = allocation.get_height();
    fftr->centerFrequency = rxCenterFreq->get_value() * 1e6;
    fftr->maxAmplitude = dispMaxAmp->get_value();
    fftr->minAmplitude = dispMinAmp->get_value();
    fftr->zoom = rxDispZoom->get_value();
    fftr->offset = rxOffset->get_value() * 1e6;
    fftr->sampleRate = rft->getCurrentSampleRate();
    rxBwSel->get_active()->get_value(1, fftr->bandwidth);
    double resolution = double(fftr->sampleRate) / double(fftLength);
    rxResolution->set_text("Resolution = " + to_string(resolution) + "Hz");
    ulong fftLengthT;
    fftLengthSel->get_active()->get_value(1, fftLengthT);
    fftLength = fftLengthT;
    fftr->FitFFTToView(2048, 0, y_buf, fftLength, fft_points);
    for (int i = 0; i < fft_points.size(); i++) {
      if (i < waterfall_wmax) {
        waterfall_points[waterfall_yptr][i] =
            (fft_points[i] - fftr->minAmplitude) /
            (fftr->maxAmplitude - fftr->minAmplitude);
      }
    }
    waterfall_yptr++;
    if (waterfall_yptr >= waterfall_hmax)
      waterfall_yptr = 0;
    fftr->RenderToContext(fft_points, cr);
  }
  waterfallArea->queue_draw();
  draw_done = true;

  return true;
}

bool on_wf_draw(const Cairo::RefPtr<Cairo::Context> &cr) {
  Gtk::Allocation allocation = waterfallArea->get_allocation();
  int width = allocation.get_width();
  int height = allocation.get_height();

  uint32_t *argb_bitmap = new uint32_t[height * width];
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (x >= waterfall_wmax)
        break;
      int waterfall_y =
          ((waterfall_yptr - y) + waterfall_hmax) % waterfall_hmax;
      float fval = waterfall_points[waterfall_y][x];
      uint8_t pixval;
      if (fval >= 1)
        pixval = 255;
      else if (fval <= 0)
        pixval = 0;
      else
        pixval = uint8_t(fval * 255);
      argb_bitmap[y * width + x] = 0xff000000 | (uint16_t(pixval) << 8);
    }
    if (y >= waterfall_hmax)
      break;
  }
  Cairo::RefPtr<Cairo::ImageSurface> tmpSurface = Cairo::ImageSurface::create(
      reinterpret_cast<unsigned char *>(argb_bitmap),
      Cairo::Format::FORMAT_ARGB32, width, height, width * 4);
  cr->set_source(tmpSurface, 0, 0);
  cr->paint();
  tmpSurface->finish();
  delete[] argb_bitmap;
  return true;
}

int main(int argc, char *argv[]) {
  #ifdef CUDA_FFT
  x_buf = (_Complex double*)malloc(max_fftLength * sizeof(_Complex double));
  y_buf = (_Complex double*)malloc(max_fftLength * sizeof(_Complex double));
  #else
  x_buf = fftw_alloc_complex(max_fftLength);
  y_buf = fftw_alloc_complex(max_fftLength);
  #endif
  fft_window = new double[max_fftLength];
  fftr = new FFTRenderer();

  Gtk::Main kit(argc, argv);

  // Load the Glade file and instiate its widgets:
  Glib::RefPtr<Gtk::Builder> builder;
  try {
    builder = Gtk::Builder::create_from_file(UI_FILE);
  } catch (const Glib::FileError &ex) {
    std::cerr << ex.what() << std::endl;
    return 1;
  }
  Gtk::Window *main_win = 0;
  builder->get_widget("main_window", main_win);
  builder->get_widget("fftArea", fft_area);
  builder->get_widget("waterfallArea", waterfallArea);

  dispMaxAmp = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(
      builder->get_object("dispMaxAmp"));
  dispMinAmp = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(
      builder->get_object("dispMinAmp"));
  rxDispZoom = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(
      builder->get_object("rxDispZoom"));
  rxOffset = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(
      builder->get_object("rxOffset"));
  rxCenterFreq = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(
      builder->get_object("rxCenterFreq"));
  rxInputGain = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(
      builder->get_object("rxInputGain"));

  builder->get_widget("fftLengthSel", fftLengthSel);
  builder->get_widget("rxBwSel", rxBwSel);
  builder->get_widget("rxInputSel", rxInputSel);
  builder->get_widget("rxAgcEnable", rxAgcEnable);
  builder->get_widget("rxCenterFreqSpinner", rxCenterFreqSpinner);
  builder->get_widget("rxGainSet", rxGainSet);
  builder->get_widget("rxResolution", rxResolution);
  builder->get_widget("cursorFreq", cursorFreq);
  builder->get_widget("autoscaleAmp", autoscaleAmp);

  if (main_win) {
    rft = new RFThread();

    fft_area->signal_draw().connect(sigc::ptr_fun(on_fft_draw));
    waterfallArea->signal_draw().connect(sigc::ptr_fun(on_wf_draw));
    main_win->set_events(Gdk::BUTTON_PRESS_MASK | Gdk::POINTER_MOTION_MASK);
    fft_area->set_events(Gdk::BUTTON_PRESS_MASK | Gdk::POINTER_MOTION_MASK);
    fft_area->signal_button_press_event().connect(
        sigc::ptr_fun(fftClickHandler));
    fft_area->signal_motion_notify_event().connect(
        sigc::ptr_fun(fftMouseMoveHandler));
    rxCenterFreqSpinner->signal_changed().connect(
        sigc::ptr_fun(centerFreqChanged));
    rxInputGain->signal_value_changed().connect(
        sigc::ptr_fun(inputGainChanged));
    rxInputSel->signal_changed().connect(sigc::ptr_fun(inputSelChanged));
    rxBwSel->signal_changed().connect(sigc::ptr_fun(rxBandwidthChanged));
    rxAgcEnable->signal_toggled().connect(sigc::ptr_fun(rxAgcChanged));
    autoscaleAmp->signal_clicked().connect(sigc::ptr_fun(autoscale_vert));
    rft->start();
    Glib::signal_timeout().connect(sigc::ptr_fun(redraw_fft), 17);
    thread uth(update_fft);
    kit.run(*main_win);
    rft->stop();
  }
  return 0;
}
