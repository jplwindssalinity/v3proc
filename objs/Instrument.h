//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef INSTRUMENT_H
#define INSTRUMENT_H

static const char rcs_id_instrument_h[] =
	"@(#) $Id$";

#include "Antenna.h"
#include "Spacecraft.h"

//======================================================================
// CLASSES
//		Event, Instrument
//======================================================================

//======================================================================
// CLASS
//      Event
//
// DESCRIPTION
//      The Event object contains an event time and an event.
//======================================================================
 
class Event
{
public:
    enum EventE { NONE, UNKNOWN, UPDATE_ORBIT,
        SCATTEROMETER_BEAM_A_MEASUREMENT, SCATTEROMETER_BEAM_B_MEASUREMENT };
 
    EventE      eventId;
    double      time;
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

	Spacecraft	spacecraft;
	Antenna		antenna;

	Event		event;
};

#endif
