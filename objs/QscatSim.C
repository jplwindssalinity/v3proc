//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_qscatsim_c[] =
    "@(#) $Id$";

#include "QscatSim.h"
#include "CheckFrame.h"
#include "InstrumentGeom.h"
#include "Sigma0.h"
#include "AccurateGeom.h"
#include "Beam.h"

//============//
// QscatEvent //
//============//

QscatEvent::QscatEvent()
:   time(0.0), eventId(NONE), beamIdx(0)
{
    return;
}

QscatEvent::~QscatEvent()
{
    return;
}

//==================//
// QscatSimBeamInfo //
//==================//

QscatSimBeamInfo::QscatSimBeamInfo()
:   txTime(0.0)
{
    return;
}

QscatSimBeamInfo::~QscatSimBeamInfo()
{
    return;
}

//==========//
// QscatSim //
//==========//

QscatSim::QscatSim()
:   startTime(0), lastEventType(QscatEvent::NONE), numLookStepsPerSlice(0),
    azimuthIntegrationRange(0.0), azimuthStepSize(0.0), dopplerBias(0.0),
    correlatedKpm(0.0), simVs1BCheckfile(NULL), uniformSigmaField(0),
    outputXToStdout(0), useKfactor(0), createXtable(0), computeXfactor(0),
    useBYUXfactor(0), rangeGateClipping(0), applyDopplerError(0),
    l1aFrameReady(0), simKpcFlag(0), simCorrKpmFlag(0), simUncorrKpmFlag(0),
    simKpriFlag(0), _spotNumber(0), _spinUpPulses(2)
{
    return;
}

QscatSim::~QscatSim()
{
    return;
}

//----------------------//
// QscatSim::Initialize //
//----------------------//

int
QscatSim::Initialize(
    Qscat*  qscat)
{
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        beamInfo[beam_idx].txTime = startTime + beam_idx * qscat->ses.pri;
    }
    return(1);
}

//------------------------------//
// QscatSim::DetermineNextEvent //
//------------------------------//

#define NINETY_DEGREE_ENCODER  8191

int
QscatSim::DetermineNextEvent(
    Qscat*       qscat,
    QscatEvent*  qscat_event)
{
    //----------------------------------------//
    // find minimum time from possible events //
    //----------------------------------------//

    int min_idx = 0;
    double min_time = beamInfo[0].txTime;
    for (int beam_idx = 1; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        if (beamInfo[beam_idx].txTime < min_time)
        {
            min_idx = beam_idx;
            min_time = beamInfo[beam_idx].txTime;
        }
    }

    //-----------------------//
    // set event information //
    //-----------------------//

    qscat_event->time = min_time;
    qscat_event->beamIdx = min_idx;

    unsigned short ideal_encoder = qscat->cds.EstimateIdealEncoder();
    switch (lastEventType)
    {
    case QscatEvent::SCATTEROMETER_MEASUREMENT:
        if (ideal_encoder > NINETY_DEGREE_ENCODER &&
            lastEventIdealEncoder <= NINETY_DEGREE_ENCODER)
        {
            qscat_event->eventId = QscatEvent::LOOPBACK_MEASUREMENT;
        }
        else
        {
            qscat_event->eventId = QscatEvent::SCATTEROMETER_MEASUREMENT;
        }
        break;
    case QscatEvent::LOOPBACK_MEASUREMENT:
        qscat_event->eventId = QscatEvent::LOAD_MEASUREMENT;
        break;
    case QscatEvent::LOAD_MEASUREMENT: case QscatEvent::NONE:
        qscat_event->eventId = QscatEvent::SCATTEROMETER_MEASUREMENT;
        break;
    }

    //----------------------------//
    // update next time for event //
    //----------------------------//

    int cycle_idx = (int)((min_time - startTime) / qscat->ses.pri + 0.5);
    beamInfo[min_idx].txTime = startTime +
        (double)(cycle_idx + NUMBER_OF_QSCAT_BEAMS) * qscat->ses.pri;

    lastEventIdealEncoder = ideal_encoder;
    lastEventType = qscat_event->eventId;

    return(1);
}

//------------------------//
// QscatSim::L1AFrameInit //
//------------------------//

