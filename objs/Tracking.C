//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_tracking_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include "Tracking.h"
#include "Array.h"


//================//
// DopplerTracker //
//================//

DopplerTracker::DopplerTracker()
:	_scale(NULL), _term(NULL), _ticksPerOrbit(0), _numberOfBeams(0),
	_dopplerSteps(0)
{
	return;
}

DopplerTracker::~DopplerTracker()
{
	free_array(_scale, 3, _numberOfBeams, 3, 2);
	free_array(_term, 3, _numberOfBeams, _dopplerSteps, 3);

	return;
}

//--------------------------//
// DopplerTracker::Allocate //
//--------------------------//

int
DopplerTracker::Allocate(
	int		number_of_beams,
	int		doppler_orbit_steps)
{
	_scale = (float ***)make_array(sizeof(float), 3, number_of_beams, 3, 2);
	if (_scale == NULL)
		return(0);

	_term = (unsigned short ***)make_array(sizeof(unsigned short), 3,
		number_of_beams, doppler_orbit_steps, 3);
	if (_term == NULL)
		return(0);

	_numberOfBeams = number_of_beams;
	_dopplerSteps = doppler_orbit_steps;

	return(1);
}

/*
//-------------------------//
// DopplerTracker::Doppler //
//-------------------------//

int
DopplerTracker::Doppler(
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

	double raw_doppler = c_term + a_term *
		cos(two_pi * ((double)azimuth_step / (double)_azimuthSteps) - p_term);

	int doppler_dn = (int)(raw_doppler / DOPPLER_TRACKING_RESOLUTION + 0.5);

	*doppler = doppler_dn * DOPPLER_TRACKING_RESOLUTION;

	return(1);
}
*/

//---------------------//
// DopplerTracker::Set //
//---------------------//

int
DopplerTracker::Set(
	double***	terms)
{
	double mins[3];
	double maxs[3];

	//---------------//
	// for each beam //
	//---------------//

	for (unsigned int beam_idx = 0; beam_idx < _numberOfBeams; beam_idx++)
	{
		//-----------------------------------------//
		// calculate the minimum and maximum terms //
		//-----------------------------------------//

		double** term_ptr = *(terms + beam_idx);

		for (int term_idx = 0; term_idx < 3; term_idx++)
		{
			mins[term_idx] = *(*(term_ptr + 0) + term_idx);
			maxs[term_idx] = mins[term_idx];
		}

		for (unsigned int orbit_step = 0; orbit_step < _dopplerSteps;
			orbit_step++)
		{
			for (int term_idx = 0; term_idx < 3; term_idx++)
			{
				if (*(*(term_ptr + orbit_step) + term_idx) < mins[term_idx])
					mins[term_idx] = *(*(term_ptr + orbit_step) + term_idx);

				if (*(*(term_ptr + orbit_step) + term_idx) > maxs[term_idx])
					maxs[term_idx] = *(*(term_ptr + orbit_step) + term_idx);
			}
		}

		//------------------------//
		// generate scale factors //
		//------------------------//

		for (int term_idx = 0; term_idx < 3; term_idx++)
		{
			*(*(*(_scale + beam_idx) + term_idx) + 0) = mins[term_idx];

			*(*(*(_scale + beam_idx) + term_idx) + 1) =
				(maxs[term_idx] - mins[term_idx]) / 65535.0;
		}

		//------------------------//
		// calculate scaled terms //
		//------------------------//

		for (unsigned int orbit_step = 0; orbit_step < _dopplerSteps;
			orbit_step++)
		{
			for (int term_idx = 0; term_idx < 3; term_idx++)
			{
				*(*(*(_term + beam_idx) + orbit_step) + term_idx) =
					(unsigned short)(
					(*(*(term_ptr + orbit_step) + term_idx) - 
					*(*(*(_scale + beam_idx) + term_idx) + 0)) /
					*(*(*(_scale + beam_idx) + term_idx) + 1) + 0.5);
			}
		}
	}

	return(1);
}

