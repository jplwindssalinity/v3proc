//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L17_H
#define L17_H

static const char rcs_id_l17_h[] =
	"@(#) $Id$";

#include "BaseFile.h"
#include "L17Frame.h"


//======================================================================
// CLASSES
//		L17
//======================================================================

//======================================================================
// CLASS
//		L17
//
// DESCRIPTION
//		The L17 object allows for the easy writing, reading, and
//		manipulating of Level 1.7 data.
//		Level 1.7 data consists of spatially co-located measurements as
//		opposed to the time ordered measurements in level 1.5 data.
//======================================================================

class L17 : public BaseFile
{
public:

	//------//
	// enum //
	//------//

	enum StatusE { OK, ERROR_READING_HEADER, ERROR_READING_FRAME,
		ERROR_UNKNOWN };

	//--------------//
	// construction //
	//--------------//

	L17();
	~L17();

	//--------------//
	// input/output //
	//--------------//

	int		WriteHeader();
	int		ReadHeader();
	int		WriteDataRec();
	int		ReadDataRec();

	//---------------------//
	// setting and getting //
	//---------------------//

	StatusE		GetStatus() { return(_status); };

	//-----------//
	// variables //
	//-----------//

	L17Header		header;
	L17Frame		frame;

protected:

	//-----------//
	// variables //
	//-----------//

	StatusE		_status;
	int			_headerTransferred;
};

#endif
