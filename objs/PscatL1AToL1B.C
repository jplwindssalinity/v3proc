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
    outputSigma0ToStdout(0), sliceGainThreshold(0.0), processMaxSlices(0),
    simVs1BCheckfile(NULL), Esn_echo_cal(0.0), Esn_noise_cal(0.0),
    En_echo_load(0.0), En_noise_load(0.0)
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
    Pscat*       pscat,
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
        if (! cf.Allocate(l1a->frame.slicesPerSpot))
        {
            fprintf(stderr, "Error allocating a CheckFrame\n");
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

    pscat->cds.SetTimeWithInstrumentTime(l1a->frame.instrumentTicks);
    pscat->cds.orbitTime = l1a->frame.orbitTicks;

    //------------------------------------//
    // Extract and prepare cal pulse data //
    // Note 8 bit shift for loopback's    //
    //------------------------------------//

    if (l1a->frame.calPosition != 255)
    {
        Esn_echo_cal = 0.0;
        En_echo_load = 0.0;
        for (int i = 0; i < l1a->frame.slicesPerSpot; i++)
        {
            Esn_echo_cal += l1a->frame.loopbackSlices[i]*256.0;
            En_echo_load += l1a->frame.loadSlices[i];
        }
        Esn_noise_cal = l1a->frame.loopbackNoise;
        En_noise_load = l1a->frame.loadNoise;
    }
    else if (Esn_echo_cal == 0.0)
    {
        // Make first frame cal pulse data using true PtGr.
        PtGr_to_Esn(NULL, pscat, 0, &Esn_echo_cal, &Esn_noise_cal);
        make_load_measurements(pscat, &En_echo_load, &En_noise_load);
    }

    //----------------------------------------------------------//
    // Estimate cal (loopback) signal and noise energies.       //
    // Kpr noise shows up in the cal signal energy.             //
    // Note that these are spot quantities (not slices).        //
    // If there isn't any cal pulse data, the preceeding values //
    // will be used.                                            //
    //----------------------------------------------------------//

    double beta = pscat->ses.rxGainNoise / pscat->ses.rxGainEcho;
    float Es_cal, En_cal;
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
    Antenna* antenna = &(pscat->sas.antenna);
    PscatL1AFrame* frame = &(l1a->frame);

    //------------------//
    // for each spot... //
    //------------------//

    for (int spot_idx = 0; spot_idx < frame->spotsPerFrame; spot_idx++)
    {
        if (simVs1BCheckfile)
        {
            // cf.idx needs to start out zeroed for each spot
            cf.Initialize();
        }

        //--------------------------//
        // skip loopbacks and loads //
        //--------------------------//

        if (spot_idx == frame->calPosition - 2 ||
            spot_idx == frame->calPosition - 1)
        {
            pulseCount++;
            continue;
        }

        // determine the spot meas location offset
        int spot_slice_offset = spot_idx * frame->slicesPerSpot;

        // determine the spot time
        double time = frame->time + spot_idx * pscat->ses.pri;

        // determine beam and beam index
        int beam_idx = spot_idx % NUMBER_OF_QSCAT_BEAMS;
        pscat->cds.currentBeamIdx = beam_idx;

        //-------------------//
        // set up spacecraft //
        //-------------------//

        if (! ephemeris->GetOrbitState(time + 0.5*pscat->ses.txPulseWidth,
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
        pscat->cds.orbitStep = orbit_step;

        //-------------//
        // set antenna //
        //-------------//

        unsigned short held_encoder = *(frame->antennaPosition + spot_idx);
        pscat->cds.heldEncoder = held_encoder;
        pscat->SetEncoderAzimuth(held_encoder, 1);
        pscat->SetOtherAzimuths(spacecraft);

        //----------------------------//
        // range and Doppler tracking //
        //----------------------------//

        SetDelayAndFrequency(spacecraft, pscat);

        if (outputSigma0ToStdout)
            printf("%g ", antenna->txCenterAzimuthAngle * rtd);

        //---------------------------//
        // create a measurement spot //
        //---------------------------//

        MeasSpot* meas_spot = new MeasSpot();
        meas_spot->time = time;

        //----------------------------------------//
        // create slice measurements for the spot //
        //----------------------------------------//

        if (! pscat->MakeSlices(meas_spot))
            return(0);

        //---------------------//
        // locate measurements //
        //---------------------//

        if (frame->slicesPerSpot <= 1)
        {
            if (! pscat->LocateSpot(spacecraft, meas_spot))
            {
                return(0);
            }
        }
        else
        {
            if (! pscat->LocateSliceCentroids(spacecraft, meas_spot,
                 sliceGainThreshold, processMaxSlices))
            {
                return(0);
            }
        }

        //--------------------//
        // set the land flags //
        //--------------------//

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

        int slice_count = pscat->ses.GetTotalSliceCount();

        //-------------------------------------------//
        // determine event type from telemetry frame //
        //-------------------------------------------//

        PscatEvent::PscatEventE eventId =
            (PscatEvent::PscatEventE)frame->eventId[spot_idx];

        //------------------------------------------------------//
        // set measurement types, add polarimetric measurements //
        //------------------------------------------------------//

        Meas* new_meas;
        for (Meas* meas = meas_spot->GetHead(); meas;
            meas = meas_spot->GetNext())
        {
            meas->scanAngle = pscat->sas.antenna.txCenterAzimuthAngle;
            meas->beamIdx = pscat->cds.currentBeamIdx;
            meas->txPulseWidth = pscat->ses.txPulseWidth;

            switch (eventId)
            {
            case PscatEvent::VV_SCAT_EVENT:
                meas->measType = Meas::VV_MEAS_TYPE;
                break;
            case PscatEvent::HH_SCAT_EVENT:
                meas->measType = Meas::HH_MEAS_TYPE;
                break;
            case PscatEvent::VV_HV_SCAT_EVENT:
                meas->measType = Meas::VV_MEAS_TYPE;
                new_meas = new Meas();
                *new_meas = *meas;
                new_meas->measType = Meas::VV_HV_CORR_MEAS_TYPE;
                meas_spot->InsertAfter(new_meas);
                break;
            case PscatEvent::HH_VH_SCAT_EVENT:
                meas->measType = Meas::HH_MEAS_TYPE;
                new_meas = new Meas();
                *new_meas = *meas;
                new_meas->measType = Meas::HH_VH_CORR_MEAS_TYPE;
                meas_spot->InsertAfter(new_meas);
                break;
            default:
                fprintf(stderr,
                    "PscatL1AToL1B::Convert: unknown event type %d\n",
                    eventId);
                return(0);
                break;
            }
        }

        //--------------------------------------------------//
        // Extract energy measurements for the measurements //
        //--------------------------------------------------//

        float Esn_echo = 0.0;

        for (Meas* meas = meas_spot->GetHead(); meas;
            meas = meas_spot->GetNext())
        {
            int slice_idx;
            rel_to_abs_idx(meas->startSliceIdx, slice_count, &slice_idx);
            switch(meas->measType)
            {
            case Meas::VV_MEAS_TYPE:
            case Meas::HH_MEAS_TYPE:
                meas->value = (float)frame->copol[spot_slice_offset +
                    slice_idx] + 0.5;
                Esn_echo += meas->value;
                break;
            case Meas::VV_HV_CORR_MEAS_TYPE:
            case Meas::HH_VH_CORR_MEAS_TYPE:
                meas->value = frame->corr[spot_slice_offset + slice_idx];
                break;
            default:
                fprintf(stderr,
                    "PscatL1AToL1B::Convert: unknown measurement type %d\n",
                    meas->measType);
                return(0);
                break;
            }
        }

        //-------------------------------------------------------------------//
        // Extract the spot noise measurement which applies to copol slices. //
        //-------------------------------------------------------------------//

        float Esn_noise = l1a->frame.spotNoise[spot_idx];

        //-------------------------//
        // for each measurement... //
        //-------------------------//

        for (Meas* meas = meas_spot->GetHead(); meas;
            meas = meas_spot->GetNext())
        {
            int slice_i;
            if (! rel_to_abs_idx(meas->startSliceIdx, slice_count, &slice_i))
            {
                fprintf(stderr, "PscatL1AToL1B::Convert, Bad slice number\n");
                exit(1);
            }

            // Kfactor: either 1.0 or taken from table
            float k_factor = 1.0;
            float x_factor = 1.0;
            float Es_slice, En_slice;
            float Esn_slice = meas->value;

            if (useKfactor)
            {
                fprintf(stderr,
                    "PscatL1AToL1B::Convert:No K factor algorithm set\n");
                exit(1);

                //    float orbit_position = pscat->cds.OrbitFraction();
                //    k_factor = kfactorTable.RetrieveByRelativeSliceNumber(
                //    pscat->cds.currentBeamIdx,
                //    pscat->sas.antenna.txCenterAzimuthAngle, orbit_position,
                //    meas->startSliceIdx);

                //-----------------//
                // set measurement //
                //-----------------//

                // meas->value is the Esn value going in, sigma0 coming out.
                // float PtGr = 0.0;
                // if (! Er_to_sigma0(&gc_to_antenna, spacecraft, pscat, meas,
                //    k_factor, meas->value, Esn_echo, Esn_noise, PtGr))
                // {
                //    return(0);
                // }
            }
            else if (useBYUXfactor)
            {
                // HACK ALERT ----- HACK ALERT ---- HACK ALERT
                // HACK to use Qscat BYU X Tables for PSCAT
                // X for all measurement types is the same
                // Outer Beam X is used for incidence angle greater than 51 deg
                // Otherwise Inner Beam X is used

                int real_beam_idx = pscat->cds.currentBeamIdx;
                double thres = 51.0*dtr;
                if (meas->incidenceAngle<thres)
                    pscat->cds.currentBeamIdx=0;
                else
                    pscat->cds.currentBeamIdx = 1;

                if (simVs1BCheckfile)
                {
                    x_factor = BYUX.GetXTotal(spacecraft, pscat, meas, Es_cal,
                        &cf);
                }
                else
                {
                    x_factor = BYUX.GetXTotal(spacecraft, pscat, meas, Es_cal,
                        NULL);
                }
                pscat->cds.currentBeamIdx = real_beam_idx;

                //-----------------//
                // set measurement //
                //-----------------//

                // meas->value is the Esn value going in, sigma0 coming out.
                if (! compute_sigma0(pscat, meas, x_factor, Esn_slice,
                    Esn_echo, Esn_noise, En_echo_load, En_noise_load,
                    &Es_slice, &En_slice))
                {
                    return(0);
                }
            }
            else
            {
                fprintf(stderr,
                    "PscatL1AToL1B::Convert:No X computation algorithm set\n");
                exit(1);
            }

            //------------------//
            // store check data //
            //------------------//

            if (simVs1BCheckfile)
            {
                Vector3 rlook = meas->centroid - spacecraft->orbitState.rsat;
                cf.R[slice_i] = (float)rlook.Magnitude();
                if (useBYUXfactor)
                {
                    // Antenna gain is not computed when using BYU X factor
                    // because the X factor already includes the normalized
                    // patterns.  Thus, to see what it actually is, we need
                    // to do the geometry work here that is normally done
                    // in radar_X() when using the K-factor approach.
//                  gc_to_antenna = AntennaFrameToGC(&(spacecraft->orbitState),
//                        &(spacecraft->attitude), &(pscat->sas.antenna),
//                        pscat->sas.antenna.txCenterAzimuthAngle);
//                    gc_to_antenna=gc_to_antenna.ReverseDirection();
                    double roundTripTime = 2.0*cf.R[slice_i]/speed_light_kps;

                    Beam* beam = pscat->GetCurrentBeam();
                    Vector3 rlook_antenna = gc_to_antenna.Forward(rlook);
                    double r, theta, phi;
                    rlook_antenna.SphericalGet(&r,&theta,&phi);
                    if (! beam->GetPowerGainProduct(theta, phi, roundTripTime,
                        pscat->sas.antenna.spinRate, &(cf.GatGar[slice_i])))
                    {
                        cf.GatGar[slice_i] = 1.0;  // set a dummy value.
                    }
                }
                else
                {
                    double lambda = speed_light_kps / pscat->ses.txFrequency;
                    cf.GatGar[slice_i] = meas->XK / k_factor * (64*pi*pi*pi *
                        cf.R[slice_i] * cf.R[slice_i]*cf.R[slice_i]*
                        cf.R[slice_i] * pscat->systemLoss) /
                        (pscat->ses.transmitPower * pscat->ses.rxGainEcho *
                        lambda * lambda);
                }

                cf.idx[slice_i] = meas->startSliceIdx;
                cf.var_esn_slice[slice_i] = 0;
                cf.Es[slice_i] = Es_slice;
                cf.En[slice_i] = En_slice;
                cf.sigma0[slice_i] = meas->value;
                cf.XK[slice_i] = meas->XK;
                cf.centroid[slice_i] = meas->centroid;
                cf.azimuth[slice_i] = meas->eastAzimuth;
                cf.incidence[slice_i] = meas->incidenceAngle;
            }

            //--------------------------------//
            // Print calculated sigma0 values //
            // to stdout.                     //
            //--------------------------------//

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

        //------------------------//
        // Output data if enabled //
        //------------------------//

        if (simVs1BCheckfile)
        {
            FILE* fptr = fopen(simVs1BCheckfile,"a");
            if (fptr == NULL)
            {
                fprintf(stderr,"Error opening %s\n",simVs1BCheckfile);
                exit(1);
            }
            cf.pulseCount = pulseCount;
            cf.ptgr = pscat->ses.transmitPower * pscat->ses.rxGainEcho;
            cf.time = time;
            cf.beamNumber = pscat->cds.currentBeamIdx;
            cf.rsat = spacecraft->orbitState.rsat;
            cf.vsat = spacecraft->orbitState.vsat;
            cf.orbitFrac = pscat->cds.OrbitFraction();
            cf.spinRate = pscat->sas.antenna.spinRate;
            cf.txDoppler = pscat->ses.txDoppler;
            cf.rxGateDelay = pscat->ses.rxGateDelay;
            cf.attitude = spacecraft->attitude;
            cf.antennaAziTx = pscat->sas.antenna.txCenterAzimuthAngle;
            cf.antennaAziGi = pscat->sas.antenna.groundImpactAzimuthAngle;
            cf.EsCal = Es_cal;
            cf.alpha = 1.0/beta * En_noise_load/En_echo_load;
            cf.WriteDataRec(fptr);
            fclose(fptr);
        }

        //----------------------//
        // add to list of spots //
        //----------------------//

        l1b->frame.spotList.Append(meas_spot);
        pulseCount++;
    }

    if (outputSigma0ToStdout)
    {
        printf("\n");
    }

    return(1);
}
