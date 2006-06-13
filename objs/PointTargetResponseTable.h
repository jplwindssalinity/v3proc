#ifndef POINT_TARGET_RESPONSE_TABLE_H
#define POINT_TARGET_RESPONSE_TABLE_H

#include <iostream.h>
#include <math.h>
#include <string.h>

#define NBEAMS 4
#define AUX_MAX_LINE 400
#define DATA_MAX_LINE 200000
#define TIME_STEP 300. // time step between PTR table, 68 cycles, 300 sec.
#define ANGLE_STEP 10. // angle step between PTR table, 10 degree
#define N_ANG_STEPS 36 // number of angles step in a revolution, 36 as ANGLE_STEP is 10 deg
#define RNG_SWATH_WIDTH 18. // footprint in range dim, 18 km
#define AZ_SWATH_WIDTH 12. // footprint in azimuth dim, 12 km
#define RNG_STEP_SIZE 0.5 // range increment of point target in footprint, 0.5 km
#define AZ_STEP_SIZE 0.5 // azimuth increment of point target in footprint, 0.5 km
#define N_RNG_BINS 37 // number of range bins in the footprint
#define N_AZ_BINS 25 // number of azimuth bins in the footprint

class PointTargetResponseTable{
 public:

  PointTargetResponseTable();
  ~PointTargetResponseTable();
  int ReadAux(char* filename, int beam_num);
  int ReadData(char* filename, int beam_num);

  float GetSemiMinorWidth(float range_km, float azimuth_km,
                          float scan_angle_rad, float orbit_time_in_rev_s,
                          int beam_num);

  float GetSemiMajorWidth(float range_km, float azimuth_km,
                          float scan_angle_rad, float orbit_time_in_rev_s,
                          int beam_num);

  int *nAux;
  float **time, **scanAngle;
  int **nRngPixel, **nAzPixel;
  float ***scPos, ***tarPos, ***rngUnit, ***azUnit;

  int *nData;
  float **rngOffset, **azOffset, **semiMajorWidth, **semiMinorWidth;

};



#endif
