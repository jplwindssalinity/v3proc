//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef EPHEMERIS_H
#define EPHEMERIS_H

static const char rcs_id_ephemeris_h[] =
	"@(#) $Id$";

#include <stdio.h>
#include "Matrix3.h"
#include "BufferedList.h"
#include "EarthPosition.h"

#define EPHEMERIS_INTERP_ORDER			8
#define SUBTRACK_INTEGRATION_STEPSIZE	2000.0
#define RANGE_TIME_TOL					0.1

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
int WriteAscii(FILE *fp);
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

class Ephemeris : public BufferedList<OrbitState>
{
public:

	//--------------//
	// construction //
	//--------------//

	Ephemeris();
	Ephemeris(const char* filename, unsigned int maxstates);
	~Ephemeris();

	// Searching

	OrbitState* FindSouthPole();

	//
	// Interpolation and extraction.
	//

	int GetPosition(double time, int order, EarthPosition *rsat);
	int GetOrbitState(double time, int order, OrbitState *os);
	int GetOrbitState_2pt(double time, OrbitState *os);
	int		GetNextOrbitState(OrbitState* os);

	//
	// Subtrack conversion.
	//

	int GetSubtrackCoordinates(EarthPosition rground,
			EarthPosition subtrack_start, double start_time,
			double measurement_time,
			float *crosstrack, float *alongtrack);
	int GetSubtrackPosition(double ctd, double atd,
		double start_time, EarthPosition* rground);

protected:

	int		_GetBracketingOrbitStates(double time, OrbitState** os1,
				OrbitState** os2);

	//-----------//
	// variables //
	//-----------//

	double	_interp_midpoint_time;
	int		_interp_order;
	double*	_interp_time;
	double*	_interp_x;
	double*	_interp_y;
	double*	_interp_z;
	double*	_interp_vx;
	double*	_interp_vy;
	double*	_interp_vz;
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

double Range(double time);

//
// Pointers that indicate which ephemeris object and surface point to use.
//

Ephemeris *ephemeris;
EarthPosition *rground;

};

#endif
