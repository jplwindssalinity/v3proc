#ifndef POINT_TARGET_RESPONSE_TABLE_H
#define POINT_TARGET_RESPONSE_TABLE_H

#include <iostream.h>
#include <math.h>
#include <string.h>

#define NBEAMS 4
#define AUX_MAX_LINE 400
#define DATA_MAX_LINE 20000

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
