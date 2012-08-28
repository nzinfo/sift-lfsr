#ifndef LFSR_H
#define LFSR_H

#include "cxcore.h"
#include "imgfeatures.h"
#include <vector>

struct region {
  int left;
  int top;
  int right;
  int bottom;
  std::vector<struct feature> features;
}

void estimate(int w, int h, std::vector<struct feature> &features, 
    std::vector<struct region> &estimates, float factor);

#endif
