//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l0_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <fcntl.h>
#include "L0.h"


//====//
// L0 //
//====//

L0::L0()
:	time(0), gcAltitude(0.0), gcLongitude(0.0), gcLatitude(0.0), gcX(0.0),
	gcY(0.0), gcZ(0.0), velX(0.0), velY(0.0), velZ(0.0),
	antennaPosition(0.0), beam(NONE), _ofd(-1)
{
	return;
}

L0::~L0()
{
	return;
}

//--------------------//
// L0::OpenForWriting //
//--------------------//

int
L0::OpenForWriting(
	const char*		filename)
{
	_ofd = creat(filename, O_RDONLY);
	if (_ofd == -1)
		return(0);
	return(1);
}

//------------------//
// L0::WriteDataRec //
//------------------//

int
L0::WriteDataRec()
{
	if (_ofd == -1)
		return(0);

	return(0);
}
