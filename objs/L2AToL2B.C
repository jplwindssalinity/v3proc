//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l17tol20_c[] =
	"@(#) $Id$";

#include "L17ToL20.h"
/*
#include "Antenna.h"
#include "Ephemeris.h"
#include "InstrumentGeom.h"
#include "GenericGeom.h"
*/


//==========//
// L17ToL20 //
//==========//

L17ToL20::L17ToL20()
{
	return;
}

L17ToL20::~L17ToL20()
{
	return;
}

//-------------------//
// L17ToL20::Convert //
//-------------------//

int
L17ToL20::Convert(
	L17*	l17,
	GMF*	gmf,
	L20*	l20)
{
	WVC* wvc = &(l20->frame.wvc);
	wvc->FreeContents();

	gmf->FindSolutions(&(l17->frame.measList), wvc, INIT_SPD, INIT_PHI);
	gmf->RefineSolutions(&(l17->frame.measList), wvc, INIT_SPD, INIT_PHI,
		FINAL_SPD, FINAL_PHI);
	wvc->RemoveDuplicates();

	return(1);
}
