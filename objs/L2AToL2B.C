//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l2atol2b_c[] =
	"@(#) $Id$";

#include "L2AToL2B.h"
#include "Constants.h"
#include "Misc.h"

//==========//
// L2AToL2B //
//==========//

L2AToL2B::L2AToL2B()
:	medianFilterWindowSize(0), medianFilterMaxPasses(0)
{
	return;
}

L2AToL2B::~L2AToL2B()
{
	return;
}

//---------------------------//
// L2AToL2B::ConvertAndWrite //
//---------------------------//
// returns 0 on failure for bad reason (memory, etc.)
// returns 1 on success
// returns higher numbers for other reasons

int
L2AToL2B::ConvertAndWrite(
	L2A*	l2a,
	GMF*	gmf,
	Kp*		kp,
	L2B*	l2b)
{
	static int last_rev_number = 0;

	//------------------------------//
	// check number of measurements //
	//------------------------------//

	MeasList* meas_list = &(l2a->frame.measList);
	if (meas_list->NodeCount() < 2)
	{
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
		return(3);
	}

	//-----------------------------------//
	// check for wind retrieval criteria //
	//-----------------------------------//

	if (! gmf->CheckRetrieveCriteria(meas_list))
	{
		return(4);
	}

	//---------------//
	// retrieve wind //
	//---------------//

	WVC* wvc = new WVC();
	if (! gmf->RetrieveWinds(meas_list, kp, wvc))
	{
		delete wvc;
		return(5);
	}

	if (wvc->ambiguities.NodeCount() < 2)
	{
		delete wvc;
		return(6);
	}
	wvc->lonLat = meas_list->AverageLonLat();

	//-------------------------//
	// determine grid indicies //
	//-------------------------//

	int rev = (int)l2a->frame.rev;
	int cti = (int)l2a->frame.cti;
	int ati = (int)l2a->frame.ati;

	//------------------------------//
	// determine if rev is complete //
	//------------------------------//

	if (rev != last_rev_number && last_rev_number)
		Flush(l2b);	// process and write

	//-------------------//
	// add to wind swath //
	//-------------------//

	if (! l2b->frame.swath.Add(cti, ati, wvc))
		return(0);

	return(1);
}

//-----------------//
// L2AToL2B::Flush //
//-----------------//

int
L2AToL2B::Flush(
	L2B*	l2b)
{
	// median filter
	l2b->frame.swath.InitWithRank(1);
	l2b->frame.swath.MedianFilter(medianFilterWindowSize,
		medianFilterMaxPasses);
	if (! l2b->WriteHeader())
		return(0);
	if (! l2b->WriteDataRec())
		return(0);
	l2b->frame.swath.DeleteWVCs();
	return(1);
}

//-------------------------------//
// L2AToL2B::WriteSolutionCurves //
//-------------------------------//

int
L2AToL2B::WriteSolutionCurves(
	L2A*			l2a,
	GMF*			gmf,
	Kp*				kp,
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

	gmf->WriteSolutionCurves(ofp, &(l2a->frame.measList), kp);

	//-------------------//
	// close output file //
	//-------------------//

	fclose(ofp);

	return(1);
}
