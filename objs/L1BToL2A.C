//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l1btol2a_c[] =
	"@(#) $Id$";

#include "L1BToL2A.h"
#include "Antenna.h"
#include "Ephemeris.h"
#include "InstrumentGeom.h"
#include "GenericGeom.h"


//==========//
// L1BToL2A //
//==========//

L1BToL2A::L1BToL2A()
{
	return;
}

L1BToL2A::~L1BToL2A()
{
	return;
}

//-----------------//
// L1BToL2A::Group //
//-----------------//

int
L1BToL2A::Group(
	Grid*		grid,
	int			do_composite)
{
	static long spot_id = 0;

	MeasSpotList* meas_spot_list = &(grid->l1b.frame.spotList);

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

			grid->Add(meas, meas_time, spot_id, do_composite);
		}
		spot_id++;
	}

	return(1);
}
