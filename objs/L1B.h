//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef L1B_H
#define L1B_H

static const char rcs_id_l1b_h[] =
    "@(#) $Id$";

#include "Meas.h"
#include "BaseFile.h"


//======================================================================
// CLASSES
//    L1BFrame, L1B
//======================================================================

//======================================================================
// CLASS
//    L1BFrame
//
// DESCRIPTION
//    The L1BFrame object contains a L1B frame.
//======================================================================

class L1BFrame
{
public:

    //--------------//
    // construction //
    //--------------//

    L1BFrame();
    ~L1BFrame();

    //-----------//
    // variables //
    //-----------//

    MeasSpotList  spotList;    // a list of spots from a single frame
};


//======================================================================
// CLASS
//    L1B
//
// DESCRIPTION
//    The L1B object allows for the easy writing, reading, and
//    manipulating of Level 1B data.
//======================================================================

class L1B : public BaseFile
{
public:

	//------//
	// enum //
	//------//

	enum StatusE { OK, ERROR_READING_FRAME, ERROR_UNKNOWN };

	//--------------//
	// construction //
	//--------------//

	L1B();
	~L1B();

	//---------------------//
	// setting and getting //
	//---------------------//

	StatusE		GetStatus() { return(_status); };

	//--------------//
	// input/output //
	//--------------//

	int		ReadDataRec() { return(frame.spotList.Read(_inputFp)); };
	int		WriteDataRec() { return(frame.spotList.Write(_outputFp)); };
	int		WriteDataRecAscii();

	//-----------//
	// variables //
	//-----------//

	L1BFrame		frame;

protected:

	//-----------//
	// variables //
	//-----------//

	StatusE		_status;
};

#endif
