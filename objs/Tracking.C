//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_tracking_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include "Tracking.h"
#include "Array.h"
#include "Instrument.h"


//==============//
// RangeTracker //
//==============//

RangeTracker::RangeTracker()
:	_tableId(0), _scale(NULL), _delay(NULL), _rangeSteps(0)
{
	for (int i = 0; i < 2; i++)
		_dither[i] = 0;

	return;
}

RangeTracker::~RangeTracker()
{
	free_array((void *)_scale, 2, 3, 2);
	free_array((void *)_delay, 2, _rangeSteps, 3);

	return;
}

//------------------------//
// RangeTracker::Allocate //
//------------------------//

int
RangeTracker::Allocate(
	int		range_steps)
{
	_scale = (float **)make_array(sizeof(float), 2, 3, 2);
	if (_scale == NULL)
		return(0);

	_delay = (unsigned char **)make_array(sizeof(unsigned char), 2,
		range_steps, 3);
	if (_delay == NULL)
		return(0);

	_rangeSteps = range_steps;

	return(1);
}

//-------------------------------------//
// RangeTracker::OrbitTicksToRangeStep //
//-------------------------------------//

unsigned short
RangeTracker::OrbitTicksToRangeStep(
	unsigned int	orbit_ticks,
	unsigned int	ticks_per_orbit)
{
	float ticks_per_range_step = (float)ticks_per_orbit / (float)_rangeSteps;
	float f_range_step = (float)(orbit_ticks % ticks_per_orbit) /
		ticks_per_range_step;
	unsigned short range_step = (unsigned short)f_range_step;
	range_step %= _rangeSteps;
	return(range_step);
}

//------------------------------//
// RangeTracker::GetRxGateDelay //
//------------------------------//

int
RangeTracker::GetRxGateDelay(
	unsigned int	range_step,
	float			xmit_pulse_width,		// seconds
	float			rx_gate_width,			// seconds
	unsigned int	antenna_dn,
	unsigned int	antenna_n,
	float*			delay)
{
	//-------------------------//
	// get the dn coefficients //
	//-------------------------//

	unsigned char* char_ptr = *(_delay + range_step);
	unsigned char a_dn = *(char_ptr + AMPLITUDE_INDEX);
	unsigned char p_dn = *(char_ptr + PHASE_INDEX);
	unsigned char c_dn = *(char_ptr + CONSTANTS_INDEX);

	//-------------------------------//
	// calculate the scaling factors //
	//-------------------------------//

	float ab = *(*(_scale + AMPLITUDE_INDEX) + 0);
	float am = *(*(_scale + AMPLITUDE_INDEX) + 1);
	float pb = *(*(_scale + PHASE_INDEX) + 0);
	float pm = *(*(_scale + PHASE_INDEX) + 1);
	float cb = *(*(_scale + CONSTANTS_INDEX) + 0);
	float cm = *(*(_scale + CONSTANTS_INDEX) + 1);

	double a_term = (double)am * (double)a_dn + (double)ab;
	double p_term = (double)pm * (double)p_dn + (double)pb;
	double c_term = (double)cm * (double)c_dn + (double)cb;

	//---------------------//
	// determine the delay //
	//---------------------//

	float table_delay = c_term + a_term * cos((two_pi/(double)antenna_n) *
		(double)antenna_dn + p_term);
	table_delay *= MS_TO_S;		// convert ms to seconds

	*delay = table_delay + (xmit_pulse_width - rx_gate_width) / 2.0;

	return(1);
}

//-----------------------------//
// RangeTracker::QuantizeWidth //
//-----------------------------//

float
RangeTracker::QuantizeWidth(
	float		width)
{
	unsigned int width_dn =
		(unsigned int)(width / RANGE_TRACKING_TIME_RESOLUTION + 0.5);
	float qwidth = (float)width_dn * RANGE_TRACKING_TIME_RESOLUTION;
	return(qwidth);
}

//-----------------------------//
// RangeTracker::QuantizeDelay //
//-----------------------------//

