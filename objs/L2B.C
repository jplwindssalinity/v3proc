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
:	medianFilterWindowSize(0), medianFilterMaxPasses(0)
{
	return;
}

L20::~L20()
{
	return;
}

//---------------//
// L20::WriteBev //
//---------------//

int
L20::WriteBev(
	const char*		filename,
	const int		rank)
{
	return(frame.swath.WriteBev(filename, rank));
}
