//==============================================================//
// Copyright (C) 1998-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_qscatsim_c[] =
    "@(#) $Id$";

#include "QscatSim.h"
#include "CheckFrame.h"
#include "InstrumentGeom.h"
#include "Sigma0.h"
#include "AccurateGeom.h"
#include "Beam.h"
#include "Sigma0Map.h"

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
:   pulseCount(0), epochTime(0.0), epochTimeString(NULL), startTime(0),
    lastEventType(QscatEvent::NONE), numLookStepsPerSlice(0),
    azimuthIntegrationRange(0.0), azimuthStepSize(0.0), dopplerBias(0.0),
    correlatedKpm(0.0), simVs1BCheckfile(NULL), uniformSigmaField(0),
    uniformSigmaValue(0.0), outputXToStdout(0), useKfactor(0),
    createXtable(0), computeXfactor(0), useBYUXfactor(0),
    rangeGateClipping(0), applyDopplerError(0), l1aFrameReady(0),
    simKpcFlag(0), simCorrKpmFlag(0), simUncorrKpmFlag(0), simKpriFlag(0),
    _spotNumber(0), _spinUpPulses(2), _calPending(0)
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
    int          spots_per_frame,
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
    case QscatEvent::SCAT_EVENT:
        if ((ideal_encoder > NINETY_DEGREE_ENCODER &&
             lastEventIdealEncoder <= NINETY_DEGREE_ENCODER) ||
            _calPending == 1)
        {
            if (_spotNumber < 5 || _spotNumber > spots_per_frame - 5)
            {
              _calPending = 1;
            }
            else
            {
              qscat_event->eventId = QscatEvent::LOOPBACK_EVENT;
              _calPending = 0;
            }
        }
        else
        {
            qscat_event->eventId = QscatEvent::SCAT_EVENT;
        }
        break;
    case QscatEvent::LOOPBACK_EVENT:
        qscat_event->eventId = QscatEvent::LOAD_EVENT;
        break;
    case QscatEvent::LOAD_EVENT:
    case QscatEvent::NONE:
        qscat_event->eventId = QscatEvent::SCAT_EVENT;
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
        l1a_frame->instrumentTicks = qscat->cds.instrumentTime;
        l1a_frame->priOfOrbitStepChange = 255;      // flag value
        l1a_frame->calPosition = 255;    // no cal pulses yet

        //---------------------------------//
        // Set GS data from the simulation //
        //---------------------------------//

        // GS Status block
        l1a_frame->status.doppler_orbit_step = l1a_frame->orbitStep;
        l1a_frame->status.prf_orbit_step_change=l1a_frame->priOfOrbitStepChange;
        l1a_frame->status.specified_cal_pulse_pos = 255;  // no cal pulses yet.
        l1a_frame->in_eu.true_cal_pulse_pos = 255; // ditto

        SesBeamInfo* ses_beam_info = qscat->GetCurrentSesBeamInfo();
        CdsBeamInfo* cds_beam_info = qscat->GetCurrentCdsBeamInfo();
        Beam* cur_beam = qscat->GetCurrentBeam();
        // all times in GS-L1A are ms (not sec).
        l1a_frame->in_eu.prf_cycle_time_eu = 1e3*qscat->ses.pri;
        if (cur_beam->polarization == H_POL)
        {
          l1a_frame->in_eu.range_gate_delay_inner = 1e3*qscat->ses.rxGateDelay;
          l1a_frame->in_eu.range_gate_width_inner =
            1e3*ses_beam_info->rxGateWidth;
          l1a_frame->status.range_gate_a_delay = qscat->cds.rxGateDelayDn;
          l1a_frame->status.range_gate_a_width = cds_beam_info->rxGateWidthDn;
          l1a_frame->in_eu.transmit_power_inner = qscat->ses.transmitPower;
        }
        else
        {
          l1a_frame->in_eu.range_gate_delay_outer = 1e3*qscat->ses.rxGateDelay;
          l1a_frame->in_eu.range_gate_width_outer =
            1e3*ses_beam_info->rxGateWidth;
          l1a_frame->status.range_gate_b_delay = qscat->cds.rxGateDelayDn;
          l1a_frame->status.range_gate_b_width = cds_beam_info->rxGateWidthDn;
          l1a_frame->in_eu.transmit_power_outer = qscat->ses.transmitPower;
        }

        l1a_frame->in_eu.transmit_pulse_width = 1e3*qscat->ses.txPulseWidth;
        l1a_frame->in_eu.precision_coupler_temp_eu =
          qscat->ses.physicalTemperature;
        l1a_frame->in_eu.rcv_protect_sw_temp_eu =
          qscat->ses.physicalTemperature;
        l1a_frame->in_eu.beam_select_sw_temp_eu =
          qscat->ses.physicalTemperature;
        l1a_frame->in_eu.receiver_temp_eu =
          qscat->ses.physicalTemperature;

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
        float eff_gate_width = 1000*(ses_beam_info->rxGateWidth -
          qscat->ses.txPulseWidth);
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

        // Use instrumentTime to make sure the time string is consistent
        // with vtcw (used by SeaPAC L1A processor).
        set_character_time(qscat->cds.instrumentTime/32.0,
                           epochTime, epochTimeString,
                           l1a_frame->frame_time);
        l1a_frame->status.prf_count = l1a_frame->spotsPerFrame;
        l1a_frame->status.prf_cycle_time = qscat->cds.priDn;
        l1a_frame->status.pulse_width = qscat->cds.txPulseWidthDn;

        // transfer instrument time in microseconds into vtcw.
        double vtcw_time = 1e6*qscat->cds.instrumentTime/32.0;
        unsigned int vtcw_time_hi4 = (int)(vtcw_time/65536);
        unsigned short vtcw_time_lo2 =
          (unsigned short)(vtcw_time - (double)(vtcw_time_hi4)*65536.0);
        memcpy((void *)(l1a_frame->status.vtcw), (void *)(&vtcw_time_hi4),
               sizeof(unsigned int));
        memcpy((void *)(l1a_frame->status.vtcw+4), (void *)(&vtcw_time_lo2),
               sizeof(unsigned short));
        // convert instrument time to 5 bytes (with zero fractional part)
        memcpy((void *)(l1a_frame->status.corres_instr_time),
               (void *)&(qscat->cds.instrumentTime),
               sizeof(unsigned int));
        l1a_frame->status.corres_instr_time[4] = 0; // zero fractional part

        //-----------------------------------------------------------------//
        // Set all temperatures in frame enginnering data to physical temp.
        //-----------------------------------------------------------------//

        static int ii[22] = {13,14,15,28,29,30,31,32,33,46,47,48,49,50,51,52,
                        53,54,55,56,57,58};
        char* ptr = (char*)&(l1a_frame->engdata);
        for (int i=0; i < 22; i++)
        {
          ptr[ii[i]] = qscat->ses.tempToDn(qscat->ses.physicalTemperature);
        }

        l1a_frame->engdata.precision_coupler_temp =
          qscat->ses.tempToDn(qscat->ses.physicalTemperature);
        l1a_frame->engdata.rcv_protect_sw_temp =
          l1a_frame->engdata.precision_coupler_temp;
        l1a_frame->engdata.beam_select_sw_temp =
          l1a_frame->engdata.precision_coupler_temp;
        l1a_frame->engdata.receiver_temp =
          l1a_frame->engdata.precision_coupler_temp;

        //-------------------------------------------------//
        // Set remaining GS data from V2B2.2 benchmark run //
        //-------------------------------------------------//

        // Status Block

        short int2 = 0;
        int int4 = 0;

        int2 = 514;
        (void)memcpy((void*)l1a_frame->status.telemetry_table_id,
                     (void*)(&int2), 2);
        l1a_frame->status.status_error_flags = 0;
        // The next 3 are just sample data, with no relevance to this sim run.
        l1a_frame->status.table_readout_type = 11;
        int2 = 1164;
        (void)memcpy((void*)l1a_frame->status.table_readout_offset,
                     (void*)(&int2), 2);
        int4 = 1498831956;
        (void)memcpy((void*)l1a_frame->status.table_readout_data,
                     (void*)(&int4), 4);
        l1a_frame->status.operational_mode = 14;  // Set WOM always.
        // prf_count set above.
        int2 = 1888;
        (void)memcpy((void*)l1a_frame->status.status_change_flags,
                     (void*)(&int2), 2);
        int2 = 0;
        (void)memcpy((void*)l1a_frame->status.error_message,
                     (void*)(&int2), 2);
        // The error message history should be irrelevant to this run.
        int2 = 110;
        (void)memcpy((void*)&(l1a_frame->status.error_message_history[0]),
                     (void*)(&int2), 2);
        int2 = 34052;
        (void)memcpy((void*)&(l1a_frame->status.error_message_history[2]),
                     (void*)(&int2), 2);
        int2 = 34062;
        (void)memcpy((void*)&(l1a_frame->status.error_message_history[4]),
                     (void*)(&int2), 2);
        int2 = 34071;
        (void)memcpy((void*)&(l1a_frame->status.error_message_history[6]),
                     (void*)(&int2), 2);
        int2 = 34084;
        (void)memcpy((void*)&(l1a_frame->status.error_message_history[8]),
                     (void*)(&int2), 2);
        l1a_frame->status.valid_command_count = 24;
        l1a_frame->status.invalid_command_count = 0;
        // specified_cal_pulse_pos set above
        // prf_cycle_time set above
        // range_gate_a_delay set above
        // range_gate_a_width set above
        // range_gate_b_delay set above
        // range_gate_b_width set above
        // doppler_shift_command_1,2 are set in QscatSim::ScatSim() when avail.
        // pulse_width set above
        // The next one is hard coded, but should come from cfg file.
        l1a_frame->status.receiver_gain = 10;  // assume 10 db attenuation
        l1a_frame->status.ses_configuration_flags = 8; // assume mod on
        l1a_frame->status.ses_data_overrun_count = 0;
        l1a_frame->status.ses_data_underrun_count = 0;
        l1a_frame->status.pred_antenna_pos_count = 0; // assume none for now
        int2 = 13;
        (void)memcpy((void*)l1a_frame->status.running_error_count,
                     (void*)(&int2), 2);
        l1a_frame->status.ses_reset_position = -1;
        // doppler_orbit_step set above
        // prf_orbit_step_change set above
        // The cmd history queue should be irrelevant to this run.
        int2 = 61454;
        (void)memcpy((void*)&(l1a_frame->status.error_message_history[0]),
                     (void*)(&int2), 2);
        int2 = 43761;
        (void)memcpy((void*)&(l1a_frame->status.error_message_history[2]),
                     (void*)(&int2), 2);
        int2 = 43737;
        (void)memcpy((void*)&(l1a_frame->status.error_message_history[4]),
                     (void*)(&int2), 2);
        int2 = 43731;
        (void)memcpy((void*)&(l1a_frame->status.error_message_history[6]),
                     (void*)(&int2), 2);
        l1a_frame->status.calc_ant_max_grp_count = 0;
        // vtcw set above
        // corres_inst_time set above
        l1a_frame->status.fsw_mission_version_num = 65;
        l1a_frame->status.fsw_build_number = 3;
        l1a_frame->status.pbi_flag = 0;

        // Engineering Block
        l1a_frame->engdata.transmit_power_a = 142; // 49.3 dBm
        l1a_frame->engdata.transmit_power_b = 142; // 49.3 dBm
        l1a_frame->engdata.a2d_p12v_xcpl = 188;
        // Look at the tlm dictionary, Table 14, pg. B-33.
        int2 = 24635;
        (void)memcpy((void*)l1a_frame->engdata.relay_status,
                     (void*)(&int2), 2);
        l1a_frame->engdata.ea_a_spin_rate = 183;
        l1a_frame->engdata.ea_b_spin_rate = 0;
        l1a_frame->engdata.eng_status_c1 = 28;
        l1a_frame->engdata.eng_status_c2 = 16;
        l1a_frame->engdata.eng_status_c3 = 0;
    }
    else if (_spotNumber == 1)
    {
        //----------------------------------//
        // Store data needed from 2nd pulse //
        //----------------------------------//

        SesBeamInfo* ses_beam_info = qscat->GetCurrentSesBeamInfo();
        CdsBeamInfo* cds_beam_info = qscat->GetCurrentCdsBeamInfo();
        Beam* cur_beam = qscat->GetCurrentBeam();
        if (cur_beam->polarization == H_POL)
        {
          l1a_frame->in_eu.range_gate_delay_inner = 1e3*qscat->ses.rxGateDelay;
          l1a_frame->in_eu.range_gate_width_inner =
            1e3*ses_beam_info->rxGateWidth;
          l1a_frame->status.range_gate_a_delay = qscat->cds.rxGateDelayDn;
          l1a_frame->status.range_gate_a_width = cds_beam_info->rxGateWidthDn;
          l1a_frame->in_eu.transmit_power_inner = qscat->ses.transmitPower;
        }
        else
        {
          l1a_frame->in_eu.range_gate_delay_outer = 1e3*qscat->ses.rxGateDelay;
          l1a_frame->in_eu.range_gate_width_outer =
            1e3*ses_beam_info->rxGateWidth;
          l1a_frame->status.range_gate_b_delay = qscat->cds.rxGateDelayDn;
          l1a_frame->status.range_gate_b_width = cds_beam_info->rxGateWidthDn;
          l1a_frame->in_eu.transmit_power_outer = qscat->ses.transmitPower;
        }

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
    Sigma0Map*   inner_map,
    Sigma0Map*   outer_map,
    GMF*         gmf,
    Kp*          kp,
    KpmField*    kpmField,
    L1AFrame*    l1a_frame)
{
    CheckFrame cf;
    if (simVs1BCheckfile)
    {
        if (!cf.Allocate(qscat->ses.GetTotalSliceCount()))
        {
            fprintf(stderr, "Error allocating a CheckFrame\n");
            return(0);
        }
    }

    MeasSpot meas_spot;

    //----------------------------------------//
    // compute frame header info if necessary //
    //----------------------------------------//

    L1AFrameInit(spacecraft, qscat, l1a_frame);

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

    //---------------------------------------------//
    // Set commanded doppler for first two pulses. //
    // This is only needed for GS processing.      //
    //---------------------------------------------//

    if (_spotNumber == 0)
    {
      (void)memcpy((void*)(&(l1a_frame->status.doppler_shift_command_1[1])),
                   (void*)(&(qscat->cds.txDopplerDn)), sizeof(short));
      Beam* cur_beam = qscat->GetCurrentBeam();
      if (cur_beam->polarization == H_POL)
      {
        l1a_frame->status.doppler_shift_command_1[0] = 0;
      }
      else
      {
        l1a_frame->status.doppler_shift_command_1[0] = 1;
      }
    }
    else if (_spotNumber == 1)
    {
      (void)memcpy((void*)(&(l1a_frame->status.doppler_shift_command_2[1])),
                   (void*)(&(qscat->cds.txDopplerDn)), sizeof(short));
      Beam* cur_beam = qscat->GetCurrentBeam();
      if (cur_beam->polarization == H_POL)
      {
        l1a_frame->status.doppler_shift_command_2[0] = 0;
      }
      else
      {
        l1a_frame->status.doppler_shift_command_2[0] = 1;
      }
    }

    //---------------------//
    // Create measurements //
    //---------------------//

    if (! qscat->MakeSlices(&meas_spot))
        return(0);

    //---------------------//
    // locate measurements //
    //---------------------//

    if (qscat->ses.scienceSlicesPerSpot <= 1)
    {
        if (! qscat->LocateSpot(spacecraft, &meas_spot))
            return(0);
    }
    else
    {
        if (! qscat->LocateSliceCentroids(spacecraft, &meas_spot))
            return(0);
    }

    //--------------------------------------//
    // determine measurement type from beam //
    //--------------------------------------//

    Beam* beam = qscat->GetCurrentBeam();
    Meas::MeasTypeE meas_type = PolToMeasType(beam->polarization);

    for (Meas* meas = meas_spot.GetHead(); meas; meas = meas_spot.GetNext())
    {
        meas->measType = meas_type;
    }

    //------------------------//
    // set measurement values //
    //------------------------//

    if (! SetMeasurements(spacecraft, qscat, &meas_spot, &cf,
        windfield, inner_map, outer_map, gmf, kp, kpmField))
    {
        return(0);
    }

    //---------------------------------------//
    // Output X values to X table if enabled //
    //---------------------------------------//

    if (createXtable)
    {
        for (Meas* meas = meas_spot.GetHead(); meas;
            meas = meas_spot.GetNext())
        {
            float orbit_position = qscat->cds.OrbitFraction();

            if (! xTable.AddEntry(meas->XK, qscat->cds.currentBeamIdx,
                qscat->sas.antenna.txCenterAzimuthAngle, orbit_position,
                meas->startSliceIdx))
            {
                return(0);
            }
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
        for (Meas* meas = meas_spot.GetHead(); meas;
            meas = meas_spot.GetNext())
        {
            if (XK_max < meas->XK) XK_max=meas->XK;
        }
        float total_spot_X=0;
        float total_spot_power=0;
        for (Meas* meas=meas_spot.GetHead(); meas; meas=meas_spot.GetNext())
        {
            int slice_count = qscat->ses.GetTotalSliceCount();
            int slice_idx;
            if (!rel_to_abs_idx(meas->startSliceIdx,slice_count,&slice_idx))
            {
                fprintf(stderr,"ScatSim: Bad slice number\n");
                exit(1);
            }
            float dummy, freq;
            qscat->ses.GetSliceFreqBw(slice_idx, &freq, &dummy);

            float Es_cal = true_Es_cal(qscat);
            double Xcaldb;
            radar_Xcal(qscat,Es_cal,&Xcaldb);
            Xcaldb = 10.0 * log10(Xcaldb);

            float XKdb=10*log10(meas->XK);

            printf("%g ",XKdb-Xcaldb);
//               float delta_freq=BYUX.GetDeltaFreq(spacecraft);
//               printf("%g ", delta_freq);
            total_spot_power+=meas->value;
            total_spot_X+=meas->XK;

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

    _spotNumber++;  // Move to next pulse
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
        cf.pulseCount = pulseCount;
        cf.ptgr = qscat->ses.transmitPower * qscat->ses.rxGainEcho;
        cf.time = qscat->cds.time;
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
        cf.EsCal = true_Es_cal(qscat);
        cf.alpha = qscat->ses.noiseBandwidth /
                     qscat->ses.GetTotalSignalBandwidth();
        cf.WriteDataRec(fptr);
        fclose(fptr);
    }

    pulseCount++;
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

    _spotNumber++;  // Move to next pulse
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

    pulseCount++;
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

    _spotNumber++;  // Move to next pulse
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

    pulseCount++;
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
    Sigma0Map*   inner_map,
    Sigma0Map*   outer_map,
    GMF*         gmf,
    Kp*          kp,
    KpmField*    kpmField)
{
    //-------------------------//
    // for each measurement... //
    //-------------------------//

    int slice_i = 0;
    Meas* meas = meas_spot->GetHead();
    while (meas)
    {
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
        meas->landFlag=landMap.IsLand(lon, lat);

        float sigma0;
        if (uniformSigmaField)
        {
            sigma0=uniformSigmaValue;
            if (simVs1BCheckfile)
            {
                cf->sigma0[slice_i] = sigma0;
                cf->wv[slice_i].spd = 0.0;
                cf->wv[slice_i].dir = 0.0;
            }
        }
        else if (meas->landFlag==1)
        {
            //-------------------------------------------//
            // LAND! Try to use the inner and outer maps //
            //-------------------------------------------//

            if (meas->measType == Meas::HH_MEAS_TYPE)
            {
                // inner beam, use inner_map
                if (inner_map != NULL)
                    sigma0 = inner_map->GetSigma0(lon, lat);
                else
                    sigma0 = landSigma0[0];
            }
            else
            {
                // outer beam, use outer_map
                if (outer_map != NULL)
                    sigma0 = outer_map->GetSigma0(lon, lat);
                else
                    sigma0 = landSigma0[1];
            }

            if (simVs1BCheckfile)
            {
                cf->sigma0[slice_i] = sigma0;
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

            gmf->GetInterpolatedValue(meas->measType, meas->incidenceAngle,
                wv.spd, chi, &sigma0);

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
/* old gaussian pdf approach
                double kpm_value;
                if (! kp->kpm.GetKpm(meas->measType, wv.spd, &kpm_value))
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
*/
                double kpm2;
                if (! kp->GetKpm2(meas->measType, wv.spd, &kpm2))
                {
                    printf("Error: Bad Kpm value in QscatSim::SetMeas\n");
                    exit(-1);
                }
                Gamma gammaRv(sigma0*sigma0*kpm2,sigma0);
                sigma0 = gammaRv.GetNumber();
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
            if (! MeasToEsnX(qscat, meas, Xfactor, sigma0,
                             &(meas->value), &Es, &En, &var_esn_slice))
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
                    meas->startSliceIdx);
            }

            //--------------------------------//
            // generate the coordinate switch //
            //--------------------------------//

            gc_to_antenna = AntennaFrameToGC(&(spacecraft->orbitState),
                &(spacecraft->attitude), &(qscat->sas.antenna),
                qscat->sas.antenna.txCenterAzimuthAngle);
            gc_to_antenna=gc_to_antenna.ReverseDirection();
            double Tp = qscat->ses.txPulseWidth;

            if (! MeasToEsnK(spacecraft, qscat, meas, Kfactor*Tp, sigma0,
                             &(meas->value), &Es, &En,
                             &var_esn_slice, &(meas->XK)))
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
                    cf->GatGar[slice_i] = 1.0;    // set a dummy value.
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
            cf->measType[slice_i] = meas->measType;
            cf->var_esn_slice[slice_i] = var_esn_slice;
            cf->Es[slice_i] = Es;
            cf->En[slice_i] = En;
            cf->XK[slice_i] = meas->XK;
            cf->centroid[slice_i] = meas->centroid;
            cf->azimuth[slice_i] = meas->eastAzimuth;
            cf->incidence[slice_i] = meas->incidenceAngle;
        }

        slice_i++;
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
    //----------------------//
    // set antenna position //
    //----------------------//

    l1a_frame->antennaPosition[_spotNumber] = qscat->cds.rawEncoder;

    //-------------------------//
    // for each measurement... //
    //-------------------------//

    int slice_number = _spotNumber * l1a_frame->slicesPerSpot;
    if (simVs1BCheckfile) cf->EsnEcho = 0.0;
    for (Meas* meas = meas_spot->GetHead(); meas;
        meas = meas_spot->GetNext())
    {
        //----------------------------//
        // update the level 0.0 frame //
        //----------------------------//

        l1a_frame->science[slice_number] = (unsigned int)(meas->value);
        if (simVs1BCheckfile) cf->EsnEcho += meas->value;
        slice_number++;
    }

    // Compute the spot noise measurement.
    float spot_noise;
    sigma0_to_Esn_noise(qscat, meas_spot, simKpcFlag, &spot_noise);
    l1a_frame->spotNoise[_spotNumber] = (unsigned int)spot_noise;
    if (simVs1BCheckfile) cf->EsnNoise = spot_noise;

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

    float Esn_echo_cal,Esn_noise_cal;
    PtGr_to_Esn(&ptgrNoise,qscat,simKpriFlag,&Esn_echo_cal,&Esn_noise_cal);

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

        l1a_frame->loopbackSlices[i] = 0;
        l1a_frame->science[base_slice_number + i] = 0;
    }

    // Now set the single slice with loopback energy.
    // Note the scale factor of 256 (8 bit shift) applied only to echo channel.
    int icenter = 4;
    l1a_frame->loopbackSlices[icenter] = (unsigned int)(Esn_echo_cal/256.0);
    l1a_frame->science[base_slice_number + icenter] =
      (unsigned int)(Esn_echo_cal/256.0);

    //----------------------------------------------//
    // Set corresponding noise channel measurements //
    //----------------------------------------------//

    l1a_frame->loopbackNoise = (unsigned int)(Esn_noise_cal);
    l1a_frame->spotNoise[_spotNumber] = (unsigned int)(Esn_noise_cal);

    //--------------------------------------------------------//
    // Set cal position indicator so that the actual position //
    // is one less than the unit offset position index        //
    // (follows instrument telemetry).                        //
    // Note that this is only done with loopbacks, and not    //
    // for load measurements which always follow a loopback.  //
    //--------------------------------------------------------//

    l1a_frame->calPosition = _spotNumber + 2;
    l1a_frame->in_eu.true_cal_pulse_pos = l1a_frame->calPosition;
    l1a_frame->status.specified_cal_pulse_pos = l1a_frame->calPosition;

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
    // a (nearly) perfect retrieval of alpha.             //
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
      // Integer truncation can cause a problem in low SNR cases because
      // each slice contributes the same truncation error.  A Kpc style
      // injection of variance would actually reduce this artificial
      // noise in the current implementation, but we aren't using it!
      //----------------------------------------------------------------//

      if (l1a_frame->slicesPerSpot == 12)
      {
        float Bs = 8314.0;   // Nominal value consistent with q-table.
        float Bg = 46190.0;  // Nominal value consistent with q-table.
        float Be = 10*Bs + 2*Bg;
        if (i == 0 || i == 11)
        {
          l1a_frame->loadSlices[i] =
            (unsigned int)(En_echo_load * qscat->ses.Qtable[i]*Bg/Be);
        }
        else
        {
          l1a_frame->loadSlices[i] =
            (unsigned int)(En_echo_load * qscat->ses.Qtable[i]*Bs/Be);
        }
      }
      else
      {  // assume slice bandwidths are all the same
        l1a_frame->loadSlices[i] =
          (unsigned int)(En_echo_load/l1a_frame->slicesPerSpot);
      }
      l1a_frame->science[base_slice_number + i] = l1a_frame->loadSlices[i];
    }

    //----------------------------------------------//
    // Set corresponding noise channel measurements //
    //----------------------------------------------//

    l1a_frame->loadNoise = (unsigned int)(En_noise_load);
    l1a_frame->spotNoise[_spotNumber] = (unsigned int)(En_noise_load);

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
    if (! IntegrateSlice(spacecraft, qscat, meas, numLookStepsPerSlice,
        azimuthIntegrationRange, azimuthStepSize, rangeGateClipping, X))
    {
        return(0);
    }
    Beam* beam = qscat->GetCurrentBeam();
    double Xcal;
    float Es_cal = true_Es_cal(qscat);

    radar_Xcal(qscat,Es_cal,&Xcal);
    (*X)*=Xcal/(beam->peakGain * beam->peakGain);
    return(1);
}

