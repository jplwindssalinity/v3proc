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

//-----------------------------------------//
// DopplerTracker::OrbitTicksToDopplerStep //
//-----------------------------------------//

unsigned int
DopplerTracker::OrbitTicksToDopplerStep(
	unsigned int	orbit_ticks)
{
	float ticks_per_doppler_step = (float)_ticksPerOrbit /
		(float)_dopplerSteps;
	float f_doppler_step = (float)(orbit_ticks % _ticksPerOrbit) /
		ticks_per_doppler_step;
	unsigned int doppler_step = (int)f_doppler_step;
	doppler_step %= _dopplerSteps;
	return(doppler_step);
}

//-------------------------------------//
// DopplerTracker::GetCommandedDoppler //
//-------------------------------------//

int
DopplerTracker::GetCommandedDoppler(
	int				beam_idx,
	unsigned int	doppler_step,
	unsigned int	antenna_dn,
	unsigned int	antenna_n,
	float*			doppler)
{
	if (doppler_step >= _dopplerSteps)
		return(0);

	unsigned short* short_ptr = *(*(_term + beam_idx) + doppler_step);
	unsigned short a_dn = *(short_ptr + AMPLITUDE_INDEX);
	unsigned short p_dn = *(short_ptr + PHASE_INDEX);
	unsigned short c_dn = *(short_ptr + CONSTANTS_INDEX);

	float** float_ptr = *(_scale + beam_idx);
	float ab = *(*(float_ptr + AMPLITUDE_INDEX) + 0);
	float am = *(*(float_ptr + AMPLITUDE_INDEX) + 1);
	float pb = *(*(float_ptr + PHASE_INDEX) + 0);
	float pm = *(*(float_ptr + PHASE_INDEX) + 1);
	float cb = *(*(float_ptr + CONSTANTS_INDEX) + 0);
	float cm = *(*(float_ptr + CONSTANTS_INDEX) + 1);

	double a_term = (double)am * (double)a_dn + (double)ab;
	double p_term = (double)pm * (double)p_dn + (double)pb;
	double c_term = (double)cm * (double)c_dn + (double)cb;

	double raw_doppler = c_term + a_term *
		cos(two_pi * (double)antenna_dn / (double)antenna_n + p_term);

	int doppler_dn = (int)(raw_doppler / DOPPLER_TRACKING_RESOLUTION + 0.5);

	// negative sign to convert "true" Doppler to additive xmit freq
	*doppler = - doppler_dn * DOPPLER_TRACKING_RESOLUTION;

	return(1);
}

//-------------------------------//
// DopplerTracker::SetInstrument //
//-------------------------------//

int
DopplerTracker::SetInstrument(
	Instrument*		instrument)
{
	int beam_idx = instrument->antenna.currentBeamIdx;
	unsigned int doppler_step =
		OrbitTicksToDopplerStep(instrument->orbitTicks);

	unsigned int antenna_dn = instrument->antenna.GetEncoderValue();
	unsigned int antenna_n = instrument->antenna.GetEncoderN();
	float doppler;
	GetCommandedDoppler(beam_idx, doppler_step, antenna_dn, antenna_n,
		&doppler);

	instrument->commandedDoppler = doppler;

	return(1);
}

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

//----------------------------------//
// DopplerTracker::SetTicksPerOrbit //
//----------------------------------//

int
DopplerTracker::SetTicksPerOrbit(
	unsigned int	period)
{
	_ticksPerOrbit = period;
	return(1);
}

//-----------------------------//
// DopplerTracker::WriteBinary //
//-----------------------------//

