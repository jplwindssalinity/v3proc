//==============================================================//
// Copyright (C) 1997-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_l1atol1b_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include "L1AToL1B.h"
#include "CheckFrame.h"
#include "InstrumentGeom.h"
#include "Sigma0.h"
#include "Qscat.h"

//==========//
// L1AToL1B //
//==========//

L1AToL1B::L1AToL1B()
:   pulseCount(0), useKfactor(0), useBYUXfactor(0), useSpotCompositing(0),
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
    Topo*        topo,
    Stable*      stable,
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
    // Note 8 bit shift for loopback's    //
    //------------------------------------//

    if (l1a->frame.calPosition != 255)
    {
        Esn_echo_cal = 0.0;
        En_echo_load = 0.0;
        for (int i=0; i < l1a->frame.slicesPerSpot; i++)
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
        PtGr_to_Esn(NULL,qscat,0,&Esn_echo_cal,&Esn_noise_cal);
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
        if (simVs1BCheckfile)
        {
          // cf.idx needs to start out zeroed for each spot
          cf.Initialize();
        }

        //--------------------------//
        // skip loopbacks and loads //
        //--------------------------//

        if (spot_idx==frame->calPosition-2 || spot_idx==frame->calPosition-1)
        {
          pulseCount++;
          continue;
        }

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

//        qscat->sas.SetAzimuthWithEncoder(held_encoder);
//        qscat->SetAntennaToTxCenter(1);

        SetDelayAndFrequency(spacecraft, qscat);

        if (outputSigma0ToStdout)
            printf("%g ", antenna->txCenterAzimuthAngle * rtd);

        //---------------------------//
        // create a measurement spot //
        //---------------------------//

        MeasSpot* meas_spot = new MeasSpot();
        meas_spot->time = time;

        //----------------------------------------//
        // Create slice measurements for the spot //
        //----------------------------------------//

        if (! qscat->MakeSlices(meas_spot))
            return(0);

        //----------------------------------------------------------//
        // Extract and load slice energy measurements for this spot //
        //----------------------------------------------------------//

        float Esn_echo = 0.0;
        Meas* meas = meas_spot->GetHead();
        for (int i=0; i < l1a->frame.slicesPerSpot; i++)
        {
          meas->value = l1a->frame.science[base_slice_idx + i];
          if (meas->value < 0.0)
          {
            fprintf(stderr,
              "L1AToL1B: Warning, found negative Esn in spot %d\n",spot_idx);
          }
          // Sum up ALL the signal+noise measurements
          Esn_echo += meas->value;

          meas = meas_spot->GetNext();
        }

        //-----------------------------------------------------------------//
        // Extract the spot noise measurement which applies to all slices. //
        //-----------------------------------------------------------------//

        float Esn_noise = l1a->frame.spotNoise[spot_idx];

        //---------------------//
        // locate measurements //
        //---------------------//

        if (l1a->frame.slicesPerSpot <= 1)
        {
            if (! qscat->LocateSpot(spacecraft, meas_spot))
            {
                return(0);
            }
        }
        else
        {
            if (! qscat->LocateSliceCentroids(spacecraft, meas_spot,
                sliceGainThreshold, processMaxSlices))
            {
                return(0);
            }
        }

        //--------------------//
        // Set the land flags //
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

        int slice_count = qscat->ses.GetTotalSliceCount();

        //--------------------------------------//
        // determine measurement type from beam //
        //--------------------------------------//

        Beam* beam = qscat->GetCurrentBeam();
        Meas::MeasTypeE meas_type = PolToMeasType(beam->polarization);

        //-------------------//
        // for each slice... //
        //-------------------//

        for (Meas* meas = meas_spot->GetHead(); meas;
            meas = meas_spot->GetNext())
        {
            meas->measType = meas_type;

            int slice_i;
            if (!rel_to_abs_idx(meas->startSliceIdx,slice_count,&slice_i))
            {
                fprintf(stderr,"L1AToL1B::Convert, Bad slice number\n");
                exit(1);
            }

            // Kfactor: either 1.0 or taken from table
            float k_factor=1.0;
            float x_factor=1.0;
            float Es_slice,En_slice;
            float Esn_slice = meas->value;

            if (useKfactor)
            {
                fprintf(stderr,
                    "PscatL1AToL1B::Convert:No K factor algorithm set\n");
                exit(1);

                /****
                float orbit_position = qscat->cds.OrbitFraction();

                k_factor = kfactorTable.RetrieveByRelativeSliceNumber(
                    qscat->cds.currentBeamIdx,
                    qscat->sas.antenna.txCenterAzimuthAngle, orbit_position,
                    meas->startSliceIdx);

                //-----------------//
                // set measurement //
                //-----------------//

                // meas->value is the Esn value going in, sigma0 coming out.
                float PtGr = 0.0;
                if (! Er_to_sigma0(&gc_to_antenna, spacecraft, qscat, meas,
                    k_factor, meas->value, Esn_echo, Esn_noise, PtGr))
                {
                    return(0);
                }
                ****/
            }
            else if (useBYUXfactor)
            {
                if (simVs1BCheckfile)
                {
                    x_factor = BYUX.GetXTotal(spacecraft, qscat, meas, Es_cal,
                        topo, stable, &cf);
                }
                else
                {
                    x_factor = BYUX.GetXTotal(spacecraft, qscat, meas, Es_cal,
                        topo, stable, NULL);
                }

                //-----------------//
                // set measurement //
                //-----------------//

                // meas->value is the Esn value going in
                // sigma0 coming out.
                if (! ComputeSigma0(qscat, meas, x_factor, Esn_slice,
                    Esn_echo, Esn_noise, En_echo_load, En_noise_load,
                    &Es_slice, &En_slice))
                {
                    return(0);
                }
            }
            else
            {
                fprintf(stderr,
                    "L1AToL1B::Convert:No X computation algorithm set\n");
                exit(0);
            }

            meas->scanAngle = qscat->sas.antenna.txCenterAzimuthAngle;
            meas->beamIdx = qscat->cds.currentBeamIdx;
            meas->txPulseWidth = qscat->ses.txPulseWidth;

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
                    double roundTripTime = 2.0*cf.R[slice_i]/speed_light_kps;

                    Beam* beam = qscat->GetCurrentBeam();
                    Vector3 rlook_antenna = gc_to_antenna.Forward(rlook);
                    double r, theta, phi;
                    rlook_antenna.SphericalGet(&r,&theta,&phi);
                    if (! beam->GetPowerGainProduct(theta, phi, roundTripTime,
                        qscat->sas.antenna.spinRate, &(cf.GatGar[slice_i])))
                    {
                        cf.GatGar[slice_i] = 1.0;  // set a dummy value.
                    }
                }
                else
                {
                    double lambda = speed_light_kps / qscat->ses.txFrequency;
                    cf.GatGar[slice_i] = meas->XK / k_factor * (64*pi*pi*pi *
                        cf.R[slice_i] * cf.R[slice_i]*cf.R[slice_i]*
                        cf.R[slice_i] * qscat->systemLoss) /
                        (qscat->ses.transmitPower * qscat->ses.rxGainEcho *
                        lambda * lambda);
                }

                cf.idx[slice_i] = meas->startSliceIdx;
                cf.measType[slice_i] = meas->measType;
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
            exit(-1);
          }
          cf.pulseCount = pulseCount;
          cf.ptgr = qscat->ses.transmitPower * qscat->ses.rxGainEcho;
          cf.time = time;
          cf.beamNumber = qscat->cds.currentBeamIdx;
          cf.rsat = spacecraft->orbitState.rsat;
          cf.vsat = spacecraft->orbitState.vsat;
          cf.orbitFrac = qscat->cds.OrbitFraction();
          cf.spinRate = qscat->sas.antenna.spinRate;
          cf.txDoppler = qscat->ses.txDoppler;
          cf.rxGateDelay = qscat->ses.rxGateDelay;
          cf.attitude = spacecraft->attitude;
          cf.antennaAziTx = qscat->sas.antenna.txCenterAzimuthAngle;
          cf.antennaAziGi = qscat->sas.antenna.groundImpactAzimuthAngle;
          cf.EsCal = Es_cal;
          cf.alpha = 1.0/beta * En_noise_load/En_echo_load;
          cf.EsnEcho = Esn_echo;
          cf.EsnNoise = Esn_noise;
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

