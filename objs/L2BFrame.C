//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l20frame_c[] =
	"@(#) $Id$";

#include <memory.h>
#include "L20Frame.h"


//===========//
// L20Header //
//===========//

L20Header::L20Header()
:	crossTrackResolution(0.0), alongTrackResolution(0.0), zeroIndex(0)
{
	return;
}

L20Header::~L20Header()
{
	return;
}

//-----------------//
// L20Header::Read //
//-----------------//
 
int
L20Header::Read(
	FILE*	fp)
{
	if (fread(&crossTrackResolution, sizeof(float), 1, fp) != 1 ||
		fread(&alongTrackResolution, sizeof(float), 1, fp) != 1 ||
		fread(&zeroIndex, sizeof(int), 1, fp) != 1)
	{
		return(0);
	}
	return(1);
}

//------------------//
// L20Header::Write //
//------------------//
 
int
L20Header::Write(
	FILE*	fp)
{
	if (fwrite(&crossTrackResolution, sizeof(float), 1, fp) != 1 ||
		fwrite(&alongTrackResolution, sizeof(float), 1, fp) != 1 ||
		fwrite(&zeroIndex, sizeof(int), 1, fp) != 1)
	{
		return(0);
	}
	return(1);
}

//==========//
// L20Frame //
//==========//

L20Frame::L20Frame()
{
	return;
}

L20Frame::~L20Frame()
{
	return;
}
