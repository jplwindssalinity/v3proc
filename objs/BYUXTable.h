//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef BYUXTable_H
#define BYUXTable_H

#define BYU_INNER_BEAM_LOOK_ANGLE       39.85
#define BYU_OUTER_BEAM_LOOK_ANGLE       45.95
#define BYU_INNER_BEAM_AZIMUTH_ANGLE     0.15
#define BYU_OUTER_BEAM_AZIMUTH_ANGLE     0.15
#define FFT_BIN_SIZE   462.0

#include <stdio.h>
#include "Spacecraft.h"
#include "Qscat.h"
#include "Meas.h"
#include "XTable.h"

static const char rcs_id_BYUXTable_h[] =
	"@(#) $Id$";

//=====================================================
// CLASSES                     
//             BYUXTable
//=====================================================

//======================================================================
// CLASS
//    BYUXTable
//
// DESCRIPTION
//    A class for manipulating tables of X and frequency compensation
//    parameters.
//
//		
//======================================================================

class BYUXTable{
 public:
  
  BYUXTable();
  ~BYUXTable();

  int    Read(const char* ibeam_file, const char* obeam_file);
  float  GetXTotal(Spacecraft* spacecraft, Qscat* qscat, Meas* meas);
  float  GetXTotal(Spacecraft* spacecraft, Qscat* qscat, Meas* meas,
             float PtGr);
  float  GetX(Spacecraft* spacecraft, Qscat* qscat, Meas* meas);
  float  GetDeltaFreq(Spacecraft* spacecraft, Qscat* qscat);
  float  GetX(int beam_number, float azimuth_angle, float orbit_position, 
            int slice_number, float delta_freq);

  XTable xnom;
  XTable a;
  XTable b;
  XTable c;
  XTable d;
};

#endif








