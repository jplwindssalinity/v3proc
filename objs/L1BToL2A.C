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
	Grid*		grid)
{
	static long spot_id = 0;

	MeasSpotList* meas_spot_list = &(grid->l15.frame.spotList);

	//----------------------//
	// for each MeasSpot... //
	//----------------------//

	for (MeasSpot* meas_spot = meas_spot_list->GetHead(); meas_spot;
		meas_spot = meas_spot_list->GetNext())
	{
		double meas_time = meas_spot->time;

		//------------------//
		// for each Meas... //
		//------------------//

		for (Meas* meas = meas_spot->GetHead(); meas;
			meas = meas_spot->GetNext())
		{
			//---------------------//
			// ...add Meas to Grid //
			//---------------------//

			grid->Add(meas, meas_time, spot_id);
		}
		spot_id++;
	}

	return(1);
}
