//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_orbit_c[] =
	"@(#) $Id$";

#include "Orbit.h"


//=======//
// Orbit //
//=======//

Orbit::Orbit()
:	gcAltitude(0.0), gcLongitude(0.0), gcLatitude(0.0)
{
	for (int i = 0; i < 3; i++)
	{
		gcVector.Set(i, 0.0);
		velocityVector.Set(i, 0.0);
	}
	return;
}

Orbit::~Orbit()
{
	return;
}
