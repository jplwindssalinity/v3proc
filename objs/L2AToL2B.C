//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l17tol20_c[] =
	"@(#) $Id$";

#include "L17ToL20.h"
#include "Constants.h"


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
	gmf->FindSolutions(&(l17->frame.measList), wvc,
		initSpdStep, initPhiStep);
	gmf->RefineSolutions(&(l17->frame.measList), wvc, initSpdStep,
		initPhiStep, finalSpdStep, finalPhiStep);
	wvc->RemoveDuplicates();
	wvc->SortByObj();
	wvc->lonLat = l17->frame.measList.AverageLonLat();

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

	l20->frame.swath.Add(cti, ati, wvc);

	return(1);
}

//-----------------//
// L17ToL20::Flush //
//-----------------//

int
L17ToL20::Flush(
	L20*	l20)
{
	// median filter
	l20->frame.swath.InitWithRank(1);
	l20->frame.swath.MedianFilter(l20->medianFilterWindowSize,
		l20->medianFilterMaxPasses);
	if (! l20->WriteDataRec())
		return(0);
	l20->frame.swath.DeleteWVCs();
	return(1);
}

//-----------------------------//
// L17ToL20::GenerateModCurves //
//-----------------------------//

int
L17ToL20::GenerateModCurves(
	L17*			l17,
	GMF*			gmf,
	const char*		output_file)
{
	FILE* ofp = fopen(output_file, "w");
	if (ofp == NULL)
		return(0);

	gmf->ModCurves(ofp, &(l17->frame.measList), finalSpdStep, finalPhiStep);

	// show selected vectors
	WVC* wvc = new WVC();
	gmf->FindSolutions(&(l17->frame.measList), wvc,
		initSpdStep, initPhiStep);

	gmf->RefineSolutions(&(l17->frame.measList), wvc, initSpdStep,
		initPhiStep, finalSpdStep, finalPhiStep);
	wvc->RemoveDuplicates();
	wvc->SortByObj();

	for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
		wvp = wvc->ambiguities.GetNext())
	{
		fprintf(ofp, "&\n");
		fprintf(ofp, "%g %g %g\n", wvp->dir * rtd, wvp->spd,
			wvp->obj * 500.0 + 5.0);
	}

	delete wvc;

	fclose(ofp);
	return(1);
}
