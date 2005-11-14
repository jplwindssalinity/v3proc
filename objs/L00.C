//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l00_c[] =
	"@(#) $Id$";

#include <memory.h>
#include <malloc.h>
#include <stdlib.h>
#include "L00.h"


//=====//
// L00 //
//=====//

L00::L00()
:	buffer(NULL), bufferSize(0), _status(OK)
{
	return;
}

L00::~L00()
{
	DeallocateBuffer();
	return;
}

//---------------------//
// L00::AllocateBuffer //
//---------------------//

int
L00::AllocateBuffer()
{
    bufferSize = frame.FrameSize();
    buffer = (char *)malloc(bufferSize);
    if (buffer == NULL)
    {
        bufferSize = 0;
        return(0);
    }
    return(1);
}

//-----------------------//
// L00::DeallocateBuffer //
//-----------------------//

int
L00::DeallocateBuffer()
{
	if (buffer)
		free(buffer);
	bufferSize = 0;
	return(1);
}

//------------------//
// L00::ReadDataRec //
//------------------//

int
L00::ReadDataRec()
{
	// this will most likely get more complicated later with headers and all
	if (! Read(buffer, bufferSize))
	{
		if (EndOfFile())
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
