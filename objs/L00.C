//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l00_c[] =
	"@(#) $Id$";

#include <memory.h>
#include <malloc.h>
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
L00::AllocateBuffer(
	int		number_of_beams,
	int		antenna_cycles_per_frame,
	int		slices_per_spot)
{
	// antenna position and sigma-0
	int power_bytes = sizeof(float) * number_of_beams *
		antenna_cycles_per_frame * slices_per_spot;
	int ant_bytes = sizeof(short) * number_of_beams * antenna_cycles_per_frame;
	int buffer_size = L00_FRAME_HEADER_SIZE + power_bytes + ant_bytes;
	buffer = (char *)malloc(buffer_size);
	if (buffer == NULL)
		return(0);
	bufferSize = buffer_size;
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
// L00::SetFilename //
//------------------//

int
L00::SetFilename(
	const char*		filename)
{
	return(file.SetFilename(filename));
}

//------------------//
// L00::ReadDataRec //
//------------------//

int
L00::ReadDataRec()
{
	// this will most likely get more complicated later with headers and all
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
