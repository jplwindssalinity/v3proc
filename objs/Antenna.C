//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_antenna_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include "Antenna.h"
#include "Constants.h"

//=========//
// Antenna //
//=========//

Antenna::Antenna()
:   numberOfBeams(0), startTime(0.0), startAzimuth(0.0), spinRate(0.0),
    encoderAzimuthAngle(0.0), txCenterAzimuthAngle(0.0),
    groundImpactAzimuthAngle(0.0)
{
    return;
}

Antenna::~Antenna()
{
    return;
}

//---------------------------------//
// Antenna::SetEncoderAzimuthAngle //
//---------------------------------//

int
Antenna::SetEncoderAzimuthAngle(
    double  angle)
{
    encoderAzimuthAngle = InRange(angle);
    return(1);
}

//----------------------------------//
// Antenna::SetTxCenterAzimuthAngle //
//----------------------------------//

int
Antenna::SetTxCenterAzimuthAngle(
    double  angle)
{
    txCenterAzimuthAngle = InRange(angle);
    return(1);
}

//--------------------------------------//
// Antenna::SetGroundImpactAzimuthAngle //
//--------------------------------------//

int
Antenna::SetGroundImpactAzimuthAngle(
    double  angle)
{
    groundImpactAzimuthAngle = InRange(angle);
    return(1);
}

//------------------------------//
// Antenna::SetPedestalAttitude //
//------------------------------//

int
Antenna::SetPedestalAttitude(
    Attitude*  attitude)
{
    _antPedToScBody.SetRotation(*attitude);
    _scBodyToAntPed = _antPedToScBody.ReverseDirection();

    return(1);
}

//-------------------------//
// Antenna::UpdatePosition //
//-------------------------//

int
Antenna::UpdatePosition(
    double  time)
{
    double angle = startAzimuth + (time - startTime) * spinRate;
    SetEncoderAzimuthAngle(angle);

    // The antenna frame is rotated away from the s/c body in yaw only.
//    antennaFrame.Set(0, 0, angle, 3, 2, 1);

    return(1);
}

//---------------------//
// Antenna::Initialize //
//---------------------//

int
Antenna::Initialize(
    double  time)
{
    startTime = time;
    return(1);
}

//==================//
// Helper Functions //
//==================//

//---------//
// InRange //
//---------//

double
InRange(
    double  angle)
{
    if (angle < 0.0)
        return(fmod(angle, two_pi) + two_pi);
    else if (angle >= two_pi)
        return(fmod(angle, two_pi));
    else
        return(angle);
}
