//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef EPHEMERIS_H
#define EPHEMERIS_H

static const char rcs_id_ephemeris_h[] =
	"@(#) $Id$";

#include "Matrix3.h"
#include "List.h"

//======================================================================
// CLASSES
//		Ephemeris
//		OrbitStateList
//		OrbitState
//======================================================================

class Ephemeris;
class OrbitStateList;
class OrbitState;

//======================================================================
// CLASS
//		OrbitState
//
// DESCRIPTION
//======================================================================

class OrbitState
{
public:

//--------------//
// construction //
//--------------//

OrbitState();
~OrbitState();

// Setup

//-----------//
// variables //
//-----------//

double time;
Vector3 rsat;
Vector3 vsat;

};

//======================================================================
// CLASS
//		OrbitStateList
//
// DESCRIPTION
//		The OrbitStateList is a List (see List.[C,h]) of OrbitState objects.
//======================================================================

class OrbitStateList : public List<OrbitState>
{
public:

//--------------//
// construction //
//--------------//

OrbitStateList();
~OrbitStateList();

// Setup

//-----------//
// variables //
//-----------//

};

//======================================================================
// CLASS
//		Ephemeris
//
// DESCRIPTION
//		The Ephemeris object specifies the position of a spacecraft at a
//		given time.  The orbit is stored as a list of time,EarthPosition
//		pairs.  They can be computed using an orbit propagator, or read in
//		from an external file.
//======================================================================

class Ephemeris
{
public:

//--------------//
// construction //
//--------------//

Ephemeris();
~Ephemeris();

// Setup

void Read(char *filename);
int GetPosition(double time, EarthPosition *rsat);

//-----------//
// variables //
//-----------//

OrbitStateList orbit_state_list;

};

#endif
