//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef L1ATOL1B_H
#define L1ATOL1B_H

static const char rcs_id_l1atol1b_h[] =
	"@(#) $Id$";

#include "L1A.h"
#include "L1B.h"
#include "Spacecraft.h"
#include "Instrument.h"
#include "LandMap.h"
#include "Ephemeris.h"
#include "XTable.h"
#include "BYUXTable.h"


//======================================================================
// CLASSES
//		L1AToL1B
//======================================================================

//======================================================================
// CLASS
//		L1AToL1B
//
// DESCRIPTION
//		The L1AToL1B object is used to convert between Level 1A data
//		and Level 1B data.  It performs all of the engineering unit
//		conversions.
//======================================================================

class L1AToL1B
{
public:

	//--------------//
	// construction //
	//--------------//

	L1AToL1B();
	~L1AToL1B();

	//------------//
	// conversion //
	//------------//

	int		Convert(L1A* l1a, Spacecraft* spacecraft, Instrument* instrument,
				Ephemeris* ephemeris, L1B* l1b);

	//-----------//
	// variables //
	//-----------//

	XTable	kfactorTable;
	XTable  xTable;
        BYUXTable  BYUX;
        LandMap landMap;

	int		useKfactor;		 // read and use kfactor table
	int		useBYUXfactor;		 // read and use Xfactor table
	int		useSpotCompositing;		// make spots by compositing slices
	int		outputSigma0ToStdout;	// output s0 values to stdout
	float	sliceGainThreshold;		// use to decide which slices to process
	int		processMaxSlices;		// maximum number of slices/spot to use
	char*	simVs1BCheckfile;		// holds cross check data
};

#endif
