//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l0tol1_c[] =
	"@(#) $Id$";

#include "L0ToL1.h"

//========//
// L0ToL1 //
//========//

L0ToL1::L0ToL1()
{
	return;
}

L0ToL1::~L0ToL1()
{
	return;
}

//-----------------//
// L0ToL1::Convert //
//-----------------//

int
L0ToL1::Convert(
	L0*		l0,
	L1*		l1)
{
	l1->time = l0->time;
	l1->gcAltitude = l0->gcAltitude;
	l1->gcLongitude = l0->gcLongitude;
	l1->gcLatitude = l0->gcLatitude;
	l1->gcX = l0->gcX;
	l1->gcY = l0->gcY;
	l1->gcZ = l0->gcZ;
	l1->velX = l0->velX;
	l1->velY = l0->velY;
	l1->velZ = l0->velZ;
	l1->antennaPosition = l0->antennaPosition;
	l1->beam = (L1::L1BeamE)l0->beam;
	l1->sigma_0 = l0->sigma_0;

	return(1);
}
