//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_pscatsim_c[] =
    "@(#) $Id$";

#include "PscatSim.h"
#include "CheckFrame.h"
#include "InstrumentGeom.h"
#include "Sigma0.h"
#include "AccurateGeom.h"
#include "Beam.h"

//==========//
// PscatSim //
//==========//

PscatSim::PscatSim()
:   epochTime(0.0), epochTimeString(NULL), startTime(0),
    numLookStepsPerSlice(0), azimuthIntegrationRange(0.0),
    azimuthStepSize(0.0), dopplerBias(0.0), correlatedKpm(0.0),
    uniformSigmaField(0), outputXToStdout(0), useKfactor(0), createXtable(0),
    computeXfactor(0), useBYUXfactor(0), rangeGateClipping(0),
    l1aFrameReady(0), simKpcFlag(0), simCorrKpmFlag(0), simUncorrKpmFlag(0),
    simKpriFlag(0), _spotNumber(0), _spinUpPulses(2)
{
    return;
}

PscatSim::~PscatSim()
{
    return;
}

//----------------------//
// PscatSim::Initialize //
//----------------------//

int
PscatSim::Initialize(
    Pscat*  pscat)
{
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        beamInfo[beam_idx].eventTime = startTime + beam_idx * pscat->ses.pri;
    }
    return(1);
}

//------------------------------//
// PscatSim::DetermineNextEvent //
//------------------------------//

int
PscatSim::DetermineNextEvent(
    Pscat*       pscat,
    PscatEvent*  pscat_event)
{
    //----------------------------------------//
    // find minimum time from possible events //
    //----------------------------------------//

    int min_idx = 0;
    double min_time = beamInfo[0].eventTime;
    for (int beam_idx = 1; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        if (beamInfo[beam_idx].eventTime < min_time)
        {
            min_idx = beam_idx;
            min_time = beamInfo[beam_idx].eventTime;
        }
    }

    //-----------------------//
    // set event information //
    //-----------------------//

    pscat_event->eventTime = min_time;
    pscat_event->beamIdx = min_idx;

    unsigned short ideal_encoder = pscat->cds.EstimateIdealEncoder();

    switch (pscat_event->beamIdx)
    {
    case 0:
        // inner beam
        pscat_event->eventId = PscatEvent::HH_HV_SCAT_EVENT;
        break;
    case 1:
        // outer beam
        pscat_event->eventId = PscatEvent::VV_SCAT_EVENT;
        break;
    default:
        return(0);
        break;
    }

    //----------------------------//
    // update next time for event //
    //----------------------------//

    int cycle_idx = (int)((min_time - startTime) / pscat->ses.pri + 0.5);
    beamInfo[min_idx].eventTime = startTime +
        (double)(cycle_idx + NUMBER_OF_QSCAT_BEAMS) * pscat->ses.pri;

    //---------------------//
    // remember last event //
    //---------------------//

    lastEventIdealEncoder = ideal_encoder;
    beamInfo[min_idx].eventId = pscat_event->eventId;

    return(1);
}

//------------------------//
// PscatSim::L1AFrameInit //
//------------------------//

int
PscatSim::L1AFrameInit(
    Spacecraft*     spacecraft,
    Pscat*          pscat,
    PscatL1AFrame*  l1a_frame)
{
    //----------------------//
    // frame initialization //
    //----------------------//

    if (_spotNumber == 0)
    {
        if (! SetL1ASpacecraft(spacecraft,l1a_frame))
            return(0);
        l1a_frame->time = pscat->cds.time;
        l1a_frame->orbitTicks = pscat->cds.orbitTime;
        l1a_frame->orbitStep = pscat->cds.SetAndGetOrbitStep();
        l1a_frame->instrumentTicks = pscat->cds.instrumentTime;
        l1a_frame->priOfOrbitStepChange = 255;      // flag value

        // extra data needed by GS for first pulse
        SesBeamInfo* ses_beam_info = pscat->GetCurrentSesBeamInfo();
        Beam* cur_beam = pscat->GetCurrentBeam();

        // Set Frame Inst. Status Flag bits.
        int inst_flag = 0;
        if (cur_beam->polarization == H_POL)
        {
          inst_flag = inst_flag & 0xFFFFFFFB; // turn off bit 2
        }
        else
        {
          inst_flag = inst_flag | 0x00000004; // turn on bit 2
        }
        float eff_gate_width = ses_beam_info->rxGateWidth -
          pscat->ses.txPulseWidth;
        unsigned int code = (int)(eff_gate_width / 0.1 + 0.5);
        if (code > 6)
        {
          fprintf(stderr,"Error: Invalid effective gate width = %g\n",
            eff_gate_width);
          exit(1);
        }
        code = code << 4;
        inst_flag = inst_flag | code; // set eff. gate width code
        inst_flag = inst_flag | 0x00000100;  // start with no cal pulse
    }

    return(1);
}

