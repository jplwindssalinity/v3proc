//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L15FRAME_H
#define L15FRAME_H

static const char rcs_id_l15frame_h[] =
	"@(#) $Id$";

#include "Meas.h"


//======================================================================
// CLASSES
//		L15Frame
//======================================================================

//======================================================================
// CLASS
//		L15Frame
//
// DESCRIPTION
//		The L15Frame object contains the contents of a Level 1.5 frame
//		as a structure.
//======================================================================

class L15Frame
{
public:

	//--------------//
	// construction //
	//--------------//

	L15Frame();
	~L15Frame();

	//-------------------//
	// data manipulation //
	//-------------------//

	//-------------------//
	// product variables //
	//-------------------//

	MeasSpotList	spotList;	// a list of spots from a single frame
};

#endif
