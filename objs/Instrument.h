//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef INSTRUMENT_H
#define INSTRUMENT_H

static const char rcs_id_instrument_h[] =
	"@(#) $Id$";

#include "Antenna.h"
#include "Tracking.h"

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

#define ORBIT_TICKS_PER_SECOND	32

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
	int			Eqx(double eqx_time);
	float		GetTotalSignalBandwidth();
	int			GetTotalSliceCount();
	int			GetSliceFreqBw(int slice_idx, float* f1, float* bw);

	//-----------//
	// functions //
	//-----------//

	unsigned int	TimeToOrbitTicks(double time);

	//-----------//
	// variables //
	//-----------//

	double			time;
	Antenna			antenna;
	RangeTracker	rangeTracker;
	DopplerTracker	dopplerTracker;

	//------------------//
	// generally varied //
	//------------------//

	unsigned int	orbitTicks;				// 32 Hz ticks
	float			commandedDoppler;		// Hz freq added to base xmit
	float			receiverGateDelay;		// sec
	float			receiverGateWidth;		// sec

	//-------------------------------------//
	// affected by noise, temperature etc. //
	//-------------------------------------//

	float		systemLoss;			// dimensionless multiplicative factor
	float		transmitPower;		// Watts
	float		receiverGain;		// dimensionless multiplicative factor

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
	unsigned char	useRgc;		// 0 - ideal delay, 1 - use RGC
	unsigned char	useDtc;		// 0 - ideal Doppler, 1 - use DTC

protected:
	double			_eqxTime;
};

#endif