//-------------------//
// PscatSim::ScatSim //
//-------------------//

int
PscatSim::ScatSim(
    Spacecraft*     spacecraft,
    Pscat*          pscat,
    PscatEvent*     pscat_event,
    WindField*      windfield,
    GMF*            gmf,
    Kp*             kp,
    KpmField*       kpmField,
    PscatL1AFrame*  l1a_frame)
{
    MeasSpot meas_spot;

    //----------------------------------------//
    // compute frame header info if necessary //
    //----------------------------------------//

    L1AFrameInit(spacecraft, pscat, l1a_frame);

    if (_spotNumber == 0)
    {
        // if this is the first or second pulse, "spin up" the //
        // Doppler and range tracking calculations //

        if (_spinUpPulses && (pscat->cds.useRgc || pscat->cds.useDtc))
        {
            SetOrbitStepDelayAndFrequency(spacecraft, pscat);
            _spinUpPulses--;    // one less spinup pulse
            return(2);    // indicate spin up
        }
    }

    //-----------------------------------------------//
    // command the range delay and Doppler frequency //
    //-----------------------------------------------//

    SetOrbitStepDelayAndFrequency(spacecraft, pscat);

    //---------------------//
    // locate measurements //
    //---------------------//

    if (pscat->ses.scienceSlicesPerSpot <= 1)
    {
        if (! pscat->LocateSpot(spacecraft, &meas_spot))
            return(0);
    }
    else
    {
        if (! pscat->LocateSliceCentroids(spacecraft, &meas_spot))
            return(0);
    }

    //-----------------------------------------------------------------//
    // set measurement types (this may increase the # of measurements) //
    //-----------------------------------------------------------------//

    if (! SetMeasTypes(pscat_event, &meas_spot))
        return(0);

    //------------------------//
    // set measurement values //
    //------------------------//

    if (! SetMeasurements(spacecraft, pscat, pscat_event, &meas_spot,
        windfield, gmf, kp, kpmField))
    {
        return(0);
    }

    //---------------------------------------//
    // Output X values to X table if enabled //
    //---------------------------------------//

    if (createXtable)
    {
        int sliceno = 0;
        for (Meas* slice=meas_spot.GetHead(); slice; slice=meas_spot.GetNext())
        {
            float orbit_position = pscat->cds.OrbitFraction();

            if (! xTable.AddEntry(slice->XK, pscat->cds.currentBeamIdx,
                pscat->sas.antenna.txCenterAzimuthAngle, orbit_position,
                sliceno))
            {
                return(0);
            }
            sliceno++;
        }
    }

    //---------------------------------//
    // Add Spot Specific Info to Frame //
    //---------------------------------//

    if (! SetL1AScience(&meas_spot, pscat, l1a_frame))
        return(0);

    //-------------------------------//
    // Output X to Stdout if enabled //
    //-------------------------------//

    if (outputXToStdout)
    {
        float XK_max=0;
        for (Meas* slice=meas_spot.GetHead(); slice; slice=meas_spot.GetNext())
        {
            if (XK_max < slice->XK) XK_max=slice->XK;
        }
        float total_spot_X=0;
        float total_spot_power=0;
        for (Meas* slice=meas_spot.GetHead(); slice; slice=meas_spot.GetNext())
        {
            int slice_count = pscat->ses.GetTotalSliceCount();
            int slice_idx;
            if (!rel_to_abs_idx(slice->startSliceIdx,slice_count,&slice_idx))
            {
                fprintf(stderr,"ScatSim: Bad slice number\n");
                exit(1);
            }
            float dummy, freq;
            pscat->ses.GetSliceFreqBw(slice_idx, &freq, &dummy);

            float Es_cal = true_Es_cal(pscat);
            double Xcaldb;
            radar_Xcal(pscat,Es_cal,&Xcaldb);
            Xcaldb = 10.0 * log10(Xcaldb);

            float XKdb=10*log10(slice->XK);

            printf("%g ",XKdb-Xcaldb);
//               float delta_freq=BYUX.GetDeltaFreq(spacecraft);
//               printf("%g ", delta_freq);
            total_spot_power+=slice->value;
            total_spot_X+=slice->XK;

          }
        printf("\n"); //HACK
//      RangeTracker* rt= &(pscat->sas.antenna.beam[instrument->antenna.currentBeamIdx].rangeTracker);


//      unsigned short orbit_step=rt->OrbitTicksToStep(pscat->cds.orbitTicks,
//                 pscat->cds.orbitTicksPerOrbit);

        //      printf("TOTALS %d %d %g %g %g %g\n",(int)orbit_step,
        //       instrument->antenna.currentBeamIdx,
        //      instrument->antenna.azimuthAngle*rtd,
        //      instrument->commandedRxGateDelay,
        //      total_spot_X,total_spot_power);
        fflush(stdout);
    }

    //---------------------------------//
    // set orbit step change indicator //
    //---------------------------------//

    unsigned short orbit_step = pscat->cds.SetAndGetOrbitStep();
    if (orbit_step != l1a_frame->orbitStep)
    {
        l1a_frame->priOfOrbitStepChange = _spotNumber;
        // remember, the CDS puts in the last orbit step (anti-documentation)
        l1a_frame->orbitStep = orbit_step;
    }

    //-----------------------------//
    // determine if frame is ready //
    //-----------------------------//

    if (_spotNumber >= l1a_frame->spotsPerFrame)
    {
        l1aFrameReady = 1;  // indicate frame is ready
        _spotNumber = 0;    // prepare to start a new frame
    }
    else
    {
        l1aFrameReady = 0;  // indicate frame is not ready
    }

    return(1);
}

