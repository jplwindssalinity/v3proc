//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef INSTRUMENT_H
#define INSTRUMENT_H

static const char rcs_id_instrument_h[] =
	"@(#) $Id$";

#include "Antenna.h"
#include "InstConsts.h"

//======================================================================
// CLASSES
//		InstrumentEvent, Instrument
//======================================================================

//======================================================================
// CLASS
//		InstrumentEvent
//
// DESCRIPTION
//		The InstrumentEvent object contains an instrument event time
//		and event ID.
//======================================================================

class InstrumentEvent
{
public:

	//-------//
	// enums //
	//-------//

	enum InstrumentEventE { NONE, SCATTEROMETER_MEASUREMENT };

	//--------------//
	// construction //
	//--------------//

	InstrumentEvent();
	~InstrumentEvent();

	//-----------//
	// variables //
	//-----------//

	double				time;
	InstrumentEventE	eventId;
	int					beamIdx;
};


//======================================================================
// CLASS
//		Instrument
//
// DESCRIPTION
//		The Instrument object contains instrument state information.
//======================================================================

class Instrument
{
public:

	//-------------//
	// contruction //
	//-------------//

	Instrument();
	~Instrument();

	//---------------------//
	// setting and getting //
	//---------------------//

	int			SetTime(double new_time);
	int			SetCommandedDoppler(float commanded_doppler);
	int			SetEqxTime(double eqx_time);
	double		GetEqxTime() { return(_eqxTime); };
	float		GetTotalSignalBandwidth();
	int			GetTotalSliceCount();
	int			GetSliceFreqBw(int slice_idx, float* f1, float* bw);
	double		OrbitFraction();

	//-----------//
	// functions //
	//-----------//

	unsigned int	TimeToOrbitTicks(double time);
	int				SetTimeWithInstrumentTicks(unsigned int ticks);
	int				SetRxGateAndDoppler();

	//-----------//
	// variables //
	//-----------//

	double			time;
	Antenna			antenna;

	//------------------//
	// generally varied //
	//------------------//

	unsigned int	instrumentTicks;		// 32 Hz ticks
	unsigned int	orbitTicks;				// 32 Hz ticks
	float			commandedDoppler;		// Hz freq added to base xmit
	float			commandedRxGateDelay;	// sec
	float			commandedRxGateWidth;	// sec

	//-------------------------------------//
	// affected by noise, temperature etc. //
	//-------------------------------------//

	float		systemLoss;			// dimensionless multiplicative factor
	float		transmitPower;		// Watts
	float		echo_receiverGain;		// dimensionless multiplicative factor
	float		noise_receiverGain;		// dimensionless multiplicative factor

	//-----------------//
	// generally fixed //
	//-----------------//

	unsigned int	orbitTicksPerOrbit;	// ticks per one orbit

	float		chirpRate;				// Hz/sec
	float		chirpStartM;			// Hz/sec
	float		chirpStartB;			// Hz
	float		systemTemperature;		// K
	float		baseTransmitFreq;		// Hz, starting frequency
	float		transmitFreq;			// Hz, actual center transmit frequency

	float		scienceSliceBandwidth;	// Hz
	int			scienceSlicesPerSpot;	// count
	float		guardSliceBandwidth;	// Hz
	int			guardSlicesPerSide;		// count

	float		noiseBandwidth;			// Hz

	//-------//
	// flags //
	//-------//

	int			simKpcFlag;			// 0 = no Kpc, 1 = with Kpc
	int			simCorrKpmFlag;		// 0 = no correlated Kpm, 1 = with Kpm
	int			simUncorrKpmFlag;	// 0 = no uncorrelated Kpm, 1 = with Kpm
	int			simKpriFlag;		// 0 = no Kpri, 1 = with Kpri

	double		corrKpm;		// dB
protected:
	double			_eqxTime;
};

#endif
