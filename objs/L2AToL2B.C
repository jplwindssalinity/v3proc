//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l17tol20_c[] =
	"@(#) $Id$";

#include "L17ToL20.h"


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
	//---------------//
	// retrieve wind //
	//---------------//

	WVC* wvc = new WVC();
	gmf->FindSolutions(&(l17->frame.measList), wvc, INIT_SPD, INIT_PHI);
	gmf->RefineSolutions(&(l17->frame.measList), wvc, INIT_SPD, INIT_PHI,
		FINAL_SPD, FINAL_PHI);
	wvc->RemoveDuplicates();
	wvc->SortByObj();

/*
	//-------------------------//
	// determine grid indicies //
	//-------------------------//

	int ati = l17->frame.ati;
	int cti = l17->frame.cti;

	//-------------------//
	// add to wind swath //
	//-------------------//

	l20->frame.swath.Add(ati, cti, wvc);
*/

	return(1);
}
