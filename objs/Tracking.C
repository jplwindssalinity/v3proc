//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_tracking_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include "Tracking.h"


/*
//=================//
// DopplerTracking //
//=================//

DopplerTracking::DopplerTracking()
:	am(0.0), ab(0.0), pm(0.0), pb(0.0), cm(0.0), cb(0.0),
	a(NULL), p(NULL), c(NULL), z1(0.0), z2(0.0), ticksPerOrbit(0), checksum(0),
	_orbitSteps(0), _azimuthSteps(0)
{
	return;
}

DopplerTracking::~DopplerTracking()
{
	free(a);
	free(p);
	free(c);
	return;
}

//---------------------------//
// DopplerTracking::Allocate //
//---------------------------//

int
DopplerTracking::Allocate(
	int		number_of_beams,
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
	//-----------------------------------//
	// check the orbit and azimuth steps //
	//-----------------------------------//

	if (orbit_step < 0 || orbit_step > _orbitSteps)
		return(0);

	if (azimuth_step < 0 || azimuth_step > _azimuthSteps)
		return(0);

	//---------------------------------//
	// calculate the Doppler frequency //
	//---------------------------------//

	double a_term = (double)am * (double)a[azimuth_step] + (double)ab;
	double p_term = (double)pm * (double)p[azimuth_step] + (double)pb;
	double c_term = (double)cm * (double)c[azimuth_step] + (double)cb;

	*doppler = c_term + a_term *
		cos(two_pi * ((double)azimuth_step / (double)_azimuthSteps) - p_term);

	return(1);
}

//-----------------------------------------//
// DopplerTracking::OrbitTimeToDopplerStep //
//-----------------------------------------//

unsigned short
DopplerTracking::OrbitTimeToDopplerStep(
	unsigned int	orbit_time)
{
	float ticks_per_doppler_step = (float)_ticksPerOrbit /
		(float)_dopplerSteps;
	float f_doppler_step = (float)(orbit_time % _ticksPerOrbit) /
		ticks_per_doppler_step;
	unsigned short doppler_step = (unsigned short)f_doppler_step;
	doppler_step %= _dopplerSteps;
	return(doppler_step);
}

*/

//===============//
// RangeTracking //
//===============//

RangeTracking::RangeTracking()
:	_delay(NULL), _duration(NULL), _ticksPerOrbit(0),
	_numberOfBeams(0), _rangeSteps(0)
{
	return;
}

RangeTracking::~RangeTracking()
{
	for (int i = 0; i < _numberOfBeams; i++)
		free(*(_delay + i));

	free(_delay);
	free(_duration);

	return;
}

//-------------------------//
// RangeTracking::Allocate //
//-------------------------//

int
RangeTracking::Allocate(
	int		number_of_beams,
	int		orbit_steps)
{
	_numberOfBeams = number_of_beams;

	//--------------------------------//
	// allocate the _delay[beam] array //
	//--------------------------------//

	int size = number_of_beams * sizeof(unsigned char *);
	_delay = (unsigned char **)malloc(size);
	if (_delay == NULL)
		return(0);

	//------------------------------------//
	// allocate the duraction[beam] array //
	//------------------------------------//

	size = number_of_beams * sizeof(unsigned char);
	_duration = (unsigned char *)malloc(size);
	if (_duration == NULL)
		return(0);

	//---------------------------------------//
	// allocate each _delay[beam][step] array //
	//---------------------------------------//

	size = sizeof(unsigned char) * orbit_steps;
	for (int i = 0; i < number_of_beams; i++)
	{
		unsigned char* ptr = (unsigned char *)malloc(size);
		if (ptr == NULL)
			return(0);

		*(_delay + i) = ptr;
	}

	return(1);
}

//-------------------------------------//
// RangeTracking::OrbitTimeToRangeStep //
//-------------------------------------//

unsigned short
RangeTracking::OrbitTimeToRangeStep(
	unsigned int		orbit_time)
{
	float ticks_per_range_step = (float)_ticksPerOrbit / (float)_rangeSteps;
	float f_range_step = (float)(orbit_time % _ticksPerOrbit) /
		ticks_per_range_step;
	unsigned short range_step = (unsigned short)f_range_step;
	range_step %= _rangeSteps;
	return(range_step);
}

//-------------------------------------//
// RangeTracking::SetReceiverGateWidth //
//-------------------------------------//

int
RangeTracking::SetReceiverGateWidth(
	float	receiver_gate_width)
{
	_receiverGateWidth = (unsigned char)(receiver_gate_width /
		RANGE_TIME_RESOLUTION + 0.5);
	return(1);
}

//----------------------------------//
// RangeTracking::SetXmitPulseWidth //
//----------------------------------//

int
RangeTracking::SetXmitPulseWidth(
	float	xmit_pulse_width)
{
	_xmitPulseWidth = (unsigned char)(xmit_pulse_width /
		RANGE_TIME_RESOLUTION + 0.5);
	return(1);
}

//---------------------------------//
// RangeTracking::DelayAndDuration //
//---------------------------------//

int
RangeTracking::DelayAndDuration(
	int		beam_idx,
	int		orbit_step,
	float*	delay,
	float*	duration)
{
	//---------------//
	// set the delay //
	//---------------//

	unsigned char delay_dn = *(*(_delay + beam_idx) + orbit_step);
	delay_dn += (_receiverGateWidth - _xmitPulseWidth) / 2;
	*delay = RANGE_TIME_RESOLUTION * (float)delay_dn;

	//------------------//
	// set the duration //
	//------------------//

	unsigned char duration_dn = *(_duration + beam_idx);
	*duration = RANGE_TIME_RESOLUTION * (float)duration_dn;

	return(1);
}
