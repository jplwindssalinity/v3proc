//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l00frame_c[] =
	"@(#) $Id$";

#include <memory.h>
#include "L00Frame.h"


//==========//
// L00Frame //
//==========//

L00Frame::L00Frame()
:	time(0), gcAltitude(0.0), gcLongitude(0.0), gcLatitude(0.0), gcX(0.0),
	gcY(0.0), gcZ(0.0), velX(0.0), velY(0.0), velZ(0.0)
{
	return;
}

L00Frame::~L00Frame()
{
	return;
}

//----------------//
// L00Frame::Pack //
//----------------//

int
L00Frame::Pack(
	char*	buffer)
{
	int idx = 0;
	int size;

	size = sizeof(double);
	memcpy((void *)(buffer + idx), (void *)&time, size);
	idx += size;

	size = sizeof(float);
	memcpy((void *)(buffer + idx), (void *)&gcAltitude, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&gcLongitude, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&gcLatitude, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&gcX, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&gcY, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&gcZ, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&velX, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&velY, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&velZ, size);
	idx += size;

	float tmp_float;
	tmp_float = attitude.GetRoll();
	memcpy((void *)(buffer + idx), (void *)&tmp_float, size);
	idx += size;

	tmp_float = attitude.GetPitch();
	memcpy((void *)(buffer + idx), (void *)&tmp_float, size);
	idx += size;

	tmp_float = attitude.GetYaw();
	memcpy((void *)(buffer + idx), (void *)&tmp_float, size);
	idx += size;

	size = sizeof(unsigned short) * SPOTS_PER_L00_FRAME;
	memcpy((void *)(buffer + idx), (void *)antennaPosition, size);
	idx += size;

	size = sizeof(float) * SPOTS_PER_L00_FRAME;
	memcpy((void *)(buffer + idx), (void *)sigma0, size);
	idx += size;

	return(idx);
}