//----------------------//
// QscatSim::MeasToEsnX //
//----------------------//

// This method converts an average sigma0 measurement (of a particular type)
// to signal + noise energy measurements.
// The received energy is the sum of the signal energy and the noise energy
// that falls within the appropriate bandwidth.
// The total X factor is a required input.
// The result is fuzzed by Kpc (if requested) and by Kpm (as supplied).

int
QscatSim::MeasToEsnX(
    Qscat*  qscat,
    Meas*   meas,
    float   X,
    float   sigma0,
    float*  Esn,
    float*  Es,
    float*  En,
    float*  var_esn_slice)
{
    //------------------------//
    // Sanity check on sigma0 //
    //------------------------//

    if (fabs(sigma0) > 1.0e5)
    {
        fprintf(stderr,
          "Error: QscatSim::MeasToEsnX encountered invalid sigma0 = %g\n",
          sigma0);
        exit(-1);
    }

    SesBeamInfo* ses_beam_info = qscat->GetCurrentSesBeamInfo();
    double Tp = qscat->ses.txPulseWidth;
    double Tg = ses_beam_info->rxGateWidth;
    double Bs = meas->bandwidth;
    double L13 = qscat->ses.receivePathLoss;

    //------------------------------------------------------------------------//
    // Signal (ie., echo) energy referenced to the point just before the
    // I-Q detection occurs (ie., including the receiver gain and system loss).
    // X has units of energy because Xcal has units of Pt * Tp.
    //------------------------------------------------------------------------//

    *Es = X*sigma0;

    //------------------------------------------------------------------------//
    // Noise power spectral densities referenced the same way as the signal.
    //------------------------------------------------------------------------//

    double N0_echo = bK * qscat->systemTemperature *
        qscat->ses.rxGainEcho / L13;

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
      fprintf(stderr,"QscatSim::MeasToEsnX: Error getting Q value\n");
      exit(1);
    }

    //------------------------------------------------------------------------//
    // Noise energy within one slice referenced like the signal energy.
    //------------------------------------------------------------------------//

    double En1_slice = N0_echo * Bs*q_slice * Tp;        // noise with signal
    double En2_slice = N0_echo * Bs*q_slice * (Tg-Tp);    // noise without signal
    *En = En1_slice + En2_slice;

    //------------------------------------------------------------------------//
    // Signal + Noise Energy within one slice referenced like the signal energy.
    //------------------------------------------------------------------------//

    *Esn = *Es + *En;

    if (simKpcFlag == 0)
    {
        *var_esn_slice = 0.0;
        return(1);
    }

    //------------------------------------------------------------------------//
    // Estimate the variance of the slice signal + noise energy measurements.
    // The variance is simply the sum of the variance when the signal
    // (and noise) are present together and the variance when only noise
    // is present.  These variances come from radiometer theory, ie.,
    // the reciprocal of the time bandwidth product is the normalized variance.
    // The variance of the power is derived from the variance of the energy.
    //------------------------------------------------------------------------//

