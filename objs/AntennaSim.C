//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_antennasim_c[] =
	"@(#) $Id$";

#include <math.h>
#include "AntennaSim.h"
#include "Constants.h"


//============//
// AntennaSim //
//============//

AntennaSim::AntennaSim()
:	_spinRate(0.0)
{
	return;
}

AntennaSim::~AntennaSim()
{
	return;
}

//-------------------------//
// AntennaSim::SetSpinRate //
//-------------------------//

int
AntennaSim::SetSpinRate(
	double		spin_rate)
{
	// convert to radians per second
	_spinRate = spin_rate * rpm_to_radps;
	return(1);
}

//----------------------------//
// AntennaSim::UpdatePosition //
//----------------------------//

int
AntennaSim::UpdatePosition(
	double		time,
	Antenna*	antenna)
{
	double angle = time * _spinRate;
	angle = fmod(angle, two_pi);
	antenna->azimuthAngle = angle;
	return(1);
}
