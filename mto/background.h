#include <stdio.h>

#include "main.h"


#ifndef BG_H
#define BG_H

#define BG_REJECT_TILE 0
#define BG_ACCEPT_TILE 1

void bg_info(image* img, FLOAT_TYPE *mean, FLOAT_TYPE *variance,
  int verbosity_level);
  
void bg_subtract(image* img, FLOAT_TYPE mean, int verbosity_level);
  
void bg_truncate(image* img, int verbosity_level);

#endif
