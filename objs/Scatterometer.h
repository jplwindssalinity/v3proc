//==============================================================//
// Copyright (C) 1998-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef SCATTEROMETER_H
#define SCATTEROMETER_H

static const char rcs_id_scatterometer_h[] =
    "@(#) $Id$";

#include "CoordinateSwitch.h"
#include "EarthPosition.h"
#include "Matrix3.h"
#include "Spacecraft.h"
#include "Meas.h"
#include "Antenna.h"

//======================================================================
// CLASSES
//    ScatTargetInfo, ScatRF, ScatDig, ScatAnt, Scatterometer
//======================================================================

//======================================================================
// CLASS
//    ScatTargetInfo
//
// DESCRIPTION
//    The ScatTargetInfo class is a base class for target information.
//======================================================================

class ScatTargetInfo
{
public:
    Vector3        gcLook;
    EarthPosition  rTarget;
    float          slantRange;       // km
    float          roundTripTime;    // ms

    int  WriteAscii();
    int  GetScatTargetInfo(CoordinateSwitch* antenna_frame_to_gc,
             EarthPosition rsat, Vector3 vector);
};

//======================================================================
// CLASS
//    ScatRF
//
// DESCRIPTION
//    The ScatRF class is a base class for the radio frequency
//    subsystem for scatterometers.
//======================================================================

class ScatRF
{
public:
    //--------------//
    // construction //
    //--------------//

    ScatRF();
    ~ScatRF();

    //-----------//
    // variables //
    //-----------//

};

//======================================================================
// CLASS
//    ScatDig
//
// DESCRIPTION
//    The ScatDig class is a base class for the digital processor
//    subsystem for scatterometers.
//======================================================================

class ScatDig
{
public:
    //--------------//
    // construction //
    //--------------//

    ScatDig();
    ~ScatDig();

    //-----------//
    // variables //
    //-----------//

    int     currentBeamIdx;
    double  time;
};

//======================================================================
// CLASS
//    ScatAnt
//
// DESCRIPTION
//    The ScatAnt class is a base class for the antenna subsystem for
//    scatterometers.
//======================================================================

class ScatAnt
{
public:
    //--------------//
    // construction //
    //--------------//

    ScatAnt();
    ~ScatAnt();

    //-----------//
    // variables //
    //-----------//

    Antenna  antenna;
};

//======================================================================
// CLASS
//    Scatterometer
//
// DESCRIPTION
//    The Scatterometer class is a base class for scatterometers.
//======================================================================

#define POINTS_PER_SPOT_OUTLINE  18
#define DEFAULT_CONTOUR_LEVEL    0.5

class Scatterometer
{
public:
    //--------------//
    // construction //
    //--------------//

    Scatterometer();
    ~Scatterometer();

    //-----------------//
    // getting/setting //
    //-----------------//

    Beam*  GetCurrentBeam();

    //----------//
    // geometry //
    //----------//

    int  LocateSpot(Spacecraft* spacecraft, MeasSpot* meas_spot,
             float contour_level = DEFAULT_CONTOUR_LEVEL);

    //-----------//
    // variables //
    //-----------//

    ScatRF*   scatRf;
    ScatDig*  scatDig;
    ScatAnt*  scatAnt;
};

//------------------//
// helper functions //
//------------------//

int  GetPeakSpatialResponse2(CoordinateSwitch* antenna_frame_to_gc,
         Spacecraft* spacecraft, Beam* beam, double azimuth_rate,
         double* look, double* azim, int ignore_range = 0);

#endif
