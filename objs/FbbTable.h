//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef FbbTable_H
#define FbbTable_H
#include"BYUXTable.h"

static const char rcs_id_FbbTable_h[] =
    "@(#) $Id$";
#endif

//======================================================================
// CLASSES
//    FbbTable
//======================================================================

//======================================================================
// CLASS
//    FbbTable
//
// DESCRIPTION
//    A class for manipulating a tables of spectral peak frequency
//    vs BYU reference vector frequency
//======================================================================

class FbbTable
{
public:
    FbbTable();
    ~FbbTable();

    int  Deallocate();
    int  Allocate();

    int    Read(const char* ibeam_file, const char* obeam_file);
    float  GetFbb(Spacecraft* spacecraft, Qscat* qscat, Meas* meas,
               CheckFrame* cf = NULL);
    float  GetFbb(int beam_number, float azimuth_angle, float orbit_position,
               float delta_freq);
    float  Interpolate(float** table, float orbit_time, float azimuth_angle);

    float***  a;
    float***  b;
    float***  c;
    float***  d;

protected:
    float  _azimuthStepSize;
};
