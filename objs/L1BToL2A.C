//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l15tol17_c[] =
	"@(#) $Id$";

#include "L15ToL17.h"
#include "Antenna.h"
#include "Ephemeris.h"
#include "InstrumentGeom.h"
#include "GenericGeom.h"


//==========//
// L15ToL17 //
//==========//

L15ToL17::L15ToL17()
{
	return;
}

L15ToL17::~L15ToL17()
{
	return;
}

//-----------------//
// L15ToL17::Group //
//-----------------//

int
L15ToL17::Group(
	L15*		l15,
	Grid*		grid)
{
	// position at start of list of spots
	MeasSpot* mspot = l15->frame.spotList.GetHead();
	// remove spot before gridding
	mspot = l15->frame.spotList.RemoveCurrent();

	//------------------//
	// for each spot... //
	//------------------//

	while (mspot != NULL)
	{
		//------------------------------//
		// ...Grid the measurement list //
		//------------------------------//

		// position at start of list of slices
		Meas* meas = mspot->slices.GetHead();
		// remove slice before gridding
		meas = mspot->slices.RemoveCurrent();
		while (meas != NULL)
		{
			// grid each slice in this measurement spot
			grid->Add(meas,mspot->time);
 			// move to next slice
			meas = mspot->slices.RemoveCurrent();
		}

 		// move to next spot
		mspot = l15->frame.spotList.RemoveCurrent();
	}

	return(1);
}
