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
:	numberOfBeams(0), beam(NULL), azimuthAngle(0.0)
{
	return;
}

Antenna::~Antenna()
{
	free(beam);
	return;
}

//---------------------------//
// Antenna::SetNumberOfBeams //
//---------------------------//

int
Antenna::SetNumberOfBeams(
	int		number_of_beams)
{
	beam = (Beam *)malloc(number_of_beams * sizeof(Beam));
	if (beam == NULL)
		return(0);
	numberOfBeams = number_of_beams;
	return(1);
}
