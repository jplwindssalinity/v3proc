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

	//------//
	// enum //
	//------//

	enum StatusE { OK, ERROR_READING_FRAME, ERROR_UNKNOWN };

	//--------------//
	// construction //
	//--------------//

	L00();
	~L00();

	int		AllocateBuffer(int spots_per_frame, int slices_per_spot);
	int		DeallocateBuffer();

	//---------------------//
	// setting and getting //
	//---------------------//

	int			SetFilename(const char* filename);
	StatusE		GetStatus() { return(_status); };

	//--------------//
	// input/output //
	//--------------//

	int		ReadDataRec();

	//-----------//
	// variables //
	//-----------//

	GenericFile		file;
	char*			buffer;
	int				bufferSize;
	L00Frame		frame;

protected:

	//-----------//
	// variables //
	//-----------//

	StatusE		_status;
};

#endif
