//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l00_c[] =
	"@(#) $Id$";

#include <memory.h>
#include "L00.h"


//=====//
// L00 //
//=====//

L00::L00()
:	_status(OK)
{
	return;
}

L00::~L00()
{
	return;
}

//------------------//
// L00::SetFilename //
//------------------//

int
L00::SetFilename(
	const char*		filename)
{
	return(file.SetFilename(filename));
}

//------------------//
// L00::ReadDataRec //
//------------------//

int
L00::ReadDataRec()
{
	// this will most likely get more complicated later with headers and all
	file.Read(buffer, L00_FRAME_SIZE);
	return(1);
}
