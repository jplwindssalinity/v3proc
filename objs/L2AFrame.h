//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L17FRAME_H
#define L17FRAME_H

static const char rcs_id_l17frame_h[] =
	"@(#) $Id$";

#include "Meas.h"


//======================================================================
// CLASSES
//		L17Frame
//======================================================================

//======================================================================
// CLASS
//		L17Frame
//
// DESCRIPTION
//		The L17Frame object contains the contents of a Level 1.7 frame
//		as a structure.
//======================================================================

class L17Frame
{
public:

	//--------------//
	// construction //
	//--------------//

	L17Frame();
	~L17Frame();

	//-------------------//
	// product variables //
	//-------------------//

	MeasList	measList;	// a list of measurements from a single frame
};

#endif
