//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l20_c[] =
	"@(#) $Id$";

#include <memory.h>
#include "L20.h"


//=====//
// L20 //
//=====//

L20::L20()
{
	return;
}

L20::~L20()
{
	return;
}

//------------------//
// L20::SetFilename //
//------------------//

int
L20::SetFilename(
	const char*		filename)
{
	return(file.SetFilename(filename));
}

//------------------//
// L20::ReadDataRec //
//------------------//

int
L20::ReadDataRec()
{
	return(0);
}

//-------------------//
// L20::WriteDataRec //
//-------------------//

int
L20::WriteDataRec()
{
	FILE* fp = file.GetFp();
	return(frame.swath.WriteL20(fp));
}

//------------//
// L20::Close //
//------------//

int
L20::Close()
{
	// flush contents here

	file.Close();
	return(1);
}
