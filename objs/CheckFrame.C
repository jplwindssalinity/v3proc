//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_checkframe_c[] =
	"@(#) $Id$";

#include <memory.h>
#include <malloc.h>
#include "CheckFrame.h"


//============//
// CheckFrame //
//============//

CheckFrame::CheckFrame()
:	slicesPerSpot(0)
{
	return;
}

CheckFrame::CheckFrame(int slices_per_spot)
{
	if (!Allocate(slices_per_spot))
	{
		fprintf(stderr,"Error allocating a CheckFrame object\n");
	}
	return;
}

CheckFrame::~CheckFrame()
{
	Deallocate();
	return;
}

//----------------------//
// CheckFrame::Allocate //
//----------------------//

int
CheckFrame::Allocate(
	int		slices_per_spot)
{
	if (slices_per_spot <= 0)
	{
		fprintf(stderr,
			"Error: Can't allocate a CheckFrame with %d slices per spot\n",
			slices_per_spot);
		return(0);
	}

	slicesPerSpot = slices_per_spot;

	//-----------------------------//
	// allocate slice measurements //
	//-----------------------------//

	sigma0 = (float *)malloc(slicesPerSpot * sizeof(float));
	if (sigma0 == NULL)
	{
		return(0);
	}
	wv = (WindVector *)malloc(slicesPerSpot * sizeof(WindVector));
	if (wv == NULL)
	{
		return(0);
	}
	XK = (float *)malloc(slicesPerSpot * sizeof(float));
	if (XK == NULL)
	{
		return(0);
	}
	centroid = (EarthPosition *)malloc(slicesPerSpot * sizeof(EarthPosition));
	if (centroid == NULL)
	{
		return(0);
	}
	azimuth = (float *)malloc(slicesPerSpot * sizeof(float));
	if (azimuth == NULL)
	{
		return(0);
	}
	incidence = (float *)malloc(slicesPerSpot * sizeof(float));
	if (incidence == NULL)
	{
		return(0);
	}

	return(1);
}

//------------------------//
// CheckFrame::Deallocate //
//------------------------//

int
CheckFrame::Deallocate()
{
	if (slicesPerSpot > 0)
	{
		if (sigma0) free(sigma0);
		if (wv) free(wv);
		if (XK) free(XK);
		if (centroid) free(centroid);
		if (azimuth) free(azimuth);
		if (incidence) free(incidence);
	}

	slicesPerSpot = 0;
	sigma0 = NULL;
	wv = NULL;
	XK = NULL;
	centroid = NULL;
	azimuth = NULL;
	incidence = NULL;
	
	return(1);
}

//--------------------------//
// CheckFrame::AppendRecord //
//--------------------------//

int
CheckFrame::AppendRecord(
	FILE*	fptr)
{
	return(1);
}
