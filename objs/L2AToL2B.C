//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l2atol2b_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <string.h>
#include "L2AToL2B.h"
#include "Constants.h"
#include "Misc.h"

//==========//
// L2AToL2B //
//==========//

L2AToL2B::L2AToL2B()
:   medianFilterWindowSize(0), medianFilterMaxPasses(0), maxRankForNudging(0),
    useManyAmbiguities(0), useAmbiguityWeights(0), useNudging(0),
    smartNudgeFlag(0), wrMethod(GS), onePeakWidth(0.0), twoPeakSep(181.0),
    probThreshold(0.0)
{
	return;
}

L2AToL2B::~L2AToL2B()
{
	return;
}

//----------------------------------//
// L2AToL2B::SetWindRetrievalMethod //
//----------------------------------//
// sets the wind retrieval method based on a passed string
// returns 0 on failure
// returns 1 on success

int
L2AToL2B::SetWindRetrievalMethod(
    const char*  wr_method)
{
    if (strcasecmp(wr_method, "GS") == 0)
    {
        wrMethod = GS;
        return(1);
    }
    else if (strcasecmp(wr_method, "GS_FIXED") == 0)
    {
        wrMethod = GS_FIXED;
        return(1);
    }
    else if (strcasecmp(wr_method, "H1") == 0)
    {
        wrMethod = H1;
        return(1);
    }
    else if (strcasecmp(wr_method, "H2") == 0)
    {
        wrMethod = H2;
        return(1);
    }
    else if (strcasecmp(wr_method, "H3") == 0)
    {
        wrMethod = H3;
        return(1);
    }
    else if (strcasecmp(wr_method, "PEAK_SPLITTING") == 0)
    {
        wrMethod = PEAK_SPLITTING;
        return(1);
    }
    else
        return(0);
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

	MeasList* meas_list = &(l2a->frame.measList);

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
    switch (wrMethod)
    {
        case GS:
            if (! gmf->RetrieveWinds_GS(meas_list, kp, wvc))
            {
                delete wvc;
                return(5);
            }
            break;
/*
        case GS_FIXED:
            if (! gmf->RetrieveWinds_GSFixed(meas_list, kp, wvc))
            {
                delete wvc;
                return(6);
            }
            break;
*/
        case H1:
            if (! gmf->RetrieveWinds_H1(meas_list, kp, wvc))
            {
                delete wvc;
                return(7);
            }
            break;
        case H2:
            if (! gmf->RetrieveWinds_H2(meas_list, kp, wvc))
            {
                delete wvc;
                return(7);
            }
            break;
        case H3:
            if (! gmf->RetrieveWinds_H2(meas_list, kp, wvc, 1))
            {
                delete wvc;
                return(8);
            }
            break;
        case PEAK_SPLITTING:
            if (! gmf->RetrieveWindsWithPeakSplitting(meas_list, kp, wvc,
                onePeakWidth, twoPeakSep, probThreshold, DESIRED_SOLUTIONS))
            {
                delete wvc;
                return(9);
            }
            break;
        default:
            return(10);
    }

	if (wvc->ambiguities.NodeCount() == 0)
	{
		delete wvc;
		return(11);
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
	//------------//
	// initialize //
	//------------//

	if (useNudging)
	{
	  if (smartNudgeFlag) 
	    l2b->frame.swath.LoResNudge(&nudgeVctrField, maxRankForNudging);
	  else
            l2b->frame.swath.Nudge(&nudgeField, maxRankForNudging);
	}
	else
	{
		l2b->frame.swath.InitWithRank(1);
	}

	//---------------//
	// median filter //
	//---------------//

	l2b->frame.swath.MedianFilter(medianFilterWindowSize,
		medianFilterMaxPasses, useAmbiguityWeights);

	//------------//
	// output l2b //
	//------------//

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
