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
:	phiStep(0.0), phiBuffer(0.0), spdTolerance(0.0)
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
	if (! gmf->RetrieveWinds(&(l17->frame.measList), wvc, phiStep, phiBuffer,
		phiMaxSmoothing, spdTolerance, DESIRED_SOLUTIONS))
	{
		return(1);
	}
	if (wvc->ambiguities.NodeCount() < 2)
	{
		delete wvc;
		return(1);
	}
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

	if (! l20->frame.swath.Add(cti, ati, wvc))
		return(0);

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

//-------------------------------//
// L17ToL20::WriteSolutionCurves //
//-------------------------------//

int
L17ToL20::WriteSolutionCurves(
	L17*			l17,
	GMF*			gmf,
	const char*		output_file)
{
	//------------------//
	// open output file //
	//------------------//

	FILE* ofp = fopen(output_file, "w");
	if (ofp == NULL)
		return(0);

	//-----------------------//
	// write solution curves //
	//-----------------------//

	gmf->WriteSolutionCurves(ofp, &(l17->frame.measList), phiStep, phiBuffer,
		phiMaxSmoothing, spdTolerance, DESIRED_SOLUTIONS);

	//--------------------//
	// indicate solutions //
	//--------------------//

	WVC* wvc = new WVC();
	gmf->RetrieveWinds(&(l17->frame.measList), wvc, phiStep, phiBuffer,
		phiMaxSmoothing, spdTolerance, DESIRED_SOLUTIONS);

	for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
		wvp = wvc->ambiguities.GetNext())
	{
		fprintf(ofp, "&\n");
		fprintf(ofp, "%g %g\n", wvp->dir * rtd, wvp->spd);
	}

	delete wvc;

	//-------------------//
	// close output file //
	//-------------------//

	fclose(ofp);

	return(1);
}
