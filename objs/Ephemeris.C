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

//
// Ephemeris
//

//
// Default constructor
//

Ephemeris::Ephemeris()
{
return;
}

//
// Destructor
//

Ephemeris::~Ephemeris()
{
return;
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

OrbitState *current_state = GetCurrent();

//
// Bracket the desired time in the ephemeris.
//

while (current_state != NULL)
{
if (current_state->time < time)
	current_state = GetNext();
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
	return(0);

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