//----------------------------//
// PscatSim::SetL1ASpacecraft //
//----------------------------//

int
PscatSim::SetL1ASpacecraft(
    Spacecraft*     spacecraft,
    PscatL1AFrame*  l1a_frame)
{
    OrbitState* orbit_state = &(spacecraft->orbitState);

    double alt, lon, lat;
    if (! orbit_state->rsat.GetAltLonGDLat(&alt, &lon, &lat))
        return(0);

    l1a_frame->gcAltitude = alt;
    l1a_frame->gcLongitude = lon;
    l1a_frame->gcLatitude = lat;
    l1a_frame->gcX = orbit_state->rsat.Get(0);
    l1a_frame->gcY = orbit_state->rsat.Get(1);
    l1a_frame->gcZ = orbit_state->rsat.Get(2);
    l1a_frame->velX = orbit_state->vsat.Get(0);
    l1a_frame->velY = orbit_state->vsat.Get(1);
    l1a_frame->velZ = orbit_state->vsat.Get(2);

    return(1);
}

//------------------------//
// PscatSim::SetMeasTypes //
//------------------------//

int
PscatSim::SetMeasTypes(
    PscatEvent*  pscat_event,
    MeasSpot*    meas_spot)
{
    switch (pscat_event->eventId)
    {
    case PscatEvent::VV_SCAT_EVENT:
        for (Meas* meas = meas_spot->GetHead(); meas;
            meas = meas_spot->GetNext())
        {
            meas->measType = Meas::VV_MEAS_TYPE;
        }
        break;
    case PscatEvent::HH_SCAT_EVENT:
        for (Meas* meas = meas_spot->GetHead(); meas;
            meas = meas_spot->GetNext())
        {
            meas->measType = Meas::HH_MEAS_TYPE;
        }
        break;
    case PscatEvent::VV_VH_SCAT_EVENT:
        for (Meas* meas = meas_spot->GetHead(); meas;
            meas = meas_spot->GetNext())
        {
            meas->measType = Meas::VV_MEAS_TYPE;
            Meas* new_meas = new Meas();
            *new_meas = *meas;
            new_meas->measType = Meas::VV_VH_CORR_MEAS_TYPE;
            // the new node is inserted after the current node and
            // is made the current node, list traversing continues
            meas_spot->InsertAfter(new_meas);
        }
        break;
    case PscatEvent::HH_HV_SCAT_EVENT:
        for (Meas* meas = meas_spot->GetHead(); meas;
            meas = meas_spot->GetNext())
        {
            meas->measType = Meas::HH_MEAS_TYPE;
            Meas* new_meas = new Meas();
            *new_meas = *meas;
            new_meas->measType = Meas::HH_HV_CORR_MEAS_TYPE;
            // the new node is inserted after the current node and
            // is made the current node, list traversing continues
            meas_spot->InsertAfter(new_meas);
        }
        break;
    default:
        return(0);
        break;
    }
    return(1);
}

