//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_antenna_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include "Antenna.h"
#include "Beam.h"
#include "Constants.h"

//=========//
// Antenna //
//=========//

Antenna::Antenna()
:	numberOfBeams(0), priPerBeam(0.0), azimuthAngle(0.0),
	commandedSpinRateDnPerMs(0.0), actualSpinRate(0.0), encoderAOffsetDn(0),
	encoderDelay(0.0), currentBeamIdx(0), _numberOfEncoderValues(0)
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

//-----------------------------------//
// Antenna::SetNumberOfEncoderValues //
//-----------------------------------//

int
Antenna::SetNumberOfEncoderValues(
	unsigned int	number)
{
	_numberOfEncoderValues = number;
	return(1);
}

//--------------------------------//
// Antenna::SetAzimuthWithEncoder //
//--------------------------------//

int
Antenna::SetAzimuthWithEncoder(
	unsigned int	encoder)
{
	// the 0.5 is to center the azimuth on the given encoder value
	azimuthAngle = two_pi *
		((double)(encoder + 0.5) / (double)_numberOfEncoderValues);
	return(1);
}

//-------------------------//
// Antenna::GetCurrentBeam //
//-------------------------//

Beam*
Antenna::GetCurrentBeam()
{
	if (currentBeamIdx < 0 || currentBeamIdx >= numberOfBeams)
		return(NULL);
	return(&(beam[currentBeamIdx]));
}

//-------------------------------//
// Antenna::GetEarlyEncoderValue //
//-------------------------------//
// simulates the early sampling of the CDS

unsigned int
Antenna::GetEarlyEncoderValue(
	double	spin_rate,		// radians/second
	double	time)			// seconds
{
	double azimuth_angle = azimuthAngle - spin_rate * time + two_pi;
	unsigned int encoder = (unsigned int)((azimuth_angle / two_pi) *
		(double)_numberOfEncoderValues);
	encoder %= _numberOfEncoderValues;
	return(encoder);
}

//--------------------------//
// Antenna::GetEncoderValue //
//--------------------------//

unsigned int
Antenna::GetEncoderValue()
{
	unsigned int encoder = (unsigned int)((azimuthAngle / two_pi) *
		(double)_numberOfEncoderValues);
	encoder %= _numberOfEncoderValues;
	return(encoder);
}

//-----------------------------//
// Antenna::GetAntennaFraction //
//-----------------------------//

double
Antenna::GetAntennaFraction()
{
	double fraction = azimuthAngle / two_pi;
	return(fraction);
}
