//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef INSTRUMENT_H
#define INSTRUMENT_H

static const char rcs_id_instrument_h[] =
	"@(#) $Id$";

#include "Antenna.h"

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

#define ORBIT_TICKS_PER_SECOND			32
#define INSTRUMENT_TICKS_PER_SECOND		32

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
        double                  GetEqxTime(){return(_eqxTime);}
	float		GetTotalSignalBandwidth();
	int			GetTotalSliceCount();
	int			GetSliceFreqBw(int slice_idx, float* f1, float* bw);

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
	unsigned int	orbitTicksPerPeriod;	// ticks per one orbit
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

	int				useKpc;		// 0 - no kpc, 1 - with kpc
	int				useKpm;		// 0 - no kpm, 1 - with kpm

protected:
	double			_eqxTime;
};

#endif



