//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L15_H
#define L15_H

static const char rcs_id_l15_h[] =
	"@(#) $Id$";

#include "GenericFile.h"
#include "L15Frame.h"


//======================================================================
// CLASSES
//		L15
//======================================================================

//======================================================================
// CLASS
//		L15
//
// DESCRIPTION
//		The L15 object allows for the easy writing, reading, and
//		manipulating of Level 1.5 data.
//======================================================================

class L15
{
public:

	//------//
	// enum //
	//------//

	enum StatusE { OK, ERROR_READING_FRAME, ERROR_UNKNOWN };

	//--------------//
	// construction //
	//--------------//

	L15();
	~L15();

	//---------------------//
	// setting and getting //
	//---------------------//

	int			SetFilename(const char* filename);
	StatusE		GetStatus() { return(_status); };

	//--------------//
	// input/output //
	//--------------//

	int		ReadDataRec();
	int		WriteDataRec();

	//-----------//
	// variables //
	//-----------//

	GenericFile		file;
	L15Frame		frame;

protected:

	//-----------//
	// variables //
	//-----------//

	StatusE		_status;
};

#endif