float
RangeTracker::QuantizeDelay(
	float		delay,
	float*		residual_delay)
{
	unsigned int delay_dn =
		(unsigned int)(delay / RANGE_TRACKING_TIME_RESOLUTION + 0.5);
	float qdelay = (float)delay_dn * RANGE_TRACKING_TIME_RESOLUTION;
	*residual_delay = delay - qdelay;
	return(qdelay);
}

//-----------------------------//
// RangeTracker::SetInstrument //
//-----------------------------//

int
RangeTracker::SetInstrument(
	Instrument*		instrument,
	float*			residual_delay)
{
	Antenna* antenna = &(instrument->antenna);
	Beam* beam = antenna->GetCurrentBeam();

	//-------//
	// width //
	//-------//
 
	instrument->commandedRxGateWidth =
		beam->rangeTracker.QuantizeWidth(beam->rxGateWidth);
 
	//-------//
	// delay //
	//-------//
 
	unsigned short range_step =
		beam->rangeTracker.OrbitTicksToRangeStep(instrument->orbitTicks,
		instrument->orbitTicksPerPeriod);
	unsigned int encoder = antenna->GetEncoderValue();
	unsigned int encoder_n = antenna->GetEncoderN();
 
	float delay;
 
	if (! beam->rangeTracker.GetRxGateDelay(range_step, beam->pulseWidth,
		instrument->commandedRxGateWidth, encoder, encoder_n, &delay))
	{
		fprintf(stderr, "RangeTracker::SetInstrument: error using RGC\n");
		return(0);
	}
	instrument->commandedRxGateDelay =
		beam->rangeTracker.QuantizeDelay(delay, residual_delay);

	return(1);
}

//--------------------------------//
// RangeTracker::SetRoundTripTime //
//--------------------------------//

int
RangeTracker::SetRoundTripTime(
	double**	terms)
{
	double mins[3];
	double maxs[3];

	//-----------------------------------------//
	// calculate the minimum and maximum terms //
	//-----------------------------------------//

	for (int term_idx = 0; term_idx < 3; term_idx++)
	{
		mins[term_idx] = *(*(terms + 0) + term_idx);
		maxs[term_idx] = mins[term_idx];
	}

	for (unsigned int range_step = 0; range_step < _rangeSteps;
		range_step++)
	{
		for (int term_idx = 0; term_idx < 3; term_idx++)
		{
			if (*(*(terms + range_step) + term_idx) < mins[term_idx])
				mins[term_idx] = *(*(terms + range_step) + term_idx);

			if (*(*(terms + range_step) + term_idx) > maxs[term_idx])
				maxs[term_idx] = *(*(terms + range_step) + term_idx);
		}
	}

	//------------------------//
	// generate scale factors //
	//------------------------//

	for (int term_idx = 0; term_idx < 3; term_idx++)
	{
		*(*(_scale + term_idx) + 0) = S_TO_MS * mins[term_idx];

		*(*(_scale + term_idx) + 1) = S_TO_MS *
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
			*(*(_delay + range_step) + term_idx) =
				(unsigned char)((S_TO_MS * *(*(terms + range_step) +
				term_idx) - *(*(_scale + term_idx) + 0)) /
				*(*(_scale + term_idx) + 1) + 0.5);
		}
	}

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

	if (fwrite((void *)&_rangeSteps, sizeof(unsigned int), 1, fp) != 1)
	{
		fclose(fp);
		return(0);
	}

	//-----------------------//
	// write the scale terms //
	//-----------------------//

	for (unsigned int term = 0; term < 3; term++)
	{
		if (fwrite((void *) *(_scale + term), sizeof(float), 2, fp) != 2)
		{
			return(0);
		}
	}

	//-----------------//
	// write the delay //
	//-----------------//

	for (unsigned int step = 0; step < _rangeSteps; step++)
	{
		if (fwrite((void *) *(_delay + step), sizeof(unsigned char),
			3, fp) != 3)
		{
			return(0);
		}
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

	if (fread((void *)&_rangeSteps, sizeof(unsigned int), 1, fp) != 1)
	{
		fclose(fp);
		return(0);
	}

	//----------//
	// allocate //
	//----------//

	if (! Allocate(_rangeSteps))
	{
		fclose(fp);
		return(0);
	}

	//----------------------//
	// read the scale terms //
	//----------------------//

	for (unsigned int term = 0; term < 3; term++)
	{
		if (fread((void *) *(_scale + term), sizeof(float), 2, fp) != 2)
		{
			return(0);
		}
	}

	//----------------//
	// read the delay //
	//----------------//

	for (unsigned int step = 0; step < _rangeSteps; step++)
	{
		if (fread((void *) *(_delay + step), sizeof(unsigned char),
			3, fp) != 3)
		{
			return(0);
		}
	}

	//----------------//
	// close the file //
	//----------------//

	fclose(fp);
	return(1);
}


