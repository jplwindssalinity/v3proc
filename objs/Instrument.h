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

	InstrumentEvent::InstrumentEvent();
	InstrumentEvent::~InstrumentEvent();

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

	//-----------//
	// variables //
	//-----------//

	double		time;
	Antenna		antenna;

	//------------------//
	// generally varied //
	//------------------//

	float		commandedDoppler;	// kHz

	//-----------------//
	// generally fixed //
	//-----------------//

	float		chirpRate;			// kHz/ms
	float		chirpStartM;		// kHz/ms
	float		chirpStartB;		// kHz
	float		systemDelay;		// ms
	float		receiveGateDelay;	// ms
	float		baseTransmitFreq;	// GHz
	float		sliceBandwidth;		// kHz
};

#endif
