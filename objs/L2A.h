//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef L2A_H
#define L2A_H

static const char rcs_id_l2a_h[] =
	"@(#) $Id$";

#include "BaseFile.h"
#include "L2AFrame.h"


//======================================================================
// CLASSES
//		L2A
//======================================================================

//======================================================================
// CLASS
//		L2A
//
// DESCRIPTION
//		The L2A object allows for the easy writing, reading, and
//		manipulating of Level 2A data.
//		Level 2A data consists of spatially co-located measurements as
//		opposed to the time ordered measurements in Level 1B data.
//======================================================================

class L2A : public BaseFile
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

	L2A();
	~L2A();

	//--------------//
	// input/output //
	//--------------//

	int		WriteHeader();
	int		ReadHeader();
	int		WriteDataRec();
	int		ReadDataRec();

	int		ReadGSDataRec();

	//---------------------//
	// setting and getting //
	//---------------------//

	StatusE		GetStatus() { return(_status); };

	//-----------//
	// variables //
	//-----------//

	L2AHeader		header;
	L2AFrame		frame;

protected:

	//-----------//
	// variables //
	//-----------//

	StatusE		_status;
	int			_headerTransferred;
};

#endif
