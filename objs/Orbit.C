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
:	gc_altitude(0.0), gc_longitude(0.0), gc_latitude(0.0)
{
	for (int i = 0; i < 3; i++)
	{
		gc_vector[i] = 0.0;
		velocity_vector[i] = 0.0;
	}
	return;
}

Orbit::~Orbit()
{
	return;
}