int
DopplerTracker::WriteBinary(
	const char*		filename)
{
	//---------------//
	// open the file //
	//---------------//

	FILE* fp = fopen(filename, "w");
	if (fp == NULL)
		return(0);

	//---------------------------------------------//
	// write the number of beams and doppler steps //
	//---------------------------------------------//

	if (fwrite((void *)&_numberOfBeams, sizeof(unsigned int), 1, fp) != 1 ||
		fwrite((void *)&_dopplerSteps, sizeof(unsigned int), 1, fp) != 1)
	{
		fclose(fp);
		return(0);
	}

	//-----------------------//
	// write the scale terms //
	//-----------------------//

	for (unsigned int beam_idx = 0; beam_idx < _numberOfBeams; beam_idx++)
	{
		for (unsigned int term = 0; term < 3; term++)
		{
			if (fwrite((void *) *(*(_scale + beam_idx) + term), sizeof(float),
				2, fp) != 2)
			{
				return(0);
			}
		}
	}

	//-----------------//
	// write the terms //
	//-----------------//

	for (unsigned int beam_idx = 0; beam_idx < _numberOfBeams; beam_idx++)
	{
		for (unsigned int step = 0; step < _dopplerSteps; step++)
		{
			if (fwrite((void *) *(*(_term + beam_idx) + step),
				sizeof(unsigned short), 3, fp) != 3)
			{
				return(0);
			}
		}
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

//----------------------------//
// DopplerTracker::ReadBinary //
//----------------------------//

int
DopplerTracker::ReadBinary(
	const char*		filename)
{
	//---------------//
	// open the file //
	//---------------//

	FILE* fp = fopen(filename, "r");
	if (fp == NULL)
		return(0);

	//--------------------------------------------//
	// read the number of beams and doppler steps //
	//--------------------------------------------//

	if (fread((void *)&_numberOfBeams, sizeof(unsigned int), 1, fp) != 1 ||
		fread((void *)&_dopplerSteps, sizeof(unsigned int), 1, fp) != 1)
	{
		fclose(fp);
		return(0);
	}

	//----------//
	// allocate //
	//----------//

	if (! Allocate(_numberOfBeams, _dopplerSteps))
	{
		fclose(fp);
		return(0);
	}

	//----------------------//
	// read the scale terms //
	//----------------------//

	for (unsigned int beam_idx = 0; beam_idx < _numberOfBeams; beam_idx++)
	{
		for (unsigned int term = 0; term < 3; term++)
		{
			if (fread((void *) *(*(_scale + beam_idx) + term), sizeof(float),
				2, fp) != 2)
			{
				return(0);
			}
		}
	}

	//----------------//
	// read the terms //
	//----------------//

	for (unsigned int beam_idx = 0; beam_idx < _numberOfBeams; beam_idx++)
	{
		for (unsigned int step = 0; step < _dopplerSteps; step++)
		{
			if (fread((void *) *(*(_term + beam_idx) + step),
				sizeof(unsigned short), 3, fp) != 3)
			{
				return(0);
			}
		}
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


//==============//
// RangeTracker //
//==============//

RangeTracker::RangeTracker()
:	_scale(NULL), _delay(NULL), _duration(NULL), _ticksPerOrbit(0),
	_numberOfBeams(0), _rangeSteps(0)
{
	return;
}

RangeTracker::~RangeTracker()
{
	free_array((void *)_scale, 3, _numberOfBeams, 3, 2);
	free_array((void *)_delay, 3, _numberOfBeams, _rangeSteps, 3);
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
	_scale = (float ***)make_array(sizeof(float), 3, number_of_beams, 3, 2);
	if (_scale == NULL)
		return(0);

	_delay = (unsigned char ***)make_array(sizeof(unsigned char), 3,
		number_of_beams, range_steps, 3);
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

//-------------------------------------//
// RangeTracker::OrbitTicksToRangeStep //
//-------------------------------------//

unsigned short
RangeTracker::OrbitTicksToRangeStep(
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
	int				beam_idx,
	unsigned int	range_step,
	float			xmit_pulse_width,		// seconds
	unsigned int	antenna_dn,
	unsigned int	antenna_n,
	float*			delay,
	float*			duration)
{
	//------------------------//
	// determine the duration //
	//------------------------//

	unsigned char duration_dn = *(_duration + beam_idx);
	float rgw = (float)duration_dn * RANGE_TRACKING_TIME_RESOLUTION;

	//-------------------------//
	// get the dn coefficients //
	//-------------------------//

	unsigned char* char_ptr = *(*(_delay + beam_idx) + range_step);
	unsigned char a_dn = *(char_ptr + AMPLITUDE_INDEX);
	unsigned char p_dn = *(char_ptr + PHASE_INDEX);
	unsigned char c_dn = *(char_ptr + CONSTANTS_INDEX);

	//-------------------------------//
	// calculate the scaling factors //
	//-------------------------------//

	float** float_ptr = *(_scale + beam_idx);
	float ab = *(*(float_ptr + AMPLITUDE_INDEX) + 0);
	float am = *(*(float_ptr + AMPLITUDE_INDEX) + 1);
	float pb = *(*(float_ptr + PHASE_INDEX) + 0);
	float pm = *(*(float_ptr + PHASE_INDEX) + 1);
	float cb = *(*(float_ptr + CONSTANTS_INDEX) + 0);
	float cm = *(*(float_ptr + CONSTANTS_INDEX) + 1);

	double a_term = (double)am * (double)a_dn + (double)ab;
	double p_term = (double)pm * (double)p_dn + (double)pb;
	double c_term = (double)cm * (double)c_dn + (double)cb;

	//---------------------//
	// determine the delay //
	//---------------------//

	float table_delay = c_term + a_term * cos((two_pi/(double)antenna_n) *
		(double)antenna_dn + p_term);
	table_delay /= 1000.0;		// convert ms to seconds

	float cmd_delay = table_delay - (rgw - xmit_pulse_width) / 2.0;

	//----------//
	// quantize //
	//----------//

	*duration = rgw;		// already quantized
	unsigned int delay_dn =
		(int)(cmd_delay / RANGE_TRACKING_TIME_RESOLUTION + 0.5);
	*delay = delay_dn * RANGE_TRACKING_TIME_RESOLUTION;

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
	int range_step = OrbitTicksToRangeStep(instrument->orbitTicks);
	Beam* beam = instrument->antenna.GetCurrentBeam();
	float xpw = beam->pulseWidth;

	unsigned int antenna_dn = instrument->antenna.GetEncoderValue();
	unsigned int antenna_n = instrument->antenna.GetEncoderN();

	float delay, duration;
	GetDelayAndDuration(beam_idx, range_step, xpw, antenna_dn, antenna_n,
		&delay, &duration);

	instrument->receiverGateDelay = delay;
	instrument->receiverGateDuration = duration;

	return(1);
}

//--------------------------------//
// RangeTracker::SetRoundTripTime //
//--------------------------------//

int
RangeTracker::SetRoundTripTime(
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

		for (unsigned int range_step = 0; range_step < _rangeSteps;
			range_step++)
		{
			for (int term_idx = 0; term_idx < 3; term_idx++)
			{
				if (*(*(term_ptr + range_step) + term_idx) < mins[term_idx])
					mins[term_idx] = *(*(term_ptr + range_step) + term_idx);

				if (*(*(term_ptr + range_step) + term_idx) > maxs[term_idx])
					maxs[term_idx] = *(*(term_ptr + range_step) + term_idx);
			}
		}

		//------------------------//
		// generate scale factors //
		//------------------------//

		for (int term_idx = 0; term_idx < 3; term_idx++)
		{
			*(*(*(_scale + beam_idx) + term_idx) + 0) = mins[term_idx];

			*(*(*(_scale + beam_idx) + term_idx) + 1) =
				(maxs[term_idx] - mins[term_idx]) / 255.0;
		}

		//------------------------//
		// calculate scaled terms //
		//------------------------//

		for (unsigned int range_step = 0; range_step < _rangeSteps;
			range_step++)
		{
			for (int term_idx = 0; term_idx < 3; term_idx++)
			{
				*(*(*(_delay + beam_idx) + range_step) + term_idx) =
					(unsigned char)(
					(*(*(term_ptr + range_step) + term_idx) -
					*(*(*(_scale + beam_idx) + term_idx) + 0)) /
					*(*(*(_scale + beam_idx) + term_idx) + 1) + 0.5);
			}
		}
	}

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

	if (fwrite((void *)&_numberOfBeams, sizeof(unsigned int), 1, fp) != 1 ||
		fwrite((void *)&_rangeSteps, sizeof(unsigned int), 1, fp) != 1)
	{
		fclose(fp);
		return(0);
	}

	//-----------------------//
	// write the scale terms //
	//-----------------------//

	for (unsigned int beam_idx = 0; beam_idx < _numberOfBeams; beam_idx++)
	{
		for (unsigned int term = 0; term < 3; term++)
		{
			if (fwrite((void *) *(*(_scale + beam_idx) + term), sizeof(float),
				2, fp) != 2)
			{
				return(0);
			}
		}
	}

	//-----------------//
	// write the delay //
	//-----------------//

	for (unsigned int beam_idx = 0; beam_idx < _numberOfBeams; beam_idx++)
	{
		for (unsigned int step = 0; step < _rangeSteps; step++)
		{
			if (fwrite((void *) *(*(_delay + beam_idx) + step),
				sizeof(unsigned char), 3, fp) != 3)
			{
				return(0);
			}
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

	if (fread((void *)&_numberOfBeams, sizeof(unsigned int), 1, fp) != 1 ||
		fread((void *)&_rangeSteps, sizeof(unsigned int), 1, fp) != 1)
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
	// read the scale terms //
	//----------------------//

	for (unsigned int beam_idx = 0; beam_idx < _numberOfBeams; beam_idx++)
	{
		for (unsigned int term = 0; term < 3; term++)
		{
			if (fread((void *) *(*(_scale + beam_idx) + term), sizeof(float),
				2, fp) != 2)
			{
				return(0);
			}
		}
	}

	//----------------//
	// read the delay //
	//----------------//

	for (unsigned int beam_idx = 0; beam_idx < _numberOfBeams; beam_idx++)
	{
		for (unsigned int step = 0; step < _rangeSteps; step++)
		{
			if (fread((void *) *(*(_delay + beam_idx) + step),
				sizeof(unsigned char), 3, fp) != 3)
			{
				return(0);
			}
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

//-------------//
// azimuth_fit //
//-------------//
 
int
azimuth_fit(
	int			count,
	double*		terms,
	double*		a,
	double*		p,
	double*		c)
{
	double wn = two_pi / (double) count;
	double real[2], imag[2];
 
	for (int i = 0; i < 2; i++)
	{
		real[i] = 0.0;
		imag[i] = 0.0;
		for (int j = 0; j < count; j++)
		{
			double arg = wn * (double)i * (double)j;
			double c = cos(arg);
			double s = sin(arg);
			real[i] += terms[j] * c;
			imag[i] += terms[j] * s;
		}
	}
 
	*a = 2.0 * sqrt(real[1] * real[1] + imag[1] * imag[1]) / (double)count;
	*p = -atan2(imag[1], real[1]);
	*c = real[0] / (double)count;
 
	return(1);
}
