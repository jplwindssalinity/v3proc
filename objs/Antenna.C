//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_antenna_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include "Antenna.h"
#include "Beam.h"
#include "Constants.h"

//=========//
// Antenna //
//=========//

Antenna::Antenna()
:	numberOfBeams(0), priPerBeam(0.0), azimuthAngle(0.0),
	_numberOfEncoderBits(0), _angularResolution(0.0)
{
	return;
}

Antenna::~Antenna()
{
	return;
}

//------------------------------//
// Antenna::SetPedestalAttitude //
//------------------------------//

int
Antenna::SetPedestalAttitude(
	Attitude*	attitude)
{
	_antPedToScBody.SetRotation(*attitude);
	_scBodyToAntPed = _antPedToScBody.ReverseDirection();

	return(1);
}

//---------------------------------//
// Antenna::SetNumberOfEncoderBits //
//---------------------------------//

int
Antenna::SetNumberOfEncoderBits(
	int		number)
{
	_numberOfEncoderBits = number;
	_angularResolution = two_pi / pow(2.0, (double)_numberOfEncoderBits);
	return(1);
}

//--------------------------//
// Antenna::GetEncoderValue //
//--------------------------//

int
Antenna::GetEncoderValue()
{
	int value = (int)(azimuthAngle / _angularResolution + 0.5);
	return(value);
}

//--------------------------------//
// Antenna::SetAzimuthWithEncoder //
//--------------------------------//

int
Antenna::SetAzimuthWithEncoder(
	int		encoder)
{
	azimuthAngle = (double)encoder * _angularResolution;
	return(1);
}
