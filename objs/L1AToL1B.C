//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l10tol15_c[] =
	"@(#) $Id$";

#include "L10ToL15.h"
#include "Antenna.h"
#include "Ephemeris.h"
#include "InstrumentGeom.h"


//==========//
// L10ToL15 //
//==========//

L10ToL15::L10ToL15()
{
	return;
}

L10ToL15::~L10ToL15()
{
	return;
}

//-------------------//
// L10ToL15::Convert //
//-------------------//

int
L10ToL15::Convert(
	L10*		l10,
	Antenna*	antenna,
	Ephemeris*	ephemeris,
	L15*		l15)
{
	//--------------//
	// unpack frame //
	//--------------//

	l10->frame.Unpack(l10->buffer);

	//------------------//
	// for each spot... //
	//------------------//

	for (int i = 0; i < SPOTS_PER_L10_FRAME; i++)
	{
		//-----------------------//
		// ...determine the beam //
		//-----------------------//

		int beam_idx = i % antenna->numberOfBeams;
		Beam* beam = &(antenna->beam[beam_idx]);

		//-----------------------//
		// ...calculate the time //
		//-----------------------//

		double spot_time = l10->frame.time + (i / antenna->numberOfBeams) *
			antenna->priPerBeam + beam->timeOffset;

		//------------------------------------------------------//
		// ...determine the spacecraft orbit state and attitude //
		//------------------------------------------------------//

		OrbitState sc_orbit_state;
		if (! ephemeris->GetOrbitState(spot_time, &sc_orbit_state))
			return(0);
		Attitude sc_attitude;
		sc_attitude = l10->frame.attitude;

		//------------------------//
		// ...free residual spots //
		//------------------------//

		l15->frame.spotList.FreeContents();

		//--------------------------//
		// ...add spot measurements //
		//--------------------------//
		// for now there is just one
		// eventually a slice loop will be needed

		// set antenna azimuth angle using encoder value
		antenna->SetAzimuthWithEncoder(l10->frame.antennaPosition[i]);

		CoordinateSwitch beam_frame_to_gc = BeamFrameToGC(&sc_orbit_state,
			&sc_attitude, antenna, beam);
		
		Meas* meas = new Meas();
		meas->value = l10->frame.sigma0[i];
//		meas->outline =
//		meas->center = 
		meas->pol = antenna->beam[beam_idx].polarization;
//		meas->eastAzimuth =
//		meas->scAzimuth =
//		meas->incidenceAngle =
		meas->estimatedKp = 0.0;

//		l15->frame.spotList.Append(meas);
	}

	return(1);
}
