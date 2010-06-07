//==============================================================//
// Copyright (C) 1998-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_ovwmsim_c[] =
    "@(#) $Id$";

#include <fstream>
#include <iomanip>

#include <iostream>
#include <string>
#include "stdlib.h"
#include "OvwmSim.h"
#include "CheckFrame.h"
#include "InstrumentGeom.h"
#include "OvwmSigma0.h"
#include "Sigma0.h"
#include "AccurateGeom.h"
#include "Beam.h"
#include "Sigma0Map.h"

#define SNR_CUTOFF 1.e-3
#define dX_THRESHOLD 0.10 // threshold for eliminate meas record with land
#define E_FACTOR 1.644    // value for sinc square to drop to 1/e

//==================//
// OvwmSimBeamInfo //
//==================//

OvwmSimBeamInfo::OvwmSimBeamInfo()
:   txTime(0.0)
{
    return;
}

OvwmSimBeamInfo::~OvwmSimBeamInfo()
{
    return;
}

//==========//
// OvwmSim //
//==========//

OvwmSim::OvwmSim()
:   pulseCount(0), epochTime(0.0), epochTimeString(NULL), startTime(0),
    lastEventType(OvwmEvent::NONE), numLookStepsPerSlice(0),
    azimuthIntegrationRange(0.0), azimuthStepSize(0.0), dopplerBias(0.0),
    correlatedKpm(0.0), simVs1BCheckfile(NULL), uniformSigmaField(0),
    uniformSigmaValue(0.0), outputXToStdout(0), useKfactor(0),
    createXtable(0), computeXfactor(0), useBYUXfactor(0),
    rangeGateClipping(0), applyDopplerError(0), l1aFrameReady(0),
    simKpcFlag(0), simCorrKpmFlag(0), simUncorrKpmFlag(0), simKpriFlag(0),
    _spotNumber(0), _spinUpPulses(2), _calPending(0),_ptr_array(NULL),
    amb_map_(NULL),X_map_(NULL),kpc_map_(NULL),gain_map_(NULL)
{
    return;
}





int OvwmSim::AllocateIntermediateArrays(float max_range_width,float max_azimuth_width){

  _max_int_range_bins=(int)ceil(integrationRangeWidthFactor*max_range_width/
    integrationStepSize); 
  _max_int_azim_bins=(int)ceil(integrationAzimuthWidthFactor*max_azimuth_width/
    integrationStepSize);  
  _ptr_array=(float**)make_array(sizeof(float),2,_max_int_range_bins,
				 _max_int_azim_bins);
 
  //generate map array for spot check
  //11 : number of pulses
  // 40: range bins
  amb_map_=(float**)make_array(sizeof(float),2,40,11);
  X_map_=(float**)make_array(sizeof(float),2,40,11);
  kpc_map_=(float**)make_array(sizeof(float),2,40,11);
  gain_map_=(float**)make_array(sizeof(float),2,40,11);


  if(_ptr_array==NULL){
    fprintf(stderr,"Error: OvwmSim unable to allocate _ptr_array\n");
    exit(1);
  }


  if(amb_map_==NULL || X_map_==NULL || kpc_map_==NULL || gain_map_ ==NULL){
    fprintf(stderr,"Error: OvwmSim unable to allocate amb, X, kpc, gain map arrays\n");
    exit(1);
  }
  return(1);
}

OvwmSim::~OvwmSim()
{
  if(_ptr_array!=NULL){
    free_array(_ptr_array,2,_max_int_range_bins);
    _ptr_array=NULL;
  }
    return;
}

//----------------------//
// OvwmSim::Initialize //
//----------------------//

int
OvwmSim::Initialize(
    Ovwm*  ovwm)
{
    for (int beam_idx = 0; beam_idx < NUMBER_OF_OVWM_BEAMS; beam_idx++)
    {
      beamInfo[beam_idx].txTime = startTime + ovwm->ses.txDelay[beam_idx];

      // lastRxTime is initialized to a a value much less than startTime
      // so that it will be irrelevant on the first transmit cycle
      beamInfo[beam_idx].lastRxTime=startTime-100;  
      beamInfo[beam_idx].rxTime=startTime-100;  

      // in ScatSim we make sure that the transmit event does not overlap
      // the receive windows of any of the beam in its own beam cycle or
      // the previous cycle. We alos make sure the transmit events all fall
      // within the current BRI.
      
      // If NUM_TWTAS=2 then inner and outer beam transmit events may not 
      // overlap.
      // If NUM_RECEIVERS = 2 then inner and outer beam receive windows may
      // not overlap.
      // Nominal case is NUM_TWTAS=4  and NUM_RECEIVERS=4 which allows overlap
      // among all transmits and among all receives.

    }
    return(1);
}

//------------------------------//
// OvwmSim::DetermineNextEvent //
//------------------------------//

#define NINETY_DEGREE_ENCODER  8191

int
OvwmSim::DetermineNextEvent(
    int          spots_per_frame,
    Ovwm*       ovwm,
    OvwmEvent*  ovwm_event)
{
    //----------------------------------------//
    // find minimum time from possible events //
    //----------------------------------------//

    int min_idx = 0;
    double min_time = beamInfo[0].txTime;
    for (int beam_idx = 1; beam_idx < NUMBER_OF_OVWM_BEAMS; beam_idx++)
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

    ovwm_event->time = min_time;
    ovwm_event->beamIdx = min_idx;

    unsigned short ideal_encoder = ovwm->cds.EstimateIdealEncoder();
    switch (lastEventType)
    {
    case OvwmEvent::SCAT_EVENT:
      ovwm_event->eventId = OvwmEvent::SCAT_EVENT;
      break;
    case OvwmEvent::NONE:
      ovwm_event->eventId = OvwmEvent::SCAT_EVENT;
      break;
    default:
      fprintf(stderr,"Bad OVWM_EVENT_TYPE: This should never happen!\n");
      exit(1);
    }

    //----------------------------//
    // update next time for event //
    //----------------------------//

    beamInfo[min_idx].txTime += ovwm->ses.bri;
    beamInfo[min_idx].lastRxTime=beamInfo[min_idx].rxTime;
    lastEventIdealEncoder = ideal_encoder;
    lastEventType = ovwm_event->eventId;

    return(1);
}

//------------------------//
// OvwmSim::L1AFrameInit //
//------------------------//

int
OvwmSim::L1AFrameInit(
    Spacecraft*  spacecraft,
    Ovwm*       ovwm,
    OvwmL1AFrame*    l1a_frame)
{
    //----------------------//
    // frame initialization //
    //----------------------//
    // incomplete
    static int first_call=1;
    if(first_call) {
      fprintf(stderr,"L1AFrameInit is still incomplete\n");
      first_call=0;
    }

    //cout << "s# " << _spotNumber << endl;

    if (_spotNumber == 0)
    {
        if (! SetL1ASpacecraft(spacecraft,l1a_frame))
            return(0);
        l1a_frame->time = ovwm->cds.time;
        //l1a_frame->orbitTicks = ovwm->cds.orbitTime;
        //l1a_frame->orbitStep = ovwm->cds.SetAndGetOrbitStep();
        //l1a_frame->instrumentTicks = ovwm->cds.instrumentTime;
        //l1a_frame->priOfOrbitStepChange = 255;      // flag value
        //l1a_frame->calPosition = 255;    // no cal pulses yet

        //---------------------------------//
        // Set GS data from the simulation //
        //---------------------------------//

        // GS Status block
/* not implement status now
        l1a_frame->status.doppler_orbit_step = l1a_frame->orbitStep;
        l1a_frame->status.prf_orbit_step_change=l1a_frame->priOfOrbitStepChange;
        l1a_frame->status.specified_cal_pulse_pos = 255;  // no cal pulses yet.
        l1a_frame->in_eu.true_cal_pulse_pos = 255; // ditto
*/

/* not implement in_eu now
        Beam* cur_beam = ovwm->GetCurrentBeam();
        // all times in GS-L1A are ms (not sec).
        l1a_frame->in_eu.prf_cycle_time_eu = 1e3*ovwm->ses.pri;
        if (cur_beam->polarization == H_POL)
        {
          l1a_frame->in_eu.range_gate_delay_inner = 1e3*ovwm->ses.rxGateDelay;
          l1a_frame->in_eu.range_gate_width_inner =
            1e3*ovwm->GetRxGateWidth();
          l1a_frame->status.range_gate_a_delay = ovwm->cds.rxGateDelayDn;
          l1a_frame->status.range_gate_a_width = ovwm->GetRxGateWidthDn();
          l1a_frame->in_eu.transmit_power_inner = ovwm->ses.transmitPower;
        }
        else
        {
          l1a_frame->in_eu.range_gate_delay_outer = 1e3*ovwm->ses.rxGateDelay;
          l1a_frame->in_eu.range_gate_width_outer =
            1e3*ovwm->GetRxGateWidth();
          l1a_frame->status.range_gate_b_delay = ovwm->cds.rxGateDelayDn;
          l1a_frame->status.range_gate_b_width = ovwm->GetRxGateWidthDn();
          l1a_frame->in_eu.transmit_power_outer = ovwm->ses.transmitPower;
        }

        l1a_frame->in_eu.transmit_pulse_width = 1e3*ovwm->ses.txPulseWidth;
        l1a_frame->in_eu.precision_coupler_temp_eu = 20;
        l1a_frame->in_eu.rcv_protect_sw_temp_eu = 20;
        l1a_frame->in_eu.beam_select_sw_temp_eu = 20;
        l1a_frame->in_eu.receiver_temp_eu = 20;
*/

        // Set Frame Inst. Status Flag bits.
/* not implement status now
        int inst_flag = 0;
        if (cur_beam->polarization == H_POL)
        {
          inst_flag = inst_flag & 0xFFFFFFFB; // turn off bit 2
        }
        else
        {
          inst_flag = inst_flag | 0x00000004; // turn on bit 2
        }
        float eff_gate_width = 1000*(ovwm->GetRxGateWidth() -
          ovwm->ses.txPulseWidth);
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
        set_character_time(ovwm->cds.instrumentTime/32.0,
                           epochTime, epochTimeString,
                           l1a_frame->frame_time);
        l1a_frame->status.prf_count = l1a_frame->spotsPerFrame;
        l1a_frame->status.prf_cycle_time = ovwm->cds.priDn;
        l1a_frame->status.pulse_width = ovwm->cds.txPulseWidthDn;

        // transfer instrument time in microseconds into vtcw.
        double vtcw_time = 1e6*ovwm->cds.instrumentTime/32.0;
        unsigned int vtcw_time_hi4 = (int)(vtcw_time/65536);
        unsigned short vtcw_time_lo2 =
          (unsigned short)(vtcw_time - (double)(vtcw_time_hi4)*65536.0);
        memcpy((void *)(l1a_frame->status.vtcw), (void *)(&vtcw_time_hi4),
               sizeof(unsigned int));
        memcpy((void *)(l1a_frame->status.vtcw+4), (void *)(&vtcw_time_lo2),
               sizeof(unsigned short));
        // convert instrument time to 5 bytes (with zero fractional part)
        memcpy((void *)(l1a_frame->status.corres_instr_time),
               (void *)&(ovwm->cds.instrumentTime),
               sizeof(unsigned int));
        l1a_frame->status.corres_instr_time[4] = 0; // zero fractional part
*/

        //-----------------------------------------------------------------//
        // Set all temperatures in frame enginnering data to 20 deg C.
        //-----------------------------------------------------------------//

/* not implement engdata now
        static int ii[22] = {13,14,15,28,29,30,31,32,33,46,47,48,49,50,51,52,
                        53,54,55,56,57,58};
        char* ptr = (char*)&(l1a_frame->engdata);
        for (int i=0; i < 22; i++)
        {
          ptr[ii[i]] = ovwm->ses.tempToDn(20);
        }

        l1a_frame->engdata.precision_coupler_temp =
          ovwm->ses.tempToDn(20);
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
        // doppler_shift_command_1,2 are set in OvwmSim::ScatSim() when avail.
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
*/
    }
    else if (_spotNumber == 1)
    {
        //----------------------------------//
        // Store data needed from 2nd pulse //
        //----------------------------------//

/* not implement in_eu or status
        Beam* cur_beam = ovwm->GetCurrentBeam();
        if (cur_beam->polarization == H_POL)
        {
          l1a_frame->in_eu.range_gate_delay_inner = 1e3*ovwm->ses.rxGateDelay;
          l1a_frame->in_eu.range_gate_width_inner =
            1e3*ovwm->GetRxGateWidth();
          l1a_frame->status.range_gate_a_delay = ovwm->cds.rxGateDelayDn;
          l1a_frame->status.range_gate_a_width = ovwm->GetRxGateWidthDn();
          l1a_frame->in_eu.transmit_power_inner = ovwm->ses.transmitPower;
        }
        else
        {
          l1a_frame->in_eu.range_gate_delay_outer = 1e3*ovwm->ses.rxGateDelay;
          l1a_frame->in_eu.range_gate_width_outer =
            1e3*ovwm->GetRxGateWidth();
          l1a_frame->status.range_gate_b_delay = ovwm->cds.rxGateDelayDn;
          l1a_frame->status.range_gate_b_width = ovwm->GetRxGateWidthDn();
          l1a_frame->in_eu.transmit_power_outer = ovwm->ses.transmitPower;
        }
*/

    }

    return(1);

}



//-------------------//
// OvwmSim::ScatSim //
//-------------------//