//================//
// DopplerTracker //
//================//

DopplerTracker::DopplerTracker()
:	_tableId(0), _scale(NULL), _term(NULL), _dopplerSteps(0)
{
	for (int i = 0; i < 2; i++)
		_dither[i] = 0;

	return;
}

DopplerTracker::~DopplerTracker()
{
	free_array(_scale, 2, 3, 2);
	free_array(_term, 2, _dopplerSteps, 3);

	return;
}

//--------------------------//
// DopplerTracker::Allocate //
//--------------------------//

int
DopplerTracker::Allocate(
	int		doppler_orbit_steps)
{
	_scale = (float **)make_array(sizeof(float), 2, 3, 2);
	if (_scale == NULL)
		return(0);

	_term = (unsigned short **)make_array(sizeof(unsigned short), 2,
		doppler_orbit_steps, 3);
	if (_term == NULL)
		return(0);

	_dopplerSteps = doppler_orbit_steps;

	return(1);
}

//-----------------------------------------//
// DopplerTracker::OrbitTicksToDopplerStep //
//-----------------------------------------//

unsigned int
DopplerTracker::OrbitTicksToDopplerStep(
	unsigned int	orbit_ticks,
	unsigned int	ticks_per_orbit)
{
	float ticks_per_doppler_step = (float)ticks_per_orbit /
		(float)_dopplerSteps;
	float f_doppler_step = (float)(orbit_ticks % ticks_per_orbit) /
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
	unsigned int	doppler_step,
	unsigned int	antenna_dn,
	unsigned int	antenna_n,
	float*			doppler,
	float			chirp_rate,
	float			residual_delay_error)
{
	if (doppler_step >= _dopplerSteps)
		return(0);

	unsigned short* short_ptr = *(_term + doppler_step);
	unsigned short a_dn = *(short_ptr + AMPLITUDE_INDEX);
	unsigned short p_dn = *(short_ptr + PHASE_INDEX);
	unsigned short c_dn = *(short_ptr + CONSTANTS_INDEX);

	float ab = *(*(_scale + AMPLITUDE_INDEX) + 0);
	float am = *(*(_scale + AMPLITUDE_INDEX) + 1);
	float pb = *(*(_scale + PHASE_INDEX) + 0);
	float pm = *(*(_scale + PHASE_INDEX) + 1);
	float cb = *(*(_scale + CONSTANTS_INDEX) + 0);
	float cm = *(*(_scale + CONSTANTS_INDEX) + 1);

	double a_term = (double)am * (double)a_dn + (double)ab;
	double p_term = (double)pm * (double)p_dn + (double)pb;
	double c_term = (double)cm * (double)c_dn + (double)cb;

	double raw_doppler = c_term + a_term *
		cos(two_pi * (double)antenna_dn / (double)antenna_n + p_term);

	double residual_range_freq = residual_delay_error * chirp_rate;
	double xmit_freq = raw_doppler - residual_range_freq;

	// negative sign converts doppler to additive xmit freq
	*doppler = -xmit_freq;
	return(1);
}

//-----------------------------------//
// DopplerTracker::QuantizeFrequency //
//-----------------------------------//

float
DopplerTracker::QuantizeFrequency(
	float		frequency)
{
	// must do it this way because Doppler frequency is signed!
	double doppler_dn = rint(frequency / DOPPLER_TRACKING_RESOLUTION);

	// negative sign to convert "true" Doppler to additive xmit freq
	float qdoppler = (float)doppler_dn * DOPPLER_TRACKING_RESOLUTION;

	return(qdoppler);
}

//---------------------//
// DopplerTracker::Set //
//---------------------//