int
QscatSim::L1AFrameInit(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    L1AFrame*    l1a_frame)
{
    //----------------------//
    // frame initialization //
    //----------------------//

    if (_spotNumber == 0)
    {
        if (! SetL1ASpacecraft(spacecraft,l1a_frame))
            return(0);
        l1a_frame->time = qscat->cds.time;
        l1a_frame->orbitTicks = qscat->cds.orbitTime;
        l1a_frame->orbitStep = qscat->cds.SetAndGetOrbitStep();
        l1a_frame->status.doppler_orbit_step = l1a_frame->orbitStep;
        l1a_frame->instrumentTicks = qscat->cds.instrumentTime;
        l1a_frame->priOfOrbitStepChange = 255;      // flag value
        l1a_frame->status.prf_orbit_step_change=l1a_frame->priOfOrbitStepChange;
        l1a_frame->calPosition = 255;	// no cal pulses yet

        // extra data needed by GS for first pulse
        l1a_frame->in_eu.prf_cycle_time_eu = qscat->ses.pri;
        l1a_frame->range_gate_delay_inner =
          (unsigned short)(qscat->ses.rxGateDelay * 1e6);
        SesBeamInfo* ses_beam_info = qscat->GetCurrentSesBeamInfo();
        l1a_frame->in_eu.range_gate_width_inner = ses_beam_info->rxGateWidth;
        l1a_frame->in_eu.transmit_pulse_width = qscat->ses.txPulseWidth;
        l1a_frame->in_eu.precision_coupler_temp_eu =
          qscat->ses.physicalTemperature;
        l1a_frame->in_eu.rcv_protect_sw_temp_eu =
          qscat->ses.physicalTemperature;
        l1a_frame->in_eu.beam_select_sw_temp_eu =
          qscat->ses.physicalTemperature;
        l1a_frame->in_eu.receiver_temp_eu =
          qscat->ses.physicalTemperature;

        // Set Frame Inst. Status Flag bits.
        Beam* cur_beam = qscat->GetCurrentBeam();
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
          qscat->ses.txPulseWidth;
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
        l1a_frame->frame_inst_status=inst_flag;
        l1a_frame->frame_err_status=0x00000000;
        l1a_frame->frame_qual_flag=0x0000;
        for (int i=0; i < 13; i++) l1a_frame->pulse_qual_flag[i]=0x00;

        l1a_frame->frame_time_secs = qscat->cds.time;
        l1a_frame->instrument_time = qscat->cds.instrumentTime;
        l1a_frame->status.prf_count = l1a_frame->spotsPerFrame;
        l1a_frame->status.prf_cycle_time = qscat->cds.priDn;
        l1a_frame->range_gate_a_delay = qscat->cds.rxGateDelayDn;
        CdsBeamInfo* cds_beam_info = qscat->GetCurrentCdsBeamInfo();
        l1a_frame->status.range_gate_a_width = cds_beam_info->rxGateWidthDn;
        l1a_frame->status.pulse_width = qscat->cds.txPulseWidthDn;
        l1a_frame->status.pred_antenna_pos_count = 0; // needs to be filled
        l1a_frame->status.vtcw[0] = 0; // needs to be filled
        l1a_frame->status.vtcw[1] = 0; // needs to be filled
        l1a_frame->engdata.precision_coupler_temp =
          qscat->ses.tempToDn(qscat->ses.physicalTemperature);
        l1a_frame->engdata.rcv_protect_sw_temp =
          l1a_frame->engdata.precision_coupler_temp;
        l1a_frame->engdata.beam_select_sw_temp =
          l1a_frame->engdata.precision_coupler_temp;
        l1a_frame->engdata.receiver_temp =
          l1a_frame->engdata.precision_coupler_temp;
    }
    else if (_spotNumber == 1)
    {
        //----------------------------------//
        // Store data needed from 2nd pulse //
        //----------------------------------//

        l1a_frame->range_gate_delay_outer =
          qscat->ses.rxGateDelay;
        SesBeamInfo* ses_beam_info = qscat->GetCurrentSesBeamInfo();
        l1a_frame->in_eu.range_gate_width_outer = ses_beam_info->rxGateWidth;

        l1a_frame->range_gate_b_delay = qscat->cds.rxGateDelayDn;
        CdsBeamInfo* cds_beam_info = qscat->GetCurrentCdsBeamInfo();
        l1a_frame->status.range_gate_b_width = cds_beam_info->rxGateWidthDn;
    }

    return(1);

}

//-------------------//
// QscatSim::ScatSim //
//-------------------//

