//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_windq_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include "WindQ.h"

//======//
// QWVC //
//======//

QWVC::QWVC()
:   qualityFlag(0)
{
    return;
}

QWVC::~QWVC()
{
    // the WVC destructor is all that is really needed
    return;
}