/*
    float var_esn_slice = (Es_slice + En1_slice)*(Es_slice + En1_slice) /
        (Bs * Tp) + En2_slice*En2_slice / (Bs*(Tg - Tp));
*/
    // the above equation reduces to the following...
    *var_esn_slice = (*Es + En1_slice)*(*Es + En1_slice) /
        (Bs * Tp) + N0_echo * N0_echo * Bs * (Tg - Tp);

    //------------------------------------------------------------------------//
    // Fuzz the Esn value by adding a random number drawn from
    // a gaussian distribution with the variance just computed and zero mean.
    // This includes both thermal noise effects, and fading due to the
    // random nature of the surface target.
    // When the snr is low, the Kpc fuzzing can be large enough that
    // the processing step will estimate a negative sigma0 from the
    // fuzzed power.  This is normal, and will occur in real data also.
    // The wind retrieval has to estimate the variance using the model
    // sigma0 rather than the measured sigma0 to avoid problems computing
    // Kpc for weighting purposes.
    // The Esn value itself, however, can never be negative because the
    // instrument integrates a sum of squares.
    //------------------------------------------------------------------------//

    Gamma rv(*var_esn_slice,*Esn);
    *Esn = rv.GetNumber();

/* old gaussian pdf approach
    Gaussian rv(*var_esn_slice,0.0);
    float rval;
    int i;
    for (i=0; i < 10; i++)
    {
      rval = rv.GetNumber();
      if (rval > -(*Esn))  break;
    }
    if (i >= 10)
    {
      fprintf(stderr,
        "Warning: Esn distribution artificially inflated at zero\n");
      *Esn = 0.0;
    }
    else
    {
      *Esn += rval;
    }
*/

    return(1);
}

