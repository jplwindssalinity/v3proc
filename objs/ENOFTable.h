//==========================================================//
// Copyright (C) 2000, California Institute of Technology.  //
// U.S. Government sponsorship acknowledged.		    //
//==========================================================//

#ifndef ENOF_TABLE_H
#define ENOF_TABLE_H
#include"Index.h"

#define REP_SPEED_MAX 26.0
#define REP_SPEED_MIN 1.0


static const char rcs_id_enof_table_h[] =
	"@(#) $Id$";



static const float s0_coef[2]={0.50, 0.05};
static const float s0_v_max=0.0617;
static const float s0_v_h_max=0.074;
static const float wind_v_h_coef[5]={1.3787e2,-7.6781e2,2.9092e4,
					 -4.9587e5,2.8720e6};
static const float wind_v_coef[5]= {1.2249e2,-6.5768e2,2.0234e4,-2.4504e5,
				     1.0783e6};
      

class ENOF_Table{
 public:
  ENOF_Table();
  ~ENOF_Table();
  int Compute(float s0_sum[2], int count[2], int cti, float enof[2]);
  int Read(char* filename);
  float Interpolate(float rep_speed, int cti, int beam);
  float GetRepSpeed(float s0_sum[2], int count[2]);
  int Allocate();
  // variables
  float*** table;
  int numCrossTrackBins;
  int numSpeedBins;
  Index speedIndex;
};
#endif








