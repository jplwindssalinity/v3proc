//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_l1atol1b_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include "PscatL1AToL1B.h"
#include "CheckFrame.h"
#include "InstrumentGeom.h"
#include "Sigma0.h"
#include "Pscat.h"

//===============//
// PscatL1AToL1B //
//===============//

PscatL1AToL1B::PscatL1AToL1B()
:   pulseCount(0), useKfactor(0), useBYUXfactor(0), useSpotCompositing(0),
    outputSigma0ToStdout(0), sliceGainThreshold(0.0), processMaxSlices(0)
{
    return;
}

PscatL1AToL1B::~PscatL1AToL1B()
{
    return;
}

//------------------------//
// PscatL1AToL1B::Convert //
//------------------------//

int
PscatL1AToL1B::Convert(
    PscatL1A*    l1a,
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

    //----------------------------//
    // ...free residual MeasSpots //
    //----------------------------//

    l1b->frame.spotList.FreeContents();

    //-----------//
    // predigest //
    //-----------//

    OrbitState* orbit_state = &(spacecraft->orbitState);
    Antenna* antenna = &(qscat->sas.antenna);
    PscatL1AFrame* frame = &(l1a->frame);

    //--------------------//
    // allocate workspace //
    //--------------------//

    float* Esn = (float*)malloc(sizeof(float)*frame->slicesPerSpot);
    if (! Esn)
    {
        printf("Error allocating memory in PscatL1AToL1B\n");
        return(0);
    }

    //------------------//
    // for each spot... //
    //------------------//

    for (int spot_idx = 0; spot_idx < frame->spotsPerFrame; spot_idx++)
    {
        // determine the spot meas location offset
        int spot_meas_offset = spot_idx * frame->measPerSpot;

        // determine the spot time
        double time = frame->time + spot_idx * qscat->ses.pri;

        // determine beam and beam index
        int beam_idx = spot_idx % NUMBER_OF_QSCAT_BEAMS;
        qscat->cds.currentBeamIdx = beam_idx;

        //-------------------//
        // set up spacecraft //
        //-------------------//

        if (! ephemeris->GetOrbitState(time+0.5*qscat->ses.txPulseWidth,
                EPHEMERIS_INTERP_ORDER, orbit_state))
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
            if (orbit_step == 0)
                orbit_step = ORBIT_STEPS - 1;
            else
                orbit_step--;
        }
        qscat->cds.orbitStep = orbit_step;

        //-------------//
        // set antenna //
        //-------------//

        unsigned short held_encoder = *(frame->antennaPosition + spot_idx);
        qscat->cds.heldEncoder = held_encoder;
        qscat->SetEncoderAzimuth(held_encoder, 1);
        qscat->SetOtherAzimuths(spacecraft);

        //----------------------------//
        // range and Doppler tracking //
        //----------------------------//

        SetDelayAndFrequency(spacecraft, qscat);

        if (outputSigma0ToStdout)
            printf("%g ", antenna->txCenterAzimuthAngle * rtd);

        //---------------------------//
        // create a measurement spot //
        //---------------------------//

        MeasSpot* meas_spot = new MeasSpot();
        meas_spot->time = time;

        //-------------------------------------------//
        // Extract energy measurements for this spot //
        //-------------------------------------------//

        float Esn_echo = 0.0;
        for (int slice_idx = 0; slice_idx < frame->slicesPerSpot;
            slice_idx++)
        {
            int slice_meas_offset = slice_idx * frame->measPerSlice;
            // co-pol measurements are first for the slice
            Esn[slice_idx] = frame->science[spot_meas_offset +
                slice_meas_offset];
            Esn_echo += Esn[slice_idx];
        }
        
        // Fetch the noise measurement which applies to all the slices.
        float Esn_noise = frame->spotNoise[spot_idx];

        //---------------------//
        // locate measurements //
        //---------------------//

        if (frame->slicesPerSpot <= 1)
        {
            if (! qscat->LocateSpot(spacecraft, meas_spot, Esn[0]))
            {
                return(0);
            }
        }
        else
        {
            if (! qscat->LocateSliceCentroids(spacecraft, meas_spot, Esn,
                sliceGainThreshold, processMaxSlices))
            {
                return(0);
            }
        }

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
            antenna, antenna->txCenterAzimuthAngle);
        CoordinateSwitch gc_to_antenna =
            antenna_frame_to_gc.ReverseDirection();

        int slice_count = qscat->ses.GetTotalSliceCount();

        //----------------------------//
        // determine measurement type //
        //----------------------------//

        PscatEvent::PscatEventE event = frame->event[spot_idx];

        //------------------------------------------------------//
        // set measurement types, add polarimetric measurements //
        //------------------------------------------------------//

        Meas* new_meas;
        for (Meas* meas = meas_spot->GetHead(); meas;
            meas = meas_spot->GetNext())
        {
            meas->scanAngle = qscat->sas.antenna.txCenterAzimuthAngle;
            meas->beamIdx = qscat->cds.currentBeamIdx;
            meas->txPulseWidth = qscat->ses.txPulseWidth;

            switch (event)
            {
            case PscatEvent::VV_SCAT_EVENT:
                meas->measType = Meas::VV_MEAS_TYPE;
                break;
            case PscatEvent::HH_SCAT_EVENT:
                meas->measType = Meas::HH_MEAS_TYPE;
                break;
            case PscatEvent::VV_VH_SCAT_EVENT:
                meas->measType = Meas::VV_MEAS_TYPE;
                new_meas = new Meas();
                *new_meas = *meas;
                new_meas->measType = Meas::VV_VH_CORR_MEAS_TYPE;
                meas_spot->InsertAfter(new_meas);
                break;
            case PscatEvent::HH_HV_SCAT_EVENT:
                meas->measType = Meas::HH_MEAS_TYPE;
                new_meas = new Meas();
                *new_meas = *meas;
                new_meas->measType = Meas::HH_HV_CORR_MEAS_TYPE;
                meas_spot->InsertAfter(new_meas);
                break;
            default:
                return(0);
                break;
            }
        }

        //-------------------------//
        // for each measurement... //
        //-------------------------//

        for (Meas* meas = meas_spot->GetHead(); meas;
            meas = meas_spot->GetNext())
        {
            //--------------------------------------//
            // determine some calibration constants //
            //--------------------------------------//

            float Es_cal = 0.0;
            float En_echo_load = 0.0;
            float En_noise_load = 0.0;

            int slice_i;
            if (!rel_to_abs_idx(meas->startSliceIdx, slice_count, &slice_i))
            {
                fprintf(stderr, "PscatL1AToL1B::Convert, Bad slice number\n");
                exit(1);
            }

            // Kfactor: either 1.0 or taken from table
			float k_factor=1.0;
			float x_factor=1.0;
            float Es_slice,En_slice;
			float Esn_slice = meas->value;
			float PtGr = l1a->frame.ptgr;

			if (useKfactor)
			{
				float orbit_position = qscat->cds.OrbitFraction();

				k_factor = kfactorTable.RetrieveByRelativeSliceNumber(
					qscat->cds.currentBeamIdx,
                    qscat->sas.antenna.txCenterAzimuthAngle, orbit_position,
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
			else if (useBYUXfactor)
            {
                x_factor = BYUX.GetXTotal(spacecraft, qscat, meas, Es_cal,
                    NULL);

			    //-----------------//
			    // set measurement //
			    //-----------------//
			    
			    // meas->value is the Esn value going in
			    // sigma0 coming out.
			    if (! compute_sigma0(qscat, meas, x_factor, Esn_slice,
						 Esn_echo, Esn_noise, En_echo_load, En_noise_load,
                         &Es_slice, &En_slice))
                {
                    return(0);
                }
            }
            else
            {
			    fprintf(stderr,
				    "PscatL1AToL1B::Convert:No X compuation algorithm set\n");
			    exit(0);
            }
			
			//----------------------------------//
			// Print calculated sigma0 values	//
			// to stdout.						//
			//----------------------------------//

			if (outputSigma0ToStdout)
				printf("%g ",meas->value);
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

	    //----------------------//
	    // add to list of spots //
	    //----------------------//

	    l1b->frame.spotList.Append(meas_spot);
        pulseCount++;
    }

    free(Esn);
    Esn = NULL;

    if (outputSigma0ToStdout)
    {
      printf("\n");
    }

    return(1);
}
