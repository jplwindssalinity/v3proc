//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L20FRAME_H
#define L20FRAME_H

static const char rcs_id_l20frame_h[] =
	"@(#) $Id$";

#include "Wind.h"


//======================================================================
// CLASSES
//		L20Header
//		L20Frame
//======================================================================

//======================================================================
// CLASS
//		L20Header
//
// DESCRIPTION
//		The L20Header object contains the contents of a Level 1.5
//		header.
//======================================================================

class L20Header
{
public:

	//--------------//
	// construction //
	//--------------//

	L20Header();
	~L20Header();

	//--------------//
	// input/output //
	//--------------//

	int		Read(FILE* fp);
	int		Write(FILE*	fp);

	//-----------//
	// variables //
	//-----------//

	float	crossTrackResolution;
	float	alongTrackResolution;
	int		zeroIndex;
};

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

	WindSwath	swath;
};

#endif