//---------------------------//
// PscatSim::SetMeasurements //
//---------------------------//

int
PscatSim::SetMeasurements(
    Spacecraft*  spacecraft,
    Pscat*       pscat,
    PscatEvent*  pscat_event,
    MeasSpot*    meas_spot,
    WindField*   windfield,
    GMF*         gmf,
    Kp*          kp,
    KpmField*    kpmField)
{
    //-------------------------//
    // for each measurement... //
    //-------------------------//

    int slice_count = pscat->ses.GetTotalSliceCount();
    int slice_i = 0;
    int sliceno = -slice_count / 2;

    // We need to track the previous meas for correlation measurements.
    Meas* prev_meas = NULL;

    Meas* meas = meas_spot->GetHead();
    while (meas)
    {
        meas->startSliceIdx = sliceno;

        //----------------------------------------//
        // get lon and lat for the earth location //
        //----------------------------------------//

        double alt, lat, lon;
        if (! meas->centroid.GetAltLonGDLat(&alt, &lon, &lat))
            return(0);

        LonLat lon_lat;
        lon_lat.longitude = lon;
        lon_lat.latitude = lat;

        // Compute Land Flag
        meas->landFlag = landMap.IsLand(lon,lat);

        float sigma0;
        if (meas->landFlag == 1)
        {
            // Set sigma0 to average NSCAT land sigma0 for appropriate
            // incidence angle and polarization
            switch (meas->measType)
            {
            case Meas::VV_MEAS_TYPE:
                sigma0 = 0.1;
                break;
            case Meas::HH_MEAS_TYPE:
                sigma0 = 0.085;
                break;
            case Meas::VV_VH_CORR_MEAS_TYPE:
            case Meas::HH_HV_CORR_MEAS_TYPE:
                sigma0 = 0.001;
                break;
            default:
                return(0);
                break;
            }
        }
        else if (uniformSigmaField)
        {
            sigma0 = 1.0;
        }
        else
        {
            //-----------------//
            // get wind vector //
            //-----------------//

            WindVector wv;
            if (! windfield->InterpolatedWindVector(lon_lat, &wv))
            {
                wv.spd = 0.0;
                wv.dir = 0.0;
            }

			//--------------------------------//
			// convert wind vector to sigma-0 //
			//--------------------------------//
			// chi is defined so that 0.0 means the wind is blowing towards
			// the s/c (the opposite direction as the look vector)

			float chi = wv.dir - meas->eastAzimuth + pi;

			gmf->GetInterpolatedValue(meas->measType, meas->incidenceAngle,
                wv.spd, chi, &sigma0);

			//---------------------------------------------------------------//
			// Fuzz the sigma0 by Kpm to simulate the effects of model function
			// error.  The resulting sigma0 is the 'true' value.
			// It does not map back to the correct wind speed for the
			// current beam and geometry because the model function is
			// not perfect.
			//---------------------------------------------------------------//

			// Uncorrelated component.
			if (simUncorrKpmFlag == 1)
			{
				double kpm_value;
				if (! kp->kpm.GetKpm(meas->measType, wv.spd, &kpm_value))
				{
					printf(
                        "Error: Bad Kpm value in PscatSim::SetMeasurements\n");
					exit(-1);
				}
				Gaussian gaussianRv(1.0,0.0);
				float rv1 = gaussianRv.GetNumber();
				float RV = rv1*kpm_value + 1.0;
                if (RV < 0.0)
                {
                    RV = 0.0;   // Do not allow negative sigma0's.
                }
				sigma0 *= RV;
			}

			// Correlated component.
			if (simCorrKpmFlag == 1)
			{
				sigma0 *= kpmField->GetRV(correlatedKpm, lon_lat);
			}
		}

		//-------------------------//
		// convert Sigma0 to Power //
		//-------------------------//

		// Kfactor: either 1.0, taken from table, or X is computed
        // directly
        float Kfactor=1.0;
        float Es,En,var_esn_slice;
		CoordinateSwitch gc_to_antenna;

		if (computeXfactor || useBYUXfactor)
        {
            // If you cannot calculate X it probably means the
            // slice is partially off the earth.
            // In this case remove it from the list and go on
            // to next slice
            if (computeXfactor)
            {
                if (! ComputeXfactor(spacecraft, pscat, meas, &(meas->XK)))
                {
                    meas=meas_spot->RemoveCurrent();
                    delete meas;
                    meas=meas_spot->GetCurrent();
                    slice_i++;
                    sliceno++;
                    if (slice_count%2==0 && sliceno==0)
                        sliceno++;
                    continue;
                }
            }
            else if (useBYUXfactor)
            {
                meas->XK = BYUX.GetXTotal(spacecraft, pscat, meas, NULL);
            }

//            if (! sigma0_to_Esn_slice_given_X(pscat, meas, Xfactor, sigma0,
//                simKpcFlag, &(meas->value), &Es, &En,
//                &var_esn_slice))

            if (! pscat->MeasToEsn(prev_meas, meas, meas->XK, sigma0,
                    simKpcFlag, &(meas->value), &Es, &En, &var_esn_slice))
            {
                return(0);
            }

		}
        else
        {
            Kfactor=1.0;  // default to use if no Kfactor specified.
            if (useKfactor)
            {
                float orbit_position = pscat->cds.OrbitFraction();

                Kfactor = kfactorTable.RetrieveByRelativeSliceNumber(
                    pscat->cds.currentBeamIdx,
                    pscat->sas.antenna.txCenterAzimuthAngle, orbit_position,
                    sliceno);
            }

            //--------------------------------//
            // generate the coordinate switch //
            //--------------------------------//

            gc_to_antenna = AntennaFrameToGC(&(spacecraft->orbitState),
                &(spacecraft->attitude), &(pscat->sas.antenna),
                pscat->sas.antenna.txCenterAzimuthAngle);
            gc_to_antenna=gc_to_antenna.ReverseDirection();
            double Tp = pscat->ses.txPulseWidth;

            if (! sigma0_to_Esn_slice(&gc_to_antenna, spacecraft, pscat, meas,
                Kfactor*Tp, sigma0, simKpcFlag, &(meas->value), &(meas->XK),
                &Es, &En, &var_esn_slice))
            {
                return(0);
            }
		}

		sliceno++;
		slice_i++;
		if (slice_count%2==0 && sliceno==0)
            sliceno++;
        prev_meas = meas;
		meas=meas_spot->GetNext();
	}

	return(1);
}

