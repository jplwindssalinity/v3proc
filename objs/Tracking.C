//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_tracking_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include "Tracking.h"


//=================//
// DopplerTracking //
//=================//

DopplerTracking::DopplerTracking()
:	am(0.0), ab(0.0), pm(0.0), pb(0.0), cm(0.0), cb(0.0),
	a(NULL), p(NULL), c(NULL), z1(0.0), z2(0.0), period(0), checksum(0),
	_orbitSteps(0), _azimuthSteps(0)
{
	return;
}

DopplerTracking::~DopplerTracking()
{
	return;
}

//---------------------------//
// DopplerTracking::Allocate //
//---------------------------//

int
DopplerTracking::Allocate(
	int		orbit_steps)
{
	int size = sizeof(short) * orbit_steps;

	a = (short *)malloc(size);
	if (! a)
		return(0);

	p = (short *)malloc(size);
	if (! p)
		return(0);

	c = (short *)malloc(size);
	if (! c)
		return(0);

	return(1);
}

//--------------------------//
// DopplerTracking::Doppler //
//--------------------------//

int
DopplerTracking::Doppler(
	int		orbit_step,
	int		azimuth_step,
	float*	doppler)
{
	return(1);
}
