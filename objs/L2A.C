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
	return(0);
}

//-------------------//
// L17::WriteDataRec //
//-------------------//

int
L17::WriteDataRec()
{
	return(0);
}
