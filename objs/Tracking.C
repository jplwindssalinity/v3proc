//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_tracking_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include "Tracking.h"
#include "Array.h"
#include "Instrument.h"

//--------//
// Cosine //
//--------//

float
Cosine(
	float	angle)
{
	//------------------//
	// the cosine table //
	//------------------//

	static const int _cosTableSize = 256;
	static const float _cosTable[] = {
		1.000000, 0.999699, 0.998795, 0.997290, 0.995185, 0.992480,
		0.989177, 0.985278, 0.980785, 0.975702, 0.970031, 0.963776,
		0.956940, 0.949528, 0.941544, 0.932993, 0.923880, 0.914210,
		0.903989, 0.893224, 0.881921, 0.870087, 0.857729, 0.844854,
		0.831470, 0.817585, 0.803207, 0.788346, 0.773010, 0.757209,
		0.740951, 0.724247, 0.707107, 0.689540, 0.671559, 0.653173,
		0.634393, 0.615231, 0.595699, 0.575808, 0.555570, 0.534997,
		0.514103, 0.492898, 0.471397, 0.449611, 0.427555, 0.405241,
		0.382684, 0.359895, 0.336890, 0.313682, 0.290285, 0.266713,
		0.242981, 0.219102, 0.195091, 0.170962, 0.146731, 0.122411,
		0.098018, 0.073565, 0.049068, 0.024542, 0.000001, -0.024540,
		-0.049067, -0.073564, -0.098016, -0.122410, -0.146729, -0.170961,
		-0.195089, -0.219100, -0.242979, -0.266711, -0.290283, -0.313680,
		-0.336888, -0.359894, -0.382682, -0.405240, -0.427554, -0.449610,
		-0.471395, -0.492897, -0.514102, -0.534997, -0.555569, -0.575807,
		-0.595698, -0.615231, -0.634392, -0.653172, -0.671558, -0.689540,
		-0.707106, -0.724247, -0.740951, -0.757209, -0.773010, -0.788346,
		-0.803207, -0.817585, -0.831470, -0.844854, -0.857729, -0.870087,
		-0.881921, -0.893224, -0.903989, -0.914210, -0.923880, -0.932993,
		-0.941544, -0.949528, -0.956941, -0.963776, -0.970031, -0.975702,
		-0.980785, -0.985278, -0.989177, -0.992480, -0.995185, -0.997291,
		-0.998796, -0.999699, -1.000000, -0.999699, -0.998795, -0.997290,
		-0.995185, -0.992479, -0.989176, -0.985277, -0.980785, -0.975702,
		-0.970031, -0.963775, -0.956940, -0.949527, -0.941543, -0.932992,
		-0.923879, -0.914209, -0.903988, -0.893223, -0.881920, -0.870086,
		-0.857727, -0.844852, -0.831468, -0.817583, -0.803206, -0.788344,
		-0.773008, -0.757207, -0.740949, -0.724245, -0.707104, -0.689538,
		-0.671556, -0.653170, -0.634390, -0.615229, -0.595696, -0.575805,
		-0.555567, -0.534994, -0.514099, -0.492894, -0.471393, -0.449607,
		-0.427551, -0.405237, -0.382679, -0.359891, -0.336885, -0.313677,
		-0.290280, -0.266708, -0.242975, -0.219096, -0.195085, -0.170957,
		-0.146725, -0.122405, -0.098012, -0.073559, -0.049062, -0.024536,
		0.000006, 0.024547, 0.049074, 0.073570, 0.098023, 0.122417,
		0.146737, 0.170968, 0.195096, 0.219107, 0.242986, 0.266719,
		0.290291, 0.313688, 0.336896, 0.359901, 0.382690, 0.405248,
		0.427561, 0.449618, 0.471403, 0.492904, 0.514109, 0.535004,
		0.555576, 0.575814, 0.595705, 0.615238, 0.634399, 0.653179,
		0.671565, 0.689546, 0.707112, 0.724253, 0.740957, 0.757214,
		0.773016, 0.788352, 0.803213, 0.817590, 0.831474, 0.844858,
		0.857733, 0.870091, 0.881925, 0.893228, 0.903993, 0.914213,
		0.923883, 0.932996, 0.941547, 0.949531, 0.956943, 0.963779,
		0.970034, 0.975704, 0.980787, 0.985279, 0.989178, 0.992481,
		0.995186, 0.997291, 0.998796, 0.999699 };

	//-----------------------------//
	// make sure angle is positive //
	//-----------------------------//

	if (angle < 0.0)
	{
		int times = (int)(-angle / two_pi) + 1;
		angle += (float)times * two_pi;
	}

	//---------------------------------//
	// convert angle to floating index //
	//---------------------------------//

	float f_idx = angle * 256.0 / two_pi;
	unsigned int idx = (int)f_idx;

	//-----------------------//
	// get bracketing values //
	//-----------------------//

	float cos_1 = _cosTable[idx % _cosTableSize];
	float cos_2 = _cosTable[(idx + 1) % _cosTableSize];

	//-------------//
	// interpolate //
	//-------------//

	float cos_3 = cos_1 + (f_idx - (float)idx) * (cos_2 - cos_1);
	return(cos_3);
}

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

	float table_delay = c_term + a_term * Cosine((two_pi/(double)antenna_n) *
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
		(unsigned int)(width / RX_GATE_WIDTH_RESOLUTION + 0.5);
	float qwidth = (float)width_dn * RX_GATE_WIDTH_RESOLUTION;
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
		(unsigned int)(delay / RX_GATE_DELAY_RESOLUTION + 0.5);
	float qdelay = (float)delay_dn * RX_GATE_DELAY_RESOLUTION;
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
		instrument->orbitTicksPerOrbit);
	unsigned int encoder = antenna->GetEncoderValue();
	unsigned int encoder_n = antenna->GetEncoderN();
 
	float delay;
 
	if (! beam->rangeTracker.GetRxGateDelay(range_step, beam->txPulseWidth,
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
		*(*(_scale + term_idx) + 0) = mins[term_idx];

		*(*(_scale + term_idx) + 1) = (maxs[term_idx] - mins[term_idx]) /
			255.0;
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
				(unsigned char)(( *(*(terms + range_step) +
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
		Cosine(two_pi * (double)antenna_dn / (double)antenna_n + p_term);

	double residual_range_freq = residual_delay_error * chirp_rate;
	double xmit_freq = raw_doppler + residual_range_freq;

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

//-------------------------------//
// DopplerTracker::SetInstrument //
//-------------------------------//

int
DopplerTracker::SetInstrument(
	Instrument*		instrument,
	float			residual_delay)
{
	Antenna* antenna = &(instrument->antenna);
	Beam* beam = antenna->GetCurrentBeam();

	unsigned short doppler_step =
		beam->dopplerTracker.OrbitTicksToDopplerStep(instrument->orbitTicks,
		instrument->orbitTicksPerOrbit);
	unsigned int encoder = antenna->GetEncoderValue();
	unsigned int encoder_n = antenna->GetEncoderN();
 
	float doppler;
 
	if (! beam->dopplerTracker.GetCommandedDoppler(doppler_step, encoder,
		encoder_n, &doppler, instrument->chirpRate, residual_delay))
	{
		fprintf(stderr, "SetInstrument: error using DTC\n");
		return(0);
	}
	float dopcom = beam->dopplerTracker.QuantizeFrequency(doppler);
	instrument->SetCommandedDoppler(dopcom);

	return(1);
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