/*
//----------------------------------------//
// DopplerTracker::OrbitTimeToDopplerStep //
//----------------------------------------//

unsigned short
DopplerTracker::OrbitTimeToDopplerStep(
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

//-----------------------------//
// DopplerTracker::WriteBinary //
//-----------------------------//

int
DopplerTracker::WriteBinary(
	const char*		filename)
{
	filename;
	return(0);
}

//----------------------------//
// DopplerTracker::ReadBinary //
//----------------------------//

int
DopplerTracker::ReadBinary(
	const char*		filename)
{
	filename;
	return(0);
}

//==============//
// RangeTracker //
//==============//

RangeTracker::RangeTracker()
:	_delay(NULL), _duration(NULL), _ticksPerOrbit(0), _numberOfBeams(0),
	_rangeSteps(0)
{
	return;
}

RangeTracker::~RangeTracker()
{
	free_array((void *)_delay, 2, _numberOfBeams, _rangeSteps);
	free_array((void *)_duration, 1, _numberOfBeams);

	return;
}

//------------------------//
// RangeTracker::Allocate //
//------------------------//

int
RangeTracker::Allocate(
	int		number_of_beams,
	int		range_steps)
{
	_delay = (unsigned char **)make_array(sizeof(unsigned char), 2,
		number_of_beams, range_steps);
	if (_delay == NULL)
		return(0);

	_duration = (unsigned char *)make_array(sizeof(unsigned char), 1,
		number_of_beams);
	if (_duration == NULL)
		return(0);

	_numberOfBeams = number_of_beams;
	_rangeSteps = range_steps;

	return(1);
}

//------------------------------------//
// RangeTracker::OrbitTimeToRangeStep //
//------------------------------------//

unsigned short
RangeTracker::OrbitTimeToRangeStep(
	unsigned int		orbit_ticks)
{
	float ticks_per_range_step = (float)_ticksPerOrbit / (float)_rangeSteps;
	float f_range_step = (float)(orbit_ticks % _ticksPerOrbit) /
		ticks_per_range_step;
	unsigned short range_step = (unsigned short)f_range_step;
	range_step %= _rangeSteps;
	return(range_step);
}

//-----------------------------------//
// RangeTracker::GetDelayAndDuration //
//-----------------------------------//

int
RangeTracker::GetDelayAndDuration(
	int		beam_idx,
	int		range_step,
	float	xmit_pulse_width,			// seconds
	float*	delay,
	float*	duration)
{
	//------------------------//
	// determine the duration //
	//------------------------//

	unsigned char duration_dn = *(_duration + beam_idx);
	*duration = RANGE_TRACKING_TIME_RESOLUTION * (float)duration_dn;

	//-----------------------//
	// scale the pulse width //
	//-----------------------//

	unsigned char pw_dn = (unsigned char)(xmit_pulse_width /
		RANGE_TRACKING_TIME_RESOLUTION + 0.5);

	//---------------------//
	// determine the delay //
	//---------------------//

	unsigned char delay_dn = (int) *(*(_delay + beam_idx) + range_step);
	delay_dn -= (duration_dn - pw_dn) / 2;
	*delay = RANGE_TRACKING_TIME_RESOLUTION * (float)delay_dn;

	return(1);
}

//-----------------------------//
// RangeTracker::SetInstrument //
//-----------------------------//

int
RangeTracker::SetInstrument(
	Instrument*		instrument)
{
	int beam_idx = instrument->antenna.currentBeamIdx;
	int range_step = OrbitTimeToRangeStep(instrument->orbitTicks);
	Beam* beam = instrument->antenna.GetCurrentBeam();
	float xpw = beam->pulseWidth;

	float delay, duration;
	GetDelayAndDuration(beam_idx, range_step, xpw, &delay, &duration);

	instrument->receiverGateDelay = delay;
	instrument->receiverGateDuration = duration;

	return(1);
}

//--------------------------------//
// RangeTracker::SetRoundTripTime //
//--------------------------------//

int
RangeTracker::SetRoundTripTime(
	int		beam_idx,
	int		range_step,
	float	round_trip_time)
{
	unsigned char delay_dn = (unsigned char)(round_trip_time /
		RANGE_TRACKING_TIME_RESOLUTION + 0.5);
	*(*(_delay + beam_idx) + range_step) = delay_dn;

	return(1);
}

//---------------------------//
// RangeTracker::SetDuration //
//---------------------------//

int
RangeTracker::SetDuration(
	int		beam_idx,
	float	duration)
{
	//------------------//
	// set the duration //
	//------------------//

	unsigned char duration_dn = (unsigned char)(duration /
		RANGE_TRACKING_TIME_RESOLUTION + 0.5);
	*(_duration + beam_idx) = duration_dn;

	return(1);
}

//--------------------------------//
// RangeTracker::SetTicksPerOrbit //
//--------------------------------//

int
RangeTracker::SetTicksPerOrbit(
	unsigned int	period)
{
	_ticksPerOrbit = period;
	return(1);
}

//---------------------------//
// RangeTracker::WriteBinary //
//---------------------------//

int
RangeTracker::WriteBinary(
	const char*		filename)
{
	//---------------//
	// open the file //
	//---------------//

	FILE* fp = fopen(filename, "w");
	if (fp == NULL)
		return(0);

	//-------------------------------------------//
	// write the number of beams and range steps //
	//-------------------------------------------//

	if (fwrite((void *)&_numberOfBeams, sizeof(int), 1, fp) != 1 ||
		fwrite((void *)&_rangeSteps, sizeof(int), 1, fp) != 1)
	{
		fclose(fp);
		return(0);
	}

	//-----------------------//
	// write the delay terms //
	//-----------------------//

	for (unsigned int beam_idx = 0; beam_idx < _numberOfBeams; beam_idx++)
	{
		if (fwrite((void *)*(_delay + beam_idx), sizeof(unsigned char),
			_rangeSteps, fp) != _rangeSteps)
		{
			return(0);
		}
	}

	//--------------------//
	// write the duration //
	//--------------------//

	if (fwrite((void *)_duration, sizeof(unsigned char), _numberOfBeams,
		fp) != _numberOfBeams)
	{
		return(0);
	}

	//---------------------------//
	// write the ticks per orbit //
	//---------------------------//

	if (fwrite((void *)&_ticksPerOrbit, sizeof(unsigned int), 1, fp) != 1)
	{
		return(0);
	}

	//----------------//
	// close the file //
	//----------------//

	fclose(fp);
	return(1);
}

//--------------------------//
// RangeTracker::ReadBinary //
//--------------------------//

int
RangeTracker::ReadBinary(
	const char*		filename)
{
	//---------------//
	// open the file //
	//---------------//

	FILE* fp = fopen(filename, "r");
	if (fp == NULL)
		return(0);

	//------------------------------------------//
	// read the number of beams and range steps //
	//------------------------------------------//

	if (fread((void *)&_numberOfBeams, sizeof(int), 1, fp) != 1 ||
		fread((void *)&_rangeSteps, sizeof(int), 1, fp) != 1)
	{
		fclose(fp);
		return(0);
	}

	//----------//
	// allocate //
	//----------//

	if (! Allocate(_numberOfBeams, _rangeSteps))
	{
		fclose(fp);
		return(0);
	}

	//----------------------//
	// read the delay terms //
	//----------------------//

	for (unsigned int beam_idx = 0; beam_idx < _numberOfBeams; beam_idx++)
	{
		if (fread((void *)*(_delay + beam_idx), sizeof(unsigned char),
			_rangeSteps, fp) != _rangeSteps)
		{
			return(0);
		}
	}

	//-------------------//
	// read the duration //
	//-------------------//

	if (fread((void *)_duration, sizeof(unsigned char), _numberOfBeams,
		fp) != _numberOfBeams)
	{
		return(0);
	}

	//--------------------------//
	// read the ticks per orbit //
	//--------------------------//

	if (fread((void *)&_ticksPerOrbit, sizeof(unsigned int), 1, fp) != 1)
	{
		return(0);
	}

	//----------------//
	// close the file //
	//----------------//

	fclose(fp);
	return(1);
}
