//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l2aframe_c[] =
	"@(#) $Id$";

#include <memory.h>
#include "L2AFrame.h"
#include "Meas.h"


//===========//
// L2AHeader //
//===========//

L2AHeader::L2AHeader()
:	crossTrackResolution(0.0), alongTrackResolution(0.0), crossTrackBins(0),
	alongTrackBins(0), zeroIndex(0), startTime(0.0)
{
	return;
}

L2AHeader::~L2AHeader()
{
	return;
}

//-----------------//
// L2AHeader::Read //
//-----------------//

int
L2AHeader::Read(
	FILE*	fp)
{
	if (fread(&crossTrackResolution, sizeof(float), 1, fp) != 1 ||
		fread(&alongTrackResolution, sizeof(float), 1, fp) != 1 ||
		fread(&crossTrackBins, sizeof(int), 1, fp) != 1 ||
		fread(&alongTrackBins, sizeof(int), 1, fp) != 1 ||
		fread(&zeroIndex, sizeof(int), 1, fp) != 1 ||
		fread(&startTime, sizeof(double), 1, fp) != 1)
	{
		return(0);
	}
	return(1);
}

//------------------//
// L2AHeader::Write //
//------------------//

int
L2AHeader::Write(
	FILE*	fp)
{
	if (fwrite(&crossTrackResolution, sizeof(float), 1, fp) != 1 ||
		fwrite(&alongTrackResolution, sizeof(float), 1, fp) != 1 ||
		fwrite(&crossTrackBins, sizeof(int), 1, fp) != 1 ||
		fwrite(&alongTrackBins, sizeof(int), 1, fp) != 1 ||
		fwrite(&zeroIndex, sizeof(int), 1, fp) != 1 ||
		fwrite(&startTime, sizeof(double), 1, fp) != 1)
	{
		return(0);
	}
	return(1);
}

//==========//
// L2AFrame //
//==========//

L2AFrame::L2AFrame()
{
	return;
}

L2AFrame::~L2AFrame()
{
	return;
}

//----------------//
// L2AFrame::Read //
//----------------//

int
L2AFrame::Read(
	FILE*	fp)
{
	if (fread((void *)&rev, sizeof(unsigned int), 1, fp) != 1 ||
		fread((void *)&ati, sizeof(int), 1, fp) != 1 ||
		fread((void *)&cti, sizeof(unsigned char), 1, fp) != 1 ||
		measList.Read(fp) != 1)
	{
		return(0);
	}

	return(1);
}

//-----------------//
// L2AFrame::Write //
//-----------------//

int
L2AFrame::Write(
	FILE*	fp)
{
	if (fwrite((void *)&rev, sizeof(unsigned int), 1, fp) != 1 ||
		fwrite((void *)&ati, sizeof(int), 1, fp) != 1 ||
		fwrite((void *)&cti, sizeof(unsigned char), 1, fp) != 1 ||
		measList.Write(fp) != 1)
	{
		return(0);
	}

	return(1);
}
