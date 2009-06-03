#include"RainDistribution.h"

//Incomplete dummy methods
RainDistribution::RainDistribution( const char* infile, float gridcellres){
}

float
RainDistribution::getRandomRainRate(){
  return(0.0);
}

int
RainDistribution::checkMeasTypeMatch(int s0idx, Meas* meas){
  return(1);
}

int
RainDistribution::rainContaminateSigma0(int s0idx, float* s0vec){
  return(1);
}
