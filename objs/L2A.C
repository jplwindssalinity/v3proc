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
:	crosstrack_res(0), alongtrack_res(0),
	crosstrack_bins(0), alongtrack_bins(0),
	zero_index(0), start_time(0),
	_firstread(1), _firstwrite(1)
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

//------------------//
// L17::ReadDataRec //
//------------------//

int
L17::ReadDataRec()
{
	FILE* fp = file.GetFp();
	if (fp == NULL) return(0);

	if (_firstread == 1)
	{	// on the first read, read in the file header first.
		if (fread(&crosstrack_res, sizeof(double), 1, fp) != 1 ||
			fread(&alongtrack_res, sizeof(double), 1, fp) != 1 ||
			fread(&crosstrack_bins, sizeof(int), 1, fp) != 1 ||
			fread(&alongtrack_bins, sizeof(int), 1, fp) != 1 ||
			fread(&zero_index, sizeof(int), 1, fp) != 1 ||
			fread(&start_time, sizeof(double), 1, fp) != 1)
		{
			printf("Error reading L1.7 header section\n");
			return(0);
		}
		_firstread = 0;	// prevent header reads for subsequent calls.
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

//-------------------//
// L17::WriteDataRec //
//-------------------//

int
L17::WriteDataRec()
{
	FILE* fp = file.GetFp();
	if (fp == NULL) return(0);

	if (_firstwrite == 1)
	{	// on the first write, write out the file header first.
		if (fwrite(&crosstrack_res, sizeof(double), 1, fp) != 1 ||
			fwrite(&alongtrack_res, sizeof(double), 1, fp) != 1 ||
			fwrite(&crosstrack_bins, sizeof(int), 1, fp) != 1 ||
			fwrite(&alongtrack_bins, sizeof(int), 1, fp) != 1 ||
			fwrite(&zero_index, sizeof(int), 1, fp) != 1 ||
			fwrite(&start_time, sizeof(double), 1, fp) != 1)
		{
			printf("Error writing L1.7 header section\n");
			return(0);
		}
		_firstwrite = 0;	// prevent header reads for subsequent calls.
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
