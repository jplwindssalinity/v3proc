#ifndef RAIN_DISTRIBIUTION_H
#define RAIN_DISTRIBIUTION_H
#include"Meas.h"
// Incomplete dummy class
class RainDistribution{
 public:
  RainDistribution(const char* infile, float gridcellres);
  float getRandomRainRate();
  int checkMeasTypeMatch(int s0idx, Meas* meas);
  int rainContaminateSigma0(int s0idx, float* s0vec);
 protected:
  int nums0types;
  Meas* example_meas;
};


#endif
