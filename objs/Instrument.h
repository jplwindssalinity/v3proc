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

	//-------//
	// enums //
	//-------//

    enum EventE { NONE, UNKNOWN, UPDATE_ORBIT,
        SCATTEROMETER_BEAM_A_MEASUREMENT, SCATTEROMETER_BEAM_B_MEASUREMENT };

	//--------------//
	// construction //
	//--------------//

	Event::Event();
	Event::~Event();

	//-----------//
	// variables //
	//-----------//
 
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

	Antenna		antenna;

	Event		event;
};

#endif
