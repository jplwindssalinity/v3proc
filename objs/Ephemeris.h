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
//		OrbitState
//		Ephemeris
//		RangeFunction
//======================================================================

class OrbitState;
class Ephemeris;
class RangeFunction;

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

int Write(FILE *outputfile);
int Read(FILE *inputfile);

//-----------//
// variables //
//-----------//

double time;
EarthPosition rsat;
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
Ephemeris(char *filename);
~Ephemeris();

// I/O

int SetFile(char *filename);
int ReadThrough(double time);

//
// Interpolation and extraction.
//

int GetPosition(double time, EarthPosition *rsat);

//
// Subtrack conversion.
//

int GetSubtrackCoordinates(
EarthPosition rground,
double start_time,
double measurement_time,
float *crosstrack,
float *alongtrack);

//-----------//
// variables //
//-----------//

FILE *inputfile;
long max_states;

};

//======================================================================
// CLASS
//		RangeFunction
//
// DESCRIPTION
//		The RangeFunction object computes the distance between an orbit
//		position and a position on the earth.  It provides the function
//		(method Range()) to be minimized when an Ephemeris object
//		invokes the method GetSubtrackCoordinates which needs to find
//		the ephemeris point with minimum range to the surface point.
//		To allow the use of standard routines, this object encapsulates
//		the information needed (ephemeris and surface point). 
//======================================================================

class RangeFunction
{
public:

//--------------//
// construction //
//--------------//

RangeFunction(Ephemeris *ephemeris, EarthPosition *rground);
RangeFunction();
~RangeFunction();

//
// Distance from s/c to surface point at a particular time.
//

float Range(double time);

//
// Pointers that indicate which ephemeris object and surface point to use.
//

Ephemeris *ephemeris;
EarthPosition *rground;

};

#endif
