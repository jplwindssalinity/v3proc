//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef L1A_H
#define L1A_H

static const char rcs_id_l1a_h[] =
	"@(#) $Id$";

#include "BaseFile.h"
#include "L1AFrame.h"
#include "L1AGSFrame.h"


//======================================================================
// CLASSES
//		L1A
//======================================================================

//======================================================================
// CLASS
//		L1A
//
// DESCRIPTION
//		The L1A object allows for the easy writing, reading, and
//		manipulating of Level 1A data.
//======================================================================

class L1A : public BaseFile
{
public:

	//------//
	// enum //
	//------//

	enum StatusE { OK, ERROR_READING_FRAME, ERROR_UNKNOWN };

	//--------------//
	// construction //
	//--------------//

	L1A();
	~L1A();

	int		AllocateBuffer();
	int		DeallocateBuffer();

	//---------------------//
	// setting and getting //
	//---------------------//

	StatusE		GetStatus() { return(_status); };

	//--------------//
	// input/output //
	//--------------//

    int  ReadDataRec();
    int  WriteDataRec();
    int  WriteDataRecAscii();

    int  WriteGSDataRec(void);

	//-----------//
	// variables //
	//-----------//

	char*		buffer;
	int			bufferSize;
	L1AFrame	frame;

    L1AGSFrame  gsFrame;

protected:

	//-----------//
	// variables //
	//-----------//

	StatusE		_status;
};

#endif
