//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_antenna_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include "Antenna.h"
#include "Beam.h"

//=========//
// Antenna //
//=========//

Antenna::Antenna()
:	numberOfBeams(0), azimuthAngle(0.0)
{
	return;
}

Antenna::~Antenna()
{
	free(beam);
	return;
}
