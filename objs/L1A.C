//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l10_c[] =
	"@(#) $Id$";

#include <memory.h>
#include "L10.h"


//=====//
// L10 //
//=====//

L10::L10()
{
	return;
}

L10::~L10()
{
	return;
}

//------------------//
// L10::SetFilename //
//------------------//

int
L10::SetFilename(
	const char*		filename)
{
	return(file.SetFilename(filename));
}

//------------------//
// L10::ReadDataRec //
//------------------//

int
L10::ReadDataRec()
{
	if (! file.Read(buffer, L10_FRAME_SIZE))
	{
		if (file.EndOfFile())
		{
			// end of file, leave status alone (typically status is OK)
			return(0);
		}
		else
		{
			// an error occurred
			_status = ERROR_READING_FRAME;
			return(0);
		}
	}
	return(1);
}

//-------------------//
// L10::WriteDataRec //
//-------------------//

int
L10::WriteDataRec()
{
	return(file.Write(buffer, L10_FRAME_SIZE));
}
