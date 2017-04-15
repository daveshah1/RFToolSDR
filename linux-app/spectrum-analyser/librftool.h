#ifndef LIBRFTOOL_H
#define LIBRFTOOL_H

#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

struct RFToolDev;

typedef RFToolDev *HRFTool;

extern HRFTool rftool_begin();
extern void rftool_end(HRFTool dev);

extern void rftool_set_center_freq(HRFTool dev, uint64_t freq);
extern uint64_t rftool_get_center_freq(HRFTool dev);

extern void rftool_set_bandwidth(HRFTool dev, uint32_t bw);
extern uint32_t rftool_get_bandwidth(HRFTool dev);

extern void rftool_set_gain(HRFTool dev, int gain);
extern int rftool_get_gain(HRFTool dev);

extern void rftool_set_agc_enabled(HRFTool dev, bool en);
extern bool rftool_get_agc_enabled(HRFTool dev);

extern void rftool_set_input(HRFTool dev, int inp);
extern int rftool_get_input(HRFTool dev);

extern void rftool_get_samples(HRFTool dev, double _Complex *buf, int n);
extern int rftool_get_new_samples(HRFTool dev, double _Complex *buf, int n);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard: LIBRFTOOL_H */
