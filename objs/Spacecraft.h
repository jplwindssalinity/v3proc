//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef SPACECRAFT_H
#define SPACECRAFT_H

static const char rcs_id_spacecraft_h[] =
	"@(#) $Id$";

#include "Constants.h"
#include "Matrix3.h"

//======================================================================
// CLASSES
//		Spacecraft
//======================================================================


//======================================================================
// CLASS
//		Spacecraft
//
// DESCRIPTION
//		The Spacecraft object contains the state of the spacecraft.
//======================================================================

class Spacecraft
{
public:

	//--------------//
	// construction //
	//--------------//

	Spacecraft();
	~Spacecraft();

	//-----------//
	// variables //
	//-----------//

	double			gcAltitude;
	double			gcLongitude;
	double			gcLatitude;
	EarthPosition	gcVector;
	Vector3		attitude;			// change to new object type!!!
    Vector3			velocityVector;
};

#endif