//-------------------------//
// PscatSim::SetL1AScience //
//-------------------------//

int
PscatSim::SetL1AScience(
    MeasSpot*       meas_spot,
    Pscat*          pscat,
    PscatL1AFrame*  l1a_frame)
{
    //----------//
    // set PtGr //
    //----------//
    // Only "noise it up" if simKpriFlag is set

    l1a_frame->ptgr = pscat->ses.transmitPower * pscat->ses.rxGainEcho;

    if (simKpriFlag)
        l1a_frame->ptgr *= (1 + ptgrNoise.GetNumber(pscat->cds.time));

    //----------------------//
    // set antenna position //
    //----------------------//

    l1a_frame->antennaPosition[_spotNumber] = pscat->cds.rawEncoder;

    //-------------------------//
    // for each measurement... //
    //-------------------------//

    int slice_number = _spotNumber * l1a_frame->slicesPerSpot;
    for (Meas* meas = meas_spot->GetHead(); meas;
        meas = meas_spot->GetNext())
    {
        //----------------------------//
        // update the level 0.0 frame //
        //----------------------------//

        l1a_frame->science[slice_number] = meas->value;
        slice_number++;
    }

    // Compute the spot noise measurement.
    sigma0_to_Esn_noise(pscat, meas_spot, simKpcFlag,
        &(l1a_frame->spotNoise[_spotNumber]));

    _spotNumber++;

    return(1);
}

//--------------------------//
// PscatSim::ComputeXfactor //
//--------------------------//
int
PscatSim::ComputeXfactor(
    Spacecraft*  spacecraft,
    Pscat*       pscat,
    Meas*        meas,
    float*       X)
{
    if (! IntegrateSlice(spacecraft, pscat, meas, numLookStepsPerSlice,
        azimuthIntegrationRange, azimuthStepSize, rangeGateClipping, X))
    {
        return(0);
    }
    Beam* beam = pscat->GetCurrentBeam();
    double Xcal;
    float Es_cal = true_Es_cal(pscat);

    radar_Xcal(pscat,Es_cal,&Xcal);
    (*X)*=Xcal/(beam->peakGain * beam->peakGain);
    return(1);
}
