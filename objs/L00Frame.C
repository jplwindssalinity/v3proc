//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_l00frame_c[] =
    "@(#) $Id$";

#include <memory.h>
#include <malloc.h>
#include "L00Frame.h"


//==========//
// L00Frame //
//==========//

L00Frame::L00Frame()
:   time(0), instrumentTicks(0), orbitTicks(0), orbitStep(0),
    priOfOrbitStepChange(255), gcAltitude(0.0), gcLongitude(0.0),
    gcLatitude(0.0), gcX(0.0), gcY(0.0), gcZ(0.0), velX(0.0), velY(0.0),
    velZ(0.0), ptgr(0.0), antennaPosition(NULL), science(NULL),
    spotNoise(NULL), spotsPerFrame(0), slicesPerSpot(0), slicesPerFrame(0)
{
    return;
}

L00Frame::~L00Frame()
{
	Deallocate();
	return;
}

//---------------//
// L00::Allocate //
//---------------//

int
L00Frame::Allocate(
	int		number_of_beams,
	int		antenna_cycles_per_frame,
	int		slices_per_spot)
{
	spotsPerFrame = number_of_beams * antenna_cycles_per_frame;
	slicesPerSpot = slices_per_spot;
	slicesPerFrame = slicesPerSpot * spotsPerFrame;

	//----------------------------//
	// allocate antenna positions //
	//----------------------------//

	antennaPosition = (unsigned short *)malloc(spotsPerFrame *
		sizeof(unsigned short));
	if (antennaPosition == NULL)
		return(0);

	//-------------------------------//
	// allocate science measurements //
	//-------------------------------//

	science = (float *)malloc(slicesPerFrame * sizeof(float));
	if (science == NULL)
	{
		return(0);
	}
	spotNoise = (float *)malloc(spotsPerFrame * sizeof(float));
	if (spotNoise == NULL)
	{
		return(0);
	}

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
	if (spotNoise)
		free(spotNoise);
	spotsPerFrame = 0;
	slicesPerSpot = 0;
	slicesPerFrame = 0;
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

	size = sizeof(unsigned int);
	memcpy((void *)(buffer + idx), (void *)&instrumentTicks, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&orbitTicks, size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)(buffer + idx), (void *)&orbitStep, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&priOfOrbitStepChange, size);
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

	memcpy((void *)(buffer +idx),(void *)&ptgr, size);
	idx += size;

	size = sizeof(unsigned short) * spotsPerFrame;
	memcpy((void *)(buffer + idx), (void *)antennaPosition, size);
	idx += size;

	size = sizeof(float) * slicesPerFrame;
	memcpy((void *)(buffer + idx), (void *)science, size);
	idx += size;

	size = sizeof(float) * spotsPerFrame;
	memcpy((void *)(buffer + idx), (void *)spotNoise, size);
	idx += size;

	return(idx);
}

//------------------//
// L00Frame::Unpack //
//------------------//

int
L00Frame::Unpack(
	char*	buffer)
{
	int idx = 0;
	int size;

	size = sizeof(double);
	memcpy((void *)&time, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned int);
	memcpy((void *)&instrumentTicks, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&orbitTicks, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)&orbitStep, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&priOfOrbitStepChange, (void *)(buffer + idx), size);
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

	memcpy((void *)&ptgr, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned short) * spotsPerFrame;
	memcpy((void *)antennaPosition, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(float) * slicesPerFrame;
	memcpy((void *)science, (void *)(buffer + idx), size);
	idx += size;

	return(idx);
}
