//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_antennasim_c[] =
	"@(#) $Id$";

#include <math.h>
#include "AntennaSim.h"
#include "Constants.h"

//==============//
// AntennaState //
//==============//

AntennaState::AntennaState()
{
	return;
}

AntennaState::~AntennaState()
{
	return;
}

//============//
// AntennaSim //
//============//

AntennaSim::AntennaSim()
:	_spinRate(0.0)
{
	for (int i = 0; i < BEAMS; i++)
	{
		_lookAngle[i] = 0.0;
		_azimuthAngle[i] = 0.0;
	}
	return;
}

AntennaSim::~AntennaSim()
{
	return;
}

//---------------------------//
// AntennaSim::DefineAntenna //
//---------------------------//

int
AntennaSim::DefineAntenna(
	double	spin_rate,			// RPM
	double	look_angle_a,
	double	look_angle_b,
	double	azimuth_angle_a,
	double	azimuth_angle_b)
{
	//---------------------------------------//
	// copy variables and convert to radians //
	//---------------------------------------//

	_spinRate = spin_rate * rpm_to_radps;
	_lookAngle[BEAM_A] = look_angle_a * dtr;
	_lookAngle[BEAM_B] = look_angle_b * dtr;
	_azimuthAngle[BEAM_A] = azimuth_angle_a * dtr;
	_azimuthAngle[BEAM_B] = azimuth_angle_b * dtr;

	return(1);
}

//-----------------------------//
// AntennaSim::GetAntennaState //
//-----------------------------//

AntennaState
AntennaSim::GetAntennaState(
	double	time)
{
	double angle = time * _spinRate;
	angle = fmod(angle, two_pi);
	return(_antennaState);
}
