//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l00tol1a_c[] =
	"@(#) $Id$";

#include "L00ToL1A.h"


//==========//
// L00ToL1A //
//==========//

L00ToL1A::L00ToL1A()
{
	return;
}

L00ToL1A::~L00ToL1A()
{
	return;
}

//-------------------//
// L00ToL1A::Convert //
//-------------------//

int
L00ToL1A::Convert(
	L00*	l00,
	L1A*	l1a)
{
	// this method will need to do real work later
	memcpy(l1a->buffer, l00->buffer, l00->bufferSize);
	return(1);
}
