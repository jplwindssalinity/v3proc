//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l2b_c[] =
	"@(#) $Id$";

#include <memory.h>
#include "L2B.h"


//=====//
// L2B //
//=====//

L2B::L2B()
:	medianFilterWindowSize(0), medianFilterMaxPasses(0)
{
	return;
}

L2B::~L2B()
{
	return;
}

//----------------//
// L2B::WriteVctr //
//----------------//

int
L2B::WriteVctr(
	const char*		filename,
	const int		rank)
{
	return(frame.swath.WriteVctr(filename, rank));
}
