//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_pscat_c[] =
    "@(#) $Id$";

#include "Pscat.h"

//============//
// PscatEvent //
//============//

PscatEvent::PscatEvent()
:   eventTime(0.0), eventId(NONE), beamIdx(0)
{
    return;
}

PscatEvent::~PscatEvent()
{
    return;
}

//=======//
// Pscat //
//=======//

Pscat::Pscat()
{
    return;
}

Pscat::~Pscat()
{
    return;
}
