//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l00frame_c[] =
	"@(#) $Id$";

#include <memory.h>
#include <malloc.h>
#include "L00Frame.h"


//==========//
// L00Frame //
//==========//

L00Frame::L00Frame()
:	time(0), gcAltitude(0.0), gcLongitude(0.0), gcLatitude(0.0), gcX(0.0),
	gcY(0.0), gcZ(0.0), velX(0.0), velY(0.0), velZ(0.0), antennaPosition(NULL),
	science(NULL), spotsPerFrame(0), slicesPerSpot(0), totalSlices(0)
{
	return;
}

L00Frame::~L00Frame()
{
	return;
}

//---------------//
// L00::Allocate //
//---------------//
 
int
L00Frame::Allocate(
	int		spots_per_frame,
	int		slices_per_spot)
{
	int total_slices = spots_per_frame * slices_per_spot;

	//----------------------------//
	// allocate antenna positions //
	//----------------------------//

	antennaPosition =
		(unsigned short *)malloc(total_slices * sizeof(unsigned short));
	if (antennaPosition == NULL)
		return(0);

	//-------------------------------//
	// allocate science measurements //
	//-------------------------------//

	science = (float *)malloc(total_slices * sizeof(float));
	if (science == NULL)
		return(0);

	spotsPerFrame = spots_per_frame;
	slicesPerSpot = slices_per_spot;
	totalSlices = total_slices;

	return(1);
}
 
//----------------------//
// L00Frame::Deallocate //
//----------------------//
 
int
L00Frame::Deallocate()
{
	if (antennaPosition)
		free(antennaPosition);
	if (science)
		free(science);
	spotsPerFrame = 0;
	slicesPerSpot = 0;
	totalSlices = 0;
	return(1);
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

	size = sizeof(unsigned short) * totalSlices;
	memcpy((void *)(buffer + idx), (void *)antennaPosition, size);
	idx += size;

	size = sizeof(float) * totalSlices;
	memcpy((void *)(buffer + idx), (void *)science, size);
	idx += size;

	return(idx);
}
