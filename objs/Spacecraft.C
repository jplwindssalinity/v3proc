//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_spacecraft_c[] =
	"@(#) $Id$";

#include "Spacecraft.h"


//=================//
// SpacecraftEvent //
//=================//

SpacecraftEvent::SpacecraftEvent()
:	eventId(NONE), time(0.0)
{
	return;
}

SpacecraftEvent::~SpacecraftEvent()
{
	return;
}


//============//
// Spacecraft //
//============//

Spacecraft::Spacecraft()
{
	return;
}

Spacecraft::~Spacecraft()
{
	return;
}

//-------------------------------//
// Spacecraft::SetOrbitPeriod    //
//-------------------------------//

int
Spacecraft::SetOrbitPeriod(
	double	semi_major_axis,
	double	eccentricity,
	double	inclination)
{
	//---------------------------------------//
	// copy variables and convert to radians //
	//---------------------------------------//

        double i = inclination * dtr;

        double rm_2=rm*rm;

	//-----------//
	// predigest //
	//-----------//

	double a_3 = semi_major_axis*semi_major_axis*semi_major_axis;
	double e_2 = eccentricity*eccentricity;

	//-------------------------------------------------------------//
	// calculate perturbations of mean anomaly, periapse, asc node //
	//-------------------------------------------------------------//

	double a = sqrt(xmu / a_3);	// two-body mean motion
	double pp = a * (1.0 - e_2);  // semi-latus rectum (nearly killed 'em!)
	double pp_2 = pp * pp;
	double gama = rj2 * rm_2 / pp_2;
	double eta = sqrt(1.0 - e_2);


	double sini = sin(i);
	double sini_2 = sini * sini;

	// perturbation of mean anomaly
	double b1 = 1.0 - 1.5 * sini_2;
	double b = 1.0 + 1.5 * gama * eta * b1;

	double ameandot = a * b;		// "anomalistic" mean motion

	// rate of precession of perigee
	double periasdot = 1.5 * gama * ameandot * (2.0 - 2.5 * sini_2);


	//----------------------//
	// calculate the period //
	//----------------------//

	orbitPeriod = two_pi / (ameandot + periasdot);

	return(1);
}
