//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_ephemeris_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "Ephemeris.h"
#include "List.h"
#include "List.C"

//
// OrbitState
//

//
// Default constructor
//

OrbitState::OrbitState()
{
return;
}

//
// Destructor
//

OrbitState::~OrbitState()
{
return;
}

//
// OrbitState::Read()
//
// Read one set of time,position,velocity components from the specified file.
// The file is assumed to be binary with time stored in double precision,
// and position and velocity components stored in single precision (float).
//

int
OrbitState::Read(FILE *inputfile)

{

float posvel[6];

if (fread(&time,sizeof(double),1,inputfile) != 1) return(0);
if (fread(&posvel,sizeof(float),6,inputfile) != 6) return(0);
rsat.Set(posvel[0],posvel[1],posvel[2]);
vsat.Set(posvel[3],posvel[4],posvel[5]);
return(1);

}

//
// OrbitState::Write()
//
// Write one set of time,position,velocity components to the specified file.
// The file is assumed to be binary with time stored in double precision,
// and position and velocity components stored in single precision (float).
//

int
OrbitState::Write(FILE *outputfile)

{

double value;
float posvel[6];

rsat.Get(0,&value);
posvel[0] = value;
rsat.Get(1,&value);
posvel[1] = value;
rsat.Get(2,&value);
posvel[2] = value;
vsat.Get(0,&value);
posvel[3] = value;
vsat.Get(1,&value);
posvel[4] = value;
vsat.Get(2,&value);
posvel[5] = value;
if (fwrite(&time,sizeof(double),1,outputfile) != 1) return(0);
if (fwrite(&posvel,sizeof(float),6,outputfile) != 6) return(0);
return(1);

}

//===========//
// Ephemeris //
//===========//

Ephemeris::Ephemeris()
{
	return;
}

Ephemeris::Ephemeris(
	char*			filename,
	unsigned int	max_states)
{
	SetInputFile(filename);
	SetMaxNodes(max_states);
	return;
}

//
// Destructor
//

Ephemeris::~Ephemeris()
{
	CloseInputFile();
	return;
}

//------------------------//
// Ephemeris::GetPosition //
//------------------------//
// Interpolate this OrbitState List (ie., Ephemeris) to the desired time
// and return the position.

int
Ephemeris::GetPosition(
	double			time,
	EarthPosition*	rsat)
{
	OrbitState* os1;
	OrbitState* os2;
	if (_GetBracketingOrbitStates(time, &os1, &os2) == 0)
	{
	//	printf("Error: Can't find requested time %g in Ephemeris\n",time);
		return(0);
	}
	
	// Linearly interpolate the position components in time.

	double time1 = os1->time;
	EarthPosition rsat1 = os1->rsat;

	double time2 = os2->time;
	EarthPosition rsat2 = os2->rsat;

	*rsat = (rsat2-rsat1)*((time-time1)/(time2-time1)) + rsat1;
	return(1);
}

//--------------------------//
// Ephemeris::GetOrbitState //
//--------------------------//
// Interpolate this OrbitState List (ie., Ephemeris) to the desired time
// and return the interpolated OrbitState

int
Ephemeris::GetOrbitState(
	double			time,
	OrbitState*		orbit_state)
{
	OrbitState* os1;
	OrbitState* os2;
	if (_GetBracketingOrbitStates(time, &os1, &os2) == 0)
	{
	//	printf("Error: Can't find requested time %g in Ephemeris\n",time);
		return(0);
	}

	// Linearly interpolate the position components in time.

	double time1 = os1->time;
	EarthPosition rsat1 = os1->rsat;
	Vector3 vsat1 = os1->vsat;

	double time2 = os2->time;
	EarthPosition rsat2 = os2->rsat;
	Vector3 vsat2 = os2->vsat;

	orbit_state->rsat = (rsat2-rsat1)*((time-time1)/(time2-time1)) + rsat1;
	orbit_state->vsat = (vsat2-vsat1)*((time-time1)/(time2-time1)) + vsat1;
	return(1);
}

//
// GetSubtrackCoordinates
//
// Convert a position in geocentric coordinates into subtrack coordinates
// using this ephemeris object to define the subtrack.
// This method locates the ephemeris point closest to the position point
// and then uses the surface distance method to get the cross track distance,
// and the starting time reference to get the along track distance.
// The measurement time is used to determine which orbit the subtrack
// coordinates are needed for.  Otherwise, the position will have subtrack
// coordinates in every rev.
//
// INPUTS:
//

int
Ephemeris::GetSubtrackCoordinates(
EarthPosition rground,
double start_time,
double measurement_time,
float *crosstrack,
float *alongtrack)

