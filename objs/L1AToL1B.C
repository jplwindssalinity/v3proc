//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l10tol15_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include "L10ToL15.h"
#include "InstrumentGeom.h"
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
	L10*			l10,
	Spacecraft*		spacecraft,
	Instrument*		instrument,
	Ephemeris*		ephemeris,
	L15*			l15)
{
	//--------------//
	// unpack frame //
	//--------------//

	l10->frame.Unpack(l10->buffer);

	//-------------------//
	// set up spacecraft //
	//-------------------//

	float roll = l10->frame.attitude.GetRoll();
	float pitch = l10->frame.attitude.GetPitch();
	float yaw = l10->frame.attitude.GetYaw();
	spacecraft->attitude.SetRPY(roll, pitch, yaw);

	//----------------------------//
	// ...free residual MeasSpots //
	//----------------------------//

	l15->frame.spotList.FreeContents();

	//------------------------//
	// for each beam cycle... //
	//------------------------//

	int total_slice_idx = 0;
	int spot_idx = 0;

	OrbitState* orbit_state = &(spacecraft->orbitState);

	for (int beam_cycle = 0; beam_cycle < l10->frame.antennaCyclesPerFrame;
		beam_cycle++)
	{
		//------------------//
		// for each beam... //
		//------------------//

		Antenna* antenna = &(instrument->antenna);

		for (int beam_idx = 0; beam_idx < antenna->numberOfBeams;
			beam_idx++)
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

			if(XMGROUT) printf("%g ",antenna->azimuthAngle/dtr);

			//---------------------------// 
			// create a measurement spot //
			//---------------------------// 

			MeasSpot* meas_spot = new MeasSpot();

			//---------------------//
			// locate measurements //
			//---------------------//

			if (l10->frame.slicesPerSpot <= 1)
			{
				if (! LocateSpot(time, spacecraft, instrument, meas_spot))
					return(0);
			}
			else
			{
				if (! LocateSlices(time, spacecraft, instrument,
					l10->frame.slicesPerSpot, meas_spot))
				{
					return(0);
				}
			}

			//----------------------------------------//
			// generate the reverse coordinate switch //
			//----------------------------------------//
			// duplicate work: forward transform already calc'd in Locate*

			CoordinateSwitch antenna_frame_to_gc =
				AntennaFrameToGC(orbit_state, &(spacecraft->attitude),
				antenna);
			CoordinateSwitch gc_to_antenna =
				antenna_frame_to_gc.ReverseDirection();

			// Sum up the signal+noise measurements
			float sumPsn = 0.0;
			for (int i=0; i < l10->frame.slicesPerSpot; i++)
			{
				sumPsn += l10->frame.science[total_slice_idx + i];
			}

			// Fetch the noise measurement which applies to all the slices.
			float Pn = l10->frame.spotNoise[spot_idx];

			//-------------------//
			// for each slice... //
			//-------------------//

			for (Meas* meas = meas_spot->GetHead(); meas;
				meas = meas_spot->GetNext())
			{
				float k_factor = 1.0;
				float Psn = l10->frame.science[total_slice_idx];
				float sigma0;
				if (! Pr_to_sigma0(&gc_to_antenna, spacecraft, instrument,
					meas, k_factor, Psn, sumPsn, Pn, &sigma0))
				{
					return(0);
				}				

				//----------------------------------//
				// Print calculated sigma0 values   //
				// to stdout.			    //
				//----------------------------------//

				if (XMGROUT) printf("%g ",1.0-sigma0);

				//-----------------//
				// set measurement //
				//-----------------//

				meas->value = sigma0;
				total_slice_idx++;
			}

			l15->frame.spotList.Append(meas_spot);
			spot_idx++;
		}
		if (XMGROUT) printf("\n");
	}

	return(1);
}

