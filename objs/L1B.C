//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l15_c[] =
	"@(#) $Id$";

#include <memory.h>
#include "L15.h"


//=====//
// L15 //
//=====//

L15::L15()
{
	return;
}

L15::~L15()
{
	return;
}

//------------------//
// L15::SetFilename //
//------------------//

int
L15::SetFilename(
	const char*		filename)
{
	return(file.SetFilename(filename));
}

//------------------//
// L15::ReadDataRec //
//------------------//

int
L15::ReadDataRec()
{
	return(0);
}

//-------------------//
// L15::WriteDataRec //
//-------------------//

int
L15::WriteDataRec()
{
	return(0);
}
