//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

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
:	time(0.0), commandedDoppler(0.0), chirpRate(0.0), chirpStartM(0.0),
	chirpStartB(0.0), systemDelay(0.0), receiveGateDelay(0.0),
	baseTransmitFreq(0.0), sliceBandwidth(0.0)
{
	return;
}

Instrument::~Instrument()
{
	return;
}
