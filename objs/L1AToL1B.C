//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l1atol1b_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include "L1AToL1B.h"
#include "InstrumentGeom.h"
#include "Sigma0.h"
#include "InstrumentSim.h"
#include "CheckFrame.h"

//==========//
// L1AToL1B //
//==========//

L1AToL1B::L1AToL1B()
:	useKfactor(0), useBYUXfactor(0), useSpotCompositing(0), 
	outputSigma0ToStdout(0),
	sliceGainThreshold(0.0), processMaxSlices(0), simVs1BCheckfile(NULL)
{
	return;
}

L1AToL1B::~L1AToL1B()
{
	return;
}

//-------------------//
// L1AToL1B::Convert //
//-------------------//

int
L1AToL1B::Convert(
	L1A*			l1a,
	Spacecraft*		spacecraft,
	Instrument*		instrument,
	Ephemeris*		ephemeris,
	L1B*			l1b)
{
	//--------------//
	// unpack frame //
	//--------------//

	l1a->frame.Unpack(l1a->buffer);

	//-------------------//
	// set up check data //
	//-------------------//

	CheckFrame cf;
	if (simVs1BCheckfile)
	{
		if (!cf.Allocate(l1a->frame.slicesPerSpot))
        {
			fprintf(stderr,"Error allocating a CheckFrame\n");
            return(0);
        }
	}

	//-------------------//
	// set up spacecraft //
	//-------------------//

	float roll = l1a->frame.attitude.GetRoll();
	float pitch = l1a->frame.attitude.GetPitch();
	float yaw = l1a->frame.attitude.GetYaw();
	spacecraft->attitude.SetRPY(roll, pitch, yaw);

	//-------------------//
	// set up instrument //
	//-------------------//

	instrument->SetTimeWithInstrumentTicks(l1a->frame.instrumentTicks);
	instrument->orbitTicks = l1a->frame.orbitTicks;

	//----------------------------//
	// ...free residual MeasSpots //
	//----------------------------//

	l1b->frame.spotList.FreeContents();

	//-----------//
	// predigest //
	//-----------//

	OrbitState* orbit_state = &(spacecraft->orbitState);
	Antenna* antenna = &(instrument->antenna);

	//------------------------//
	// for each beam cycle... //
	//------------------------//

	int base_slice_idx = 0;
	int spot_idx = 0;

	for (int beam_cycle = 0; beam_cycle < l1a->frame.antennaCyclesPerFrame;
		beam_cycle++)
	{
		//------------------//
		// for each beam... //
		//------------------//

		for (int beam_idx = 0; beam_idx < antenna->numberOfBeams;
			beam_idx++)
		{
			//--------------------//
			// calculate the time //
			//--------------------//

			antenna->currentBeamIdx = beam_idx;
			Beam* beam = antenna->GetCurrentBeam();
			double time = l1a->frame.time + beam_cycle * antenna->priPerBeam +
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

			if (spot_idx == l1a->frame.priOfOrbitTickChange)
				instrument->orbitTicks++;

			antenna->SetAzimuthWithEncoder(
				l1a->frame.antennaPosition[spot_idx]);

			if (outputSigma0ToStdout)
				printf("%g ",antenna->azimuthAngle/dtr);

			//---------------------------//
			// create a measurement spot //
			//---------------------------//

			MeasSpot* meas_spot = new MeasSpot();

			//-------------------------------------------------------------//
			// command the range delay, range width, and Doppler frequency //
			//-------------------------------------------------------------//

			SetRangeAndDoppler(spacecraft, instrument);

			//-------------------------------------------//
			// Extract energy measurements for this spot //
			//-------------------------------------------//

			float* Esn = (float*)malloc(sizeof(float)*l1a->frame.slicesPerSpot);
			if (! Esn)
			{
				printf("Error allocating memory in L1AToL1B\n");
				return(0);
			}

			float sumEsn = 0.0;
			for (int i=0; i < l1a->frame.slicesPerSpot; i++)
			{
				Esn[i] = l1a->frame.science[base_slice_idx + i];
				// Sum up the signal+noise measurements
				sumEsn += Esn[i];
			}

			// Fetch the noise measurement which applies to all the slices.
			float En = l1a->frame.spotNoise[spot_idx];

			//---------------------//
			// locate measurements //
			//---------------------//

			if (l1a->frame.slicesPerSpot <= 1)
			{
				if (! LocateSpot(spacecraft, instrument, meas_spot, Esn[0]))
					return(0);
			}
			else
			{
				if (! LocateSliceCentroids(spacecraft, instrument, meas_spot,
					Esn, sliceGainThreshold, processMaxSlices))
				{
					return(0);
				}
			}

			free(Esn);
			Esn = NULL;

                        for(Meas* meas=meas_spot->GetHead();meas;meas=meas_spot->GetNext()){  
			  double alt,lat,lon;
			  if (! meas->centroid.GetAltLonGDLat(&alt, &lon, &lat))
			    return(0);
			  
			  LonLat lon_lat;
			  lon_lat.longitude = lon;
			  lon_lat.latitude = lat;
			
			  // Compute Land Flag
			  meas->landFlag=landMap.IsLand(lon,lat);
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

			//-------------------//
			// for each slice... //
			//-------------------//

			int slice_i = 0;
			for (Meas* meas = meas_spot->GetHead(); meas;
				meas = meas_spot->GetNext())
			{
				// Kfactor: either 1.0 or taken from table
				float k_factor=1.0;
                                float x_factor=1.0;
				float PtGr = l1a->frame.ptgr;
				if (useKfactor)
				{
					float orbit_position = instrument->OrbitFraction();

					k_factor = kfactorTable.RetrieveByRelativeSliceNumber(
						instrument->antenna.currentBeamIdx,
						instrument->antenna.azimuthAngle, orbit_position,
						meas->startSliceIdx);

					//-----------------//
					// set measurement //
					//-----------------//

					// meas->value is the Esn value going in, sigma0 coming out.
					if (! Er_to_sigma0(&gc_to_antenna, spacecraft, instrument,
							   meas, k_factor, meas->value, sumEsn, En, PtGr))
					  {
					    return(0);
					  }
				}
				else if(useBYUXfactor){
				  x_factor=BYUX.GetXTotal(spacecraft,instrument,meas,PtGr);
				  //-----------------//
				  // set measurement //
				  //-----------------//

				  // meas->value is the Esn value going in, sigma0 coming out.
				  if (! Er_to_sigma0_given_X(instrument, meas, x_factor, meas->value, 
							     sumEsn, En))
				    {
				      return(0);
				    }
				}
				else{
				  fprintf(stderr,"L1AToL1B::Convert:No X compuation algorithm set\n");
				  exit(0);
				}



				meas->scanAngle = instrument->antenna.azimuthAngle;
				meas->beamIdx = instrument->antenna.currentBeamIdx;
				meas->txPulseWidth = beam->txPulseWidth;

				//------------------//
				// store check data //
				//------------------//

        		if (simVs1BCheckfile)
        		{
					cf.sigma0[slice_i] = meas->value;
            		cf.XK[slice_i] = meas->XK;
            		cf.centroid[slice_i] = meas->centroid;
            		cf.azimuth[slice_i] = meas->eastAzimuth;
            		cf.incidence[slice_i] = meas->incidenceAngle;
        		}

				//----------------------------------//
				// Print calculated sigma0 values	//
				// to stdout.						//
				//----------------------------------//

				if (outputSigma0ToStdout)
					printf("%g ",meas->value);

				slice_i++;
			}

			//-----------------------------------------------------//
			// composite into single spot measurement if necessary //
			//-----------------------------------------------------//

			if (useSpotCompositing)
			{
				Meas* comp = new Meas();
				if (comp == NULL)
					return(0);

				if (! comp->Composite(meas_spot))
					return(0);

				meas_spot->FreeContents();
				if (! meas_spot->Append(comp))
					return(0);
			}

        	//--------------------------------//
        	// Output data if enabled //
        	//--------------------------------//

	    	if (simVs1BCheckfile)
   	 		{
   	    		FILE* fptr = fopen(simVs1BCheckfile,"a");
   	    		if (fptr == NULL)
        		{
            		fprintf(stderr,"Error opening %s\n",simVs1BCheckfile);
            		exit(-1);
        		}
				cf.ptgr = l1a->frame.ptgr;
        		cf.time = time;
        		cf.rsat = spacecraft->orbitState.rsat;
        		cf.vsat = spacecraft->orbitState.vsat;
        		cf.attitude = spacecraft->attitude;
        		cf.AppendRecord(fptr);
        		fclose(fptr);
    		}

			//----------------------//
			// add to list of spots //
			//----------------------//

			l1b->frame.spotList.Append(meas_spot);
			spot_idx++;
			base_slice_idx += l1a->frame.slicesPerSpot;
		}
		if (outputSigma0ToStdout) printf("\n");

	}

	return(1);
}



