//==============================================================//
// Copyright (C) 1997-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef MISCTABLE_H
#define MISCTABLE_H

static const char rcs_id_misctable_h[] =
    "@(#) $Id$";

#include "Meas.h"

//======================================================================
// CLASSES
//    MiscTable
//======================================================================

//======================================================================
// CLASS
//    MiscTable
//
// DESCRIPTION
//    The MiscTable object is a base class for GMF and Kp.  It
//    contains a table of values (model function or Kp) and some
//    access and I/O functions.
//
// NOTES
//    Misc is an acronym for Measurement type, Incidence angle, Speed,
//    and Chi (relative wind direction).
//======================================================================

class MiscTable
{
public:

    //--------------//
    // construction //
    //--------------//

    MiscTable();
    ~MiscTable();

    //--------------//
    // input/output //
    //--------------//

    int  Read(const char* filename);
    int  Write(const char* filename);

    //--------//
    // access //
    //--------//

    float  GetMinSpd() { return(_spdMin); };
    float  GetMaxSpd() { return(_spdMax); };

    int  GetInterpolatedValue(Meas::MeasTypeE met, float inc, float spd,
             float chi, float* value);
    int  GetNearestValue(Meas::MeasTypeE met, float inc, float spd, float chi,
             float* value);
    int  GetMaxValueForSpeed(Meas::MeasTypeE met, float inc, float spd,
             float* value);
    int  GetAvgValueForSpeed(Meas::MeasTypeE met, float inc, float spd,
             float* value);

protected:

    //--------------//
    // construction //
    //--------------//

    int  _GetMaxValueForSpeed(int met_idx, int inc_idx, int spd_idx,
             float* value);
    int  _Allocate();
    int  _Deallocate();

    //--------------//
    // input/output //
    //--------------//

    int  _ReadHeader(int fd);
    int  _ReadTable(int fd);
    int  _WriteHeader(int fd);
    int  _WriteTable(int fd);

    //--------//
    // access //
    //--------//

    int    _MetToIndex(Meas::MeasTypeE met);
    int    _IncToIndex(float inc);
    int    _SpdToIndex(float spd);
    int    _ChiToIndex(float chi);

    int    _ClipMetIndex(int met_idx);
    int    _ClipIncIndex(int inc_idx);
    int    _ClipSpdIndex(int spd_idx);
    int    _ClipChiIndex(int chi_idx);

    float  _IndexToSpd(int spd_idx);
    float  _IndexToChi(int chi_idx);

    //-----------//
    // variables //
    //-----------//

    int    _metCount;     // the number of measurement types

    int    _incCount;     // the number of incidence angles
    float  _incMin;       // the minimum incidence angle
    float  _incMax;       // the maximum incidence angle
    float  _incStep;      // the incidence angle step size

    int    _spdCount;     // the number of wind speeds
    float  _spdMin;       // the minimum wind speed
    float  _spdMax;       // the maximum wind speed
    float  _spdStep;      // the wind speed step size

    int    _chiCount;     // the number of relative azimuth angles
    float  _chiStep;      // the relative azimuth angle step size

    float****  _value;    // the array of values
    float***   _maxValueForSpeed;
};

#endif
