//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l17frame_c[] =
	"@(#) $Id$";

#include <memory.h>
#include "L17Frame.h"
#include "Meas.h"


//===========//
// L17Header //
//===========//

L17Header::L17Header()
:	crossTrackResolution(0.0), alongTrackResolution(0.0), crossTrackBins(0),
	alongTrackBins(0), zeroIndex(0), startTime(0.0)
{
	return;
}

L17Header::~L17Header()
{
	return;
}

//-----------------//
// L17Header::Read //
//-----------------//

int
L17Header::Read(
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
// L17Header::Write //
//------------------//

int
L17Header::Write(
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
// L17Frame //
//==========//

L17Frame::L17Frame()
{
	return;
}

L17Frame::~L17Frame()
{
	return;
}

//----------------//
// L17Frame::Read //
//----------------//

int
L17Frame::Read(
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
// L17Frame::Write //
//-----------------//

int
L17Frame::Write(
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
