//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l10_c[] =
	"@(#) $Id$";

#include <memory.h>
#include <malloc.h>
#include "L10.h"

//=====//
// L10 //
//=====//

L10::L10()
:	buffer(NULL), bufferSize(0), _status(OK)
{
	return;
}

L10::~L10()
{
	DeallocateBuffer();
	return;
}

//---------------------//
// L10::AllocateBuffer //
//---------------------//
 
int
L10::AllocateBuffer(
    int     beam_cycles_per_frame,
    int     slices_per_spot)
{
    // antenna position and sigma-0
    int bytes_per_slice = sizeof(short) + sizeof(float);
    int total_slices = beam_cycles_per_frame * slices_per_spot;
    int buffer_size = L10_FRAME_HEADER_SIZE + total_slices * bytes_per_slice;
    buffer = (char *)malloc(buffer_size);
    if (buffer == NULL)
        return(0);
    bufferSize = buffer_size;
    return(1);
}
 
//-----------------------//
// L10::DeallocateBuffer //
//-----------------------//
 
int
L10::DeallocateBuffer()
{
    if (buffer)
        free(buffer);
    bufferSize = 0;
    return(1);
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
	if (! file.Read(buffer, bufferSize))
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
	return(file.Write(buffer, bufferSize));
}
