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

	//-----------//
	// variables //
	//-----------//


	double		time;
	Antenna		antenna;

	//------------------//
	// generally varied //
	//------------------//

	unsigned short	orbitTime;			// 32 Hz ticks
	float			commandedDoppler;	// Hz
	float			receiverGateDelay;	// sec

	//-------------------------------------//
	// affected by noise, temperature etc. //
	//-------------------------------------//

	float		systemLoss;			// dimensionless multiplicative factor
	float		transmitPower;		// Watts
	float		receiverGain;		// dimensionless multiplicative factor

	//-----------------//
	// generally fixed //
	//-----------------//

	float		chirpRate;			// Hz/sec
	float		chirpStartM;		// Hz/sec
	float		chirpStartB;		// Hz
	float		systemDelay;		// sec
	float		systemTemperature;	// K
	float		xmitPulsewidth;		// sec
	float		receiverGateWidth;	// sec
	float		baseTransmitFreq;	// Hz
	float		sliceBandwidth;		// Hz
	float		noiseBandwidth;		// Hz
	float		signalBandwidth;	// Hz
	int			useKpc;				// flag, 0 - no kpc, 1 - with kpc

protected:
	double		_eqxTime;
};

#endif
