//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L10_H
#define L10_H

static const char rcs_id_l10_h[] =
	"@(#) $Id$";

#include "GenericFile.h"
#include "L10Frame.h"


//======================================================================
// CLASSES
//		L10
//======================================================================

//======================================================================
// CLASS
//		L10
//
// DESCRIPTION
//		The L10 object allows for the easy writing, reading, and
//		manipulating of Level 0.0 data.
//======================================================================

class L10
{
public:

	//--------------//
	// construction //
	//--------------//

	L10();
	~L10();

	//---------------------//
	// setting and getting //
	//---------------------//

	int		SetFilename(const char* filename);

	//--------------//
	// input/output //
	//--------------//

	int		WriteDataRec();

	//-----------//
	// variables //
	//-----------//

	GenericFile		file;
	char			buffer[L10_FRAME_SIZE];
	L10Frame		frame;
};

#endif
