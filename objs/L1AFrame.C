//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l10frame_c[] =
	"@(#) $Id$";

#include <memory.h>
#include "L10Frame.h"


//==========//
// L10Frame //
//==========//

L10Frame::L10Frame()
:	time(0), gcAltitude(0.0), gcLongitude(0.0), gcLatitude(0.0), gcX(0.0),
	gcY(0.0), gcZ(0.0), velX(0.0), velY(0.0), velZ(0.0)
{
	return;
}

L10Frame::~L10Frame()
{
	return;
}

//----------------//
// L10Frame::Pack //
//----------------//

int
L10Frame::Pack(
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

	size = sizeof(float) * SPOTS_PER_L10_FRAME;
	memcpy((void *)(buffer + idx), (void *)antennaPosition, size);
	idx += size;

	size = sizeof(float) * SPOTS_PER_L10_FRAME;
	memcpy((void *)(buffer + idx), (void *)sigma0, size);
	idx += size;

	return(idx);
}

//------------------//
// L10Frame::Unpack //
//------------------//

int
L10Frame::Unpack(
	char*	buffer)
{
	int idx = 0;
	int size;

	size = sizeof(double);
	memcpy((void *)&time, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(float);
	memcpy((void *)&gcAltitude, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&gcLongitude, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&gcLatitude, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&gcX, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&gcY, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&gcZ, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&velX, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&velY, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&velZ, (void *)(buffer + idx), size);
	idx += size;

	float tmp_float;
	memcpy((void *)&tmp_float, (void *)(buffer + idx), size);
	attitude.SetRoll(tmp_float);
	idx += size;

	memcpy((void *)&tmp_float, (void *)(buffer + idx), size);
	attitude.SetPitch(tmp_float);
	idx += size;

	memcpy((void *)&tmp_float, (void *)(buffer + idx), size);
	attitude.SetYaw(tmp_float);
	idx += size;

	size = sizeof(float) * SPOTS_PER_L10_FRAME;
	memcpy((void *)antennaPosition, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(float) * SPOTS_PER_L10_FRAME;
	memcpy((void *)sigma0, (void *)(buffer + idx), size);
	idx += size;

	return(idx);
}
