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

	double		gcAltitude;
	double		gcLongitude;
	double		gcLatitude;
	Vector3		gcVector;
    Vector3		velocityVector;
};

#endif
