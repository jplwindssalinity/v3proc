//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_attitude_c[] =
	"@(#) $Id$";

#include "Attitude.h"


//==========//
// Attitude //
//==========//

Attitude::Attitude()
:	roll(0.0), pitch(0.0), yaw(0.0)
{
	return;
}

Attitude::~Attitude()
{
	return;
}
