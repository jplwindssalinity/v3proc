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
//		OrbitState
//======================================================================

class Ephemeris;
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

// I/O

int Write(int output_fd);
int Read(int input_fd);

//-----------//
// variables //
//-----------//

double time;
Vector3 rsat;
Vector3 vsat;

};

//======================================================================
// CLASS
//		Ephemeris
//
// DESCRIPTION
//		The Ephemeris object specifies the position of a spacecraft at a
//		given time.  The orbit is stored as a list of OrbitState objects
//		that define the position and velocity for a set of times.
//		This data is read in from an external file.
//======================================================================

class Ephemeris : public List<OrbitState>
{
public:

//--------------//
// construction //
//--------------//

Ephemeris();
~Ephemeris();

// I/O

int Write(int output_fd);
int Read(int input_fd);

// Interpolation and extraction.

int GetPosition(double time, EarthPosition *rsat);

//-----------//
// variables //
//-----------//

};

#endif
