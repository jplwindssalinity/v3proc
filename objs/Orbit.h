//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef ORBIT_H
#define ORBIT_H

static const char rcs_id_orbit_h[] =
	"@(#) $Id$";

#include "Constants.h"
#include "Matrix3.h"

//======================================================================
// CLASSES
//		Orbit
//======================================================================


//======================================================================
// CLASS
//		Orbit
//
// DESCRIPTION
//		The Orbit object contains orbit state information.
//======================================================================

class Orbit
{
public:

	//--------------//
	// construction //
	//--------------//

	Orbit();
	~Orbit();

	//-----------//
	// variables //
	//-----------//

	double		gc_altitude;
	double		gc_longitude;
	double		gc_latitude;
	Vector3		gc_vector;
    Vector3		velocity_vector;
};

#endif
