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
	antennaPosition(0.0), beam(NONE), sigma_0(0.0)
{
	AllocateFrame(L1_DATA_REC_SIZE);
	return;
}

L1::~L1()
{
	return;
}

//---------------//
// L1::PackFrame //
//---------------//

int
L1::PackFrame()
{
	int idx = 0;

	memcpy((void *)(_frame + idx), (void *)&time, sizeof(double));
	idx += sizeof(double);

	memcpy((void *)(_frame + idx), (void *)&gcAltitude, sizeof(double));
	idx += sizeof(double);

	memcpy((void *)(_frame + idx), (void *)&gcLongitude, sizeof(double));
	idx += sizeof(double);

	memcpy((void *)(_frame + idx), (void *)&gcLatitude, sizeof(double));
	idx += sizeof(double);

	memcpy((void *)(_frame + idx), (void *)&gcX, sizeof(double));
	idx += sizeof(double);

	memcpy((void *)(_frame + idx), (void *)&gcY, sizeof(double));
	idx += sizeof(double);

	memcpy((void *)(_frame + idx), (void *)&gcZ, sizeof(double));
	idx += sizeof(double);

	memcpy((void *)(_frame + idx), (void *)&velX, sizeof(double));
	idx += sizeof(double);

	memcpy((void *)(_frame + idx), (void *)&velY, sizeof(double));
	idx += sizeof(double);

	memcpy((void *)(_frame + idx), (void *)&velZ, sizeof(double));
	idx += sizeof(double);

	memcpy((void *)(_frame + idx), (void *)&antennaPosition, sizeof(double));
	idx += sizeof(double);

	memcpy((void *)(_frame + idx), (void *)&beam, sizeof(L1BeamE));
	idx += sizeof(L1BeamE);

	memcpy((void *)(_frame + idx), (void *)&sigma_0, sizeof(double));
	idx += sizeof(double);

	return(1);
}

//-----------------//
// L1::UnpackFrame //
//-----------------//
 
int
L1::UnpackFrame()
{
	int idx = 0;
 
	memcpy((void *)&time, (void *)(_frame + idx), sizeof(double));
	idx += sizeof(double);
 
	memcpy((void *)&gcAltitude, (void *)(_frame + idx), sizeof(double));
	idx += sizeof(double);
 
	memcpy((void *)&gcLongitude, (void *)(_frame + idx), sizeof(double));
	idx += sizeof(double);
 
	memcpy((void *)&gcLatitude, (void *)(_frame + idx), sizeof(double));
	idx += sizeof(double);
 
	memcpy((void *)&gcX, (void *)(_frame + idx), sizeof(double));
	idx += sizeof(double);
 
	memcpy((void *)&gcY, (void *)(_frame + idx), sizeof(double));
	idx += sizeof(double);
 
	memcpy((void *)&gcZ, (void *)(_frame + idx), sizeof(double));
	idx += sizeof(double);
 
	memcpy((void *)&velX, (void *)(_frame + idx), sizeof(double));
	idx += sizeof(double);
 
	memcpy((void *)&velY, (void *)(_frame + idx), sizeof(double));
	idx += sizeof(double);
 
	memcpy((void *)&velZ, (void *)(_frame + idx), sizeof(double));
	idx += sizeof(double);
 
	memcpy((void *)&antennaPosition, (void *)(_frame + idx), sizeof(double));
	idx += sizeof(double);
 
	memcpy((void *)&beam, (void *)(_frame + idx), sizeof(L1BeamE));
	idx += sizeof(L1BeamE);
 
	memcpy((void *)&sigma_0, (void *)(_frame + idx), sizeof(double));
	idx += sizeof(double);
 
	return(1);
}
