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

	*doppler = c_term + a_term *
		cos(two_pi * ((double)azimuth_step / (double)_azimuthSteps) - p_term);

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
:	_delay(NULL), _duration(NULL), _ticksPerOrbit(0),
	_numberOfBeams(0), _rangeSteps(0)
{
	return;
}

RangeTracker::~RangeTracker()
{
	for (unsigned int beam_idx = 0; beam_idx < _numberOfBeams; beam_idx++)
		free(*(_delay + beam_idx));

	free(_delay);
	free(_duration);

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
	//---------------------------------//
	// allocate the _delay[beam] array //
	//---------------------------------//

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

	//----------------------------------------//
	// allocate each _delay[beam][step] array //
	//----------------------------------------//

	size = sizeof(unsigned char) * range_steps;
	for (int beam_idx = 0; beam_idx < number_of_beams; beam_idx++)
	{
		unsigned char* ptr = (unsigned char *)malloc(size);
		if (ptr == NULL)
			return(0);

		*(_delay + beam_idx) = ptr;
	}

	_numberOfBeams = number_of_beams;
	_rangeSteps = range_steps;

	return(1);
}

//------------------------------------//
// RangeTracker::OrbitTimeToRangeStep //
//------------------------------------//

unsigned short
RangeTracker::OrbitTimeToRangeStep(
	unsigned int		orbit_time)
{
	float ticks_per_range_step = (float)_ticksPerOrbit / (float)_rangeSteps;
	float f_range_step = (float)(orbit_time % _ticksPerOrbit) /
		ticks_per_range_step;
	unsigned short range_step = (unsigned short)f_range_step;
	range_step %= _rangeSteps;
	return(range_step);
}

//--------------------------------//
// RangeTracker::DelayAndDuration //
//--------------------------------//

int
RangeTracker::DelayAndDuration(
	int		beam_idx,
	int		orbit_step,
	float	receiver_gate_width,		// seconds
	float	xmit_pulse_width,			// seconds
	float*	delay,
	float*	duration)
{
	//------------------------//
	// scale input parameters //
	//------------------------//

	unsigned char rgw = (unsigned char)(receiver_gate_width /
		RANGE_TRACKING_TIME_RESOLUTION + 0.5);
	unsigned char pw = (unsigned char)(xmit_pulse_width /
		RANGE_TRACKING_TIME_RESOLUTION + 0.5);

	//---------------//
	// set the delay //
	//---------------//

	unsigned char delay_dn = (int) *(*(_delay + beam_idx) + orbit_step);
	delay_dn -= (rgw - pw) / 2;
	*delay = RANGE_TRACKING_TIME_RESOLUTION * (float)delay_dn;

	//------------------//
	// set the duration //
	//------------------//

	unsigned char duration_dn = *(_duration + beam_idx);
	*duration = RANGE_TRACKING_TIME_RESOLUTION * (float)duration_dn;

	return(1);
}

//------------------------//
// RangeTracker::SetDelay //
//------------------------//

int
RangeTracker::SetDelay(
	int		beam_idx,
	int		orbit_step,
	float	receiver_gate_width,		// seconds
	float	xmit_pulse_width,			// seconds
	float	delay)
{
	//------------------------//
	// scale input parameters //
	//------------------------//

	unsigned char rgw = (unsigned char)(receiver_gate_width /
		RANGE_TRACKING_TIME_RESOLUTION + 0.5);
	unsigned char pw = (unsigned char)(xmit_pulse_width /
		RANGE_TRACKING_TIME_RESOLUTION + 0.5);

	//---------------//
	// set the delay //
	//---------------//

	float table_delay = delay + (float)((rgw - pw) / 2.0);
	unsigned char delay_dn = (unsigned char)(table_delay /
		RANGE_TRACKING_TIME_RESOLUTION + 0.5);
	*(*(_delay + beam_idx) + orbit_step) = delay_dn;

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
		fp) != 1)
	{
		return(1);
	}

	//---------------------------//
	// write the ticks per orbit //
	//---------------------------//

	if (fwrite((void *)&_ticksPerOrbit, sizeof(unsigned short), 1, fp) != 1)
	{
		return(1);
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
		fp) != 1)
	{
		return(1);
	}

	//--------------------------//
	// read the ticks per orbit //
	//--------------------------//

	if (fread((void *)&_ticksPerOrbit, sizeof(unsigned short), 1, fp) != 1)
	{
		return(1);
	}

	//----------------//
	// close the file //
	//----------------//

	fclose(fp);
	return(1);
}
