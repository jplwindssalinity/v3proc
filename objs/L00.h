//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L00_H
#define L00_H

static const char rcs_id_l00_h[] =
	"@(#) $Id$";

#include "GenericFile.h"
#include "L00Frame.h"


//======================================================================
// CLASSES
//		L00
//======================================================================

//======================================================================
// CLASS
//		L00
//
// DESCRIPTION
//		The L00 object allows for the easy writing, reading, and
//		manipulating of Level 0.0 data.
//======================================================================

class L00
{
public:

	//--------------//
	// construction //
	//--------------//

	L00();
	~L00();

	//---------------------//
	// setting and getting //
	//---------------------//

	int		SetFilename(const char* filename);

	//-----------//
	// variables //
	//-----------//

	GenericFile		file;
	char			buffer[MAX_L00_FRAME_SIZE];
	L00Frame		frame;
};

#endif
