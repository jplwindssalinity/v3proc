//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_l1atol1b_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include "L1AToL1B.h"
#include "CheckFrame.h"
#include "InstrumentGeom.h"
#include "Sigma0.h"

//==========//
// L1AToL1B //
//==========//

L1AToL1B::L1AToL1B()
:   useKfactor(0), useBYUXfactor(0), useSpotCompositing(0),
    outputSigma0ToStdout(0), sliceGainThreshold(0.0), processMaxSlices(0),
    simVs1BCheckfile(NULL), Esn_echo_cal(0.0), Esn_noise_cal(0.0),
    En_echo_load(0.0), En_noise_load(0.0)
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
    L1A*         l1a,
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    Ephemeris*   ephemeris,
    L1B*         l1b)
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

	qscat->cds.SetTimeWithInstrumentTime(l1a->frame.instrumentTicks);
	qscat->cds.orbitTime = l1a->frame.orbitTicks;

	//------------------------------------//
	// Extract and prepare cal pulse data //
	//------------------------------------//

    if (l1a->frame.calPosition != 255)
    {
        Esn_echo_cal = 0.0;
        En_echo_load = 0.0;
        for (int i=0; i < l1a->frame.slicesPerSpot; i++)
        {
            Esn_echo_cal += l1a->frame.loopbackSlices[i];
            En_echo_load += l1a->frame.loadSlices[i];
        }
        Esn_noise_cal = l1a->frame.loopbackNoise;
        En_noise_load = l1a->frame.loadNoise;
    }
    else if (Esn_echo_cal == 0.0)
    {
        // Make first frame cal pulse data using true PtGr.
        float PtGr = qscat->ses.transmitPower * qscat->ses.rxGainEcho;
        PtGr_to_Esn(PtGr,NULL,qscat,0,&Esn_echo_cal,&Esn_noise_cal);
        make_load_measurements(qscat,&En_echo_load,&En_noise_load);
    }

    //----------------------------------------------------------//
    // Estimate cal (loopback) signal and noise energies.       //
    // Kpr noise shows up in the cal signal energy.             //
    // Note that these are spot quantities (not slices).        //
    // If there isn't any cal pulse data, the preceeding values //
    // will be used.                                            //
    //----------------------------------------------------------//

    double beta = qscat->ses.rxGainNoise / qscat->ses.rxGainEcho;
    float Es_cal,En_cal;
    if (! Er_to_Es(beta, Esn_echo_cal, Esn_echo_cal, Esn_noise_cal,
        En_echo_load, En_noise_load, 1.0, &Es_cal, &En_cal))
    {
        return(0);
    }

    //----------------------------//
    // ...free residual MeasSpots //
    //----------------------------//

    l1b->frame.spotList.FreeContents();

	//-----------//
	// predigest //
	//-----------//

	OrbitState* orbit_state = &(spacecraft->orbitState);
	Antenna* antenna = &(qscat->sas.antenna);
    L1AFrame* frame = &(l1a->frame);

	//------------------//
	// for each spot... //
	//------------------//

    for (int spot_idx = 0; spot_idx < frame->spotsPerFrame; spot_idx++)
    {
        // determine the spot time
        double time = frame->time + spot_idx * qscat->ses.pri;

        // determine beam and beam index
        int beam_idx = spot_idx % NUMBER_OF_QSCAT_BEAMS;
        qscat->cds.currentBeamIdx = beam_idx;

        // determine starting slice index
        int base_slice_idx = spot_idx * frame->slicesPerSpot;

        //-------------------//
        // set up spacecraft //
        //-------------------//

        if (! ephemeris->GetOrbitState(time, EPHEMERIS_INTERP_ORDER,
            orbit_state))
        {
            return(0);
        }

        //----------------//
        // set orbit step //
        //----------------//

        unsigned int orbit_step = frame->orbitStep;
        if (frame->priOfOrbitStepChange != 255 &&
            spot_idx < frame->priOfOrbitStepChange)
        {
            orbit_step--;
        }
        qscat->cds.orbitStep = orbit_step;

        //----------------------------//
        // range and Doppler tracking //
        //----------------------------//

        // determine the CDS tracking azimuth angle
        unsigned short held_encoder = *(frame->antennaPosition + spot_idx);
        qscat->cds.heldEncoder = held_encoder;    // for tracking
        SetDelayAndFrequency(spacecraft, qscat);

        if (outputSigma0ToStdout)
            printf("%g ", antenna->azimuthAngle * rtd);

        //---------------------------//
        // create a measurement spot //
        //---------------------------//

		MeasSpot* meas_spot = new MeasSpot();
        meas_spot->time = time;

		//-------------------------------------------//
		// Extract energy measurements for this spot //
		//-------------------------------------------//

		float* Esn = (float*)malloc(sizeof(float)*l1a->frame.slicesPerSpot);
		if (! Esn)
		{
			printf("Error allocating memory in L1AToL1B\n");
			return(0);
		}

		float Esn_echo = 0.0;
		for (int i=0; i < l1a->frame.slicesPerSpot; i++)
		{
			Esn[i] = l1a->frame.science[base_slice_idx + i];
			// Sum up the signal+noise measurements
			Esn_echo += Esn[i];
		}

		// Fetch the noise measurement which applies to all the slices.
		float Esn_noise = l1a->frame.spotNoise[spot_idx];

		//---------------------//
		// locate measurements //
		//---------------------//

        // correctly locate antenna first
        qscat->sas.SetAzimuthWithEncoder(held_encoder);
        qscat->RotateAntennaToTxCenter(1);
		if (l1a->frame.slicesPerSpot <= 1)
		{
            if (! LocateSpot(spacecraft, qscat, meas_spot, Esn[0]))
            {
                return(0);
            }
		}
		else
        {
            if (! LocateSliceCentroids(spacecraft, qscat, meas_spot, Esn,
                sliceGainThreshold, processMaxSlices))
            {
                return(0);
            }
        }

		free(Esn);
		Esn = NULL;

        for (Meas* meas = meas_spot->GetHead(); meas;
            meas = meas_spot->GetNext())
        {
            double alt,lat,lon;
            if (! meas->centroid.GetAltLonGDLat(&alt, &lon, &lat))
                return(0);

            // Compute Land Flag
            meas->landFlag = landMap.IsLand(lon, lat);
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
			float Esn_slice = meas->value;
			float PtGr = l1a->frame.ptgr;

			if (useKfactor)
			{
				float orbit_position = qscat->cds.OrbitFraction();

				k_factor = kfactorTable.RetrieveByRelativeSliceNumber(
					qscat->cds.currentBeamIdx,
					qscat->sas.antenna.azimuthAngle, orbit_position,
					meas->startSliceIdx);

				//-----------------//
				// set measurement //
				//-----------------//

				// meas->value is the Esn value going in, sigma0 coming out.
				if (! Er_to_sigma0(&gc_to_antenna, spacecraft, qscat, meas,
                    k_factor, meas->value, Esn_echo, Esn_noise, PtGr))
                {
                    return(0);
                }
			}
			else if(useBYUXfactor)
			  {
			    x_factor = BYUX.GetXTotal(spacecraft, qscat, meas, Es_cal);

			    //-----------------//
			    // set measurement //
			    //-----------------//
			    
			    // meas->value is the Esn value going in
			    // sigma0 coming out.
			    if (! compute_sigma0(qscat, meas, x_factor, Esn_slice,
						 Esn_echo, Esn_noise, En_echo_load, En_noise_load))
			      {
				return(0);
			      }
			  }
			else
			  {
			    fprintf(stderr,
				    "L1AToL1B::Convert:No X compuation algorithm set\n");
			    exit(0);
			  }
			
            // scan angle is at the center of the Tx pulse
			meas->scanAngle = qscat->sas.antenna.azimuthAngle;
			meas->beamIdx = qscat->cds.currentBeamIdx;
			meas->txPulseWidth = qscat->ses.txPulseWidth;

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

			//------------------------//
			// Output data if enabled //
			//------------------------//

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
		}
		//----------------------//
		// add to list of spots //
		//----------------------//

		l1b->frame.spotList.Append(meas_spot);
    }
    if (outputSigma0ToStdout){
      printf("\n");
    }

    return(1);
}