int
OvwmSim::ScatSim(
    Spacecraft*  spacecraft,
    Ovwm*       ovwm,
    WindField*   windfield,
    Sigma0Map*   inner_map,
    Sigma0Map*   outer_map,
    GMF*         gmf,
    Kp*          kp,
    KpmField*    kpmField,
    Topo*        topo,
    Stable*      stable,
    OvwmL1AFrame*    l1a_frame,
    PointTargetResponseTable* ptrTable,
    AmbigTable* ambigTable,
    L1B*         l1b)
{
  // incomplete 
  CheckFrame cf;
  if (simVs1BCheckfile)
    {
      if (!cf.Allocate(ovwm->GetNumberOfPixels()))
        {
	  fprintf(stderr, "Error allocating a CheckFrame\n");
	  return(0);
        }
    }
  
  MeasSpot meas_spot;

  //----------------------------------------//
  // compute frame header info if necessary //
  //----------------------------------------//

  /* now comment it out for testing ovwm_sim */
  L1AFrameInit(spacecraft, ovwm, l1a_frame);

  //printf("after l1a frame init: %20.8f\n", spacecraft->orbitState.time);

  // For now useDtc and useRgc are disabled.
  if(ovwm->cds.useRgc || ovwm->cds.useDtc){
    fprintf(stderr,"Error: USE_RGC and USE_DTC are disabled for now.\n");
    exit(1);
  }

  if (_spotNumber == 0)
    {
      // if this is the first or second pulse, "spin up" the //
      // Doppler and range tracking calculations //
      
      if (_spinUpPulses && (ovwm->cds.useRgc || ovwm->cds.useDtc))
        {
	  SetOrbitStepDelayAndFrequency(spacecraft, ovwm);
	  _spinUpPulses--;    // one less spinup pulse
	  return(2);    // indicate spin up
        }
    }

    //-----------------------------------------------//
    // command the range delay and Doppler frequency //
    //-----------------------------------------------//


    // need to modify SetDelayandFrequency and beam object to allow separate
    // tx/rx feeds
    SetOrbitStepDelayAndFrequency(spacecraft, ovwm);

    //printf("after set orbit: %20.8f\n", spacecraft->orbitState.time);

    if (applyDopplerError)
    {
        fprintf(stderr, "Need to implement Doppler errors\n");
        exit(1);
    }

    //---------------------------
    // Check Latitude bounds
    //---------------------------

    // predigest //


    Antenna* antenna = &(ovwm->sas.antenna);
    Beam* beam = ovwm->GetCurrentBeam();
    OrbitState* orbit_state = &(spacecraft->orbitState);
    Attitude* attitude = &(spacecraft->attitude);


    //--------------------------------//
    // generate the coordinate switch //
    //--------------------------------//

    CoordinateSwitch antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
        attitude, antenna, antenna->groundImpactAzimuthAngle);

    //------------------//
    // find beam center //
    //------------------//

    double look, azim;
    if (! beam->GetElectricalBoresight(&look, &azim))
        return(0);
    Vector3 vector;
    vector.SphericalSet(1.0,look,azim);

    

    // compute boresight vector,range and doppler   //
    OvwmTargetInfo oti;
    if(! ovwm->TargetInfo(&antenna_frame_to_gc,spacecraft,vector,&oti)){
      fprintf(stderr,"Warning:LocatePixels cannot find boresight on surface\n");
      return(1);
    }
    // compute radius and center for best fit local sphere
    // should probably make this a Earth Position method
    double alt,lon,gdlat;
    oti.rTarget.GetAltLonGDLat(&alt,&lon,&gdlat);
    printf("gdlat=%g lon=%g\n",gdlat*rtd,lon*rtd);
    if(gdlat<latMin || gdlat> latMax) return(0);
    int check_lon=0;
    if(lon>=lonMin && lon<= lonMax) check_lon=1;
    if(lon>=lonMin+two_pi && lon<= lonMax+two_pi) check_lon=1;
    if(check_lon==0) return(0);
    //---------------------//
    // Create measurements //
    //---------------------//

    if (! ovwm->MakePixels(&meas_spot))
        return(0);

    //---------------------//
    // locate measurements //
    //---------------------//

    if (! ovwm->LocatePixels(spacecraft, &meas_spot))
      return(0);
   
    //printf("after locate pixel: %20.8f\n", spacecraft->orbitState.time);

    CheckTiming(ovwm);

    //printf("after check time: %20.8f\n", spacecraft->orbitState.time);

    l1a_frame->spotTime[_spotNumber] = spacecraft->orbitState.time;
    l1a_frame->spotBeamIdx[_spotNumber] = ovwm->cds.currentBeamIdx;

    printf("spot # and time: %d %20.8f\n", _spotNumber, l1a_frame->spotTime[_spotNumber]);
    //exit(1);

    //--------------------------------------//
    // determine measurement type from beam //
    //--------------------------------------//

    Meas::MeasTypeE meas_type = PolToMeasType(beam->polarization,ovwm->ses.txFrequency);
    
    for (Meas* meas = meas_spot.GetHead(); meas; meas = meas_spot.GetNext())
    {
        meas->measType = meas_type;
        meas->beamIdx = ovwm->cds.currentBeamIdx;
    }

    //------------------------//
    // set measurement values //
    //------------------------//
    
    if (! SetMeasurements(spacecraft, ovwm, &meas_spot, windfield,
        inner_map, outer_map, gmf, kp, kpmField, topo, stable, &cf, ptrTable,ambigTable, (l1b!=NULL)))
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
            float orbit_position = ovwm->cds.OrbitFraction();

            if (! xTable.AddEntry(meas->XK, ovwm->cds.currentBeamIdx,
                ovwm->sas.antenna.txCenterAzimuthAngle, orbit_position,
                meas->startSliceIdx))
            {
                return(0);
            }
        }
    }
  
    //---------------------------------//
    // Add Spot Specific Info to Frame //
    //---------------------------------//

    if (! SetL1AScience(&meas_spot, &cf, ovwm, l1a_frame))
        return(0);

    //-------------------------------//
    // Output X to Stdout if enabled //
    //-------------------------------//
    /***** commented out for now
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
            int slice_count = ovwm->GetNumberOfPixels();
            int slice_idx=meas->startSliceIdx;
            float dummy, freq;
            ovwm->ses.GetSliceFreqBw(slice_idx, &freq, &dummy);

            float Es_cal = true_Es_cal(ovwm);
            double Xcaldb;
            radar_Xcal(ovwm,Es_cal,&Xcaldb);
            Xcaldb = 10.0 * log10(Xcaldb);

            float XKdb=10*log10(meas->XK);

            printf("%g ",XKdb-Xcaldb);
//               float delta_freq=BYUX.GetDeltaFreq(spacecraft);
//               printf("%g ", delta_freq);
            total_spot_power+=meas->value;
            total_spot_X+=meas->XK;

          }
        printf("\n"); //HACK
//      RangeTracker* rt= &(ovwm->sas.antenna.beam[instrument->antenna.currentBeamIdx].rangeTracker);


//      unsigned short orbit_step=rt->OrbitTicksToStep(ovwm->cds.orbitTicks,
//                 ovwm->cds.orbitTicksPerOrbit);

        //      printf("TOTALS %d %d %g %g %g %g\n",(int)orbit_step,
        //       instrument->antenna.currentBeamIdx,
        //      instrument->antenna.azimuthAngle*rtd,
        //      instrument->commandedRxGateDelay,
        //      total_spot_X,total_spot_power);
        fflush(stdout);
    }
    ***************/
    //---------------------------------//
    // set orbit step change indicator //
    //---------------------------------//

/* now no status in l1a_frame
    unsigned short orbit_step = ovwm->cds.SetAndGetOrbitStep();
    if (orbit_step != l1a_frame->orbitStep)
    {
        l1a_frame->priOfOrbitStepChange = _spotNumber;
        l1a_frame->status.prf_orbit_step_change=l1a_frame->priOfOrbitStepChange;
        // remember, the CDS puts in the last orbit step (anti-documentation)
        l1a_frame->orbitStep = orbit_step;
        l1a_frame->status.doppler_orbit_step = l1a_frame->orbitStep;
    }
*/

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
        cf.ptgr = ovwm->ses.transmitPower * ovwm->ses.rxGainEcho;
        cf.time = ovwm->cds.time;
        cf.beamNumber = ovwm->cds.currentBeamIdx;
        cf.rsat = spacecraft->orbitState.rsat;
        cf.vsat = spacecraft->orbitState.vsat;
        cf.orbitFrac = ovwm->cds.OrbitFraction();
        cf.spinRate = ovwm->sas.antenna.spinRate;
        cf.txDoppler = ovwm->ses.txDoppler;
        cf.rxGateDelay = ovwm->ses.rxGateDelay;
        cf.attitude = spacecraft->attitude;
        cf.antennaAziTx = ovwm->sas.antenna.txCenterAzimuthAngle;
        cf.antennaAziGi = ovwm->sas.antenna.groundImpactAzimuthAngle;
        cf.EsCal = true_Es_cal(ovwm);
        
        // commented out until we fix GetTotalSignalBandwidth
        //cf.alpha = ovwm->ses.noiseBandwidth /
        //             ovwm->ses.GetTotalSignalBandwidth();
        cf.WriteDataRec(fptr);
        fclose(fptr);
    }

    // output L1B directly if l1b is not NULL
    if(l1b!=NULL){
      MeasSpot* ms= new MeasSpot;
      ms->time = meas_spot.time;
      ms->scOrbitState = meas_spot.scOrbitState;
      ms->scAttitude = meas_spot.scAttitude;
      for (Meas* meas = meas_spot.GetHead(); meas; meas = meas_spot.GetNext())
	{
	  Meas* m = new Meas;
	  *m=*meas;
	  ms->Append(m);
	}
      l1b->frame.spotList.Append(ms);
    }

    pulseCount++;
    return(1);
}

//-----------------------//
// OvwmSim::CheckTimimg  //
//-----------------------//

//#define DEBUG_BAD_TIMING
int OvwmSim::CheckTiming(Ovwm* ovwm){
  static int first_call=1;
  static int num_calls=0;
  static int nBurstInit = 0;
  num_calls++;
  static double worst_so_far=ovwm->ses.bri;
  static double tx_margin=ovwm->ses.bri;
  // Check that transmission cycles don't overlap incorrectly
  // once.
  double worst_margin=tx_margin;
  double margin=0;
  if(first_call==1){
    first_call=0;
    // check for BAD numTWTAs
    if(ovwm->ses.numTWTAs!=2 && ovwm->ses.numTWTAs!=4){
      fprintf(stderr,"Error: %d TWTAs option not supported\n",
	      ovwm->ses.numTWTAs);
      exit(1);
    }
   
    // check for bad numReceievers
    if(ovwm->ses.numReceivers!=2 && ovwm->ses.numReceivers!=4){
      fprintf(stderr,"Error: %d Receiver chains option not supported\n",
	      ovwm->ses.numReceivers);
      exit(1);
    }
       
    
    for(int b=0;b<NUMBER_OF_OVWM_BEAMS;b++){

      // check for negative txDelay (should never happen)
      if(ovwm->ses.txDelay[b]<0){
	fprintf(stderr,"Error: Negative TX delay for beam %d\n",b+1);
	exit(1);
      }

      // check for transmit past end of BRI
      float tx_end=ovwm->ses.txDelay[b]+ovwm->ses.burstLength;
      margin=ovwm->ses.bri - tx_end;
      if(margin<worst_margin) worst_margin=margin;
      if(margin<0){
	fprintf(stderr,"Error: TX end time %g exceeds BRI %g for beam %d\n",
		tx_end,ovwm->ses.bri,b+1);
	exit(1);
      }
      // check for more than 2 overlapping transmits AND numTWTA==2
      if(ovwm->ses.numTWTAs==2){
	if(b+2<NUMBER_OF_OVWM_BEAMS){
          margin=ovwm->ses.txDelay[b+2]-tx_end;
          if(margin<worst_margin) margin=worst_margin;
	  if(margin<0){
	    fprintf(stderr,"Error: Too few TWTAs\n");
	    exit(1);
	  }
	}
      }

    } // end of beams loop
    tx_margin=worst_margin; // set static variable tx_margin to the worst
                            // TX margin value

    // get number of burst in air

    nBurstInit = int(ovwm->ses.rxGateDelay/ovwm->ses.bri);
    cout << "# of burst between tx and rec: " << nBurstInit << endl;
    //cout << ovwm->ses.rxGateDelay << endl;

  } // end of first call


  // set rxTime in beamInfo object
  int b=ovwm->cds.currentBeamIdx;
  beamInfo[b].rxTime=beamInfo[b].txTime+ovwm->ses.rxGateDelay;

  int nBurst = int(ovwm->ses.rxGateDelay/ovwm->ses.bri);
  //printf("n burst: %d\n", nBurst);
  
  if(nBurst!=nBurstInit){
    fprintf(stderr,"Error: number of burst between tx and rec changed!!\n");
    if(!disableTimingCheck) exit(1);
  }

  // only perform rx/tx and rx overlap checks on last beam event
  if(b==NUMBER_OF_OVWM_BEAMS-1){

    // make sure no transmission window overlaps with a receiver window from
    // two or more cycles ago.
    double r2end=beamInfo[0].lastRxTime-nBurst*ovwm->ses.bri + 
      ovwm->ses.GetRxGateWidth(0);
    for(int rb=1;rb<NUMBER_OF_OVWM_BEAMS;rb++){
      double value=beamInfo[rb].lastRxTime-nBurst*ovwm->ses.bri + 
      ovwm->ses.GetRxGateWidth(rb);
      if(value>r2end) r2end=value;
    }
    for(int tb=0;tb<NUMBER_OF_OVWM_BEAMS;tb++){
      margin=beamInfo[tb].txTime-r2end;
      //printf("beam and margin: %d %12.8f\n", tb, margin);
      if(margin<worst_margin) worst_margin=margin;
      if(margin<0){
	fprintf(stderr,"Error: Tx window MAY overlap Rx Window from 2 cycles back\n");
        if(!disableTimingCheck)   exit(1);
      }
    }
    // check for overlap between Tx and Rx in current cycle and last cycle 
    for(int tb=0;tb<NUMBER_OF_OVWM_BEAMS;tb++){
      double tstart=beamInfo[tb].txTime;
      double tend=tstart+ovwm->ses.burstLength;
      for(int rb=0;rb<NUMBER_OF_OVWM_BEAMS;rb++){

        // current cycle check
	double rstart=beamInfo[rb].rxTime - (nBurst-1)*ovwm->ses.bri;
	double rend=rstart+ovwm->ses.GetRxGateWidth(rb);
        
	if(rstart<tstart){
	  margin=tstart-rend;
	}
	else{
	  margin=rstart-tend;
	}
	if(margin<worst_margin) worst_margin=margin;
	if(margin<0){
	  fprintf(stderr,"Error: Tx window (beam %d) overlaps Rx Window (beam %d same cycle)\n",tb+1,rb+1);
	  if(!disableTimingCheck) exit(1);
	}


        // previous cycle check
	rstart=beamInfo[rb].lastRxTime - (nBurst-1)*ovwm->ses.bri;
	rend=rstart+ovwm->ses.GetRxGateWidth(rb);

	if(rstart<tstart){
	  margin=tstart-rend;
	}
	else{
	  margin=rstart-tend;
	}
	if(margin<worst_margin) worst_margin=margin;
	if(margin<0){
	  fprintf(stderr,"Error: Tx window (beam %d) overlaps Rx Window (beam %d last cycle)\n",tb+1,rb+1);
        if(!disableTimingCheck)   exit(1);

	}
      } // end receiver beam loop
    } // end transmit beam loop

    // for right now require 4 receiver chains
    //if(ovwm->ses.numReceivers==2){
    //  fprintf(stderr,"Error 2 Receiver chain case not yet implemented\n");
    //  exit(1);
    //}

    if(worst_so_far>worst_margin) worst_so_far=worst_margin;
#ifdef DEBUG_BAD_TIMING
    if(num_calls%500==4){
      fprintf(stderr,"time=%g worst_margin=%g worst_so_far=%g\n",
	      ovwm->cds.time-startTime,worst_margin,worst_so_far);
    }
#endif 
  
#ifdef DEBUG_BAD_TIMING
  double tstart=beamInfo[0].txTime-startTime;
  double tend=beamInfo[0].txTime+ovwm->ses.burstLength-startTime;
  double rstart=beamInfo[0].rxTime-startTime;
  double rend=beamInfo[0].rxTime+ovwm->ses.GetRxGateWidth(1)-startTime;
  fprintf(stderr,"%d ts %g te %g rs %g re %g bl %g pri %g npul %d\n", 0,tstart,tend,rstart,rend,ovwm->ses.burstLength,ovwm->ses.pri, ovwm->ses.numPulses);

  for(int i=1;i<NUMBER_OF_OVWM_BEAMS;i++){
      double tstart2=beamInfo[i].txTime-startTime;
      double tend2=beamInfo[i].txTime+ovwm->ses.burstLength-startTime;
      double rstart2=beamInfo[i].rxTime-startTime;
      double rend2=beamInfo[i].rxTime+ovwm->ses.GetRxGateWidth(1)-startTime;
      if(rend2>rend) rend=rend2;
      if(tend2>tend) tend=tend2;
      if(rstart2<rstart) rstart=rstart2;
      if(tstart2<tstart) tstart=tstart2;
      //fprintf(stderr,"%d ts %g te %g rs %g re %g \n", i,tstart2,tend2,rstart2,rend2);

  }
  printf("%g 0 %g 0\n%g 1 %g 1\n%g 1 %g 1\n%g 0 %g 0\n",
	 tstart,rstart,tstart,rstart,tend,rend,tend,rend);
  fflush(stdout);

#endif       
  } // end last beam in cycle case

  
  
  return(1);
}
//-----------------------//
// OvwmSim::LoopbackSim //
//-----------------------//

