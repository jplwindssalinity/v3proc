//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef L2ATOL2B_H
#define L2ATOL2B_H

static const char rcs_id_l2atol2b_h[] =
	"@(#) $Id$";

#include "L2A.h"
#include "L2B.h"
#include "GMF.h"

#define DESIRED_SOLUTIONS		4


//======================================================================
// CLASSES
//		L2AToL2B
//======================================================================

//======================================================================
// CLASS
//		L2AToL2B
//
// DESCRIPTION
//		The L2AToL2B object is used to convert between Level 2A data
//		and Level 2B data.  It performs wind retrieval.
//======================================================================

class L2AToL2B
{
public:

	//--------------//
	// construction //
	//--------------//

	L2AToL2B();
	~L2AToL2B();

	//------------//
	// conversion //
	//------------//

	int		ConvertAndWrite(L2A* l2a, GMF* gmf, Kp* kp, L2B* l2b);
	int		Flush(L2B* l2b);

	//-----------//
	// debugging //
	//-----------//

	int		WriteSolutionCurves(L2A* l2a, GMF* gmf, Kp* kp,
				const char* output_file);

	//----------------------//
	// processing variables //
	//----------------------//

	int		medianFilterWindowSize;
	int		medianFilterMaxPasses;

	//-------//
	// flags //
	//-------//

	int		useManyAmbiguities;
	int		useAmbiguityWeights;
	int		usePeakSplitting;
	int		useNudging;

	//-----------------------------------------//
	// Parameters for Peak Splitting Algorithm //
	//-----------------------------------------//

	float	onePeakWidth;
	float	twoPeakSep;
	float	probThreshold;

	//-------------------//
	// nudging variables //
	//-------------------//

	WindField	nudgeField;
};

#endif
