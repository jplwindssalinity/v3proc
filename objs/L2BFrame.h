//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L20FRAME_H
#define L20FRAME_H

static const char rcs_id_l20frame_h[] =
	"@(#) $Id$";

#include "WVC.h"


//======================================================================
// CLASSES
//		L20Frame
//======================================================================

//======================================================================
// CLASS
//		L20Frame
//
// DESCRIPTION
//		The L20Frame object contains the contents of a Level 1.5 frame
//		as a structure.
//======================================================================

class L20Frame
{
public:

	//--------------//
	// construction //
	//--------------//

	L20Frame();
	~L20Frame();

	//-------------------//
	// data manipulation //
	//-------------------//

	//-------------------//
	// product variables //
	//-------------------//

	WVC		wvc;
};

#endif
