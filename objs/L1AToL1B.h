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
#include "Ephemeris.h"
#include "XTable.h"


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

	// If this is nonzero the kfactor table is read in and used.
	int		useKfactor;

	// Output sigma0 values to stdout? 1/0=YES/NO (format readable by xmgr)
	int		outputSigma0ToStdout;
};

#endif
