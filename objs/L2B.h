//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L20_H
#define L20_H

static const char rcs_id_l20_h[] =
	"@(#) $Id$";

#include "GenericFile.h"
#include "L20Frame.h"


//======================================================================
// CLASSES
//		L20
//======================================================================

//======================================================================
// CLASS
//		L20
//
// DESCRIPTION
//		The L20 object allows for the easy writing, reading, and
//		manipulating of Level 2.0 data.
//======================================================================

class L20
{
public:

	//------//
	// enum //
	//------//

	enum StatusE { OK, ERROR_READING_FRAME, ERROR_UNKNOWN };

	//--------------//
	// construction //
	//--------------//

	L20();
	~L20();

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
	int		Close();

	//-----------//
	// variables //
	//-----------//

	GenericFile		file;
	L20Header		header;
	L20Frame		frame;

	//----------------------//
	// processing variables //
	//----------------------//

	int				medianFilterWindowSize;
	int				medianFilterMaxPasses;

protected:

	//-----------//
	// variables //
	//-----------//

	StatusE		_status;
};

#endif
