//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef L1BFRAME_H
#define L1BFRAME_H

static const char rcs_id_l1bframe_h[] =
	"@(#) $Id$";

#include "Meas.h"


//======================================================================
// CLASSES
//		L1BFrame
//======================================================================

//======================================================================
// CLASS
//		L1BFrame
//
// DESCRIPTION
//		The L1BFrame object contains the contents of a Level 1B frame
//		as a structure.
//======================================================================

class L1BFrame
{
public:

	//--------------//
	// construction //
	//--------------//

	L1BFrame();
	~L1BFrame();

	//-------------------//
	// data manipulation //
	//-------------------//

	//-------------------//
	// product variables //
	//-------------------//

	MeasSpotList	spotList;	// a list of spots from a single frame
};

#endif
