//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_instrumentsim_c[] =
	"@(#) $Id$";

#include "Instrument.h"


//=================//
// InstrumentEvent //
//=================//

InstrumentEvent::InstrumentEvent()
:	time(0.0), eventId(NONE), beamIdx(0)
{
	return;
}

InstrumentEvent::~InstrumentEvent()
{
	return;
}


//============//
// Instrument //
//============//

Instrument::Instrument()
:	time(0.0), instrumentTicks(0), orbitTicks(0), commandedDoppler(0.0),
	commandedRxGateDelay(0.0), commandedRxGateWidth(0.0), systemLoss(0.0),
	transmitPower(0.0), echo_receiverGain(0.0), noise_receiverGain(0.0),
	orbitTicksPerOrbit(0), chirpRate(0.0), chirpStartM(0.0), chirpStartB(0.0),
	systemTemperature(0.0), baseTransmitFreq(0.0), transmitFreq(0.0),
	scienceSliceBandwidth(0.0), scienceSlicesPerSpot(0),
	guardSliceBandwidth(0.0), guardSlicesPerSide(0), noiseBandwidth(0.0),
	simKpcFlag(1), simKpmFlag(1), simKpriFlag(1), _eqxTime(0)
{
	return;
}

Instrument::~Instrument()
{
	return;
}

//---------------------//
// Instrument::SetTime //
//---------------------//

int
Instrument::SetTime(
	double		new_time)
{
	time = new_time;
	double time_since_eqx = time - _eqxTime;
	orbitTicks = TimeToOrbitTicks(time_since_eqx);
	instrumentTicks = (unsigned int)(new_time * INSTRUMENT_TICKS_PER_SECOND);
	return(1);
}

//---------------------------------//
// Instrument::SetCommandedDoppler //
//---------------------------------//

int
Instrument::SetCommandedDoppler(
	float		commanded_doppler)
{
	commandedDoppler = commanded_doppler;
	transmitFreq = baseTransmitFreq + commandedDoppler;
	return(1);
}

//------------------------//
// Instrument::SetEqxTime //
//------------------------//

int
Instrument::SetEqxTime(
	double	eqx_time)
{
	_eqxTime = eqx_time;
	return(1);
}

//-------------------------------------//
// Instrument::GetTotalSignalBandwidth //
//-------------------------------------//

float
Instrument::GetTotalSignalBandwidth()
{
	float bw = (float)scienceSlicesPerSpot * scienceSliceBandwidth +
		(float)(guardSlicesPerSide * 2) * guardSliceBandwidth;
	return(bw);
}

//--------------------------------//
// Instrument::GetTotalSliceCount //
//--------------------------------//

int
Instrument::GetTotalSliceCount()
{
	int slice_count = scienceSlicesPerSpot + 2 * guardSlicesPerSide;
	return(slice_count);
}

//----------------------------//
// Instrument::GetSliceFreqBw //
//----------------------------//

int
Instrument::GetSliceFreqBw(
	int		slice_idx,
	float*	f1,
	float*	bw)
{
	//----------------------------//
	// determine slice zero index //
	//----------------------------//

	float center_idx = ((float)GetTotalSliceCount() - 1.0) / 2.0;
	float zidx = (float)slice_idx - center_idx;		// zeroed index

	//-----------------------------------------------------------//
	// determine science index, guard index, and slice bandwidth //
	//-----------------------------------------------------------//

	float half_sci = (float)scienceSlicesPerSpot / 2.0;
	if (zidx < -half_sci)
	{
		// lower guard slice
		*f1 = -half_sci * scienceSliceBandwidth +
			(zidx + half_sci - 0.5) * guardSliceBandwidth;
		*bw = guardSliceBandwidth;
	}
	else if (zidx > half_sci)
	{
		// in upper guard slice
		*f1 = half_sci * scienceSliceBandwidth +
			(zidx - half_sci - 0.5) * guardSliceBandwidth;
		*bw = guardSliceBandwidth;
	}
	else
	{
		// guard slices
		*f1 = (zidx - 0.5) * scienceSliceBandwidth;
		*bw = scienceSliceBandwidth;
	}

	return(1);
}

//---------------------------//
// Instrument::OrbitFraction //
//---------------------------//

double
Instrument::OrbitFraction()
{
	double frac = (double)(orbitTicks % orbitTicksPerOrbit) /
		(double)orbitTicksPerOrbit;
	return(frac);
}

//------------------------------//
// Instrument::TimeToOrbitTicks //
//------------------------------//

unsigned int
Instrument::TimeToOrbitTicks(
	double		time)
{
	unsigned int ticks = (unsigned int)(time * ORBIT_TICKS_PER_SECOND);
	return(ticks);
}

//----------------------------------------//
// Instrument::SetTimeWithInstrumentTicks //
//----------------------------------------//

int
Instrument::SetTimeWithInstrumentTicks(
	unsigned int	ticks)
{
	instrumentTicks = ticks;
	time = ((double)ticks + 0.5) / INSTRUMENT_TICKS_PER_SECOND;
	return(1);
}
