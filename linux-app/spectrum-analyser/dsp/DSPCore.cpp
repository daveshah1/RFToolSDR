#include "DSPCore.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace std;

namespace DSP {
void CBufferToCPP(const _Complex float *in, size_t n, complex<float> *out) {
  memcpy((void *)out, (void *)in, n * sizeof(_Complex float));
}
};
