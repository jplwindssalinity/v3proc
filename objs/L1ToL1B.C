//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l1tol15_c[] =
	"@(#) $Id$";

#include "L1ToL15.h"

//=========//
// L1ToL15 //
//=========//

L1ToL15::L1ToL15()
{
	return;
}

L1ToL15::~L1ToL15()
{
	return;
}

//------------------//
// L1ToL15::Convert //
//------------------//

int
L1ToL15::Convert(
	L1*		l1,
	L15*	l15)
{
	l15->time = l1->time;
	l15->gcAltitude = l1->gcAltitude;
	l15->gcLongitude = l1->gcLongitude;
	l15->gcLatitude = l1->gcLatitude;
	l15->gcX = l1->gcX;
	l15->gcY = l1->gcY;
	l15->gcZ = l1->gcZ;
	l15->velX = l1->velX;
	l15->velY = l1->velY;
	l15->velZ = l1->velZ;
	l15->antennaPosition = l1->antennaPosition;
	l15->beam = (L15::L15BeamE)l1->beam;
	l15->sigma_0 = l1->sigma_0;

	return(1);
}
