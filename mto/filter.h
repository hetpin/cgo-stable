#include "main.h"

#ifndef FILTER_H
#define FILTER_H

#define FWHM_TO_SIGMA(X) (X * 0.424660900144009521360)
image gaussian_filter(INT_TYPE height, INT_TYPE width,
  FLOAT_TYPE sigma, int verbosity_level);
image filter(const image* in, const image* H, int verbosity_level);

#endif
