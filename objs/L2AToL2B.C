//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l17tol20_c[] =
	"@(#) $Id$";

#include "L17ToL20.h"
#include "Constants.h"
#include "Misc.h"

#define AZIMUTH_DIVERSITY		(5.0*dtr)
#define INCIDENCE_DIVERSITY		(5.0*dtr)

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
// returns 0 on failure for bad reason (memory, etc.)
// returns 1 on success
// returns higher numbers for other reasons

int
L17ToL20::ConvertAndWrite(
	L17*	l17,
	GMF*	gmf,
	L20*	l20)
{
	static int last_rev_number = 0;

	//------------------------------//
	// check number of measurements //
	//------------------------------//

	MeasList* meas_list = &(l17->frame.measList);
	if (meas_list->NodeCount() < 2)
	{
		fprintf(stderr, "too few measurements\n");
		return(2);
	}

	//-----------------------------------//
	// check for missing wind field data //
	//-----------------------------------//
	// this should be handled by some kind of a flag!

	int any_zero = 0;
	for (Meas* meas = meas_list->GetHead(); meas; meas = meas_list->GetNext())
	{
		if (! meas->value)
		{
			any_zero = 1;
			break;
		}
	}
	if (any_zero)
	{
		fprintf(stderr, "insuffient wind data\n");
		return(3);
	}

	//---------------//
	// retrieve wind //
	//---------------//

	WVC* wvc = new WVC();
	if (! gmf->RetrieveWinds(meas_list, wvc, phiStep, phiBuffer,
		phiMaxSmoothing, spdTolerance, DESIRED_SOLUTIONS))
	{
		fprintf(stderr, "wind retrieval failed\n");
//		meas_list->WriteAscii(stdout);
		delete wvc;
		return(5);
	}

	if (wvc->ambiguities.NodeCount() < 2)
	{
		fprintf(stderr, "deleting node count %d\n",
			wvc->ambiguities.NodeCount());
		delete wvc;
		return(6);
	}
	wvc->lonLat = meas_list->AverageLonLat();

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
	if (! l20->WriteHeader())
		return(0);
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

	//-------------------//
	// close output file //
	//-------------------//

	fclose(ofp);

	return(1);
}
