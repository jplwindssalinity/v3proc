//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L10_H
#define L10_H

static const char rcs_id_l10_h[] =
	"@(#) $Id$";

#include "BaseFile.h"
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

class L10 : public BaseFile
{
public:

	//------//
	// enum //
	//------//

	enum StatusE { OK, ERROR_READING_FRAME, ERROR_UNKNOWN };

	//--------------//
	// construction //
	//--------------//

	L10();
	~L10();

	int		AllocateBuffer(int number_of_beams, int antenna_cycles_per_frame,
				int slices_per_spot);
	int		DeallocateBuffer();

	//---------------------//
	// setting and getting //
	//---------------------//

	StatusE		GetStatus() { return(_status); };

	//--------------//
	// input/output //
	//--------------//

	int			ReadDataRec();
	int			WriteDataRec();

	//-----------//
	// variables //
	//-----------//

	char*		buffer;
	int			bufferSize;
	L10Frame	frame;

protected:

	//-----------//
	// variables //
	//-----------//

	StatusE		_status;
};

#endif
