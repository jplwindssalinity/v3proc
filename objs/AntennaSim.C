//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_antennasim_c[] =
	"@(#) $Id$";

#include <math.h>
#include "AntennaSim.h"
#include "Constants.h"


//============//
// AntennaSim //
//============//

AntennaSim::AntennaSim()
:	startTime(0.0), startAzimuth(0.0)
{
	return;
}

AntennaSim::~AntennaSim()
{
	return;
}

//----------------------------//
// AntennaSim::UpdatePosition //
//----------------------------//

int
AntennaSim::UpdatePosition(
	double		time,
	Antenna*	antenna)
{
	double angle = startAzimuth + (time-startTime) * antenna->actualSpinRate;
	angle = fmod(angle, two_pi);
	antenna->azimuthAngle = angle;

	// The antenna frame is rotated away from the s/c body in yaw only.
	antenna->antennaFrame.Set(0, 0, antenna->azimuthAngle, 3, 2, 1);

	return(1);
}
