//==========================================================//
// Copyright (C) 1997, California Institute of Technology.  //
// U.S. Government sponsorship acknowledged.		    //
//==========================================================//

static const char rcs_id_l10tol15_c[] =
	"@(#) $Id$";

#include "L10ToL15.h"
#include "Antenna.h"
#include "Ephemeris.h"
#include "InstrumentGeom.h"
#include "GenericGeom.h"
#include "Sigma0.h"

#define XMGROUT	0	// Output sigma0 values to stdout? 1/0=YES/NO

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
	Spacecraft*	spacecraft,
	Instrument*	instrument,
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

	//-----------//
	// predigest //
	//-----------//


	Antenna* antenna = &(instrument->antenna);
	OrbitState* orbit_state = &(spacecraft->orbitState);
	Attitude* attitude = &(spacecraft->attitude);

	//---------------------------//
	// determine slice frequency //
	//---------------------------//
	// these should probably go into the instrument object

	int slice_count = l10->frame.slicesPerSpot;
	float total_freq = slice_count * instrument->sliceBandwidth;
	float min_freq = -total_freq / 2.0;

	//-------------------//
	// set up spacecraft //
	//-------------------//

	float roll,pitch,yaw;
	roll = l10->frame.attitude.GetRoll();
	pitch = l10->frame.attitude.GetPitch();
	yaw = l10->frame.attitude.GetYaw();
	attitude->SetRPY(roll,pitch,yaw);

	//------------------------//
	// for each beam cycle... //
	//------------------------//

	int total_slice_idx = 0;
	int spot_idx = 0;

	for (int beam_cycle = 0; beam_cycle < l10->frame.antennaCyclesPerFrame;
		beam_cycle++)
	{
		//------------------//
		// for each beam... //
		//------------------//

		for (int beam_idx = 0; beam_idx < antenna->numberOfBeams; beam_idx++)
		{
			//--------------------//
			// calculate the time //
			//--------------------//

			antenna->currentBeamIdx = beam_idx;
			Beam* beam = antenna->GetCurrentBeam();

			double time = l10->frame.time + beam_cycle * antenna->priPerBeam +
				beam->timeOffset;

			//-------------------//
			// set up spacecraft //
			//-------------------//

			if (! ephemeris->GetOrbitState(time, EPHEMERIS_INTERP_ORDER,
				orbit_state))
			{
				return(0);
			}

			//-------------------//
			// set up instrument //
			//-------------------//

			instrument->time = time;
			antenna->SetAzimuthWithEncoder(
				l10->frame.antennaPosition[spot_idx]);
			// commanded doppler
			// commanded receiver gate delay

			if(XMGROUT) printf("%g ",antenna->azimuthAngle);

			//----------------------------//
			// generate coordinate switch //
			//----------------------------//

			CoordinateSwitch antenna_frame_to_gc =
				AntennaFrameToGC(orbit_state, attitude, antenna);

			//---------------//
			// get boresight //
			//---------------//

			double look, azimuth;
			if (! beam->GetElectricalBoresight(&look, &azimuth))
				return(0);

			//-------------------------------------------------//
			// calculate ideal Doppler and range tracking info //
			//-------------------------------------------------//
 
			Vector3 vector;
			vector.SphericalSet(1.0, look, azimuth);	//boresight
			DopplerAndDelay(&antenna_frame_to_gc, spacecraft, instrument, vector);

			//-------------------------//
			// make a measurement spot //
			//-------------------------//

			MeasSpot* meas_spot = new MeasSpot();
			meas_spot->time = time;
			meas_spot->scOrbitState = *orbit_state;
			meas_spot->scAttitude = *attitude;

			//-------------------//
			// for each slice... //
			//-------------------//

			for (int slice_idx = 0; slice_idx < l10->frame.slicesPerSpot;
				slice_idx++)
			{
				//--------------------//
				// make a measurement //
				//--------------------//

				Meas* meas = new Meas();
				meas->pol = beam->polarization;

				//----------------------------------------//
				// determine the baseband frequency range //
				//----------------------------------------//
 
				float f1 = min_freq + slice_idx * instrument->sliceBandwidth;
				float f2 = f1 + instrument->sliceBandwidth;

				//----------------//
				// find the slice //
				//----------------//
 
				EarthPosition centroid;
				Vector3 look_vector;
				// guess at a reasonable slice frequency tolerance of 1%
				float ftol = fabs(f1 - f2) / 100.0;
				if (! FindSlice(&antenna_frame_to_gc, spacecraft, instrument,
					look, azimuth, f1, f2, ftol, &(meas->outline),
					&look_vector, &centroid))
				{
					return(0);
				}

				meas->centroid = centroid;

				//---------------------------//
				// generate measurement data //
				//---------------------------//
 
				// get local measurement azimuth
				CoordinateSwitch gc_to_surface =
					centroid.SurfaceCoordinateSystem();
				Vector3 rlook_surface = gc_to_surface.Forward(look_vector);
				double r, theta, phi;
				rlook_surface.SphericalGet(&r, &theta, &phi);
				meas->eastAzimuth = phi;
 
				// get incidence angle
				meas->incidenceAngle = centroid.IncidenceAngle(look_vector);

				// Calculate Sigma0 from the received Power

				// Eventually Kfactor should be computed (read from table)
				float Kfactor=1.0;
				float sigma0, Pr;
				Pr=l10->frame.science[total_slice_idx];
				CoordinateSwitch gc_to_antenna =
					antenna_frame_to_gc.ReverseDirection();

				if(! Pr_to_sigma0(spacecraft, instrument,
					meas, Kfactor, &gc_to_antenna,
					Pr, &sigma0)) return(0);	
			

				//----------------------------------//
				// Print calculated sigma0 values   //
				// to stdout.                       //
				//----------------------------------//

				if (XMGROUT) printf("%g ",1.0-sigma0);

				//-----------------//
				// add measurement //
				//-----------------//
				meas->value=sigma0;
				meas_spot->slices.Append(meas);
				total_slice_idx++;
			}
			l15->frame.spotList.Append(meas_spot);
			spot_idx++;
			if (XMGROUT) printf("\n");
		}
	}

	return(1);
}