int
OvwmSim::LoopbackSim(
    Spacecraft*  spacecraft,
    Ovwm*       ovwm,
    OvwmL1AFrame*    l1a_frame)
{
    //----------------------------------------//
    // compute frame header info if necessary //
    //----------------------------------------//

/* consider loop back not as a separate spot, so no need to do init */
    //L1AFrameInit(spacecraft,ovwm,l1a_frame);

    //-----------------------------//
    // Set cal pulse sequence flag //
    // (turn off Bit position 8)   //
    //-----------------------------//

    //l1a_frame->frame_inst_status = l1a_frame->frame_inst_status & 0xFFFFFEFF;

    //-------------------------------------------------//
    // tracking must be done to update state variables //
    //-------------------------------------------------//

    //SetOrbitStepDelayAndFrequency(spacecraft, ovwm);

    //--------------------------------------//
    // Add Cal-pulse Specific Info to Frame //
    //--------------------------------------//

    if (_spotNumber==0) {
      if (! SetL1ALoopback(ovwm, l1a_frame))
          return(0);
    }

    return 1;

    //---------------------------------//
    // set orbit step change indicator //
    //---------------------------------//

/* not implement status now
    unsigned short orbit_step = ovwm->cds.SetAndGetOrbitStep();
    if (orbit_step != l1a_frame->orbitStep)
    {
        l1a_frame->priOfOrbitStepChange = _spotNumber;
        l1a_frame->status.prf_orbit_step_change=l1a_frame->priOfOrbitStepChange;
        // remember, the CDS puts in the last orbit step (anti-documentation)
        l1a_frame->orbitStep = orbit_step;
        l1a_frame->status.doppler_orbit_step = l1a_frame->orbitStep;
    }
*/

    //-----------------------------//
    // determine if frame is ready //
    //-----------------------------//

/* consider loop back not as a separate spot, so no need to do init */
/*
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
*/
}

//-------------------//
// OvwmSim::LoadSim //
//-------------------//

int
OvwmSim::LoadSim(
    Spacecraft*  spacecraft,
    Ovwm*       ovwm,
    OvwmL1AFrame*    l1a_frame)
{
    //----------------------------------------//
    // compute frame header info if necessary //
    //----------------------------------------//

/* consider load not as a separate spot, so no need to do init */
    //L1AFrameInit(spacecraft,ovwm,l1a_frame);

    //-------------------------------------------------//
    // tracking must be done to update state variables //
    //-------------------------------------------------//

    //SetOrbitStepDelayAndFrequency(spacecraft, ovwm);

    //--------------------------------------//
    // Add Cal-pulse Specific Info to Frame //
    //--------------------------------------//

/* happen only in the beginning of a frame */

    if (_spotNumber==0) {
      if (! SetL1ALoad(ovwm, l1a_frame))
          return(0);
    }

    //---------------------------------//
    // set orbit step change indicator //
    //---------------------------------//

    //unsigned short orbit_step = ovwm->cds.SetAndGetOrbitStep();
    //if (orbit_step != l1a_frame->orbitStep)
    //{
    //    l1a_frame->priOfOrbitStepChange = _spotNumber;
    //    l1a_frame->status.prf_orbit_step_change=l1a_frame->priOfOrbitStepChange;
    //    // remember, the CDS puts in the last orbit step (anti-documentation)
    //    l1a_frame->orbitStep = orbit_step;
    //    l1a_frame->status.doppler_orbit_step = l1a_frame->orbitStep;
    //}

    //-----------------------------//
    // determine if frame is ready //
    //-----------------------------//

/* consider loop back not as a separate spot, so no need to do init */
/*
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
*/
    return(1);
}

//----------------------------//
// OvwmSim::SetL1ASpacecraft //
//----------------------------//

int
OvwmSim::SetL1ASpacecraft(
    Spacecraft*  spacecraft,
    OvwmL1AFrame*    l1a_frame)
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
// OvwmSim::SetMeasurements //
//---------------------------//
// MOST IMPORTANT METHOD IN THIS FILE !!!!                 //
// This is the main routine that ovwm_sim spends time in   //
// In HIRES mode, which is the commonly used configuration, //
// Each sigma0 is integrated over the wind field, antenna //
// pattern and point target response                     //

