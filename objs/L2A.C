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
    if (fwrite((void *)&(frame.rev), sizeof(unsigned int), 1, fp) != 1 ||
        fwrite((void *)&(frame.ati), sizeof(int), 1, fp) != 1 ||
        fwrite((void *)&(frame.cti), sizeof(unsigned char), 1, fp) != 1 ||
		frame.measList.Write(fp) != 1)

    {
        return(0);
	}

	return(1);
}
