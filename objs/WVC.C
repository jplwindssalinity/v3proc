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
	for (WindVector* wv = ambiguities.GetHead(); wv;
		wv = ambiguities.GetNext())
	{
		fprintf(ofp, "%g %g\n", wv->dir * rtd, wv->spd);
	}
	return(1);
}

//-----------------------//
// WVC::RemoveDuplicates //
//-----------------------//

int
WVC::RemoveDuplicates()
{
	int count = 0;

	//---------------------------------------//
	// use a new list to identify duplicates //
	//---------------------------------------//

	WVC new_wvc;
	WindVector* wv = ambiguities.GetHead();
	while (wv)
	{
		if (new_wvc.ambiguities.Find(wv))
		{
			wv = ambiguities.RemoveCurrent();
			delete wv;
			wv = ambiguities.GetCurrent();
			count++;
		}
		else
		{
			new_wvc.ambiguities.Append(wv);
			wv = ambiguities.GetNext();
		}
	}

	//---------------------//
	// get rid of new list //
	//---------------------//

	wv = new_wvc.ambiguities.GetHead();
	while (wv)
		wv = new_wvc.ambiguities.RemoveCurrent();

	return(count);
}
