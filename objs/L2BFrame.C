//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l2bframe_c[] =
	"@(#) $Id$";

#include <memory.h>
#include "L2BFrame.h"


//===========//
// L2BHeader //
//===========//

L2BHeader::L2BHeader()
:	crossTrackResolution(0.0), alongTrackResolution(0.0), zeroIndex(0)
{
	return;
}

L2BHeader::~L2BHeader()
{
	return;
}

//-----------------//
// L2BHeader::Read //
//-----------------//

int
L2BHeader::Read(
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
// L2BHeader::Write //
//------------------//

int
L2BHeader::Write(
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
// L2BFrame //
//==========//

L2BFrame::L2BFrame()
{
	return;
}

L2BFrame::~L2BFrame()
{
	return;
}