int
OvwmSim::SetMeasurements(
    Spacecraft*  spacecraft,
    Ovwm*       ovwm,
    MeasSpot*    meas_spot,
    WindField*   windfield,
    Sigma0Map*   inner_map,
    Sigma0Map*   outer_map,
    GMF*         gmf,
    Kp*          kp,
    KpmField*    kpmField,
    Topo*        topo,
    Stable*      stable,
    CheckFrame*  cf,
    PointTargetResponseTable* ptrTable,
    AmbigTable* ambigTable,
    int sim_l1b_direct)
{

    //--------------------------------------------------------//
    // generate the Antenna frame to/from Geocentric switches //
    //--------------------------------------------------------//
    // This object is used to move from geocentric coordinates to antenna frame
    CoordinateSwitch gc_to_antenna;
    gc_to_antenna = AntennaFrameToGC(&(spacecraft->orbitState),
				   &(spacecraft->attitude), 
				   &(ovwm->sas.antenna),
				   ovwm->sas.antenna.txCenterAzimuthAngle);
                // commented out debugging tool
                //spacecraft->orbitState.rsat.Show();
    CoordinateSwitch antenna_to_gc;
    antenna_to_gc = gc_to_antenna;
    // This object is used to move from antenna frame to geocentric coordinates
    gc_to_antenna=gc_to_antenna.ReverseDirection();

    //--------------------------
    // set beam (inner or outer)
    //--------------------------
    Beam* beam=ovwm->GetCurrentBeam();
    double borelook, boreazim;
     
    //------------------------------------------------------------------------------
    // compute maximum antenna gain for use in determining which slices to throw away
    // slices more than some thresho;ld below peak of pattern are tossed
    //------------------------------------------------------------------------------
    beam->GetElectricalBoresight(&borelook,&boreazim);

         // commented out debugging tools
         //cout << "look: " << borelook << endl;
         //cout << "azim: " << boreazim << endl;
    float maxgain;
    beam->GetPowerGain(borelook,boreazim,&maxgain);

         // commented out debugging tools    
         //cout << "maxgain:" << maxgain << endl;
    
    //-------------------------------------------
    // compute antenna boresight position
    // position is used to check if loaction is within simulation bounds
    //-------------------------------------------
    Vector3 boresight;
    boresight.SphericalSet(1.0,borelook,boreazim);
    OvwmTargetInfo oti;
    if(! ovwm->TargetInfo(&antenna_to_gc,spacecraft,boresight,&oti)){
      fprintf(stderr,"Error:SetMeasurements cannot find boresight on surface\n");
      exit(1);
    }
    EarthPosition spot_centroid=oti.rTarget;


    //--------------------------------------------------------------//
    // Compute Geocentric to range and azimuth coordinate switch    //
    // Used because point target response is in range/azimuth frame //
    //--------------------------------------------------------------//
    Vector3 zvec=oti.rTarget.Normal(); // Z-axis is normal vector at boresight
    Vector3 yvec=zvec & oti.gcLook;            // az vector
    Vector3 xvec=yvec & zvec;                  // rng vector
    CoordinateSwitch gc_to_rangeazim(xvec,yvec,zvec);




    //-------------------------------------------------------------//
    // Compute cross track/along track coordinate switch           //
    // Used because SAR ambiguity patterns are in cross/along frame//
    //-------------------------------------------------------------//
    double r_a= r1_earth*1000.0;
    double r_e2=e2;
    SchToXyz sch(r_a,r_e2);//earth radius in meter and eccentricity square
    //get position and velocity
    OrbitState* sc;
    sc= &(spacecraft->orbitState);

    //get position and velocity
    Vector3 position, velocity;
    position=sc->rsat;//km
    velocity=sc->vsat;//km/s
    //transform in MKS unit
    position *=1000.0;//in meters
    velocity *=1000.0;//in meter/s

    //compute sc position 6 seconds before and after    
    Vector3 position1, position2, delta_position;
    delta_position= velocity;
    delta_position *= 6.0;   
 
    //compute two nearby sc position separated by 6*2 seconds
    position1 =position - delta_position;//6 seconds before
    position2 =position + delta_position;//6 seconds after

    //compute sc lat lon
    Vector3 r_llh,r_llh1, r_llh2;
    xyz_to_llh(r_a,r_e2,position,  r_llh);
    xyz_to_llh(r_a,r_e2,position1, r_llh1);
    xyz_to_llh(r_a,r_e2,position2, r_llh2);
    
    //compute heading
    double r_heading;
    geo_hdg(r_a,r_e2,r_llh1(0), r_llh1(1),r_llh2(0),r_llh2(1),r_heading);

    //set peg point
    sch.SetPegPoint(r_llh(0), r_llh(1), r_heading);

       // commented out debugging tools
       //cout<<"nadir lat lon  "<< r_llh(0)*rtd<<" "<<r_llh(1)*rtd<<endl;
       //cout<<"r_llh "<< r_llh1(0)*rtd<<" "<<r_llh1(1)*rtd<<endl;
       //cout<<"r_llh "<< r_llh2(0)*rtd<<" "<<r_llh2(1)*rtd<<endl;
       //
       //cout<<"heading "<< r_heading*180/3.14<<endl;
       //now we can convert surface location into sch
       //call the following two functions will do the job
       // sch.xyz_to_sch(r_xyz,r_sch)
       // or sch.sch_to_xyz(r_sch,r_xyz)


    //--------------------------------------------------------//
    // Compute boresight in sch (cross/along) coordinates     //
    // Used for ambiguity table lookup                        //
    //--------------------------------------------------------//
    //need boresight xyz in meter scale
    EarthPosition bore_in_meter=spot_centroid;
    bore_in_meter *=1000.0;//km to m
    Vector3 bore_sch_in_meter,bore_llh;
    xyz_to_llh(r_a,r_e2,bore_in_meter,bore_llh);
        //cout<<"bore llh "<< bore_llh(0)*rtd<<" "<<bore_llh(1)*rtd<<" "<<bore_llh(3)<<endl;
    sch.xyz_to_sch(bore_in_meter,bore_sch_in_meter);
        //cout<<"s c h of bore "<< bore_sch_in_meter(0)<<" "<<bore_sch_in_meter(1)<<" "<<bore_sch_in_meter(2)<<endl;
    double bore_along, bore_cross;
    bore_along= bore_sch_in_meter(0)/1000.0;
    bore_cross= bore_sch_in_meter(1)/1000.0;
   
    
   


    //--------------------------------------------------------//
    // Start with first measurement which has already been    //
    // located by LocatePixels                                //
    //--------------------------------------------------------//
    int slice_i = 0;
    Meas* meas = meas_spot->GetHead();
    

    //-------------------------------------------------------------
    // Setup for SAR ambiguity calculation
    //for each measurement, scan angle and beam number are fixed
    //code done by ygim: phone 4-4299
    //scan angle and beam index
    //-------------------------------------------------------------
    double scanangle=meas->scanAngle;
    scanangle *= rtd;
    double bs_scanangle=scanangle;
    if(scanangle<=270.0) 
      scanangle=  scanangle + 90.0;
    else
      scanangle= scanangle-270.0;
    
    if(scanangle <0.0 || scanangle >360.0){
      fprintf(stderr,"Error:SetMeasurements scan angle is out of range\n");
      exit(1);
    }
    unsigned int beam_id=meas->beamIdx;
            // Commented out debugging tools
            // cout<<"beam id and BS and amb scan angles "<< beam_id<<" "<<bs_scanangle<<" "<<scanangle<<endl;
            // cout<<"bore along cross in km "<< bore_along<<" "<<bore_cross<<endl;

   
    //----------------------------------------------------------------------------------------------//
    // SPECIAL CASE for initializing maps of SAR ambiguities, X,gain and Kpc                        //
    // when a particular debugging keyword is set in the config file                                //
    // gain and Kpc only works when numPulses==11                                                   //
    // and numRangePixels < 40  due to hard-coded array sizes                                       //
    // SHOULD MODIFY THIS CODE and CODE BELOW that populates maps to work for nonSAR case           //
    //----------------------------------------------------------------------------------------------//
    bool generate_map=false;
    if(spot_check_generate_map && beam_id == (unsigned int) spot_check_beam_number && int(bs_scanangle+0.5) == spot_check_scan_angle){
      //need to make an array to save data
      //reset map arrays
      //make sure we have big enough array
      if(ovwm->ses.numPulses!= 11){
	fprintf(stderr,"Error: OvwmSim:setMeasurements: number of pulses is not  11\n");
	exit(1);
      }
      if(ovwm->ses.numRangePixels>40){
	fprintf(stderr,"Error: OvwmSim:setMeasurements : number of range pixels is larger than 40\n");
	exit(1);
      }
      //reset the values, those values could be changed 
      for(unsigned int i=0; i<40;++i)
	for(unsigned int j=0;j<11;++j){
	  amb_map_[i][j]=0.0;
	  X_map_[i][j]=0.0;
	  gain_map_[i][j]=0.0;
	  kpc_map_[i][j]=0.0;
	}   
      generate_map=true;
      cout<<"range doppler pixel "<< ovwm->ses.numRangePixels<<" "<<ovwm->ses.numPulses<<endl;
    }

    //-------------------------=-------------------------------------------------//
    // Variables used by ambiguity calculation measurement by measurement loop  //
    //--------------------------------------------------------------------------//
    unsigned int range_index, azimuth_index;
    Vector3 centroid_llh, centroid_xyz_in_meter, centroid_sch_in_meter;
    double centroid_along, centroid_cross;
    Vector3 loc_llh, loc_xyz_in_meter, loc_sch_in_meter;
    double loc_along, loc_cross;
    double amb1_along, amb1_cross;//first ambigous point
    double amb2_along, amb2_cross;//second ambigous point
    double dummy_along, dummy_cross;// dummy placeholders used in amb integration


    //-----------------------------//
    // Measurement Loop            //
    //-----------------------------//
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
	
    

	//-------------------------------------//
        // Compute Land Flag                   //
        // Determines if measurement centroid  //
        // is over land. The more general case //
        // of land somewhere in the measurement//
        // is handled below                    //
        //-------------------------------------//
        if (lon != 0. && lat != 0.) {
          meas->landFlag = landMap.IsLand(lon, lat);
        }

        //------------------------------------------//
        //  In direct to L1B mode                   //
        //  Measurements with centroids over land   //
        // are tossed and pointers are modified     //
        // IF SIM_LAND_FLAG is 1 in the config file //
        //------------------------------------------//

        if (meas->landFlag == 1 && simLandFlag == 0 && sim_l1b_direct)
        {
            //cout << "land and not sim" << endl;

            // this is land, but we don't want land
            // remove this measurement, and go to the next
            meas = meas_spot->RemoveCurrent();
            delete meas;
            meas = meas_spot->GetCurrent();
            slice_i++;
            continue;
        }


        //  Variables used below
        float sigma0; // backscatter
        float Es;     // signal returned energy
        float En;     // noise returned energy
        float var_esn_slice; // Kpc variance of measurement



        //---------------------------------//
        // Low resolution simulation case  //
        // Seldom used: commentrs sparse   //
        //---------------------------------//
        if(!simHiRes){
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
	  else if (meas->landFlag == 1 && simLandFlag == 0)
	    {
	      //-------------------------------------------//
	      // LAND! Try to use the inner and outer maps //
	      //-------------------------------------------//

	      // for now land map option is disallowed; set constant values
	      // for each beam
	      //           sigma0 = inner_map->GetSigma0(lon, lat);
	      sigma0 = landSigma0[meas->beamIdx];

	      // This part is INCOMPLETE needs to set other L1A/L1B fields
	      if (simVs1BCheckfile)
		{
		  cf->sigma0[slice_i] = sigma0;
		  cf->wv[slice_i].spd = 0.0;
		  cf->wv[slice_i].dir = 0.0;
		}
	    }
	  else if(!simHiRes)
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
	      
	      // add rain contamination if simRain
	      if(simRain){
                if (!rainField.flag_3d) {
		  float a,b;
		  int goodrain=rainField.InterpolateABLinear(lon_lat,meas->incidenceAngle,a,b);
		  if(goodrain) sigma0= sigma0/a + b;
                } else if (rainField.flag_3d) {
		  printf("Error: Rain 3D model is not implemented in this mode.\n");
                  exit(-1);
                }
	      }
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
		  double kpm2;
		  if (! kp->GetKpm2(meas->measType, wv.spd, &kpm2))
		    {
		      printf("Error: Bad Kpm value in OvwmSim::SetMeas\n");
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
	
	      //-------------------------//
	      // convert Sigma0 to Power //
	      //-------------------------//
	      
	      // Kfactor: either 1.0, taken from table, or X is computed
	      // directly
	      float Xfactor = 0.0;
	      float Kfactor = 1.0;
	  
	      if (computeXfactor)
		{
		  if (! ComputeXfactor(spacecraft, ovwm, meas, &Xfactor))
		  {
		    meas = meas_spot->RemoveCurrent();
		    delete meas;
		    meas = meas_spot->GetCurrent();
		    slice_i++;
		    continue;
		  }
		  
		  if (! MeasToEsnX(ovwm, meas, Xfactor, sigma0, &(meas->value),
				   &Es, &En, &var_esn_slice))
		    {
		      return(0);
		    }
		  meas->XK = Xfactor;
		}
	      else
		{
		  Kfactor = 1.0;  // default to use if no Kfactor specified.
		  if (useKfactor)
		    {
		      float orbit_position = ovwm->cds.OrbitFraction();
		      
		      Kfactor = 
			kfactorTable.RetrieveByRelativeSliceNumber(
								   ovwm->cds.currentBeamIdx,
								   ovwm->sas.antenna.txCenterAzimuthAngle, orbit_position,
								   meas->startSliceIdx);
		    }
		  
		  double Tp = ovwm->ses.txPulseWidth;
		  
		  if (! MeasToEsnK(spacecraft, ovwm, meas, Kfactor*Tp, sigma0,
				   &(meas->value), &Es, &En, &var_esn_slice, &(meas->XK)))
		    {
		      return(0);
		    }
		}


	      if (simVs1BCheckfile)
		{
		  Vector3 rlook = meas->centroid - spacecraft->orbitState.rsat;
		  cf->R[slice_i] = (float)rlook.Magnitude();
		  if (computeXfactor)
		    {
		      // Antenna gain is not computed when computing X factor
		      // because the X factor already includes the normalized
		      // patterns.  Thus, to see what it actually is, we need
		      // to do the geometry work here that is normally done
		      // in radar_X() when using the K-factor approach.
		      gc_to_antenna = AntennaFrameToGC(&(spacecraft->orbitState),
						       &(spacecraft->attitude), &(ovwm->sas.antenna),
						       ovwm->sas.antenna.txCenterAzimuthAngle);
		      gc_to_antenna=gc_to_antenna.ReverseDirection();
		      double roundTripTime = 2.0 * cf->R[slice_i] / speed_light_kps;
		      
		      Beam* beam = ovwm->GetCurrentBeam();
		      Vector3 rlook_antenna = gc_to_antenna.Forward(rlook);
		      double r, theta, phi;
		      rlook_antenna.SphericalGet(&r,&theta,&phi);
		      if (! beam->GetPowerGainProduct(theta, phi, roundTripTime,
						      ovwm->sas.antenna.spinRate, &(cf->GatGar[slice_i])))
			{
			  cf->GatGar[slice_i] = 1.0;    // set a dummy value.
			}
		    }
		  else
		    {
		      double lambda = speed_light_kps / ovwm->ses.txFrequency;
		      cf->GatGar[slice_i] = meas->XK / Kfactor * (64*pi*pi*pi *
								  cf->R[slice_i] * cf->R[slice_i]*cf->R[slice_i]*
								  cf->R[slice_i] * ovwm->systemLoss) /
			(ovwm->ses.transmitPower * ovwm->ses.rxGainEcho *
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
	    }

	} 

        //-----------------------------------------------------//
        // end of low res case!--------------------------------//
        //-----------------------------------------------------//

        //-----------------------------------------------------//
        // High Resolution Simulation   STARTS HERE!           //
        //-----------------------------------------------------//   
        else{
          
          //-----------------------------------------------------//
	  // Indices for special SAR map generation case         //
          // num_pulses must equal 11 when this case is          //
          // used. Need to generalize it to make it useful again //
          //-----------------------------------------------------//
	  azimuth_index = slice_i%11;
	  range_index = int(slice_i/11);

          //-----------------------------------------------------//
          // Sanity check to toss meaurements whose centroids were//
          // set to zero because they were not located on the earth//
          //-------------------------------------------------------//
	  if(meas->centroid.Magnitude() < 1000.0 && sim_l1b_direct)
	    { 
              //cout << "meas # " << slice_i << "centroid dist < 1000" << endl;

	      meas=meas_spot->RemoveCurrent();
	      delete meas;
	      meas=meas_spot->GetCurrent();
	      slice_i++;
	      continue;
	    }

          //----------------------------------------------------------//
          // Set up a normal distribution for use in Kpc              //
          //----------------------------------------------------------//
	  Gaussian normrv(1.0,0.0); 
 
          //----------------------------------------------------------//
          // Compute look vector and gain at centroid of measurement  //
          //----------------------------------------------------------//
          Vector3 rlook = meas->centroid - spacecraft->orbitState.rsat;
          Vector3 rlook_ant=gc_to_antenna.Forward(rlook);
              // commented out debugging tool
	      //gc_to_antenna.Show();
          double r,theta,phi;
	  rlook_ant.SphericalGet(&r,&theta,&phi);
	      //cout << theta*rtd << endl;
	      //cout << phi*rtd << endl;
          double gain;
          if(!beam->GetPowerGain(theta,phi,&gain)){
	    gain=0;
	  }
          gain/=maxgain;
              //cout << gain << endl;


          //-----------------------------------------------//
          // For debugging special case produce gain map   //
          // Only works for SAR num_pulses==11             //
          // Need to generalize this                       //
          //-----------------------------------------------//
	  if(generate_map)
	    gain_map_[range_index][azimuth_index]=gain;
	    

          //-------------------------------------------------//
          // Throw away measurements that are too far down   //
          // on gain pattern if we are going directly to L1B //
          // Otherwise these measurements are kept and the   //
          // OvwmL1AToL1B processing tosses them.            //
          //-------------------------------------------------//
	  if(!generate_map && gain<minOneWayGain && sim_l1b_direct){
            //cout << "meas # " << slice_i << "too small gain" << endl;

	    meas=meas_spot->RemoveCurrent();
            delete meas;
            meas=meas_spot->GetCurrent();
	    slice_i++;
            continue;
	  }
	
	 
	 
	       // commented out debugging tool
	       //cout<<"slice "<< slice_i<<endl;



          //-----------------------------------------------------//
          // Compute centroid in cross/along coordinates  for    //
          // SAR ambiguity table lookup.                         //
          //-----------------------------------------------------//
	  centroid_xyz_in_meter= meas->centroid;
	  centroid_xyz_in_meter *=1000.0;//change km to meter
	  xyz_to_llh(r_a, r_e2, centroid_xyz_in_meter, centroid_llh);
	  sch.xyz_to_sch(centroid_xyz_in_meter,centroid_sch_in_meter);
	  centroid_along= centroid_sch_in_meter(0)/1000.0;// km
	  centroid_cross= centroid_sch_in_meter(1)/1000.0;// km
	  
	  

	  //----------------------------------------------------//
	  // Access SAR ambiguity table BY                      //
	  // beam number, azimuth angle                         //
	  // alongtrack wrt boresight                           //
	  // crosstrack wrt boresight                           //
          // SignalToAmbiguity Ratio and locations of two largest//
	  // ambiguities                                        //
          // When table is not configured SignalToAmbiguityRatio//       
          // is set to 10^10 = 100 dB                           //
          //----------------------------------------------------//
	  double amb1=ambigTable->GetAmbRat1(beam_id, scanangle,
					     centroid_along-bore_along,
					     centroid_cross-bore_cross,
					     amb1_along,
					     amb1_cross);
					 
	  double amb2=ambigTable->GetAmbRat2(beam_id, scanangle,
					     centroid_along - bore_along
					     ,centroid_cross - bore_cross, 
					     amb2_along,
					     amb2_cross);
					 

          EarthPosition amb1pos,amb2pos;
          EarthPosition amb1pos_sch(amb1_along+bore_along,amb1_cross+bore_cross,0.0);
	  EarthPosition amb2pos_sch(amb2_along+bore_along,amb2_cross+bore_cross,0.0);
	  sch.sch_to_xyz(amb1pos_sch*1000.0,amb1pos);
          amb1pos=amb1pos/1000.0;
	  sch.sch_to_xyz(amb2pos_sch*1000.0,amb2pos);
          amb2pos=amb2pos/1000.0;

          //--------------------------------------------------------------//
          //  Debugging Tools for when the SAR Ambiguity Table            //
          //      output is valid.                                        //
          //--------------------------------------------------------------//
	  if(amb1 != 0.0 && amb2 !=0.0){
	    /* Debuggig tools commented out
	    cout<<"scan angle and beam index "<< scanangle<<" "<<beam_id<<endl;
	    cout<<"centroid lat lon height "<< centroid_llh(0)*rtd<<" "<<centroid_llh(1)*rtd<<" "<<centroid_llh(2)<<endl;
	    cout<<"centroid xyz "<< centroid_xyz_in_meter(0)<<" "<<centroid_xyz_in_meter(1)<<" "<<centroid_xyz_in_meter(2)<<endl;
	    cout<<"centroid sch  "<< centroid_sch_in_meter(0)<<" "<<centroid_sch_in_meter(1)<<" "<<centroid_sch_in_meter(2)<<endl;
	    
	    cout<<"along cross  "<< centroid_along<<"  "<<centroid_cross<<endl;
	    cout<<"along cross w.r.t. bore "<< centroid_along - bore_along<<" "<<centroid_cross-bore_cross<<endl;
	    cout<<"amb1 and 2 in dB: "<< 10*log(amb1)/log(10.0)<<" "<<10*log(amb2)/log(10.0)<<endl;
	    */
	  }

	  //--------------------------------------------------------------//
          //  INVALID SAR Ambiguity case:                                 //
	  //  Typically this occurs when the measurement centroid is      //
          //  outside the processing window.
          //--------------------------------------------------------------// 
	  if(amb1==0 || amb2==0){

            //---------------------------------------------//
            // For the special generate_map case           //
            // invalid signal to ambiguity ratio are set   //
            // to unity in order to plot those measurements//
            // which would normally be thrown out          //
            //---------------------------------------------//
	    if(generate_map && amb1==0 && amb2==0){
	      amb1=1;
	      amb2=1;//this is needed to move beyond 1/amb1 computation
	    }

            //------------------------------------------------------------------//
            // When using simulating directly to L1B in SAR mode num_pulse>1    //
            // measurements with centroids outside processing window are tossed //
            // out.                                                             //
            //------------------------------------------------------------------//
	    else if(!generate_map &&( amb2 ==0 && amb1 ==0) && sim_l1b_direct){

              //cout << "meas # " << slice_i << "amb too big" << endl;

	      meas=meas_spot->RemoveCurrent();
	      delete meas;
	      meas=meas_spot->GetCurrent();
	      slice_i++;
	      continue;
	    }
           //------------------------------------------------------------------//
            // Should Never Occur. Cas in which only one of the computed        //
            // Signal to ambiguity ratios is invalid                            //
            //------------------------------------------------------------------//
	    else if(!generate_map && sim_l1b_direct){

              //cout << "meas # " << slice_i;
	      fprintf(stderr,"Warning: Bad Ambiguity Condition 0 value for one ambig only\n");
	      meas=meas_spot->RemoveCurrent();
	      delete meas;
	      meas=meas_spot->GetCurrent();
	      slice_i++;
	      continue;
	    }
            //----------------------------------------------------//
            // Invalid Signal To Ambiguity measurements still     //
            // get recorded whwn L1A data is output. They should be// 
            // thrown out during L1A to L1B processing             //
            // In this case the signal to ambiguity ratio is set   //
            // to unity                                            //
            //-----------------------------------------------------//
            else if(!generate_map && !sim_l1b_direct){ // create meas in L1A
	      //fprintf(stderr,"Warning: Bad Ambiguity Condition, set ambig 1\n");
              amb1=1;
              amb2=1;
            }
	  }
	  
          //---------------------------------//
          // Compute overall Ambiguity To    //
          // Signal Ratio as the sum or the  //
          // recipricol of the two signal    //
          // to ambiguity ratios             //
          //---------------------------------//
          amb1=1/amb1;
          amb2=1/amb2;
	  double amb=amb1+amb2;
	  
          // Populate ambiguity map in special spot_check mode
          // this only works when num_pulses==11
	  if(generate_map)
	    amb_map_[range_index][azimuth_index]=amb;

          //-------------------------------------//
          // When simulating L1B directly and no //
          // special debugging is involved       //
          // exclude measurements with Signal to //
          // ambiguity Ratios less than the      //
          // SIM_MIN_SIG_TO_AMB dB in the config //
          // file                                //
          //-------------------------------------//
	  if(!generate_map && amb> 1/minSignalToAmbigRatio && sim_l1b_direct){

            //cout << "meas # " << slice_i << "amb signal" << endl;

	    meas=meas_spot->RemoveCurrent();
            delete meas;
            meas=meas_spot->GetCurrent();
	    slice_i++;
            continue;
	  }
	   
	
          //---------------------------------------------------------------//
	  // Compute point target response array for measurement           //
          // The resultant array is then used to modulate the backscatter  //
          // integration.                                                  //
          //---------------------------------------------------------------//

          //-------------------------------------------------------------------------------
          // compute location of centroid of measurement in the range/azim coordinate system
          // in which the antenna footprint center is the origin
          //--------------------------------------------------------------------------------
          Vector3 offset = meas->centroid - spot_centroid; // offset in geocentric frame
          offset =gc_to_rangeazim.Forward(offset);
          float range_km = offset.GetX();
          float azimuth_km = offset.GetY();

          //-------------------------------------------------------------------------------
          // these three values are used to look up values in the point target response table
          // if it is being used. For this purpose the orbit period is assumed to be the
          // QuikSCAT orbit period of 6060 s, but this will not matter when the point
          // target response is computed and not obtained from a table. The table is 
          // never used when num_pulses==1 (the QuikSCAT and DFS configurations)
          //--------------------------------
          float scan_angle = meas->scanAngle;
          float orbit_time = ovwm->cds.OrbitFraction()*6060.;
          int beam_num = beam_id+1; // Now set the default value, later update from Beam object

          
          //--------------------------------------------------//
          // These values (rangewid, azimwid) are half the    //
          //             3-dB spatial                         //
          // resolution of the point target response (antenna //
          // pattern effects not included)                    //
          // 1) read from a table                             //
          // or                                               //
          // 2) for SAR a sinc^2 in Doppler or  a sum of      //
          //    Gaussian terms for range simulating the averaging//
          //    or multiple range bins                         //
          // 3) for nonSAR (num_pulses==1) same as 2 except    //
          //    point target response in azimuth is unity      //
          //    everywhere.                                    //
          // Below rangewid is half the width of a single      //
          // range bin NOT the whole measurement.              //
          //---------------------------------------------------//
          float rangewid, azimwid; // half widths

          rangewid = 0.;
          azimwid = 0.;

          //----------------
          // read widths from PTR table
          //---------------
          if (ptrTable->use_PTR_table) {

            rangewid=ptrTable->GetSemiMinorWidth(range_km, azimuth_km, scan_angle,
                                           orbit_time, beam_num)/1000.;
            azimwid=ptrTable->GetSemiMajorWidth(range_km, azimuth_km, scan_angle,
                                          orbit_time, beam_num)/1000.;
            //cout << "width from Table: " << rangewid << " " << azimwid << endl;

          } else {

            // ovwm->rngRes and ovwm->azRes were set in Ovwm::LocatePixels
            // rngRes = speed_light/2/chirp_bandwidth/sin(incidence)
            // azRes = the interval between pixels in azimuth
            rangewid = ovwm->rngRes/2.;
            azimwid = ovwm->azRes/2.;

            // Range Correction factor for 1/e in Point Target Response
            // The scale difference between the half power point and the
            // sigma parameter of the Gaussian function
            // rangewid *= 1.6;
	    // Got rid of this code here, since rangewid should not change.


            // Range Correction factor for 1/e in Point Target Response
            // The scale difference between the half power point and the
            // "width" parameter of the sinc^2 function
            // The factors differ for the inner and outer beam
            // The azimuth width only matter for the XOWVM SAR case 
            // for which these values were hard-coded 
            // I am unsure why they vary by beam in any case. -- BWS 07/21/2009

	    // Commented the following out because they make no sense
	    // It cannot be the sinc^2 >> Gaussian correction, because the sinc^2
	    // is actually used in the azimuth PTR construction below.
	    // If the these corrections are valid, it's something else - JMM 6/7/2010
            // if (beam_id%2==0) { // outer beam
            //  azimwid *= 1.2;
            // } else if (beam_id%2==1) { // inner beam
            //  azimwid *= 1.4;
	    // }

            if (azimwid >= ptrTable->azGroundWidthMax) {
              azimwid = ptrTable->azGroundWidthMax;
            }
            //cout << "width from LP: " << rangewid << " " << azimwid << endl;

          }

          //----------------------------------------------------------
          // For nonSAR case the azimuth width is set to maximal value
          //----------------------------------------------------------
          if(ovwm->ses.numPulses==1) azimwid=ptrTable->azGroundWidthMax/2;
 


          //----------------------------------------------------------
          // Widths are set to zero if measurement centroid is outside the    
          // range and azimuth window specified in the config file by
          // PTRTAB_RANGE_MAX_GROUND_WIDTH and PTRTAB_AZIMUTH_MAX_GROUND_WIDTH
          //-----------------------------------------------------------
          if (fabs(range_km) >= ptrTable->rngGroundWidthMax/2.) rangewid = 0.;
          if (fabs(azimuth_km) >= ptrTable->azGroundWidthMax/2.) azimwid = 0.;

          //----------------------------------------------------
          // If the widths are zero and we are not in a special
          // map generation mode         
          //--------------------------------------------------
	  if(!generate_map &&(rangewid==0 || azimwid==0)) {

            //--- Remove measurement if we are simulating directly to l1b
            if (sim_l1b_direct){

              // Commented out debugging tools
              //cout << "rng or az width" << endl;
              //cout << "meas # " << slice_i << " " << range_km << " " << azimuth_km <<endl;

	      meas=meas_spot->RemoveCurrent();
              delete meas;
              meas=meas_spot->GetCurrent();
              slice_i++;
              continue;

            } 
            
            //---- Otherwise set nominal widths and let L1B processing toss out these
            //     measurements          
	    else if (!sim_l1b_direct) {

             /* to get over creating meas record */
             /* assign nominal values for widths     */

              rangewid = 0.06;
              azimwid = 1.00;
              meas->azimuth_width = 2.*azimwid;
              meas->range_width=2.*rangewid;
            }

	  } // end of zero width case
	
          //---------------------------------------
          // Setting up number of integration steps
          // and bounds in range and azimuth
          //---------------------------------------
	  int nL=ovwm->ses.numRangeLooksAveraged;
          int nrsteps=(int)ceil(integrationRangeWidthFactor*rangewid*2*nL/integrationStepSize)+1;
          int nasteps=(int)ceil(integrationAzimuthWidthFactor*azimwid*2/integrationStepSize)+1;


          //------------------------------------------------
          // If not in SAR mode OR maximum number of azimuth
          // integration steps is exceeded because of flag invalid 
          // value in PTR table then set
          // nasteps to maximum value
          //------------------------------------------------
          if(ovwm->ses.numPulses==1) nasteps=_max_int_azim_bins;
          if (!ptrTable->use_PTR_table && nasteps>_max_int_azim_bins) {
            nasteps = _max_int_azim_bins;
          }

          //--------------------------------------------------
          // If maximum number of steps in either direction is
          // exceeded print error message and exit
          //--------------------------------------------------
          if(nrsteps>_max_int_range_bins || nasteps > _max_int_azim_bins){
	    fprintf(stderr,"Error SetMeasurements too many integrations bins\n");
	    exit(1);
	  }
	  if(nL%2!=0){
	    fprintf(stderr,"Error SetMeasurements rangeLooksAveraged must be even\n");
	    exit(1);
	  }
       
          //------------------------------------------------
          // set up center within integration window
          // for each range look to be averaged
          //------------------------------------------------
	  float center_azim_idx= (nasteps-1)/2.0;

          // 1 range center is computed for each independent range look
          // prior to averaging
          float center_range_idx[100]; // More than 100 looks averaged together will cause this to fail

          float center_range_idx_ave=(nrsteps-1)/2.0;
          for(int n=0;n<nL;n++){
	    int n2=n-nL/2;
            // This is an approximation.
            // Really the actual spacing of range looks on ground should be used
            // instead of rangewid because the spacing is not exactly constant across the whole
            // footprint.
            // In the azimuth direction the real spacing is used because the centroid for each
            // measurement is computed and only one azimuth look is used per measurement.
	    center_range_idx[n]=(nrsteps-1)/2.0+(2*n2+1)*rangewid/integrationStepSize;
	    
	  }

	 //---------------------------------------------------
         // Initialize point target response array to 0s
         //---------------------------------------------------
         for(int i=0;i<nrsteps;i++){
	    for(int j=0;j<nasteps;j++){
	      _ptr_array[i][j]=0;
	    }
	 }


         // areaeff = Effective area of point target response
         // It is computed and could be used for debugging but right now it is not used
         // for anything.
         // areaeff_SL is not used or even computed.
         double areaeff=0, areaeff_SL=0;


	 // Do not estimate spatial response, presume boxcar in range and no azimuth compression
	 if(useBoxCar){
	   float i0=nrsteps/2 - (nL*rangewid/integrationStepSize);
	   float i1=nrsteps/2 + (nL*rangewid/integrationStepSize);
	   for(int i=0;i<nrsteps;i++){
	     if(i>=i0 & i<=i1){
	       for(int j=0;j<nasteps;j++){
		 _ptr_array[i][j]=0.87*sin(1)*sin(1);		      
	       }
	     }
	   }
	 }
	 // estimate spatial response
	 else{
	   //------------------------------------
	   // Compute Point Target response Array
	   //------------------------------------         
	   for(int i=0;i<nrsteps;i++){
	     float ii=i;

	     for(int j=0;j<nasteps;j++){
	       float jj=j;

	       float val2;

	       //---- If in SAR mode include azimuth compression response
	       if(ovwm->ses.numPulses!=1){
		 val2=(jj-center_azim_idx)*integrationStepSize; 
		 val2/=azimwid;
		 //val2*=val2;
		 val2*=E_FACTOR;
	       }
	       //---- If not in SAR mode do not include azimuth compression response
	       else{
		 val2=1.0; // ERROR?? this should probably be zero; using 1  puts in a constant scale factor of 0.708
		 // I would expect this to reduce SNR by 1.5 dB but there may be some reason for this
		 // that I am missing.  The fact that using 1 here instead of 0 scales the whole point target
		 // response array by 1/sqrt(2) is highly suspicious. I suspect that Samuel Chan or I
		 // used this a kludge to get the correct SNR values. I need to check this --- BWS 07/22/2009
	       }

	       for(int n=0;n<nL;n++){
		 float val1=(ii-center_range_idx[n])*integrationStepSize;
		 val1/=rangewid*1.6; // rangewid correction factor for 1/e
		 // The scale difference between the half power point of the sinc
		 // and the sigma parameter of the Gaussian function
		 // correction by JMM 07/06/2010
		 val1*=val1;

		 //--------------------------------------------------------------------------------
		 // assumes azimuth PTR is sinc^2 function and range PTR is a Gaussian for each look
		 //--------------------------------------------------------------------------------
		 if (val2 != 0.) {
		   _ptr_array[i][j]+=exp(-val1)*sin(val2)*sin(val2)/val2/val2;
		 } else {
		   _ptr_array[i][j]+=exp(-val1);
		 }

		 // commented out debugging tool
		 //cout << i << " " << j << " vals: " << val1 << " " << val2 << " " << _ptr_array[i][j] << endl;
		 areaeff+=_ptr_array[i][j]*integrationStepSize*integrationStepSize;
	       }
	     }
	   }
	 } // end (else) compute point target response array
       
	 //#define PTRIM_TO_STDOUT_AND_EXIT
#ifdef PTRIM_TO_STDOUT_AND_EXIT
	 for(int i=0;i<nrsteps;i++){
	    for(int j=0;j<nasteps;j++){
	      printf("%g ",_ptr_array[i][j]);
	    }
	    printf("\n");
	 }
	
         fflush(stdout);
         exit(1);
#endif 
	
          //--------------------------------------------//
          // Initialize quantities to be integrated     //
          //--------------------------------------------//
          meas->XK=0; // calibration factor that converts power to backscatter for measurement
          float dX_land = 0.; // portion of measurement energy over land if backscatter were uniform
          float dX_ocean = 0.;// portion of measurement energy over ocean if backscatter were uniform
          Es=0;  // Total signal energy in measurement
	  En=0;  // Total noise energy in measurement
	

          //-----------------------------------------------//
          // Set contributions to energy from ambiguities  //
          // to zero if we are optional integrating those  //
          // quantities as well. Otherwise they are assigned //
          // from values at measurement centroid-- see above //
          //-------------------------------------------------//
          if(integrateAmbig){
            amb1=0;
            amb2=0;
          }

          //----------------------------------//
          // Compute measurement centroid     //
          // in range and azimuth coordinates //
          // (centered at footprint center)   //
          //----------------------------------//
          Vector3 center_ra=gc_to_rangeazim.Forward(meas->centroid-spot_centroid);        double maxdX=0;
	  double r0=center_ra.Get(0);
	  double a0=center_ra.Get(1);

          // commented out debugging tool
          //cout << "r&a: " << r0 << " " << a0 << endl;

          //--- Initialize quantities computed when 3-D rain effects are simulated
          float attn = 0.;
          float **rainRngAz;
          float Es_rain = 0.;
          float X_rain = 0.;
	
          //------------------------------------
          // Code for simulating 3-D effects of rain
          // Currently we do not use this code as
          // it is buggy. Configuration file parameters
          // are set to avoid this right now.
          // Comments are sparse in this block
          //------------------------------------
          if (simRain && rainField.flag_3d) {

            // find rain attenuation
            rainField.ComputeAttn(spacecraft, spot_centroid, meas->centroid,
                                  meas->incidenceAngle, gc_to_rangeazim, &attn);
            //cout << "rain attn: " << attn << endl;

            // find rain cell locations with same range and doppler as meas centroid
            rainRngAz = new float*[40];
            for (int ii=0; ii<40; ii++) {
              rainRngAz[ii] = new float[2];
            }
            rainField.ComputeLoc(spacecraft, meas, spot_centroid, gc_to_rangeazim, rainRngAz);

          }
	
	
          if (simRain && rainField.flag_3d) { // find out rain contribution with 3d model

            for (int nn=1; nn<=N_LAYERS; nn++) {
              //cout << "layer: " << nn << endl;

              X_rain = 0.;
              float layer_attn;
              float layer_Es_rain = 0.;

              for (int ii=0; ii<nrsteps; ii++) {
	        double rr = rainRngAz[nn-1][0]+(ii-center_range_idx_ave)*integrationStepSize;

                for (int jj=0; jj<nasteps; jj++) {

	          double aa = rainRngAz[nn-1][1]+(jj-center_azim_idx)*integrationStepSize;
                  EarthPosition rainCell;
                  rainCell.SetPosition(rr, aa, nn*DZ_LAYER);
                  rainCell = gc_to_rangeazim.Backward(rainCell) + spot_centroid;
                  //rainCell.Show();

                  // find attn for this layer
                  if (ii==nrsteps/2 && jj==nasteps/2) {
                    rainField.ComputeAttn(spacecraft, spot_centroid, rainCell,
                                          meas->incidenceAngle, gc_to_rangeazim, &layer_attn);
                    //cout << "layer, attn: " << nn << " " << layer_attn << endl;
                  }

                  double alt, lon, lat;
                  if (!rainCell.GetAltLonGDLat(&alt, &lon, &lat))
                    return (0);
                  //cout << "alt, lon, lat: " << alt << " " << lon << " " << lat << endl;

                  float rainRefl, rainS0;
                  rainField.GetRefl(alt,lon,lat,&rainRefl);
                  //cout << rainRefl << endl;
                  rainS0 = rainField.const_ZtoSigma*rainRefl*DZ_LAYER*1000.
                           /cos(meas->incidenceAngle); // 1000. for DZ_LAYER from km to m 
                  //cout << rainS0 << endl;

                  Vector3 rl = rainCell - spacecraft->orbitState.rsat;
                  Vector3 rl_ant = gc_to_antenna.Forward(rl);
                  double range,theta,phi;
                  rl_ant.SphericalGet(&range,&theta,&phi);
                  double gain;
                  if(!beam->GetPowerGain(theta,phi,&gain)){
                    gain=0;
                  }
                  double GatGar=gain*gain; // assumes separate transmit and receive feeds
                  //cout << range << " " << _ptr_array[ii][jj] << " " << integrationStepSize << endl;
                  //cout << gain <<  " " << theta*rtd << " " << phi*rtd << endl;

                  float dX=GatGar*_ptr_array[ii][jj]*integrationStepSize*
                           integrationStepSize/(range*range*range*range);
                  X_rain += dX;
                  layer_Es_rain += dX*rainS0;

                  //cout << "rain dX, Es: " << dX << " " << X_rain << " " << rainS0 << " " << layer_Es_rain << endl;
                } // az step loop

              } // rng step loop

              Es_rain += layer_Es_rain*exp(-2.*layer_attn);
              //cout << "layer, Es rain: " << nn << " " << Es_rain << endl;
	    
            } // layer loop

          } // simRain && rainField.flag_3d
          //----------------------------- End of 3-D rain simulation block and end of comment sparse area
	

          //-------------------------------------------------------------
          // Main Loop for computing X and signal energy and noise energy
          // Loops over range and azimuth integration bins
          // Currently only the center location of each bin is used to
          // obtain the quantities to integrate (ie, point target response, antenna gain, wind/sigma0) and
          // they are treated as if they were constant within the bin, so that the integration step size
          // has to be small to ascertain accurate results.
          //
          // Perhaps a faster algorithm with similiar accuracy could be obtained by
          // a piecewise linear or quadratic analytical integration which a larger bin size.
          // Linear might work for sigma0 or antenna gain, but point target response would require
          // something. Perhaps the point target response could be integrated at a finer bin size
          // than every thing else or the fact that its functional form is often known could be
          // utilized.
          //
          // Any such change and changes in integration bin size itself should be
          // checked against a small bin size slow approach for accuracy. Ideally everything
          // should be checked against a 25 m bin size, or at least shown to not change much
          // from 1.0 km down to 0.5 km and 0.25 km. Debugging tools can be uncommented to aid
          // this effort.(Probably a define statement should be used to aid switching debug
          // tools in and out.) BWS  07/22/2009
          //--------------------------------------------------------------
	  for(int i=0;i<nrsteps;i++){
	    double r=r0+(i-center_range_idx_ave)*integrationStepSize; // range compoment of location of integration bin
	    for(int j=0;j<nasteps;j++){
	      double a=a0+(j-center_azim_idx)*integrationStepSize;  // azimuth component of integration bin

              // Location of bin  in range and azimuth coordinates
	      Vector3 locra(r,a,0.0);

              // Location of bin in geocentric xyz coordinates
	      EarthPosition locgc=gc_to_rangeazim.Backward(locra);
              locgc+=spot_centroid;

              //-----------------------------------------------------------
	      // compute geodetic latitude, longitude of bin on the ground
              //-----------------------------------------------------------
	      double alt, lat, lon;
              float s0;
	      WindVector wv;
	      if (! locgc.GetAltLonGDLat(&alt, &lon, &lat))
		return(0);  

              // package lon and lat for use by Windfield routines
	      LonLat lon_lat;
	      lon_lat.longitude = lon;
	      lon_lat.latitude = lat;

 
                 // debugging tool commented out
                 //cout << "latlon: " << lat*rtd << " " << lon*rtd << endl;

              //---------------------------------------------------------------------
              // determine if bin is over land if SIM_COAST config file option is used
              // otherwise assume the bin is over open ocean. Cases in which land is
              // masked out will not make it this far if a conservative land map is used 
              //----------------------------------------------------------------------
              int island=0;
              if(simCoast){
		island=landMap.IsLand(lon, lat);
	      }

              // Implement constant sigma0 case if desired
              if(uniformSigmaField){
		s0=uniformSigmaValue;
	      }

              // flag measurement as land inclusive if land is found in bin
              // and set sigma0 to land value assigned in Config file for the current beam
	      else if(island){
		meas->landFlag=3;
		s0=landSigma0[meas->beamIdx];
	      }

              //-------------------------------------------------------
              // if bin is not over land read wind vector from windfield
              // and compute sigma0
              //-------------------------------------------------------
              else{
		if (! windfield->InterpolatedWindVector(lon_lat, &wv))
		  {
		    wv.spd = 0.0;
		    wv.dir = 0.0;
		  }

                // We need the relative azimuth, chi, to get sigma0 from the GMF table
		// chi is defined so that 0.0 means the wind is blowing towards
		// the s/c (the opposite direction as the look vector)
		float chi = wv.dir - meas->eastAzimuth + pi;

                // get sigma0 from the geophysical model function
                // using measurement type, incidence angle, chi and wind speed
		gmf->GetInterpolatedValue(meas->measType, meas->incidenceAngle,
					  wv.spd, chi, &s0); 

                
                     // commented out debugging tool
                     //cout << "wind: " << wv.spd << " " << chi << endl;

                //--------------------------------------------------------------
		// add rain contamination if SIM_RAIN option in config file is 1
                //--------------------------------------------------------------
                if(simRain){
                  // 2-d rain simulation version, until 3-D code is fixed only use this version
                  if (!rainField.flag_3d) {
		    float a,b;

                    // compute rain attenuation and rain backscatter
                    // goodrain is set to 0 is rain data is unavailable or flagged invalid
                    int goodrain=rainField.InterpolateABLinear(lon_lat,meas->incidenceAngle,a,b);

                    // Print a warning message if uncontaminated s0, rain attenuation, rain backscatter, or rain contaminated s0 are Not a Number
                    // This should only occur when wind speed is 0 and attenuation is infinite (or very large) so that surface is not visible at all
		    if (isnan(s0)|| isnan(a) || isnan(b) || isnan(s0/a+b)) {
                      fprintf(stderr," NANBUG flag %d, s0 before %g, a %g, b %g, s0 after %g, lat %g, lon %g, inc %g\n",goodrain,s0,a,b,s0/a+b,lat*rtd,lon*rtd,meas->incidenceAngle*rtd);
                    }

                    // contaminate s0 with rain if rain data is valid
		    if(goodrain) s0= s0/a + b;
                  }
		} // end of if rain simulation on condition
	      } // end of bin over open ocean condition

              //---------------------------------------------------------------//
              // Fuzz the sigma0 by Kpm to simulate the effects of model function
              // error.  The resulting sigma0 is the 'true' value.
              // It does not map back to the correct wind speed for the
              // current beam and geometry because the model function is
              // not perfect.
              //---------------------------------------------------------------//

              //cout << "s0 from GMF: " << s0 << endl;

              //----------------------------------------------
              // Put in uncorrelated component of model function error (Kpm)
              // As implemented this value is taken from a Gamma distribution
              // with variance = kpm*kpm*s0*s0 such that kpm is a function of
              // speed and measurement type
              //
              // Since this is incorporated within the integration loop the kpm
              // values in the table need to be consistent with the resolution of the
              // integration bins. The current kpm.dat table was developed from 25 km
              // NSCAT data. This seems inappropriate. Additionally the sources of
              // kpm error are somewhat mysterious since they involve unknown non-wind
              // effects on sigma0. The one part we are sure about, errors due to wind
              // gradients should already be accounted for by the high resolution integration
              // over the wind fields
              //
              // For these reasons I recommend omitting this step by setting SIM_UNCORR_KPM=0
              // in the config file. -- BWS 07/09/2009
	      //---------------------------------------------------------------------
              if (simUncorrKpmFlag == 1)
              {
                double kpm2;
                if (! kp->GetKpm2(meas->measType, wv.spd, &kpm2))
                {
                  printf("Error: Bad Kpm value in OvwmSim::SetMeas\n");
                  exit(-1);
                }
                Gamma gammaRv(s0*s0*kpm2,s0);
                s0 = gammaRv.GetNumber();
              }

                  // commented out debugging tool
                  //cout << "s0 after uncorr kpm: " << s0 << endl;

              //----------------------------------------
              // Correlated component of Kpm
              // This alternative envisioned that Kpm was not really
              // uncorrelated but rather had strong spatial correlations.
              // However, in practise utilizing this only has the effect
              // of requiring more simulations to get accurate global
              // statistics. Recommend setting SIM_CORR_KPM=0
              //-------------------------------------------------
              if (simCorrKpmFlag == 1)
               
              {
                s0 *= kpmField->GetRV(correlatedKpm, lon_lat);
              }

                  // commented out debugging tool
                  //cout << "s0 after corr kpm: " << s0 << endl;


 
              //-----------------------------------------
              // Estimate backscatter contribution due
              // to raindrops hitting the surface if 3-D rain
              // simulation is used. This portion of the rain
              // backscatter is already incorporated if 2-D simulation
              // is used. Since it occurs at the surface only for the 3-D
              // case only, this part is integrated during the 2-D integration,
              // rather than in the  3-D integration loop above for 3-D rain
              //-------------------------------------------
              if (simRain && rainField.flag_3d) {
                // get splash sigma
                float rainSpl;
                rainField.GetSplash(lon, lat, meas->incidenceAngle, &rainSpl);
                //cout << "s0 for wind, spl: " << s0 << " " << rainSpl << endl;
                s0 += rainSpl;
              }

              //-----------------------------        
	      // Location of bin in antenna frame
              // and determine antenna gain for bin
              //-----------------------------
	      Vector3 rl = locgc - spacecraft->orbitState.rsat;
	      Vector3 rl_ant=gc_to_antenna.Forward(rl);
	      double range,theta,phi;
	      rl_ant.SphericalGet(&range,&theta,&phi);
	      double gain;
	      if(!beam->GetPowerGain(theta,phi,&gain)){
		gain=0;
	      }


	      // This statement assumes separate transmit and receive feeds
              // Scan loss has to be incorporated by changing peak gain in config file
              // The code should be modified to utilize a moving pattern and thus
              // more directly compute scan loss. This is particularly important
              // for coastal simulations with realistic antenna patterns where the
              // side lobe structure is important.
              double GatGar=gain*gain; 

                 // commented out debugging tool
                 //cout << "gain: " << gain << " " << theta*rtd << " " << phi*rtd  << endl;

              //----------------------------------------------------
              // Compute contribution of bin to calibration factor X
              //----------------------------------------------------
	      float dX=GatGar*_ptr_array[i][j]*integrationStepSize*
		integrationStepSize/(range*range*range*range);


              //-----------------------------------------------
              // Integrate contributions from SAR ambiguties if desired
              // See ConfigOvwmSim routine  in OvwmConfig.C for details
              // on how to turn this on and off
              // This block sparsely commented.
              //-----------------------------------------------
              if(integrateAmbig){

                loc_xyz_in_meter= locgc;
                loc_xyz_in_meter *=1000.0;//change km to meter
                xyz_to_llh(r_a, r_e2, loc_xyz_in_meter, loc_llh);
                sch.xyz_to_sch(loc_xyz_in_meter,loc_sch_in_meter);
                loc_along= loc_sch_in_meter(0)/1000.0;// km
                loc_cross= loc_sch_in_meter(1)/1000.0;// km
                double damb1, damb2;
                damb1=ambigTable->GetAmbRat1(beam_id, scanangle,
                                                   loc_along-bore_along,
                                                   loc_cross-bore_cross,
                                                   dummy_along,
                                                   dummy_cross);

                damb2=ambigTable->GetAmbRat2(beam_id, scanangle,
                                                   loc_along - bore_along
                                                   ,loc_cross - bore_cross,
                                                   dummy_along,
                                             dummy_cross);
                if(damb1==0) damb1=0.1;
                if(damb2==0) damb2=0.1;
                amb1+=dX/damb1;
                amb2+=dX/damb2;
              } // end of ambiguity integration; sparsely commented block
             

                 // commented out debugging tool
                 //cout << "gain, ptr: " << i << " " << j << " " << GatGar << " " << _ptr_array[i][j] << endl;
              
              //-------------------------------------------------------------
              // Add bin contribution to portion of X over land or over ocean
              //-------------------------------------------------------------
              if (island) {
                dX_land += dX;
              } else {
                dX_ocean += dX;
              }

              //--------------------------------------------------------------
              // Add bin contributions of X and Es to totals for measurement
              //-------------------------------------------------------------
	      meas->XK+=dX;
	      Es+=dX*s0;

              //--- Print warning message if Es is now not a number 
              // --- Can occur wind speed =0 and very high rain conditions
	      if(isnan(Es)){
		fprintf(stderr,"AAAAAAAAAARRRRRRRGGGHHHHHHH CATCH BUG Es %g dX %g s0 %g\n",Es,dX,s0);
	      }
	  
              //------------------------------------------------------------------
              // reassign ptr_array to dX. This is used to compute a more accurate
              // half power azimuth and range resolution quantity below
              //------------------------------------------------------------------
              _ptr_array[i][j]=dX;

              // determine maximal dX value
              if(dX> maxdX) maxdX=dX;
	    } // end of loop of azimuth steps
	  } // end of loop over range steps
          //--------------------------------------------------
          // End of main surface integration loop
          //---------------------------------------------------

              // commented out debugging tools
              //cout << "target X and E before rain effect: " << meas->XK << " " << Es << endl;

          //---------------------------------------------------------------------------------------------
          // When 3-D rain computation is enabled apply
          // 3-D integrated estimates of backscatter from rain column
          // and attenuation.
          // --- This may be why the 3-D rain version is buggy
          // --- The height integration should be an extra loop within the main 2-D integration loops
          // --- Integrating the rain effects separately and then applying them to the 2-D integrated rain
          // --- free measurement at the end is WRONG!!!!
          //----------------------------------------------------------------------------------------------
          if (simRain && rainField.flag_3d) {
            Es *= exp(-2.*attn);
            Es += Es_rain;
          }

              // commented out debugging toold
              //cout << "rain (scat, attn): " << Es_rain << " " << attn << endl;
              //cout << "target E with rain: " << Es << endl;

          
          //--------------------------------------------------------------------
          // If we are going directly to L1B eliminate measurements with an X portion
          // over land that is greater than a hard-coded threshold value -- see above
          // dX_THRESHOLD should be a config file parameter!
          // If we are processing to L1A, then OvwmL1AToL1B::Convert will remove these
          // measurements.
          //---------------------------------------------------------------------
          if (dX_land/meas->XK >= dX_THRESHOLD && sim_l1b_direct && !sim_all_land) {
            meas=meas_spot->RemoveCurrent();
            delete meas;
            meas=meas_spot->GetCurrent();
            slice_i++;
            continue;
          }

          //----------------------------------------------------------------
          // Use the ptr_array that now contains the X contributions from
          // each bin to determine the half power range and azimuth width.
          //---------------------------------------------------------------
          int jmin= int(nasteps);
          int jmax= int(0);
          int imin= int(nrsteps);
	  int imax= int(0);
	  for(int i=0;i<nrsteps;i++){
	    for(int j=0;j<nasteps;j++){
	      if(_ptr_array[i][j]>0.5*maxdX){
		if(i<imin) imin=i;
		if(i>imax) imax=i;
		if(j<jmin) jmin=j;
		if(j>jmax) jmax=j;
	      }
	    }
	  }
          meas->azimuth_width=integrationStepSize*(jmax-jmin);
          meas->range_width=integrationStepSize*(imax-imin);

	
             // commented out debugging tools
             //printf("Es: %g\n", Es);
             //cout << "Es: " << Es << endl;


          // Special XOWVM (SAR MODE num_pulses = 11 )
          // debugging option (see above for other parts of this)
	  if(generate_map){
	    X_map_[range_index][azimuth_index]=meas->XK;
            // commented out debugging tools
	    //cout<<"range azimuth index "<< range_index<< " "<<azimuth_index<<endl;
	    //cout<<"range azimuth in km "<< range_km<<" "<<azimuth_km<<endl;
	  }
	
          //-------------------------------------------------------------
          // Put in constants from radar equation to get values in Joules
          // and to estimate noise power. Used to estimate noise power for
          // very low SNR cases
	  //------------------------------------------------------------

          //----------------------------------------------------------------
          // Compute noise density and wavelength (bK is boltzmann's constant)
          //------------------------------------------------------------------
          double N0=bK*ovwm->systemTemperature; // noise density
	  double lambda = speed_light_kps/ ovwm->ses.txFrequency; // wavelength

          //--------------------------------------------------------------
          // Compute SNR and constant factor in X for measurement
          //--------------------------------------------------------------

          //----- constant portion of X factor
          double ksig=ovwm->ses.transmitPower*ovwm->ses.rxGainEcho*lambda*
	    lambda*ovwm->ses.txPulseWidth*ovwm->ses.numPulses
	    /(64*pi*pi*pi*ovwm->systemLoss);
                 // commented out debugging tool
                 //cout << "ksig: " << ksig << endl;

          //---- constant to convert signal energy to SNR
          double kSNR=ovwm->ses.transmitPower*lambda*
	    lambda*ovwm->ses.txPulseWidth*ovwm->ses.numPulses
	    /(64*pi*pi*pi*ovwm->systemLoss*N0*float(nL))
            *ovwm->ses.receivePathLoss; // the last term added as noise has this factor

          //---- SNR
          double SNR=Es*kSNR;
          
          //--------------------------------------------------
          // Normalize power contribution from SAR ambiguities
          // if they have been integrated
          //--------------------------------------------------
          if(integrateAmbig){
            amb1/=meas->XK;
            amb2/=meas->XK;
          }

          //----------------------------------------------------------------
          // Scale Es AND X factor by appropriate constant now Es=XK*sigma0
          //  when Es in Joules and sigma0 is unitless
          //--------------------------------------------------
          meas->XK*=ksig;
              // commented out debugging tool
              //cout << "XK: " << meas->XK << endl;
          Es*=ksig;
	      // commented out debugging tool
              //cout << "Es after ksig: " << Es << endl;

          //-------------------------------------------------------
          // Compute Noise Energy in Joules from Es and SNR
          // (print warning if Es is Not a Number)
          //-------------------------------------------------------
          En=Es/SNR;
          if(isnan(Es)){
            fprintf(stderr,"AAAAAAAAAARRRRRRRGGGHHHHHHH CATCH BUG En %g Es %g SNR %g kSNR %g ksig %g N0 %g lambda %g\n",En,Es,SNR,kSNR,ksig,N0,lambda);
	  }

             // commented out debugging tool
             //printf("SNR, Es, En: %g %g %g\n", SNR, Es, En);

          //-------------------------------------------------
          // Set Noise value this way when SNR is very low
          // to avoid numerical error
          //-------------------------------------------------
          if (SNR < SNR_CUTOFF) { 
            En = N0*float(nL)/ovwm->ses.receivePathLoss;
          }
	

          //---------------------------------------------
          //  Assign noise energy quantity to Meas object
          //---------------------------------------------
          meas->EnSlice=En;
        
         
               // debug tools  commented out
	       //cout << "MeasType=" << meas_type_map[(int) meas->measType]<<" s0 =" << Es/meas->XK << " Es="<<Es << " En=" << En <<" XK="<< meas->XK << endl;
	  
          //-----------------------------------
          // compute bias due to SAR ambiguities
          // This block of code uses the previously
          // computed SAR ambiguity locations and
          // isolation ratio to determine the ambiguous
          // contribution to the signal energy.
          // It should probably be turned off for
          // num_pulses = 1. Instead of doing that
          // the isolation quantities have been set
          // so that the signal energy is unchanged
          // Comments are sparse in this block
          //----------------------------------
          float amb1s0, amb2s0;
	

	  double alt, lat, lon;
	  WindVector wv;
	  if (! amb1pos.GetAltLonGDLat(&alt, &lon, &lat)){
	    amb1s0=0;
	    fprintf(stderr,"Warning amb1 not on surface\n");
	  }
          else{
	    LonLat lon_lat;
	    lon_lat.longitude = lon;
	    lon_lat.latitude = lat;
	    int island=0;
	    island=landMap.IsLand(lon, lat);
            if(uniformSigmaField) amb1s0=uniformSigmaValue;
	    else if(island){
	      meas->landFlag=3;
	      amb1s0=landSigma0[meas->beamIdx];
	    }
	    else{
	      if (! windfield->InterpolatedWindVector(lon_lat, &wv))
		{
		  wv.spd = 0.0;
		  wv.dir = 0.0;
		}
	      
	      // chi is defined so that 0.0 means the wind is blowing towards
	      // the s/c (the opposite direction as the look vector)
	      float chi = wv.dir - meas->eastAzimuth + pi;
	      
	      gmf->GetInterpolatedValue(meas->measType, meas->incidenceAngle,
					wv.spd, chi, &amb1s0); 

	      // add rain contamination if simRain
	      if(simRain){
                if (!rainField.flag_3d) {
                  float a,b;
                  int goodrain=rainField.InterpolateABLinear(lon_lat,meas->incidenceAngle,a,b);
                  if(goodrain) amb1s0= amb1s0/a + b;
                } else if (rainField.flag_3d) {
                  //cout << lon*rtd << " " << lat*rtd << endl;

                  // get splash
                  float rainSpl;
                  rainField.GetSplash(lon, lat, meas->incidenceAngle, &rainSpl);

                  //cout << "s0: amb1, spl: " << amb1s0 << " " << rainSpl << endl;

                  amb1s0 += rainSpl;

                  float comb1s0;
                  rainField.ComputeAmbEs(spacecraft, spot_centroid, amb1pos,
                                         meas->incidenceAngle, gc_to_rangeazim,
                                         amb1s0, &comb1s0);
                  amb1s0 = comb1s0;
                  //cout << "amb1s0 after rain: " << amb1s0 << endl;
                }
	      }
	    }
	  }

	  if (! amb2pos.GetAltLonGDLat(&alt, &lon, &lat)){
	    amb2s0=0;
	    fprintf(stderr,"Warning amb2 not on surface\n");
	  }
          else{
	    LonLat lon_lat;
	    lon_lat.longitude = lon;
	    lon_lat.latitude = lat;
	    int island=0;
	    island=landMap.IsLand(lon, lat);
            if(uniformSigmaField)amb2s0=uniformSigmaValue;
	    else if(island){
	      meas->landFlag=3;
	      amb2s0=landSigma0[meas->beamIdx];
	    }
	    else{
	      if (! windfield->InterpolatedWindVector(lon_lat, &wv))
		{
		  wv.spd = 0.0;
		  wv.dir = 0.0;
		}
	      
	      // chi is defined so that 0.0 means the wind is blowing towards
	      // the s/c (the opposite direction as the look vector)
	      float chi = wv.dir - meas->eastAzimuth + pi;
	      
	      gmf->GetInterpolatedValue(meas->measType, meas->incidenceAngle,
					wv.spd, chi, &amb2s0); 

	      // add rain contamination if simRain
	      if(simRain){
                if (!rainField.flag_3d) {
		  float a,b;
		  int goodrain=rainField.InterpolateABLinear(lon_lat,meas->incidenceAngle,a,b);
		  if(goodrain) amb2s0= amb2s0/a + b;
                } else if (rainField.flag_3d) {

                  // get splash
                  float rainSpl;
                  rainField.GetSplash(lon, lat, meas->incidenceAngle, &rainSpl);

                  //cout << "s0: amb2, spl: " << amb2s0 << " " << rainSpl << endl;

                  amb2s0 += rainSpl;

                  float comb2s0;
                  rainField.ComputeAmbEs(spacecraft, spot_centroid, amb2pos,
                                         meas->incidenceAngle, gc_to_rangeazim,
                                         amb2s0, &comb2s0);
                  amb2s0 = comb2s0;
                  //cout << "amb2s0 after rain: " << amb2s0 << endl;
                }
	      }
	    }
	  }
	
	  Es+=meas->XK*amb1*amb1s0+meas->XK*amb2*amb2s0;
             // commented out debugging tool
             //cout << "Es after amb: " << Es << endl;

          //----------------------------------------
          // End of add ambiguity bias to Es block
          // and end of sparse comments
          //------------------------------------------

          //------------------------------------------------
          // compute Kpc (Thermal and Fading Noise) Variance
          // (variance in Es after noise subtraction)
          //------------------------------------------------
          double kpc2=(1/(float)nL)*(1+2/SNR+1/(SNR*SNR));
	  var_esn_slice=kpc2*Es*Es;

          // For very low SNR turn off Kpc. 
          // This should only occur for cases in which sigma0 is 30 dB
          // below the noise floor and thus wind speed is identically 0
          // It is meant for invalid wind speed regions not real
          // low winds.
          // It keeps Kpc from blowing up and generating numerical error
          if (SNR < SNR_CUTOFF) { // deal with L1A pixel with low gain
            var_esn_slice=0.;
          }

          //--------------------------------
          // Turn off Kpc if SIM_KPC = 0
          // in the config file
          //---------------------------------
          if(!simKpcFlag){
	    var_esn_slice=0;
	  }

          // special case debugging output specific to XOVWM design
	  if(generate_map)
	    kpc_map_[range_index][azimuth_index]=kpc2;


          //----------------------------------
          // Set noise variance parameters in
          // Meas object if we are simulating
          // directly to L1B
          //----------------------------------
	  if(sim_l1b_direct){
            float s0ne=En/meas->XK; // noise equivalent s0
            float alpha=1/(float)nL;
            meas->A=alpha+1.0;
            meas->B=2.0*s0ne/(float)nL;
            meas->C=s0ne*s0ne/(float)nL;
          }
	
          //--------------------------------------------------          
          // Set Value in Measurement to Noise + Signal Energy
          // This is the place to put Es+En when writing a
          // measurement to a L1A file
          //---------------------------------------------------
          meas->value=Es+En;


        
             // commented out debugging tools
             //cout << "Es after noise, meas value: " << meas->value << endl;

          //----------------------------------------
          // Add random Kpc noise to the value field
          //----------------------------------------
          double v=normrv.GetNumber()*sqrt(var_esn_slice); 
          meas->value+=v;
	
            // commented out debugging tools
            //cout << "meas value after kpc: " << meas->value << endl;
            //if (slice_i > 100) exit(1);

          //-------------------------------------------------------------------
          // Noise subtract/calibrate to get s0 if L1B direct output is desired
          //-------------------------------------------------------------------
	  if(sim_l1b_direct){
	    meas->value-=En;
	    meas->value/=meas->XK;

	  }
	  
          //----------------------------------------------------------------
          // implement special debugging routine to replace value with Ambiguity ratio
          // see ConfigOvwmSim for keyword to implement this if desired.
          //----------------------------------------------------------------
          if(replaceValueWithAmbRat) meas->value=amb1+amb2;
          
            // commented out debugging tool
            //cout << meas->value << endl;
	  
	


 
	}  
	//------------------------------------------//
        // end of SIM_HIGH_RES=1  case              //
        //------------------------------------------//

        slice_i++;
        meas = meas_spot->GetNext(); // gets next measurement in footprint 
                                     // or NULL pointer no more left
    }
    //--------------------------------//
    // end of loop over measurements  //
    // in antenna footprint           //
    //--------------------------------//

    //-------------------------------------------
    // write amb,x,kpc, and x map to output files
    // for special XOVWM debugging case
    //-------------------------------------------
    if(generate_map){
     
      char angle_str[7],beam_str[1];
      sprintf(beam_str,"%d",beam_id+1);
      sprintf(angle_str,"%d",int(bs_scanangle*100.0));
     
      char amb_filename[80];
      char gain_filename[80];
      char  kpc_filename[80];
      char X_filename[80];
      sprintf(amb_filename,"%s%d%s%d%s","amb_",beam_id+1,"_",int(bs_scanangle*100.0),".dat");
      sprintf(gain_filename,"%s%d%s%d%s","gain_",beam_id+1,"_",int(bs_scanangle*100.0),".dat");
      sprintf(kpc_filename,"%s%d%s%d%s","kpc_",beam_id+1,"_",int(bs_scanangle*100.0),".dat");
      sprintf(X_filename,"%s%d%s%d%s","X_",beam_id+1,"_",int(bs_scanangle*100.0),".dat");

     ;
      std::ofstream amb_file(amb_filename);
      if(!amb_file.is_open()) 
	fprintf(stderr,"Uable to open amb file to write map info to file\n");
      for(int i=0;i<ovwm->ses.numRangePixels;++i){
	for(int j=0;j<ovwm->ses.numPulses;++j){
	  amb_file<<amb_map_[i][j]<<" ";
	}
	amb_file<<endl;
      }
      amb_file.close();


     
      std::ofstream gain_file(gain_filename);
      if(!gain_file.is_open()) 
	fprintf(stderr,"Uable to open gain file to write map info to file\n");
      for(int i=0;i<ovwm->ses.numRangePixels;++i){
	for(int j=0;j<ovwm->ses.numPulses;++j){
	  gain_file<<gain_map_[i][j]<<" ";
	}
	gain_file<<endl;
      }
      gain_file.close();

     
      std::ofstream kpc_file(kpc_filename);
      if(!kpc_file.is_open()) 
	fprintf(stderr,"Uable to open kpc file to write map info to file\n");
      for(int i=0;i<ovwm->ses.numRangePixels;++i){
	for(int j=0;j<ovwm->ses.numPulses;++j){
	  kpc_file<<kpc_map_[i][j]<<" ";
	}
	kpc_file<<endl;
      }
      kpc_file.close();


      
      std::ofstream X_file(X_filename);
      if(!X_file.is_open()) 
	fprintf(stderr,"Uable to open X file to write map info to file\n");
      for(int i=0;i<ovwm->ses.numRangePixels;++i){
	for(int j=0;j<ovwm->ses.numPulses;++j){
	  X_file<<X_map_[i][j]<<" ";
	}
	X_file<<endl;
      }
      X_file.close();


    } // end of XOVWM specific debugging output
    

    // returns 1 on completion of routine
    return(1);
}

//-------------------------//
// OvwmSim::SetL1AScience //
//-------------------------//

int
OvwmSim::SetL1AScience(
    MeasSpot*    meas_spot,
    CheckFrame*  cf,
    Ovwm*       ovwm,
    OvwmL1AFrame*    l1a_frame)
{
    //----------------------//
    // set antenna position //
    //----------------------//

    //l1a_frame->antennaPosition[_spotNumber] = ovwm->cds.rawEncoder;

    //-------------------------//
    // for each measurement... //
    //-------------------------//

    int meas_number = _spotNumber * l1a_frame->maxMeasPerSpot;
    if (simVs1BCheckfile) cf->EsnEcho = 0.0;

    for (Meas* meas = meas_spot->GetHead(); meas;
        meas = meas_spot->GetNext())
    {
        if (meas_number == _spotNumber*l1a_frame->maxMeasPerSpot) {
          l1a_frame->spotScanAngle[_spotNumber] = meas->scanAngle;
        }

        //----------------------------//
        // update the level 0.0 frame //
        //----------------------------//

        //l1a_frame->science[meas_number] = (unsigned int)(meas->value);

        l1a_frame->science[meas_number] = meas->value;
        //cout << meas_number << " " << meas->value << " " << l1a_frame->science[meas_number] << endl;

        if (simVs1BCheckfile) cf->EsnEcho += meas->value;

        meas_number++;
    }

    l1a_frame->dataCountSpots[_spotNumber] = meas_number - _spotNumber*l1a_frame->maxMeasPerSpot;

    //cout << "meas count " << _spotNumber << " " << l1a_frame->dataCountSpots[_spotNumber] << endl;

    // Compute the spot noise measurement.

    float spot_noise;
    if (noiseEstMethod == 0)	// Quikscat noise computation method
	    sigma0_to_Esn_noise(ovwm, meas_spot, simKpcFlag, &spot_noise);
    else {	// more general noise computation method
    	double Tg = ovwm->GetRxGateWidth();
    	double Bn = ovwm->ses.noiseBandwidth;
    	double var_noise = 1/(Tg*Bn);
    	Gaussian rv(var_noise,0.0);
    	double En_sum = 0;
    	int En_count = 0;
    	for (Meas* meas = meas_spot->GetHead(); meas != NULL; meas = meas_spot->GetNext()) {
    		En_sum += meas->EnSlice;
    		En_count++;
    	}
    	
    	// NOTE: when using this method, 'spot_noise' is actually the /slice/ noise
    	// the name of the variable does not match what it actually is b/c the it needs
    	// to store conceptually different values depending on the method used
    	spot_noise = (En_sum/En_count) * (1 + rv.GetNumber());
    }

    l1a_frame->spotNoise[_spotNumber] = spot_noise;

    if (simVs1BCheckfile) cf->EsnNoise = spot_noise;

    //cout << "spot noise: " << _spotNumber << " " << spot_noise << endl;

    return(1);
}

//--------------------------//
// OvwmSim::SetL1ALoopback //
//--------------------------//

int
OvwmSim::SetL1ALoopback(
    Ovwm*       ovwm,
    OvwmL1AFrame*    l1a_frame)
{

    //----------------------//
    // set antenna position //
    //----------------------//

    //l1a_frame->antennaPosition[_spotNumber] = ovwm->cds.rawEncoder;

    //-------------------------------------------//
    // Set Es_cal using PtGr.                    //
    // Only "noise it up" if simKpriFlag is set. //
    //-------------------------------------------//

    float Esn_echo_cal,Esn_noise_cal;
    PtGr_to_Esn(&ptgrNoise,ovwm,simKpriFlag,&Esn_echo_cal,&Esn_noise_cal);
    //cout << "Loop back: " << endl;
    //cout << simKpriFlag << endl;
    //cout << Esn_echo_cal << endl;
    //cout << Esn_noise_cal << endl;

    //-------------------//
    // for each slice... //
    //-------------------//

    //int base_slice_number = _spotNumber * l1a_frame->slicesPerSpot;
    //for (int i=0; i < l1a_frame->slicesPerSpot; i++)
    //{
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

    //    l1a_frame->loopbackSlices[i] = 0;
    //    l1a_frame->science[base_slice_number + i] = 0;
    //}

    // Now set the single slice with loopback energy.
    // Note the scale factor of 256 (8 bit shift) applied only to echo channel.
    //int icenter = 4;
    //l1a_frame->loopbackSlices[icenter] = (unsigned int)(Esn_echo_cal/256.0);
    //l1a_frame->science[base_slice_number + icenter] =
    //  (unsigned int)(Esn_echo_cal/256.0);

    l1a_frame->loopbackSpots[_spotNumber] = Esn_echo_cal;

    //----------------------------------------------//
    // Set corresponding noise channel measurements //
    //----------------------------------------------//

    l1a_frame->loopbackNoise = Esn_noise_cal;
    //l1a_frame->spotNoise[_spotNumber] = (unsigned int)(Esn_noise_cal);

    //cout << "In L1A: " << endl;
    //cout << l1a_frame->loopbackSpots[_spotNumber] << endl;
    //cout << l1a_frame->loopbackNoise << endl;
    //cout << l1a_frame->spotNoise[_spotNumber] << endl;

    //--------------------------------------------------------//
    // Set cal position indicator so that the actual position //
    // is one less than the unit offset position index        //
    // (follows instrument telemetry).                        //
    // Note that this is only done with loopbacks, and not    //
    // for load measurements which always follow a loopback.  //
    //--------------------------------------------------------//

/* not implement status and in_eu now
    //l1a_frame->calPosition = _spotNumber + 2;
    //l1a_frame->in_eu.true_cal_pulse_pos = l1a_frame->calPosition;
    //l1a_frame->status.specified_cal_pulse_pos = l1a_frame->calPosition;
*/

    return(1);
}

//----------------------//
// OvwmSim::SetL1ALoad //
//----------------------//

int
OvwmSim::SetL1ALoad(
    Ovwm*       ovwm,
    OvwmL1AFrame*    l1a_frame)
{

    //----------------------//
    // set antenna position //
    //----------------------//

    //l1a_frame->antennaPosition[_spotNumber] = ovwm->cds.rawEncoder;

    //-------------------------------------------//
    // Compute load noise measurements to assure //
    // a (nearly) perfect retrieval of alpha.             //
    //-------------------------------------------//

    float En_echo_load;
    float En_noise_load;
    make_load_measurements(ovwm,&En_echo_load,&En_noise_load);

    //cout << "Load: " << endl;
    //cout << En_echo_load << endl;
    //cout << En_noise_load << endl;

    //-------------------//
    // for each slice... //
    //-------------------//

    //int base_slice_number = _spotNumber * l1a_frame->slicesPerSpot;
    //for (int i=0; i < l1a_frame->slicesPerSpot; i++)
    //{
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

    //  if (l1a_frame->slicesPerSpot == 12)
    //  {
    //    float Bs = 8314.0;   // Nominal value consistent with q-table.
    //    float Bg = 46190.0;  // Nominal value consistent with q-table.
    //    float Be = 10*Bs + 2*Bg;
    //    if (i == 0 || i == 11)
    //    {
    //      l1a_frame->loadSlices[i] =
    //        (unsigned int)(En_echo_load * ovwm->ses.Qtable[i]*Bg/Be);
    //    }
    //    else
    //    {
    //      l1a_frame->loadSlices[i] =
    //        (unsigned int)(En_echo_load * ovwm->ses.Qtable[i]*Bs/Be);
    //    }
    //  }
    //  else
    //  {  // assume slice bandwidths are all the same
    //    l1a_frame->loadSlices[i] =
    //      (unsigned int)(En_echo_load/l1a_frame->slicesPerSpot);
    //  }
    //  l1a_frame->science[base_slice_number + i] = l1a_frame->loadSlices[i];
    //}

/*** check whether I need to divide # of meas in the spot ***/
/*** now we didn't do the division                        ***/

    //l1a_frame->loadSpots[_spotNumber] = (unsigned int)(En_echo_load);
    l1a_frame->loadSpots[_spotNumber] = En_echo_load;

    //----------------------------------------------//
    // Set corresponding noise channel measurements //
    //----------------------------------------------//

    //l1a_frame->loadNoise = (unsigned int)(En_noise_load);
    l1a_frame->loadNoise = En_noise_load;
    //l1a_frame->spotNoise[_spotNumber] = (unsigned int)(En_noise_load);

    //cout << "L1A:" << endl;
    //cout << l1a_frame->loadSpots[_spotNumber] << endl;
    //cout << l1a_frame->loadNoise << endl;
    //cout << l1a_frame->spotNoise[_spotNumber] << endl;
    return(1);
}

//--------------------------//
// OvwmSim::ComputeXfactor //
//--------------------------//
int
OvwmSim::ComputeXfactor(
    Spacecraft*  spacecraft,
    Ovwm*       ovwm,
    Meas*        meas,
    float*       X)
{
    // Incomplete eventually this routine needs to be replaced
  fprintf(stderr,"OvwmSim::ComputeXFactor is an invalid routine\n");
  exit(1);
    /*
    if (! IntegratePixel(spacecraft, ovwm, meas, numLookStepsPerSlice,
        azimuthIntegrationRange, azimuthStepSize, rangeGateClipping, X))
    {
        return(0);
    }
    Beam* beam = ovwm->GetCurrentBeam();
    double Xcal;
    float Es_cal = true_Es_cal(ovwm);

    radar_Xcal(ovwm,Es_cal,&Xcal);
    (*X)*=Xcal/(beam->peakGain * beam->peakGain);
    return(1);
    */
}

//----------------------//
// OvwmSim::MeasToEsnX //
//----------------------//

// This method converts an average sigma0 measurement (of a particular type)
// to signal + noise energy measurements.
// The received energy is the sum of the signal energy and the noise energy
// that falls within the appropriate bandwidth.
// The total X factor is a required input.
// The result is fuzzed by Kpc (if requested) and by Kpm (as supplied).

int
OvwmSim::MeasToEsnX(
    Ovwm*  ovwm,
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
          "Error: OvwmSim::MeasToEsnX encountered invalid sigma0 = %g\n",
          sigma0);
        exit(-1);
    }

    double Tp = ovwm->ses.txPulseWidth;
    double Tg = ovwm->GetRxGateWidth();
    double Bs = meas->bandwidth;
    double L13 = ovwm->ses.receivePathLoss;

    //------------------------------------------------------------------------//
    // Signal (ie., echo) energy referenced to the point just before the
    // I-Q detection occurs (ie., including the receiver gain and system loss).
    // X has units of energy because Xcal has units of Pt * Tp.
    //------------------------------------------------------------------------//

    *Es = X*sigma0;

    //------------------------------------------------------------------------//
    // Noise power spectral densities referenced the same way as the signal.
    //------------------------------------------------------------------------//

    double N0_echo = bK * ovwm->systemTemperature *
        ovwm->ses.rxGainEcho / L13;

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
    if (! ovwm->ses.GetQRel(meas->startSliceIdx, &q_slice))
    {
      fprintf(stderr,"OvwmSim::MeasToEsnX: Error getting Q value\n");
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
// OvwmSim::MeasToEsnK //
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
//    ovwm = pointer to current Ovwm object
//    meas = pointer to current measurement (sigma0, cell center, area etc.)
//    Kfactor = Radar equation correction factor for this cell.
//    sigma0 = true sigma0 to assume.
//    Esn_slice = pointer to signal+noise energy in a slice.
//    X = pointer to true total X (ie., X = x*Kfactor).
//

int
OvwmSim::MeasToEsnK(
    Spacecraft*  spacecraft,
    Ovwm*       ovwm,
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
    &(spacecraft->attitude), &(ovwm->sas.antenna),
    ovwm->sas.antenna.txCenterAzimuthAngle);
  gc_to_antenna = gc_to_antenna.ReverseDirection();

  //----------------------------------------------------------------------//
  // Compute the radar parameter x which includes gain, loss, and geometry
  // factors in the received power.  This is the true value.  Processing
  // uses a modified value that includes fuzzing by Kpr.  The total X
  // includes the K-factor. ie., X = x*K
  //----------------------------------------------------------------------//

  double x;
  radar_X(&gc_to_antenna, spacecraft, ovwm, meas, &x);
  *X = x*Kfactor;

  return(MeasToEsnX(ovwm, meas, *X, sigma0,
                    Esn, Es, En, var_Esn));
}
