//==============================================================//
// Copyright (C) 1998-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_FbbTable_h[] =
    "@(#) $Id$";

#ifndef FbbTable_H
#define FbbTable_H

#include "BYUXTable.h"

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
    float  GetFbb(Spacecraft* spacecraft, Qscat* qscat, Topo* topo = NULL,
               Stable* stable = NULL, CheckFrame* cf = NULL);
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

#endif
