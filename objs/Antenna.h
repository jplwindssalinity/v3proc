//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef ANTENNA_H
#define ANTENNA_H

static const char rcs_id_antenna_h[] =
    "@(#) $Id$";

#include "Beam.h"

#define MAX_NUMBER_OF_BEAMS  10

//======================================================================
// CLASSES
//    Angle, Antenna
//======================================================================

//======================================================================
// CLASS
//    Angle
//
// DESCRIPTION
//    The Angle object contains an angle between 0.0 and 2 * PI
//======================================================================

/*
class Angle : public double
{
public:
    Angle operator=(Angle angle);
};
*/

//======================================================================
// CLASS
//    Antenna
//
// DESCRIPTION
//    The Antenna object contains antenna information.  It represents
//    the state of the antenna.
//======================================================================

class Antenna
{
public:

    //--------------//
    // construction //
    //--------------//

    Antenna();
    ~Antenna();

    //-----------------//
    // setting/getting //
    //-----------------//

    int               SetAzimuthAngle(double angle);
    int               TimeRotation(double time);
    int               SetPedestalAttitude(Attitude* attitude);
    int               UpdatePosition(double time);
    int               Initialize(double time);
    CoordinateSwitch  GetAntPedToScBody() { return(_antPedToScBody); };
    CoordinateSwitch  GetScBodyToAntPed() { return(_scBodyToAntPed); };

    //-----------//
    // variables //
    //-----------//

    int     numberOfBeams;
    Beam    beam[MAX_NUMBER_OF_BEAMS];
    double  startTime;      // the time of the starting azimuth
    double  startAzimuth;   // the initial azimuth angle
    double  spinRate;       // rad/second

    //----------------//
    // azimuth angles //
    //----------------//
    // all azimuth angles are in the range 0..two_pi

    double  azimuthAngle;   // antenna azimuth angle (0..two_pi)

protected:

    //-----------//
    // variables //
    //-----------//

    CoordinateSwitch  _antPedToScBody;
    CoordinateSwitch  _scBodyToAntPed;
};

#endif
