//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l10_c[] =
	"@(#) $Id$";

#include <memory.h>
#include "L10.h"


//=====//
// L10 //
//=====//

L10::L10()
{
	return;
}

L10::~L10()
{
	return;
}

//------------------//
// L10::SetFilename //
//------------------//

int
L10::SetFilename(
	const char*		filename)
{
	return(file.SetFilename(filename));
}

//-------------------//
// L10::WriteDataRec //
//-------------------//

int
L10::WriteDataRec()
{
	return(file.Write(buffer, L10_FRAME_SIZE));
}