//-------------------------//
// L1AToL1B::ComputeSigma0 //
//-------------------------//
//
// The ComputeSigma0 method estimates sigma0 from five energy
// measurements and the tabulated X factor.
// Various outputs are put in the Meas object passed in.
// Note that the rho-factor is assumed to be 1.0. ie., we assume that
// all of the signal power falls in the slices.
//
// Inputs:
//    qscat = pointer to current Qscat object
//    meas = pointer to current measurement (holds results)
//    Xfactor = Total radar equation parameter for this slice.
//    Esn_slice = the received slice energy.
//    Esn_echo = the sum of all the slice energies for this spot.
//    Esn_noise = the noise channel measured energy.
//  En_echo_load = reference load echo channel measurement
//  En_noise_load = reference load noise channel measurement
//

int
L1AToL1B::ComputeSigma0(
    Qscat*  qscat,
    Meas*   meas,
    float   Xfactor,
    float   Esn_slice,
    float   Esn_echo,
    float   Esn_noise,
    float   En_echo_load,
    float   En_noise_load,
    float*  Es_slice,
    float*  En_slice)
{

    //--------------------------------//
    // Extract some useful quantities.
    //--------------------------------//

    double Tp = qscat->ses.txPulseWidth;
    SesBeamInfo* ses_beam_info = qscat->GetCurrentSesBeamInfo();
    double Tg = ses_beam_info->rxGateWidth;
    double Bn = qscat->ses.noiseBandwidth;
    double Bs = meas->bandwidth;
    double Be = qscat->ses.GetTotalSignalBandwidth();
    double beta = qscat->ses.rxGainNoise / qscat->ses.rxGainEcho;

    //-------------------------------------------------------------------//
    // Get the noise energy ratio correction from a table.
    // We define the noise energy ratio correction so that actual slice
    // bandwidth B = Bs * q where Bs is the nominal slice bandwidth.
    // This is different from IOM-3347-98-043 where B = Be * q with Be
    // being the total echo channel bandwidth.  The table is computed
    // from the tables in the memo so that the end effect is the same,
    // while retaining the ability to set the nominal slice bandwidth.
    //-------------------------------------------------------------------//

    float q_slice;
    if (! qscat->ses.GetQRel(meas->startSliceIdx, &q_slice))
    {
      fprintf(stderr,"compute_sigma0: Error getting Q value\n");
      exit(1);
    }

    //-----------------------------------------------------------------//
    // Convert to Q as defined in IOM-3347-98-043, but consistent with
    // the values of Bs and Be being used currently.
    //-----------------------------------------------------------------//

    q_slice *= Bs/Be;

    //-------------------------------------------//
    // Estimate slice signal and noise energies.
    //-------------------------------------------//

    if (! Er_to_Es(beta, Esn_slice, Esn_echo, Esn_noise, En_echo_load,
                   En_noise_load, q_slice, Es_slice, En_slice))
    {
      return(0);
    }

    //------------------------------------------------------------------//
    // Compute sigma0 from estimated signal energy and X factors.
    // The resulting sigma0 should have a variance equal to Kpc^2+Kpr^2.
    // Kpc comes from Es_slice.
    // Kpr comes from 1/X (ie., from Es_cal when computing X)
    // Xfactor has units of energy because Xcal has units of Pt * Tp.
    //------------------------------------------------------------------//

    meas->value = *Es_slice / Xfactor;
    meas->EnSlice = *En_slice;

    //------------------------------------------------------------------//
    // Store the total X factor.
    //------------------------------------------------------------------//

    meas->XK = Xfactor;

    //------------------------------------------------------------------//
    // Estimate Kpc coefficients using the
    // approximate equations in Mike Spencer's Kpc memos.
    //------------------------------------------------------------------//

    meas->A = 1.0 / (Bs * Tp);
    meas->B = 2.0 / (Bs * Tg);
    meas->C = meas->B/2.0 * (1.0 + Bs/Bn);

    return(1);
}