int
QscatSim::ScatSim(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    WindField*   windfield,
    GMF*         gmf,
    Kp*          kp,
    KpmField*    kpmField,
    L1AFrame*    l1a_frame)
{
    CheckFrame cf;
    if (simVs1BCheckfile)
    {
        if (!cf.Allocate(qscat->ses.scienceSlicesPerSpot))
        {
            fprintf(stderr,"Error allocating a CheckFrame\n");
            return(0);
        }
    }

    MeasSpot meas_spot;

    //----------------------------------------//
    // compute frame header info if necessary //
    //----------------------------------------//

    L1AFrameInit(spacecraft,qscat,l1a_frame);

    if (_spotNumber == 0)
    {
        // if this is the first or second pulse, "spin up" the //
        // Doppler and range tracking calculations //

        if (_spinUpPulses && (qscat->cds.useRgc || qscat->cds.useDtc))
        {
            SetOrbitStepDelayAndFrequency(spacecraft, qscat);
            _spinUpPulses--;    // one less spinup pulse
            return(2);    // indicate spin up
        }
    }

    //-----------------------------------------------//
    // command the range delay and Doppler frequency //
    //-----------------------------------------------//

    SetOrbitStepDelayAndFrequency(spacecraft, qscat);

    if (applyDopplerError)
    {
        fprintf(stderr, "Need to implement Doppler errors\n");
        exit(1);
    }

    //---------------------//
    // locate measurements //
    //---------------------//

    if (qscat->ses.scienceSlicesPerSpot <= 1)
    {
        if (! LocateSpot(spacecraft, qscat, &meas_spot))
            return(0);
    }
    else
    {
        if (! LocateSliceCentroids(spacecraft, qscat, &meas_spot))
            return(0);
    }

    //------------------------//
    // set measurement values //
    //------------------------//

    if (! SetMeasurements(spacecraft, qscat, &meas_spot, &cf,
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
            float orbit_position = qscat->cds.OrbitFraction();

            if (! xTable.AddEntry(slice->XK, qscat->cds.currentBeamIdx,
                qscat->sas.antenna.txCenterAzimuthAngle, orbit_position,
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

    if (! SetL1AScience(&meas_spot, &cf, qscat, l1a_frame))
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
            int slice_count = qscat->ses.GetTotalSliceCount();
            int slice_idx;
            if (!rel_to_abs_idx(slice->startSliceIdx,slice_count,&slice_idx))
            {
                fprintf(stderr,"ScatSim: Bad slice number\n");
                exit(1);
            }
            float dummy, freq;
            qscat->ses.GetSliceFreqBw(slice_idx, &freq, &dummy);

//          float gain=10*log10(slice->XK/XK_max);
            float lambda = speed_light_kps / qscat->ses.txFrequency;
            Beam* beam = qscat->GetCurrentBeam();
            float kdb = qscat->ses.transmitPower * qscat->ses.rxGainEcho *
                lambda * lambda * beam->peakGain * beam->peakGain /
                (64.0 * pi * pi * pi * qscat->systemLoss);
            kdb = 10.0 * log10(kdb);


            float XKdb=10*log10(slice->XK);
//          double range=(spacecraft->orbitState.rsat -
//                   slice->centroid).Magnitude();
//          float rtt=2.0*range/speed_light_kps;
//          float pf=GetPulseFractionReceived(range);
            //          printf("%d %g %g %g %g %g %g %g\n",
            //   qscat->cds.currentBeamIdx,
            //   qscat->sas.antenna.azimuthAngle*rtd,freq,
            //   gain, rtt, pf,slice->XK, slice->value);

            printf("%g ",XKdb-kdb); //HACK
//                    float delta_freq=BYUX.GetDeltaFreq(spacecraft);
                    //printf("%g ", delta_freq);
            total_spot_power+=slice->value;
            total_spot_X+=slice->XK;

          }
        printf("\n"); //HACK
//      RangeTracker* rt= &(qscat->sas.antenna.beam[instrument->antenna.currentBeamIdx].rangeTracker);


//      unsigned short orbit_step=rt->OrbitTicksToStep(qscat->cds.orbitTicks,
//                 qscat->cds.orbitTicksPerOrbit);

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

    unsigned short orbit_step = qscat->cds.SetAndGetOrbitStep();
    if (orbit_step != l1a_frame->orbitStep)
    {
        l1a_frame->priOfOrbitStepChange = _spotNumber;
        l1a_frame->status.prf_orbit_step_change=l1a_frame->priOfOrbitStepChange;
        // remember, the CDS puts in the last orbit step (anti-documentation)
        l1a_frame->orbitStep = orbit_step;
        l1a_frame->status.doppler_orbit_step = l1a_frame->orbitStep;
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
        cf.time = qscat->cds.time;
        cf.beamNumber = qscat->cds.currentBeamIdx;
        cf.rsat = spacecraft->orbitState.rsat;
        cf.vsat = spacecraft->orbitState.vsat;
        cf.orbitFrac = qscat->cds.OrbitFraction();
        cf.spinRate = qscat->sas.antenna.spinRate;
        cf.txDoppler = qscat->ses.txDoppler;
        cf.attitude = spacecraft->attitude;
        cf.antennaAziTx = qscat->sas.antenna.txCenterAzimuthAngle;
        cf.antennaAziGi = qscat->sas.antenna.groundImpactAzimuthAngle;
        cf.EsCal = qscat->ses.transmitPower * qscat->ses.rxGainEcho /
                 qscat->ses.loopbackLoss / qscat->ses.loopbackLossRatio *
                 qscat->ses.txPulseWidth;
        cf.WriteDataRec(fptr);
        fclose(fptr);
    }

    return(1);
}

//-----------------------//
// QscatSim::LoopbackSim //
//-----------------------//

int
QscatSim::LoopbackSim(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    L1AFrame*    l1a_frame)
{
    //----------------------------------------//
    // compute frame header info if necessary //
    //----------------------------------------//

    L1AFrameInit(spacecraft,qscat,l1a_frame);

    //-----------------------------//
    // Set cal pulse sequence flag //
    // (turn off Bit position 8)   //
    //-----------------------------//

    l1a_frame->frame_inst_status = l1a_frame->frame_inst_status & 0xFFFFFEFF;

    //-------------------------------------------------//
    // tracking must be done to update state variables //
    //-------------------------------------------------//

    SetOrbitStepDelayAndFrequency(spacecraft, qscat);

    //--------------------------------------//
    // Add Cal-pulse Specific Info to Frame //
    //--------------------------------------//

    if (! SetL1ALoopback(qscat, l1a_frame))
        return(0);

    //---------------------------------//
    // set orbit step change indicator //
    //---------------------------------//

    unsigned short orbit_step = qscat->cds.SetAndGetOrbitStep();
    if (orbit_step != l1a_frame->orbitStep)
    {
        l1a_frame->priOfOrbitStepChange = _spotNumber;
        l1a_frame->status.prf_orbit_step_change=l1a_frame->priOfOrbitStepChange;
        // remember, the CDS puts in the last orbit step (anti-documentation)
        l1a_frame->orbitStep = orbit_step;
        l1a_frame->status.doppler_orbit_step = l1a_frame->orbitStep;
    }

    //-----------------------------//
    // determine if frame is ready //
    //-----------------------------//

    if (_spotNumber >= l1a_frame->spotsPerFrame)
    {
        // Again, cal pulses will probably never end a frame, but just
        // in case, we include this code...
        l1aFrameReady = 1;  // indicate frame is ready
        _spotNumber = 0;    // prepare to start a new frame
    }
    else
    {
        l1aFrameReady = 0;  // indicate frame is not ready
    }

    return(1);
}

//-------------------//
// QscatSim::LoadSim //
//-------------------//

int
QscatSim::LoadSim(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    L1AFrame*    l1a_frame)
{
    //----------------------------------------//
    // compute frame header info if necessary //
    //----------------------------------------//

    L1AFrameInit(spacecraft,qscat,l1a_frame);

    //-------------------------------------------------//
    // tracking must be done to update state variables //
    //-------------------------------------------------//

    SetOrbitStepDelayAndFrequency(spacecraft, qscat);

    //--------------------------------------//
    // Add Cal-pulse Specific Info to Frame //
    //--------------------------------------//

    if (! SetL1ALoad(qscat, l1a_frame))
        return(0);

    //---------------------------------//
    // set orbit step change indicator //
    //---------------------------------//

    unsigned short orbit_step = qscat->cds.SetAndGetOrbitStep();
    if (orbit_step != l1a_frame->orbitStep)
    {
        l1a_frame->priOfOrbitStepChange = _spotNumber;
        l1a_frame->status.prf_orbit_step_change=l1a_frame->priOfOrbitStepChange;
        // remember, the CDS puts in the last orbit step (anti-documentation)
        l1a_frame->orbitStep = orbit_step;
        l1a_frame->status.doppler_orbit_step = l1a_frame->orbitStep;
    }

    //-----------------------------//
    // determine if frame is ready //
    //-----------------------------//

    if (_spotNumber >= l1a_frame->spotsPerFrame)
    {
        // Again, cal pulses will probably never end a frame, but just
        // in case, we include this code...
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
// QscatSim::SetL1ASpacecraft //
//----------------------------//

int
QscatSim::SetL1ASpacecraft(
    Spacecraft*  spacecraft,
    L1AFrame*    l1a_frame)
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

//---------------------------//
// QscatSim::SetMeasurements //
//---------------------------//

int
QscatSim::SetMeasurements(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    MeasSpot*    meas_spot,
    CheckFrame*  cf,
    WindField*   windfield,
    GMF*         gmf,
    Kp*          kp,
    KpmField*    kpmField)
{
	//-------------------------//
	// for each measurement... //
	//-------------------------//

	int slice_count = qscat->ses.GetTotalSliceCount();
	int slice_i = 0;
	int sliceno = -slice_count / 2;
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
        meas->landFlag=landMap.IsLand(lon,lat);

		float sigma0;
        if (meas->landFlag==1)
        {
            // Set sigma0 to average NSCAT land sigma0 for appropriate
            // incidence angle and polarization
            if (meas->pol == H_POL)
                sigma0=0.085;
			else
                sigma0=0.1;
			if (simVs1BCheckfile)
			{
				cf->sigma0[slice_i] = sigma0;
				cf->wv[slice_i].spd = 0.0;
				cf->wv[slice_i].dir = 0.0;
			}
		}
		else if (uniformSigmaField)
		{
			sigma0=1;
			if (simVs1BCheckfile)
			{
				cf->sigma0[slice_i] = 1.0;
				cf->wv[slice_i].spd = 0.0;
				cf->wv[slice_i].dir = 0.0;
			}
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

			gmf->GetInterpolatedValue(meas->pol, meas->incidenceAngle, wv.spd,
				chi, &sigma0);

			if (simVs1BCheckfile)
			{
				cf->sigma0[slice_i] = sigma0;
				cf->wv[slice_i].spd = wv.spd;
				cf->wv[slice_i].dir = wv.dir;
			}

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
				if (! kp->kpm.GetKpm(meas->pol,wv.spd,&kpm_value))
				{
					printf("Error: Bad Kpm value in QscatSim::SetMeas\n");
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
        float Xfactor=0;
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
                if (! ComputeXfactor(spacecraft, qscat, meas, &Xfactor))
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
		        if (simVs1BCheckfile)
		        {
                  Xfactor = BYUX.GetXTotal(spacecraft, qscat, meas, cf);
                }
                else
		        {
                  Xfactor = BYUX.GetXTotal(spacecraft, qscat, meas, NULL);
                }
            }
            if (! sigma0_to_Esn_slice_given_X(qscat, meas, Xfactor, sigma0,
                simKpcFlag, &(meas->value), &Es, &En,
                &var_esn_slice))
            {
                return(0);
            }
            meas->XK=Xfactor;
		}
        else
        {
            Kfactor=1.0;  // default to use if no Kfactor specified.
            if (useKfactor)
            {
                float orbit_position = qscat->cds.OrbitFraction();

                Kfactor = kfactorTable.RetrieveByRelativeSliceNumber(
                    qscat->cds.currentBeamIdx,
                    qscat->sas.antenna.txCenterAzimuthAngle, orbit_position,
                    sliceno);
            }

            //--------------------------------//
            // generate the coordinate switch //
            //--------------------------------//

            gc_to_antenna = AntennaFrameToGC(&(spacecraft->orbitState),
                &(spacecraft->attitude), &(qscat->sas.antenna),
                qscat->sas.antenna.txCenterAzimuthAngle);
            gc_to_antenna=gc_to_antenna.ReverseDirection();
            double Tp = qscat->ses.txPulseWidth;

            if (! sigma0_to_Esn_slice(&gc_to_antenna, spacecraft, qscat, meas,
                Kfactor*Tp, sigma0, simKpcFlag, &(meas->value), &(meas->XK),
                &Es, &En, &var_esn_slice))
            {
                return(0);
            }
		}

		if (simVs1BCheckfile)
		{
            Vector3 rlook = meas->centroid - spacecraft->orbitState.rsat;
            cf->R[slice_i] = (float)rlook.Magnitude();
            if (computeXfactor || useBYUXfactor)
            {
                // Antenna gain is not computed when using BYU X factor
                // because the X factor already includes the normalized
                // patterns.  Thus, to see what it actually is, we need
                // to do the geometry work here that is normally done
                // in radar_X() when using the K-factor approach.
                gc_to_antenna = AntennaFrameToGC(&(spacecraft->orbitState),
                    &(spacecraft->attitude), &(qscat->sas.antenna),
                    qscat->sas.antenna.txCenterAzimuthAngle);
                gc_to_antenna=gc_to_antenna.ReverseDirection();
                double roundTripTime = 2.0 * cf->R[slice_i] / speed_light_kps;

                Beam* beam = qscat->GetCurrentBeam();
                Vector3 rlook_antenna = gc_to_antenna.Forward(rlook);
                double r, theta, phi;
                rlook_antenna.SphericalGet(&r,&theta,&phi);
                if (! beam->GetPowerGainProduct(theta, phi, roundTripTime,
                    qscat->sas.antenna.spinRate, &(cf->GatGar[slice_i])))
                {
                    cf->GatGar[slice_i] = 1.0;	// set a dummy value.
                }
            }
            else
            {
                double lambda = speed_light_kps / qscat->ses.txFrequency;
                cf->GatGar[slice_i] = meas->XK / Kfactor * (64*pi*pi*pi *
                    cf->R[slice_i] * cf->R[slice_i]*cf->R[slice_i]*
                    cf->R[slice_i] * qscat->systemLoss) /
                    (qscat->ses.transmitPower * qscat->ses.rxGainEcho *
                    lambda * lambda);
            }

            cf->idx[slice_i] = meas->startSliceIdx;
            cf->var_esn_slice[slice_i] = var_esn_slice;
            cf->Es[slice_i] = Es;
            cf->En[slice_i] = En;
			cf->XK[slice_i] = meas->XK;
			cf->centroid[slice_i] = meas->centroid;
			cf->azimuth[slice_i] = meas->eastAzimuth;
			cf->incidence[slice_i] = meas->incidenceAngle;
		}

		sliceno++;
		slice_i++;
		if (slice_count%2==0 && sliceno==0)
            sliceno++;
		meas=meas_spot->GetNext();
	}

	return(1);
}

//-------------------------//
// QscatSim::SetL1AScience //
//-------------------------//

int
QscatSim::SetL1AScience(
    MeasSpot*    meas_spot,
    CheckFrame*  cf,
    Qscat*       qscat,
    L1AFrame*    l1a_frame)
{
    //----------//
    // set PtGr //
    //----------//
    // Only "noise it up" if simKpriFlag is set

    l1a_frame->ptgr = qscat->ses.transmitPower * qscat->ses.rxGainEcho;
    if (simVs1BCheckfile)
    {
        cf->ptgr = l1a_frame->ptgr;
    }

    if (simKpriFlag)
        l1a_frame->ptgr *= (1 + ptgrNoise.GetNumber(qscat->cds.time));

    //----------------------//
    // set antenna position //
    //----------------------//

    l1a_frame->antennaPosition[_spotNumber] = qscat->cds.rawEncoder;

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
    sigma0_to_Esn_noise(qscat, meas_spot, simKpcFlag,
        &(l1a_frame->spotNoise[_spotNumber]));

    _spotNumber++;

    return(1);
}

//--------------------------//
// QscatSim::SetL1ALoopback //
//--------------------------//

int
QscatSim::SetL1ALoopback(
    Qscat*       qscat,
    L1AFrame*    l1a_frame)
{

    //----------------------//
    // set antenna position //
    //----------------------//

    l1a_frame->antennaPosition[_spotNumber] = qscat->cds.rawEncoder;

    //-------------------------------------------//
    // Set Es_cal using PtGr.                    //
    // Only "noise it up" if simKpriFlag is set. //
    //-------------------------------------------//

    float PtGr = qscat->ses.transmitPower * qscat->ses.rxGainEcho;
//    if (simVs1BCheckfile)
//    {
//        cf->ptgr = PtGr;
//    }

    float Esn_echo_cal,Esn_noise_cal;
    PtGr_to_Esn(PtGr,&ptgrNoise,qscat,simKpriFlag,&Esn_echo_cal,&Esn_noise_cal);

    //-------------------//
    // for each slice... //
    //-------------------//

    int base_slice_number = _spotNumber * l1a_frame->slicesPerSpot;
    for (int i=0; i < l1a_frame->slicesPerSpot; i++)
    {
        //----------------------------------------------------------------//
        // Update the level 1A frame.
        // Here, we set each slice to zero because the loopback energy
        // is concentrated in one slice (which we add after this loop).
        // A higher fidelity simulation would set some background thermal
        // noise along with Kpc style variance in the noise and signal
        // energies. Data is set into the science data just like the
        // instrument, and into separate storage just like the ground
        // processing system.
        //----------------------------------------------------------------//

        l1a_frame->loopbackSlices[i] = 0.0;
        l1a_frame->science[base_slice_number + i] = 0.0;
    }

    // Now set the single slice with loopback energy.
    int icenter = l1a_frame->slicesPerSpot/2;
    l1a_frame->loopbackSlices[icenter] = Esn_echo_cal;
    l1a_frame->science[base_slice_number + icenter] = Esn_echo_cal;

    //----------------------------------------------//
    // Set corresponding noise channel measurements //
    //----------------------------------------------//

    l1a_frame->loopbackNoise = Esn_noise_cal;
    l1a_frame->spotNoise[_spotNumber] = Esn_noise_cal;

    //--------------------------------------------------------//
    // Set cal position indicator so that the actual position //
    // is one less than the unit offset position index        //
    // (follows instrument telemetry).                        //
    // Note that this is only done with loopbacks, and not    //
    // for load measurements which always follow a loopback.  //
    // Then increment the spot counter.                       //
    //--------------------------------------------------------//

    l1a_frame->calPosition = _spotNumber + 2;
    l1a_frame->in_eu.true_cal_pulse_pos = l1a_frame->calPosition + 1;
    _spotNumber++;

    return(1);
}

//----------------------//
// QscatSim::SetL1ALoad //
//----------------------//

int
QscatSim::SetL1ALoad(
    Qscat*       qscat,
    L1AFrame*    l1a_frame)
{

    //----------------------//
    // set antenna position //
    //----------------------//

    l1a_frame->antennaPosition[_spotNumber] = qscat->cds.rawEncoder;

    //-------------------------------------------//
    // Compute load noise measurements to assure //
    // a perfect retrieval of alpha.             //
    //-------------------------------------------//

    float En_echo_load;
    float En_noise_load;
    make_load_measurements(qscat,&En_echo_load,&En_noise_load);

    //-------------------//
    // for each slice... //
    //-------------------//

    int base_slice_number = _spotNumber * l1a_frame->slicesPerSpot;
    for (int i=0; i < l1a_frame->slicesPerSpot; i++)
    {
        //----------------------------------------------------------------//
        // Update the level 0.0 frame
        // Here, we set each slice to the same number so that they
        // add up to the echo channel energy computed above.
        // A higher fidelity simulation would set each with its own
        // variance (Kpc style) which would introduce some noise into alpha.
        // Data is set into the science data just like the instrument,
        // and into separate storage just like the ground processing system.
        //----------------------------------------------------------------//

        l1a_frame->loadSlices[i] = En_echo_load/l1a_frame->slicesPerSpot;
        l1a_frame->science[base_slice_number + i] =
          En_echo_load/l1a_frame->slicesPerSpot;
    }

    //----------------------------------------------//
    // Set corresponding noise channel measurements //
    //----------------------------------------------//

    l1a_frame->loadNoise = En_noise_load;
    l1a_frame->spotNoise[_spotNumber] = En_noise_load;

    _spotNumber++;

    return(1);
}

//--------------------------//
// QscatSim::ComputeXfactor //
//--------------------------//

int
QscatSim::ComputeXfactor(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    Meas*        meas,
    float*       X)
{
    double lambda = speed_light_kps / qscat->ses.txFrequency;
    if (! IntegrateSlice(spacecraft, qscat, meas, numLookStepsPerSlice,
        azimuthIntegrationRange, azimuthStepSize, rangeGateClipping, X))
    {
        return(0);
    }

    (*X) *= qscat->ses.transmitPower * qscat->ses.rxGainEcho *
        lambda*lambda / (64*pi*pi*pi * qscat->systemLoss);
    return(1);
}
