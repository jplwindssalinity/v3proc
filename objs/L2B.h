//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef L2B_H
#define L2B_H

static const char rcs_id_l2b_h[] =
	"@(#) $Id$";

#include "BaseFile.h"
#include "L2BFrame.h"


//======================================================================
// CLASSES
//		L2B
//======================================================================

//======================================================================
// CLASS
//		L2B
//
// DESCRIPTION
//		The L2B object allows for the easy writing, reading, and
//		manipulating of Level 2B data.
//======================================================================

class L2B : public BaseFile
{
public:

	//------//
	// enum //
	//------//

	enum StatusE { OK, ERROR_READING_FRAME, ERROR_UNKNOWN };

	//--------------//
	// construction //
	//--------------//

	L2B();
	~L2B();

	//---------------------//
	// setting and getting //
	//---------------------//

	StatusE		GetStatus() { return(_status); };

	//--------------//
	// input/output //
	//--------------//

	int		ReadHeader() { return(header.Read(_fp)); };
	int		WriteHeader() { return(header.Write(_fp)); };

	int		ReadDataRec() { return(frame.swath.ReadL2B(_fp)); };
	int		WriteDataRec() { return(frame.swath.WriteL2B(_fp)); };

	int		WriteVctr(const char* filename, const int rank);

	//-----------//
	// variables //
	//-----------//

	L2BHeader		header;
	L2BFrame		frame;

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
