//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l1a_c[] =
	"@(#) $Id$";

#include <memory.h>
#include <malloc.h>
#include "L1A.h"

//=====//
// L1A //
//=====//

L1A::L1A()
:	buffer(NULL), bufferSize(0), _status(OK)
{
	return;
}

L1A::~L1A()
{
	DeallocateBuffer();
	return;
}

//---------------------//
// L1A::AllocateBuffer //
//---------------------//

int
L1A::AllocateBuffer(
	int		number_of_beams,
	int		antenna_cycles_per_frame,
	int		slices_per_spot)
{
	// antenna position and sigma-0
	int power_bytes = sizeof(float) * number_of_beams *
			antenna_cycles_per_frame * slices_per_spot;
	int noise_bytes = sizeof(float) * number_of_beams *
		antenna_cycles_per_frame;
	int ant_bytes = sizeof(short) * number_of_beams * antenna_cycles_per_frame;
    int cal_bytes = sizeof(float)*2*(slices_per_spot + 1);
	int buffer_size = L1A_FRAME_HEADER_SIZE + power_bytes + ant_bytes +
		noise_bytes + cal_bytes;
	buffer = (char *)malloc(buffer_size);
	if (buffer == NULL)
		return(0);
	bufferSize = buffer_size;
	return(1);
}

//-----------------------//
// L1A::DeallocateBuffer //
//-----------------------//

int
L1A::DeallocateBuffer()
{
	if (buffer)
		free(buffer);
	bufferSize = 0;
	return(1);
}

//------------------//
// L1A::ReadDataRec //
//------------------//

int
L1A::ReadDataRec()
{
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

//-------------------//
// L1A::WriteDataRec //
//-------------------//

int
L1A::WriteDataRec()
{
	return(Write(buffer, bufferSize));
}

//--------------------------//
// L1A::WriteDataRecAscii   //
//--------------------------//
int
L1A::WriteDataRecAscii(){
  if(_outputFp==NULL) return(0);
  if(!frame.WriteAscii(_outputFp)) return(0);
  return(1);
}

