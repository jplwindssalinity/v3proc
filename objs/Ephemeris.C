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

//
// Ephemeris
//

//
// Default constructor
//

Ephemeris::Ephemeris()
{
inputfile = NULL;
return;
}

Ephemeris::Ephemeris(char *filename, long maxstates)
{
Ephemeris::SetFile(filename);

//
// Set the maximum number of OrbitState's that this Ephemeris object will
// hold in memory.  A larger number of states can still be accessed because
// the Ephemeris object will automatically read in new data and flush old
// data as needed.
//

max_states = maxstates;

return;
}

//
// Destructor
//

Ephemeris::~Ephemeris()
{
if (inputfile != NULL) fclose(inputfile);
return;
}

//
// Ephemeris::SetFile
//
// Open a file to read ephemeris data from.
//

int
Ephemeris::SetFile(char *filename)

{

if (filename == NULL)
{
	inputfile = NULL;
	return(0);
}

inputfile = fopen(filename,"r");

return(1);

}

//
// Ephemeris::GetPosition
//
// Interpolate this OrbitState List (ie., Ephemeris) to the desired time
// and return the position.
//

int
Ephemeris::GetPosition(
double time,
EarthPosition *rsat)

{

//
// Determine if the desired time is covered by ephemeris data in memory.
// If not, then more data needs to be read in.  This may also cause data
// from the other end of the list of orbit states to be deallocated
// (to maintain a fixed amount of memory).
// Currently, only forward reading is implemented to maintain high performance.
// The BufferedList object handles the reading and flushing.
//

//
// Bracket the desired time in the ephemeris.
//

OrbitState *current_state = GetCurrent();
while (current_state != NULL)
{
if (current_state->time < time)
	current_state = ReadNext();
else
	break;
}
while (current_state != NULL)
{
if (current_state->time > time)
	current_state = GetPrev();
else
	break;
}

// Check for out of range desired time.
if (current_state == NULL)
{
	printf("Error: Couldn't locate ephemeris data\n");
	exit(-1);
}

// Linearly interpolate the position components in time.

EarthPosition rsat1 = current_state->rsat;
double time1 = current_state->time;
current_state = GetNext();
EarthPosition rsat2 = current_state->rsat;
double time2 = current_state->time;

*rsat = (rsat2-rsat1)*((time-time1)/(time2-time1)) + rsat1;
return(1);

}

//
// Ephemeris::GetOrbitState
//
// Interpolate this OrbitState List (ie., Ephemeris) to the desired time
// and return the orbit state.
//

int
Ephemeris::GetOrbitState(
double time,
OrbitState *os)

{

//
// Determine if the desired time is covered by ephemeris data in memory.
// If not, then more data needs to be read in.  This may also cause data
// from the other end of the list of orbit states to be deallocated
// (to maintain a fixed amount of memory).
// Currently, only forward reading is implemented to maintain high performance.
// The BufferedList object handles the reading and flushing.
//

//
// Bracket the desired time in the ephemeris.
//

OrbitState *current_state = GetCurrent();
while (current_state != NULL)
{
if (current_state->time < time)
	current_state = ReadNext();
else
	break;
}
while (current_state != NULL)
{
if (current_state->time > time)
	current_state = GetPrev();
else
	break;
}

// Check for out of range desired time.
if (current_state == NULL)
{
	printf("Error: Couldn't locate ephemeris data\n");
	exit(-1);
}

// Linearly interpolate the position components in time.

EarthPosition rsat1 = current_state->rsat;
Vector3 vsat1 = current_state->vsat;
double time1 = current_state->time;
current_state = GetNext();
EarthPosition rsat2 = current_state->rsat;
Vector3 vsat2 = current_state->vsat;
double time2 = current_state->time;

os->rsat = (rsat2-rsat1)*((time-time1)/(time2-time1)) + rsat1;
os->vsat = (vsat2-vsat1)*((time-time1)/(time2-time1)) + vsat1;
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

float ax,bx,cx,fa,fb,fc;
ax = start_time - 1;
bx = measurement_time;

// Create an object which supplies the range function to be minimized by
// standard routines like brent() below.
RangeFunction rangefunc(this,&rground);

// Bracket the minimum range.
void mnbrak(float*,float*,float*,float*,float*,float*,float (*f)(float));
//mnbrak(&ax,&bx,&cx,&fa,&fb,&fc,rangefunc.Range);

// Locate the ephemeris position with minimum range to the EarthPosition.
float brent(float,float,float,float (*f)(float),float,float*);
float tol = 1e-4;
double min_time;
//float minrange = brent(ax,bx,cx,rangefunc.Range,tol,&min_time);

// Compute the s/c position at the start of the subtrack grid, and at
// the minimum range position.
EarthPosition min_position,start_position;
GetPosition(min_time,&min_position);
GetPosition(start_time,&start_position);

// Compute the corresponding nadir points on the earth's surface.
EarthPosition subtrack_min = min_position.Nadir();
EarthPosition subtrack_start = start_position.Nadir();

// Compute surface distances in the crosstrack and alongtrack directions.
*crosstrack = subtrack_min.surface_distance(rground);
*alongtrack = subtrack_min.surface_distance(subtrack_start);

return(1);
}

//
// RangeFunction
//

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

float
RangeFunction::Range(double time)

{

EarthPosition rsat;
ephemeris->GetPosition(time,&rsat);
EarthPosition rlook = *rground - rsat;
return(rlook.Magnitude());

}
