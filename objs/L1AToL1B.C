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
#include "GenericGeom.h"


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

	//----------------------------//
	// ...free residual MeasSpots //
	//----------------------------//

	l15->frame.spotList.FreeContents();

	//------------------//
	// for each spot... //
	//------------------//

	for (int i = 0; i < SPOTS_PER_L10_FRAME; i++)
	{
		//------------------------//
		// ...generate a MeasSpot //
		//------------------------//

		MeasSpot* meas_spot = new MeasSpot;

		//-----------------------//
		// ...determine the beam //
		//-----------------------//

		int beam_idx = i % antenna->numberOfBeams;
		Beam* beam = &(antenna->beam[beam_idx]);

		//-----------------------//
		// ...calculate the time //
		//-----------------------//

		meas_spot->time = l10->frame.time + (i / antenna->numberOfBeams) *
			antenna->priPerBeam + beam->timeOffset;

		//------------------------------------------------------//
		// ...determine the spacecraft orbit state and attitude //
		//------------------------------------------------------//

		if (! ephemeris->GetOrbitState(l10->frame.time,
			&(meas_spot->scOrbitState)))
		{
			return(0);
		}
		meas_spot->scAttitude = l10->frame.attitude;

		//-----------------------------//
		// ...do geometry for the spot //
		//-----------------------------//

		antenna->SetAzimuthWithEncoder(l10->frame.antennaPosition[i]);

		CoordinateSwitch beam_frame_to_gc =
			BeamFrameToGC(&(meas_spot->scOrbitState), &(meas_spot->scAttitude),
			antenna, beam);

		//----------------------------------//
		// ...add measurements to spot list //
		//----------------------------------//

		Vector3 rlook_beam;
		rlook_beam.SphericalSet(1.0, 0.0, 0.0);
		Vector3 rlook_gc = beam_frame_to_gc.Forward(rlook_beam);

		EarthPosition spot_on_earth =
			 earth_intercept(meas_spot->scOrbitState.rsat, rlook_gc);

		Meas* meas = new Meas();
		meas->value = l10->frame.sigma0[i];
		double alt,lat,lon;
		if (spot_on_earth.GetAltLatLon(EarthPosition::GEODETIC, &alt,
				&lat, &lon) == 0)
		{
			printf("Error: L10ToL15 can't convert spot_on_earth\n");
			return(0);
		}
		meas->center.longitude = (float)lon;
		meas->center.latitude = (float)lat;
//		meas->outline = 
		meas->pol = antenna->beam[beam_idx].polarization;

		// get local measurement azimuth
		CoordinateSwitch gc_to_surface =
			spot_on_earth.SurfaceCoordinateSystem();
		Vector3 rlook_surface = gc_to_surface.Forward(rlook_gc);
		double r, theta, phi;
		rlook_surface.SphericalGet(&r, &theta, &phi);
		meas->eastAzimuth = phi;

		meas->incidenceAngle = spot_on_earth.IncidenceAngle(rlook_gc);
		meas->estimatedKp = 0.0;

		meas_spot->slices.Append(meas);
		l15->frame.spotList.Append(meas_spot);
	}

	return(1);
}
