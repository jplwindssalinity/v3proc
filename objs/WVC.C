//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_wvc_c[] =
	"@(#) $Id$";

#include "WVC.h"
#include "Constants.h"


//=====//
// WVC //
//=====//

WVC::WVC()
{
	return;
}

WVC::~WVC()
{
	return;
}

//-----------------------//
// WVC::WriteAmbigsAscii //
//-----------------------//

int
WVC::WriteAmbigsAscii(
	FILE*		ofp)
{
	for (WindVector* wv = ambiguities.GetHead(); wv; wv= ambiguities.GetNext())
	{
		fprintf(ofp, "%g %g\n", wv->spd, wv->dir * rtd);
	}
	return(1);
}
