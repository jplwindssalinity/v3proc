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
	gcY(0.0), gcZ(0.0), velX(0.0), velY(0.0), velZ(0.0),
	antennaPosition(0.0)
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

	memcpy((void *)(buffer + idx), (void *)&time, sizeof(double));
	idx += sizeof(double);

	memcpy((void *)(buffer + idx), (void *)&gcAltitude, sizeof(float));
	idx += sizeof(float);

	memcpy((void *)(buffer + idx), (void *)&gcLongitude, sizeof(float));
	idx += sizeof(float);

	memcpy((void *)(buffer + idx), (void *)&gcLatitude, sizeof(float));
	idx += sizeof(float);

	memcpy((void *)(buffer + idx), (void *)&gcX, sizeof(float));
	idx += sizeof(float);

	memcpy((void *)(buffer + idx), (void *)&gcY, sizeof(float));
	idx += sizeof(float);

	memcpy((void *)(buffer + idx), (void *)&gcZ, sizeof(float));
	idx += sizeof(float);

	memcpy((void *)(buffer + idx), (void *)&velX, sizeof(float));
	idx += sizeof(float);

	memcpy((void *)(buffer + idx), (void *)&velY, sizeof(float));
	idx += sizeof(float);

	memcpy((void *)(buffer + idx), (void *)&velZ, sizeof(float));
	idx += sizeof(float);

	memcpy((void *)(buffer + idx), (void *)&antennaPosition, sizeof(float));
	idx += sizeof(float);

	return(idx);
}
