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
:	initSpdStep(0.0), initPhiStep(0.0), finalSpdStep(0.0), finalPhiStep(0.0)
{
	return;
}

L17ToL20::~L17ToL20()
{
	return;
}

//---------------------------//
// L17ToL20::ConvertAndWrite //
//---------------------------//

int
L17ToL20::ConvertAndWrite(
	L17*	l17,
	GMF*	gmf,
	L20*	l20)
{
	static int last_rev_number = 0;

	//---------------//
	// retrieve wind //
	//---------------//

	WVC* wvc = new WVC();
	int count = gmf->FindSolutions(&(l17->frame.measList), wvc,
		initSpdStep, initPhiStep);
	gmf->RefineSolutions(&(l17->frame.measList), wvc, initSpdStep,
		initPhiStep, finalSpdStep, finalPhiStep);
	wvc->RemoveDuplicates();
	wvc->SortByObj();

	//-------------------------//
	// determine grid indicies //
	//-------------------------//

	int rev = (int)l17->frame.rev;
	int cti = (int)l17->frame.cti;
	int ati = (int)l17->frame.ati;

	//------------------------------//
	// determine if rev is complete //
	//------------------------------//

	if (rev != last_rev_number && last_rev_number)
		Flush(l20);	// process and write

	//-------------------//
	// add to wind swath //
	//-------------------//

	l20->frame.swath.Add(ati, cti, wvc);

	return(1);
}

//-------//
// Flush //
//-------//

int
L17ToL20::Flush(
	L20*	l20)
{
	// median filter
	l20->frame.swath.MedianFilter(l20->medianFilterWindowSize,
		l20->medianFilterMaxPasses);
	if (! l20->WriteDataRec())
		return(0);
	l20->frame.swath.DeleteWVCs();
	return(1);
}
