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

//---------------//
// WVC::WriteL20 //
//---------------//

int
WVC::WriteL20(
	FILE*	fp)
{
	for (WindVector* wv = ambiguities.GetHead(); wv;
		wv = ambiguities.GetNext())
	{
		wv->WriteL20(fp);
	}
	return(1);
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

	//--------------------------------------//
	// use a new WVC to identify duplicates //
	//--------------------------------------//

	WVC new_wvc;
	WindVector* wv = ambiguities.GetHead();
	while (wv)
	{
		int match_found = 0;
		for (WindVector* wv_tmp = new_wvc.ambiguities.GetHead();
			wv_tmp && wv_tmp != wv; wv_tmp = new_wvc.ambiguities.GetNext())
		{
			if (*wv_tmp == *wv)
			{
				match_found = 1;
				break;
			}
		}
		if (match_found)
		{
			wv = ambiguities.RemoveCurrent();	// next becomes current
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

//-------------------//
// WVC::FreeContents //
//-------------------//

void
WVC::FreeContents()
{
	WindVector* wv;
	ambiguities.GotoHead();
	while ((wv = ambiguities.RemoveCurrent()))
		delete wv;
	return;
}
