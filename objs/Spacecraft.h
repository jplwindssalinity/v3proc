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
#include "Ephemeris.h"
#include "Attitude.h"

//======================================================================
// CLASSES
//		SpacecraftEvent, Spacecraft
//======================================================================

//======================================================================
// CLASS
//		SpacecraftEvent
//
// DESCRIPTION
//		The SpacecraftEvent object contains an spacecraft event time
//		and event ID.
//======================================================================
 
class SpacecraftEvent
{
public:
 
	//-------//
	// enums //
	//-------//
 
	enum SpacecraftEventE { NONE, UNKNOWN, UPDATE_STATE };
 
	//--------------//
	// construction //
	//--------------//
 
	SpacecraftEvent::SpacecraftEvent();
	SpacecraftEvent::~SpacecraftEvent();
 
	//-----------//
	// variables //
	//-----------//
 
	SpacecraftEventE	eventId;
	double				time;
};


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

	OrbitState		orbitState;
	Attitude		attitude;
};

#endif