{

double t1,t2,t3,t;
double r1,r2,r3,r;

// default returns
*crosstrack = 0.0;
*alongtrack = 0.0;

// Create an object which supplies the range function to be minimized by
// standard routines like brent() below.
RangeFunction rangefunc(this,&rground);

//
// Initial guesses start at the measurement time and go out 40 sec's which
// is about the time it takes to cover 300 km.
//

t1 = measurement_time;
t2 = measurement_time + 40.0;

//
// Bracket the minimum range between t1 and t3.
//

r1 = rangefunc.Range(t1);
r2 = rangefunc.Range(t2);
if ((r1 < 0) || (r2 < 0)) return(0);	// t1 or t2 out of Ephemeris range

if (r2 > r1)
{	// switch t1 and t2 so that range decreases going from t1 to t2.
	double tmp = t1;
	t1 = t2;
	t2 = tmp;
	tmp = r1;
	r1 = r2;
	r2 = tmp;
}

//
// Initial guess for t3 by stepping downhill a factor of 1.6.
//

t3 = t2 + 1.6*(t2 - t1);
r3 = rangefunc.Range(t3);
if (r3 < 0) return(0);	// t3 out of Ephemeris range

while (r2 > r3)
{	// step downhill until the range increases.
	t = t3 + 1.6*(t3 - t2);
	r = rangefunc.Range(t);
	if (r < 0) return(0);	// t out of Ephemeris range
	t1 = t2;
	r1 = r2;
	t2 = t3;
	r2 = r3;
	t3 = t;
	r3 = r;
}

//
// Locate the ephemeris position with minimum range to the EarthPosition
// using a golden section search and the bracketing points t1,t2,t3.
//

double tol = 1e-6;
double R = 0.61803399;
double C = 1.0 - R;
double t0 = t1;

//
// Put the smaller interval between t0 and t1 and setup for the search
//

if (fabs(t3-t2) > fabs(t2-t1))
{
	t1 = t2;
	r1 = r2;
	t2 = t2 + C*(t3-t2);	// first golden section step
	r2 = rangefunc.Range(t2);
	if (r2 < 0) return(0);	// t2 out of Ephemeris range
}
else
{
	t1 = t2 - C*(t2-t1);	// first golden section step
	r1 = rangefunc.Range(t1);
	if (r1 < 0) return(0);	// t1 out of Ephemeris range
}

//
// Golden Section search loop
//

while (fabs(r2-r1)/r1 > tol)
{
	if (r2 < r1)
	{
		t0 = t1;
		t1 = t2;
		t2 = R*t1 + C*t3;
		r1 = r2;
		r2 = rangefunc.Range(t2);
		if (r2 < 0) return(0);	// t2 out of Ephemeris range
	}
	else
	{
		t3 = t2;
		t2 = t1;
		t1 = R*t2 + C*t0;
		r2 = r1;
		r1 = rangefunc.Range(t1);
		if (r1 < 0) return(0);	// t1 out of Ephemeris range
	}
}

double min_time;
if (r1 < r2) min_time = t1; else min_time = t2;

// Compute the s/c position at the start of the subtrack grid, and at
// the minimum range position.
EarthPosition min_position,start_position;
if (GetPosition(min_time,&min_position) == 0) return(0);
if (GetPosition(start_time,&start_position) == 0) return(0);

// Compute the corresponding nadir points on the earth's surface.
EarthPosition subtrack_min = min_position.Nadir();
EarthPosition subtrack_start = start_position.Nadir();

// Compute surface distances in the crosstrack and alongtrack directions.
*crosstrack = subtrack_min.surface_distance(rground);
*alongtrack = subtrack_min.surface_distance(subtrack_start);

return(1);
}

//--------------------------------------//
// Ephemeris::_GetBracketingOrbitStates //
//--------------------------------------//

int
Ephemeris::_GetBracketingOrbitStates(
	double			time,
	OrbitState**	os1,
	OrbitState**	os2)
{
	// make sure there is data in the list (if there is any at all)
	OrbitState* current_state = GetCurrent();
	if (current_state == NULL)
		current_state = GetOrReadNext();

	if (current_state == NULL) return(0);

	// search forward
	while (current_state && current_state->time < time)
		current_state = GetOrReadNext();

	if (current_state == NULL) return(0);

	// search backward
	while (current_state && current_state->time > time)
		current_state = GetPrev();

	if (current_state == NULL) return(0);

	OrbitState* next_state = GetOrReadNext();

	// check
	if (next_state == NULL) return(0);

	// set states
	*os1 = current_state;
	*os2 = next_state;
	return(1);
}


//===============//
// RangeFunction //
//===============//

//
// Initialize with a pointer to the object defining the current ephemeris,
// and with a pointer to the surface EarthPosition object.
//

RangeFunction::RangeFunction(Ephemeris *eptr, EarthPosition *rptr)
{

ephemeris = eptr;
rground = rptr;

}

RangeFunction::RangeFunction()
{
return;
}

RangeFunction::~RangeFunction()
{
return;
}

//
// Compute range between s/c location at indicated time and the surface point.
//

double
RangeFunction::Range(double time)

{

EarthPosition rsat;
if (ephemeris->GetPosition(time,&rsat) == 0) return(-1.0);
EarthPosition rlook = *rground - rsat;
return(rlook.Magnitude());

}
