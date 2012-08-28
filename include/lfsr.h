#ifndef LFSR_H
#define LFSR_H

#include "imgfeatures.h"
#include "cv.h"
#include "cxcore.h"

#include <vector>

struct region {
  int left;
  int top;
  int right;
  int bottom;
  std::vector<struct feature> features;
};

CvSeq* lfsr(IplImage *img, CvSeq *seq, CvMemStorage *storage);

#endif
