//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_beam_c[] =
	"@(#) $Id$";

#include "Beam.h"

//======//
// Beam //
//======//

Beam::Beam()
:	lookAngle(0.0), azimuthAngle(0.0), polarization(NONE)
{
	return;
}

Beam::~Beam()
{
	return;
}