int
DopplerTracker::Set(
	double**	terms)
{
	double mins[3];
	double maxs[3];

	//-----------------------------------------//
	// calculate the minimum and maximum terms //
	//-----------------------------------------//

	for (int term_idx = 0; term_idx < 3; term_idx++)
	{
		mins[term_idx] = *(*(terms + 0) + term_idx);
		maxs[term_idx] = mins[term_idx];
	}

	for (unsigned int orbit_step = 0; orbit_step < _dopplerSteps;
		orbit_step++)
	{
		for (int term_idx = 0; term_idx < 3; term_idx++)
		{
			double value = *(*(terms + orbit_step) + term_idx);

			if (value < mins[term_idx])
				mins[term_idx] = value;

			if (value > maxs[term_idx])
				maxs[term_idx] = value;
		}
	}

	//------------------------//
	// generate scale factors //
	//------------------------//

	for (int term_idx = 0; term_idx < 3; term_idx++)
	{
		*(*(_scale + term_idx) + 0) = mins[term_idx];

		*(*(_scale + term_idx) + 1) = (maxs[term_idx] - mins[term_idx]) /
			65535.0;
	}

	//------------------------//
	// calculate scaled terms //
	//------------------------//

	for (unsigned int orbit_step = 0; orbit_step < _dopplerSteps;
		orbit_step++)
	{
		for (int term_idx = 0; term_idx < 3; term_idx++)
		{
			*(*(_term + orbit_step) + term_idx) =
				(unsigned short)( (*(*(terms + orbit_step) + term_idx) -
				*(*(_scale + term_idx) + 0)) / *(*(_scale + term_idx) + 1) +
				0.5);
		}
	}

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

	if (fwrite((void *)&_dopplerSteps, sizeof(unsigned int), 1, fp) != 1)
	{
		fclose(fp);
		return(0);
	}

	//-----------------------//
	// write the scale terms //
	//-----------------------//

	for (unsigned int term = 0; term < 3; term++)
	{
		if (fwrite((void *) *(_scale + term), sizeof(float), 2, fp) != 2)
		{
			return(0);
		}
	}

	//-----------------//
	// write the terms //
	//-----------------//

	for (unsigned int step = 0; step < _dopplerSteps; step++)
	{
		if (fwrite((void *) *(_term + step), sizeof(unsigned short),
			3, fp) != 3)
		{
			return(0);
		}
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

	if (fread((void *)&_dopplerSteps, sizeof(unsigned int), 1, fp) != 1)
	{
		fclose(fp);
		return(0);
	}

	//----------//
	// allocate //
	//----------//

	if (! Allocate(_dopplerSteps))
	{
		fclose(fp);
		return(0);
	}

	//----------------------//
	// read the scale terms //
	//----------------------//

	for (unsigned int term = 0; term < 3; term++)
	{
		if (fread((void *) *(_scale + term), sizeof(float), 2, fp) != 2)
		{
			return(0);
		}
	}

	//----------------//
	// read the terms //
	//----------------//

	for (unsigned int step = 0; step < _dopplerSteps; step++)
	{
		if (fread((void *) *(_term + step), sizeof(unsigned short),
			3, fp) != 3)
		{
			return(0);
		}
	}

	//----------------//
	// close the file //
	//----------------//

	fclose(fp);
	return(1);
}

//--------------------------//
// DopplerTracker::WriteHex //
//--------------------------//

int
DopplerTracker::WriteHex(
	const char*		filename)
{
	//---------------//
	// open the file //
	//---------------//

	FILE* fp = fopen(filename, "w");
	if (fp == NULL)
		return(0);

/*
	//--------------//
	// write header //
	//--------------//

	write_hex(fp, &_tableId, sizeof(_tableId));
	unsigned short file_size = _dopplerSteps * sizeof(unsigned short) +
		6 * sizeof(float) + 2 * sizeof(unsigned short) + 

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
*/

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

//-----------//
// write_hex //
//-----------//

int
write_hex(
	FILE*				fp,
	unsigned short*		buffer,
	int					words)
{
	for (int i = 0; i < words; i++)
	{
		fprintf(fp, "%hx\n", *(buffer + i));
	}
	return(1);
}
