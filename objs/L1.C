//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l1_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include "L1.h"


//========//
// L1File //
//========//

L1File::L1File()
{
	return;
}

L1File::~L1File()
{
	return;
}


//====//
// L1 //
//====//

L1::L1()
:	time(0), gcAltitude(0.0), gcLongitude(0.0), gcLatitude(0.0), gcX(0.0),
	gcY(0.0), gcZ(0.0), velX(0.0), velY(0.0), velZ(0.0),
	antennaPosition(0.0), beam(NONE)
{
	AllocateBuffer(L1_DATA_REC_SIZE);
	return;
}

L1::~L1()
{
	return;
}

//------------------//
// L1::WriteDataRec //
//------------------//

int
L1::WriteDataRec()
{
	if (! InsertAll())
		return(0);

	if (! WriteBuffer())
		return(0);

	return(1);
}

//---------------//
// L1::InsertAll //
//---------------//

int
L1::InsertAll()
{
	int idx = 0;

	memcpy(_frame + idx, &time, sizeof(double));
	idx += sizeof(double);

	memcpy(_frame + idx, &gcAltitude, sizeof(double));
	idx += sizeof(double);

	memcpy(_frame + idx, &gcLongitude, sizeof(double));
	idx += sizeof(double);

	memcpy(_frame + idx, &gcLatitude, sizeof(double));
	idx += sizeof(double);

	memcpy(_frame + idx, &gcX, sizeof(double));
	idx += sizeof(double);

	memcpy(_frame + idx, &gcY, sizeof(double));
	idx += sizeof(double);

	memcpy(_frame + idx, &gcZ, sizeof(double));
	idx += sizeof(double);

	memcpy(_frame + idx, &velX, sizeof(double));
	idx += sizeof(double);

	memcpy(_frame + idx, &velY, sizeof(double));
	idx += sizeof(double);

	memcpy(_frame + idx, &velZ, sizeof(double));
	idx += sizeof(double);

	memcpy(_frame + idx, &antennaPosition, sizeof(double));
	idx += sizeof(double);

	memcpy(_frame + idx, &beam, sizeof(L1BeamE));
	idx += sizeof(L1BeamE);

	return(1);
}
