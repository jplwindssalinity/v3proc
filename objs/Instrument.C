//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_instrumentsim_c[] =
	"@(#) $Id$";

#include "Instrument.h"


//=======//
// Event //
//=======//

Event::Event()
:	eventId(NONE), time(0.0)
{
	return;
}

Event::~Event()
{
	return;
}


//============//
// Instrument //
//============//

Instrument::Instrument()
:	time(0.0)
{
	return;
}

Instrument::~Instrument()
{
	return;
}
