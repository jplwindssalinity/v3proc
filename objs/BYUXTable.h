//==============================================================//
// Copyright (C) 1997-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef BYUXTable_H
#define BYUXTable_H

#define BYU_NUM_BEAMS                    2
#define BYU_AZIMUTH_BINS                 36
#define BYU_NOMINAL_ORBIT_PERIOD         6063.16
#define BYU_ORBIT_POSITION_BINS          32
#define BYU_TIME_INTERVAL_BETWEEN_STEPS  190.0
#define BYU_NUM_SCIENCE_SLICES           10
#define BYU_NUM_GUARD_SLICES_PER_SIDE    1
#define FFT_BIN_SIZE                     462.0

#include <stdio.h>
#include "Spacecraft.h"
#include "Qscat.h"
#include "Meas.h"
#include "Array.h"
#include "CheckFrame.h"
#include "Topo.h"

static const char rcs_id_BYUXTable_h[] =
    "@(#) $Id$";

//=====================================================
// CLASSES
//    BYUXTable
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

    int  Deallocate();
    int  Allocate();

    int    Read(const char* ibeam_file, const char* obeam_file);
    float  GetXTotal(Spacecraft* spacecraft, Qscat* qscat, Meas* meas,
               CheckFrame* cf);
    float  GetXTotal(Spacecraft* spacecraft, Qscat* qscat, Meas* meas,
               float PtGr, CheckFrame* cf);
    float  GetX(Spacecraft* spacecraft, Qscat* qscat, Meas* meas,
               CheckFrame* cf = NULL);
    float  GetDeltaFreq(Spacecraft* spacecraft, Qscat* qscat,
               CheckFrame* cf = NULL, Topo* topo = NULL,
               Stable* stable = NULL);
    float  GetX(int beam_number, float azimuth_angle, float orbit_position,
               int slice_number, float delta_freq);
    float  GetXegg(int beam_number, float azimuth_angle, float orbit_position,
               float delta_freq);
    float  Interpolate(float** table, float orbit_time, float azimuth_angle);

    float****  xnom;
    float****  a;
    float****  b;
    float****  c;
    float****  d;

    float***  xnomEgg;
    float***  aEgg;
    float***  bEgg;
    float***  cEgg;
    float***  dEgg;

protected:
    float  _azimuthStepSize;
    int    _numSlices;
};

//------------------//
// helper functions //
//------------------//

int  GetBYUBoresight(Spacecraft* spacecraft, Qscat* qscat, double* look,
         double* azim);

#endif
