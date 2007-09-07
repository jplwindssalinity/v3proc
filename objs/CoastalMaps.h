#ifndef  COASTAL_MAPS_H
#define  COASTAL_MAPS_H
#include "LandMap.h"
class Meas;
#include "CoordinateSwitch.h"
class L2AFrame;
#define S0_CORR_LANDFRAC_THRESH 0.5
#define S0_FLAG_LANDFRAC_THRESH 0.5
#define S0_CORR_LANDCORR_THRESH 0.005
#define S0_FLAG_LANDCORR_THRESH 0.0005

#define SLICE_GAIN_THRESH 0.25
class CoastalMaps : public LandMap
{
 public:
  CoastalMaps();
  int Accumulate(Meas* meas, float s0corr, CoordinateSwitch* gc_to_spot, float** gain, 
		 float xmin, float dx, int nxsteps, float ymin, float dy, int nysteps);
  int ReadPrecomputeLands0(char* prefix);
  float GetPrecomputedLands0(Meas* meas,double lon, double lat);
  int Normalize();
  int Write(char* prefix);
  ~CoastalMaps();
  int InitExtraMaps(double latstart, double lonstart, double res, double latsize, double lonsize, int makel2a=0);
  int lands0Read;
  int WriteL2A(char* filename);
 protected:
  int _AllocateExtraMaps();
  int _DeallocateExtraMaps();

  int _ResampleGainMap( Meas* meas, CoordinateSwitch* gc_to_spot, float** gain, float xmin, float dx,
	     int nxsteps, float ymin, float dy, int nysteps);
  float** _g;
  float**** _sumg;
  int**** _n;
  float**** _s0;
  float**** _landfrac;
  float*** _prelands0;
  float*** _eastAzimuth;
  int _nlooks;
  int _nlats;
  int _nlons;
  int _neastaz;
  int accum_l2a;
  double _latstart;
  double _lonstart;
  double _latres;
  double _lonres;
  L2AFrame*** l2aframes;
};
#endif
