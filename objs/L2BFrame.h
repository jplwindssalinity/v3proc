//===============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//===============================================================//

#ifndef L2BFRAME_H
#define L2BFRAME_H

static const char rcs_id_l2bframe_h[] =
	"@(#) $Id$";

#include "Wind.h"


//======================================================================
// CLASSES
//		L2BHeader
//		L2BFrame
//======================================================================

//======================================================================
// CLASS
//		L2BHeader
//
// DESCRIPTION
//		The L2BHeader object contains the contents of a Level 2B
//		header.
//======================================================================

class L2BHeader
{
public:

	//--------------//
	// construction //
	//--------------//

	L2BHeader();
	~L2BHeader();

	//--------------//
	// input/output //
	//--------------//

	int		Read(FILE* fp);
	int		Write(FILE*	fp);
	int		WriteAscii(FILE*	fp);

	//-----------//
	// variables //
	//-----------//

	float	crossTrackResolution;
	float	alongTrackResolution;
	int		zeroIndex;
};

//======================================================================
// CLASS
//		L2BFrame
//
// DESCRIPTION
//		The L2BFrame object contains the contents of a Level 2B frame
//		as a structure.
//======================================================================

class L2BFrame
{
public:

	//--------------//
	// construction //
	//--------------//

	L2BFrame();
	~L2BFrame();

	//-------------------//
	// data manipulation //
	//-------------------//

	//-------------------//
	// product variables //
	//-------------------//

	WindSwath	swath;
};

#endif