//----------------------//
// QscatSim::MeasToEsnK //
//----------------------//

//
// This method converts an average sigma0 measurement (of a particular type)
// to signal + noise energy measurements using a K-factor approach.
// The K-factor is used along with other geometry information to compute
// an approximate X-factor which is then passed to MeasToEsnX to
// finish the conversion to energy.
// This method assumes that the
// geometry related factors such as range and antenna gain can be replaced
// by effective values (instead of integrating over the bandwidth).
// K-factor should remove any error introduced by this assumption.
// The result is fuzzed by Kpc (if requested) and by Kpm (as supplied).
//
// Inputs:
//    gc_to_antenna = pointer to a CoordinateSwitch from geocentric coordinates
//        to the antenna frame for the prevailing geometry.
//    spacecraft = pointer to current spacecraft object
//    qscat = pointer to current Qscat object
//    meas = pointer to current measurement (sigma0, cell center, area etc.)
//    Kfactor = Radar equation correction factor for this cell.
//    sigma0 = true sigma0 to assume.
//    Esn_slice = pointer to signal+noise energy in a slice.
//    X = pointer to true total X (ie., X = x*Kfactor).
//

int
QscatSim::MeasToEsnK(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    Meas*        meas,
    float        Kfactor,
    float        sigma0,
    float*       Esn,
    float*       Es,
    float*       En,
    float*       var_Esn,
    float*       X)
{
  //--------------------------------------------------------------------//
  // Setup the geometry transformation needed to compute an approximate
  // x on the fly. Little x-factor is the total X factor without the
  // K-factor.
  //--------------------------------------------------------------------//

  CoordinateSwitch gc_to_antenna = AntennaFrameToGC(&(spacecraft->orbitState),
    &(spacecraft->attitude), &(qscat->sas.antenna),
    qscat->sas.antenna.txCenterAzimuthAngle);
  gc_to_antenna = gc_to_antenna.ReverseDirection();

  //----------------------------------------------------------------------//
  // Compute the radar parameter x which includes gain, loss, and geometry
  // factors in the received power.  This is the true value.  Processing
  // uses a modified value that includes fuzzing by Kpr.  The total X
  // includes the K-factor. ie., X = x*K
  //----------------------------------------------------------------------//

  double x;
  radar_X(&gc_to_antenna, spacecraft, qscat, meas, &x);
  *X = x*Kfactor;

  return(MeasToEsnX(qscat, meas, *X, sigma0,
                    Esn, Es, En, var_Esn));
}
