//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l17_c[] =
	"@(#) $Id$";

#include <memory.h>
#include "L17.h"


//=====//
// L17 //
//=====//

L17::L17()
:	crossTrackResolution(0.0), alongTrackResolution(0.0), crossTrackBins(0),
	alongTrackBins(0), zeroIndex(0), startTime(0.0), _status(OK),
	_headerTransferred(0)
{
	return;
}

L17::~L17()
{
	return;
}

//------------------//
// L17::SetFilename //
//------------------//

int
L17::SetFilename(
	const char*		filename)
{
	return(file.SetFilename(filename));
}

//-----------------//
// L17::ReadHeader //
//-----------------//

int
L17::ReadHeader()
{
	FILE* fp = file.GetFp();
	if (fp == NULL) return(0);

	if (fread(&crossTrackResolution, sizeof(double), 1, fp) != 1 ||
		fread(&alongTrackResolution, sizeof(double), 1, fp) != 1 ||
		fread(&crossTrackBins, sizeof(int), 1, fp) != 1 ||
		fread(&alongTrackBins, sizeof(int), 1, fp) != 1 ||
		fread(&zeroIndex, sizeof(int), 1, fp) != 1 ||
		fread(&startTime, sizeof(double), 1, fp) != 1)
	{
		return(0);
	}
	_headerTransferred = 1;
	return(1);
}

//------------------//
// L17::ReadDataRec //
//------------------//

int
L17::ReadDataRec()
{
	FILE* fp = file.GetFp();
	if (fp == NULL) return(0);

	if (! _headerTransferred)
	{
		if (! ReadHeader())
			return(0);
	}

    if (fread((void *)&(frame.rev), sizeof(unsigned int), 1, fp) != 1 ||
        fread((void *)&(frame.ati), sizeof(int), 1, fp) != 1 ||
        fread((void *)&(frame.cti), sizeof(unsigned char), 1, fp) != 1 ||
		frame.measList.Read(fp) != 1)

    {
        return(0);
	}

	return(1);
}

//------------------//
// L17::WriteHeader //
//------------------//

int
L17::WriteHeader()
{
	FILE* fp = file.GetFp();
	if (fp == NULL) return(0);

	if (fwrite(&crossTrackResolution, sizeof(double), 1, fp) != 1 ||
		fwrite(&alongTrackResolution, sizeof(double), 1, fp) != 1 ||
		fwrite(&crossTrackBins, sizeof(int), 1, fp) != 1 ||
		fwrite(&alongTrackBins, sizeof(int), 1, fp) != 1 ||
		fwrite(&zeroIndex, sizeof(int), 1, fp) != 1 ||
		fwrite(&startTime, sizeof(double), 1, fp) != 1)
	{
		return(0);
	}
	_headerTransferred = 1;
	return(1);
}

//-------------------//
// L17::WriteDataRec //
//-------------------//

int
L17::WriteDataRec()
{
	FILE* fp = file.GetFp();
	if (fp == NULL) return(0);

	if (! _headerTransferred)
	{
		if (! WriteHeader())
			return(0);
	}

    if (fwrite((void *)&(frame.rev), sizeof(unsigned int), 1, fp) != 1 ||
        fwrite((void *)&(frame.ati), sizeof(int), 1, fp) != 1 ||
        fwrite((void *)&(frame.cti), sizeof(unsigned char), 1, fp) != 1 ||
		frame.measList.Write(fp) != 1)

    {
        return(0);
	}

	return(1);
}
