//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_measurement_c[] =
	"@(#) $Id$";

#include <unistd.h>
#include "Meas.h"
#include "Beam.h"


//======//
// Meas //
//======//

Meas::Meas()
:	value(0.0), pol(NONE), eastAzimuth(0.0), scAzimuth(0.0),
	incidenceAngle(0.0), estimatedKp(0.0)
{
	return;
}

Meas::~Meas()
{
	return;
}
