//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l00tol10_c[] =
	"@(#) $Id$";

#include "L00ToL10.h"


//==========//
// L00ToL10 //
//==========//

L00ToL10::L00ToL10()
{
	return;
}

L00ToL10::~L00ToL10()
{
	return;
}

//-------------------//
// L00ToL10::Convert //
//-------------------//

int
L00ToL10::Convert(
	L00*	l00,
	L10*	l10)
{
	// this method will need to do real work later
	memcpy(l10->buffer, l00->buffer, L00_FRAME_SIZE);
	return(1);
}
