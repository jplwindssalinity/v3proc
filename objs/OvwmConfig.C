//==============================================================//
// Copyright (C) 1998-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_ovwmconfig_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <string.h>
#include "Ovwm.h"
#include "ConfigList.h"
#include "OvwmConfig.h"
#include "OvwmConfigDefs.h"
#include "Misc.h"
#include "OvwmSim.h"
#include "ConfigSimDefs.h"
#include "Constants.h"
#include "ConfigSim.h"
#include "Interpolate.h"
#include "ETime.h"

//----------------//
// ConfigOvwmSas //
//----------------//

int
ConfigOvwmSas(
    OvwmSas*    ovwm_sas,
    ConfigList*  config_list)
{
    //-----------------------//
    // configure the antenna //
    //-----------------------//

    if (! ConfigAntenna(&(ovwm_sas->antenna), config_list))
        return(0);

    //-----------------------------//
    // set the SAS encoder offsets //
    //-----------------------------//

    // assum zero encoder offset for now
    ovwm_sas->encoderOffset = 0;

    //-----------------------------//
    // set spin rate               //
    //-----------------------------//
    

    float spin_rate;
    if (! config_list->GetFloat(ANTENNA_SPIN_RATE_KEYWORD,
        &spin_rate))
    {
        return(0);
    }
    ovwm_sas->SetSpinRate(spin_rate);

    return(1);
}

//----------------//
// ConfigOvwmSes //
//----------------//

int
ConfigOvwmSes(
    OvwmSes*    ovwm_ses,
    ConfigList*  config_list)
{
    //-------------------------//
    // base transmit frequency //
    //-------------------------//

    float base_tx_frequency;   // GHz
    if (! config_list->GetFloat(BASE_TRANSMIT_FREQUENCY_KEYWORD,
        &base_tx_frequency))
    {
        return(0);
    }
    ovwm_ses->baseTxFrequency = base_tx_frequency * GHZ_TO_HZ;

    //----------------//
    // transmit power //
    //----------------//

    float transmit_power;    // W
    if (! config_list->GetFloat(TRANSMIT_POWER_KEYWORD, &transmit_power))
        return(0);
    ovwm_ses->transmitPower = transmit_power;

    //----------------//
    // receiver gains //
    //----------------//

    float rx_gain_echo;    // dB
    if (! config_list->GetFloat(RX_GAIN_ECHO_KEYWORD, &rx_gain_echo))
        return(0);
    ovwm_ses->rxGainEcho = (float)pow(10.0, 0.1 * rx_gain_echo);

    float rx_gain_noise;    // dB
    if (! config_list->GetFloat(RX_GAIN_NOISE_KEYWORD, &rx_gain_noise))
        return(0);
    ovwm_ses->rxGainNoise = (float)pow(10.0, 0.1 * rx_gain_noise);

    //----------------//
    // receiver gains //
    //----------------//

    float calibration_bias;    // dB
    if (! config_list->GetFloat(CALIBRATION_BIAS_KEYWORD, &calibration_bias))
        return(0);
    ovwm_ses->calibrationBias = (float)pow(10.0, 0.1 * calibration_bias);

   
    //--------------//
    // loss factors //
    //--------------//

    if (! config_list->GetFloat(RECEIVE_PATH_LOSS_KEYWORD, &(ovwm_ses->receivePathLoss)))   // dB
        return(0);
    if (! config_list->GetFloat(TRANSMIT_PATH_LOSS_KEYWORD, &(ovwm_ses->transmitPathLoss))) // dB
        return(0);


    // convert to real units
    ovwm_ses->receivePathLoss = pow(10.0,0.1*ovwm_ses->receivePathLoss);
    ovwm_ses->transmitPathLoss = pow(10.0,0.1*ovwm_ses->transmitPathLoss);


    //----------//
    // chirping //
    //----------//

    float chirp_rate;   // kHz/ms
    if (! config_list->GetFloat(CHIRP_RATE_KEYWORD, &chirp_rate))
        return(0);
    ovwm_ses->chirpRate = chirp_rate * KHZ_PER_MS_TO_HZ_PER_S;


    //-----------------//
    // noise bandwidth //
    //-----------------//

    float noise_bandwidth;    // KHz
    if (! config_list->GetFloat(NOISE_BANDWIDTH_KEYWORD, &noise_bandwidth))
        return(0);
    ovwm_ses->noiseBandwidth = noise_bandwidth * KHZ_TO_HZ;


    //-----------------//
    // numTWTAs        //
    //-----------------//

    int num_TWTAs;    
    if (! config_list->GetInt(NUM_TWTAS_KEYWORD, &num_TWTAs))
        return(0);
    ovwm_ses->numTWTAs = num_TWTAs;

    //-----------------//
    // numReceivers    //
    //-----------------//

    int num_receivers;    
    if (! config_list->GetInt(NUM_RECEIVERS_KEYWORD, &num_receivers))
        return(0);
    ovwm_ses->numReceivers = num_receivers;

    char keyword[1024];
    char number[8];
    double prev_delay=0;
    for(int b=0;b<NUMBER_OF_OVWM_BEAMS;b++){
      sprintf(number, "%d", b+1);
      substitute_string(BEAM_x_TX_DELAY_KEYWORD, "x", number, keyword);
      double tx_delay;    // dB
      if (! config_list->GetDouble(keyword, &tx_delay))
	return(0);
      tx_delay*=MS_TO_S;
      if(tx_delay<prev_delay){
	fprintf(stderr,"Error: beams must be defined in order of occurrence\n");
	exit(1);
      }
      prev_delay=tx_delay;
      ovwm_ses->txDelay[b]=tx_delay;
    }
    

    return(1);
}

//----------------//
// ConfigOvwmCds //
//----------------//

int
ConfigOvwmCds(
    OvwmCds*    ovwm_cds,
    ConfigList*  config_list)
{
    //----------------//
    // tracking flags //
    //----------------//

    int use_rgc;
    if (! config_list->GetInt(USE_RGC_KEYWORD, &use_rgc))
        return(0);
    ovwm_cds->useRgc = use_rgc;


    int use_dtc;
    if (! config_list->GetInt(USE_DTC_KEYWORD, &use_dtc))
        return(0);
    ovwm_cds->useDtc = use_dtc;



    //--------------//
    // orbit period //
    //--------------//

    unsigned int orbit_ticks;
    if (! config_list->GetUnsignedInt(ORBIT_TICKS_PER_ORBIT_KEYWORD,
        &orbit_ticks))
    {
        return(0);
    }
    ovwm_cds->orbitTicksPerOrbit = orbit_ticks;


    //------------------//
    // for each beam... //
    //------------------//

    for (int beam_idx = 0; beam_idx < NUMBER_OF_OVWM_BEAMS; beam_idx++)
    {
        //---------------------------------------//
        // ...set up the beam number as a string //
        //---------------------------------------//

        char number[8];
        int beam_number = beam_idx + 1;
        sprintf(number, "%d", beam_number);

        char keyword[1024];
        if (use_rgc)
        {
            //----------------//
            // range tracking //
            //----------------//

            substitute_string(BEAM_x_RGC_FILE_KEYWORD, "x", number, keyword);
            char* rgc_file = config_list->Get(keyword);
            if (rgc_file == NULL)
            {
                fprintf(stderr,
                "ConfigOvwmCds: error determining RGC filename for beam %d\n",
                    beam_number);
                return(0);
            }

            if (! ovwm_cds->LoadRgc(beam_idx, rgc_file))
            {
                fprintf(stderr, "ConfigOvwmCds: error loading RGC file %s\n",
                    rgc_file);
                return(0);
            }
        }

        if (use_dtc)
        {
            //------------------//
            // Doppler tracking //
            //------------------//

            substitute_string(BEAM_x_DTC_FILE_KEYWORD, "x", number, keyword);
            char* dtc_file = config_list->Get(keyword);
            if (dtc_file == NULL)
            {
                fprintf(stderr,
                "ConfigOvwmCds: error determining DTC filename for beam %d\n",
                    beam_number);
                return(0);
            }

            if (! ovwm_cds->LoadDtc(beam_idx, dtc_file))
            {
                fprintf(stderr, "ConfigOvwmCds: error loading DTC file %s\n",
                    dtc_file);
                return(0);
            }

        }
    }

    return(1);
}

//-------------//
// ConfigOvwm //
//-------------//

int
ConfigOvwm(
    Ovwm*       ovwm,
    ConfigList*  config_list)
{
    //-----//
    // SAS //
    //-----//

    if (! ConfigOvwmSas(&(ovwm->sas), config_list))
    {
        fprintf(stderr, "ConfigOvwm: error configuring SAS\n");
        return(0);
    }

    //-----//
    // SES //
    //-----//

    if (! ConfigOvwmSes(&(ovwm->ses), config_list))
    {
        fprintf(stderr, "ConfigOvwm: error configuring SES\n");
        return(0);
    }

    //-----//
    // CDS //
    //-----//

    if (! ConfigOvwmCds(&(ovwm->cds), config_list))
    {
        fprintf(stderr, "ConfigOvwm: error configuring CDS\n");
        return(0);
    }

    //----------------------------------------------------//
    // Burst and Pulse Timing and Miscellaneous SAR Stuff //
    //----------------------------------------------------//

    // BRI in ms
    if (! config_list->GetFloat(BURST_REPETITION_INTERVAL_KEYWORD, 
				&(ovwm->ses.bri)))
        return(0);
    ovwm->ses.bri*=MS_TO_S;

    // Number of Pulses Transmitted
    if (! config_list->GetInt(NUM_PULSES_KEYWORD, 
				&(ovwm->ses.numPulses)))
        return(0);


    // Maximum Range Window Width in km
    if (! config_list->GetFloat(MAX_RANGE_WIDTH_KEYWORD, 
				&(ovwm->ses.maxRangeWidth)))
        return(0);


    // Number of Range Looks Averaged 
    if (! config_list->GetInt(NUM_RANGE_LOOKS_AVERAGED_KEYWORD, 
			      &(ovwm->ses.numRangeLooksAveraged)))
        return(0);


    //----------------------//
    // transmit pulse width //
    //----------------------//

    float tx_pulse_width;    // ms
    if (! config_list->GetFloat(TX_PULSE_WIDTH_KEYWORD, &tx_pulse_width))
        return(0);
    tx_pulse_width *= MS_TO_S;
    ovwm->cds.CmdTxPulseWidthEu(tx_pulse_width, &(ovwm->ses));

    //-----//
    // PRI //
    //-----//

    float pri;
    if (! config_list->GetFloat(PRI_KEYWORD, &pri))
        return(0);
    pri*=MS_TO_S;
    ovwm->cds.CmdPriEu(pri, &(ovwm->ses)); // takes PRI in ms

    //-------------//
    // system loss //
    //-------------//

    float system_loss;    // dB
    if (! config_list->GetFloat(SYSTEM_LOSS_KEYWORD, &system_loss))
        return(0);
    system_loss = (float)pow(10.0, 0.1*system_loss);
    ovwm->systemLoss = system_loss;

    //--------------------//
    // system temperature //
    //--------------------//

    float system_temperature;    // K
    if (! config_list->GetFloat(SYSTEM_TEMPERATURE_KEYWORD,
        &system_temperature))
    {
        return(0);
    }
    ovwm->systemTemperature = system_temperature;

    // use real aperture
    if (! config_list->GetInt(USE_REAL_APERTURE_KEYWORD, 
			      &(ovwm->useRealAperture)))
        return(0);
    
    //--------------------------------------//
    // COMMANDS TO PASS TO OTHER SUBSYSTEMS //
    //--------------------------------------//

 
    //---------------//
    // silent beams? //
    //---------------//

    config_list->DoNothingForMissingKeywords();
    int silent_beams = 0;
    config_list->GetInt(SILENT_BEAMS_KEYWORD, &silent_beams);
    config_list->ExitForMissingKeywords();

    //------------------//
    // for each beam... //
    //------------------//

    for (int beam_idx = 0; beam_idx < NUMBER_OF_OVWM_BEAMS; beam_idx++)
    {
        //------------//
        // ...silent? //
        //------------//

        if (silent_beams)
            ovwm->sas.antenna.beam[beam_idx].silentFlag = 1;

        //---------------------------------------//
        // ...set up the beam number as a string //
        //---------------------------------------//

        char number[8];
        int beam_number = beam_idx + 1;
        sprintf(number, "%d", beam_number);

        //---------------------//
        // receiver gate width //
        //---------------------//

        char keyword[1024];
        substitute_string(BEAM_x_RX_GATE_WIDTH_KEYWORD, "x", number, keyword);
        float rx_gate_width;    // ms
        if (! config_list->GetFloat(keyword, &rx_gate_width))
            return(0);
        rx_gate_width *= MS_TO_S;
        ovwm->cds.CmdRxGateWidthEu(beam_idx, rx_gate_width, &(ovwm->ses));
    }

    // compute various derived quantities
    ovwm->ses.chirpBandwidth=ovwm->ses.chirpRate*ovwm->ses.txPulseWidth;
    ovwm->ses.rangeRes=speed_light_kps/(2*ovwm->ses.chirpBandwidth)
    *ovwm->ses.numRangeLooksAveraged;
    ovwm->ses.dopplerRes=(1.0/(ovwm->ses.pri))/ovwm->ses.numPulses;
    ovwm->ses.numRangePixels=(int)ceil(ovwm->ses.maxRangeWidth/
				      ovwm->ses.rangeRes);
    if(ovwm->ses.numRangePixels%2==1) ovwm->ses.numRangePixels++;
    ovwm->ses.burstLength=ovwm->ses.pri*ovwm->ses.numPulses;
    return(1);
}



//----------------//
// ConfigOvwmSim //
//----------------//

int
ConfigOvwmSim(
    OvwmSim*    ovwm_sim,
    ConfigList*  config_list)
{

    //-----------------//
    // time references //
    //-----------------//

    if (! config_list->GetDouble(ORBIT_EPOCH_KEYWORD, &(ovwm_sim->epochTime)))
        return(0);
    ovwm_sim->epochTimeString = config_list->Get(EPOCH_TIME_STRING_KEYWORD);
    ETime tmp_time;
    tmp_time.FromCodeA(ovwm_sim->epochTimeString);
    ovwm_sim->epochOffset = tmp_time.GetTime();


    //-------------------------/
    // Latitude Bounds         /
    //-------------------------/

     config_list->DoNothingForMissingKeywords();  
     if(! config_list->GetDouble(SIM_LAT_MIN_KEYWORD,&(ovwm_sim->latMin))){
       ovwm_sim->latMin=-90.0;
     }
     if(! config_list->GetDouble(SIM_LAT_MAX_KEYWORD,&(ovwm_sim->latMax))){
       ovwm_sim->latMax=90.0;
     }
     ovwm_sim->latMin*=dtr;
     ovwm_sim->latMax*=dtr;



     if(! config_list->GetDouble(SIM_LON_MIN_KEYWORD,&(ovwm_sim->lonMin))){
       ovwm_sim->lonMin=-0.5;
     }
     if(! config_list->GetDouble(SIM_LON_MAX_KEYWORD,&(ovwm_sim->lonMax))){
       ovwm_sim->lonMax=360.5;
     }
     ovwm_sim->lonMin*=dtr;
     ovwm_sim->lonMax*=dtr;
     config_list->ExitForMissingKeywords();  

    //----------//
    // land map //
    //----------//

    int use_land_map;
    config_list->GetInt(USE_LANDMAP_KEYWORD, &use_land_map);
    if (use_land_map)
    {
        char* landfile = config_list->Get(LANDMAP_FILE_KEYWORD);
        if (! ovwm_sim->landMap.Initialize(landfile, use_land_map))
        {
            fprintf(stderr, "Cannot Initialize Land Map\n");
            return(0);
        }
        
	config_list->GetInt(SIM_COAST_KEYWORD,&(ovwm_sim->simCoast));
	char keyword[1024];
	char number[8];

	for (int beam_idx = 0; beam_idx < NUMBER_OF_OVWM_BEAMS; beam_idx++) {

	  int beam_number = beam_idx + 1;
	  sprintf(number, "%d", beam_number);

	  substitute_string(LANDSIGMA0_BEAM_x_KEYWORD, "x",
			    number, keyword);
	  config_list->GetFloat(keyword,
            &(ovwm_sim->landSigma0[beam_idx]));
	}
    }
    else
    {
        ovwm_sim->landMap.Initialize(NULL, use_land_map);
    }

    //-----------------------//
    // initialize PTGR noise //
    //-----------------------//

    float kp_ptgr, ptgr_bias, ptgr_corrlength;
    if (! config_list->GetFloat(PTGR_NOISE_KP_KEYWORD, &kp_ptgr))
    {
        fprintf(stderr,"Could not find PtGr noise variance in config file\n");
        return(0);
    }
    if (! config_list->GetFloat(PTGR_NOISE_BIAS_KEYWORD, &ptgr_bias))
    {
        fprintf(stderr,"Could not find PtGr noise mean in config file\n");
        return(0);
    }

    if (! config_list->GetFloat(PTGR_NOISE_CORRLENGTH_KEYWORD,
        &ptgr_corrlength))
    {
        fprintf(stderr,
            "Could not find PtGr noise correlation length in config file\n");
        return(0);
    }
    kp_ptgr=pow(10,0.1*kp_ptgr)-1.0;
    ovwm_sim->ptgrNoise.SetVariance(kp_ptgr*kp_ptgr);
    ptgr_bias=pow(10,0.1*ptgr_bias)-1.0;
    ovwm_sim->ptgrNoise.SetMean(ptgr_bias);
    ovwm_sim->ptgrNoise.SetCorrelationLength(ptgr_corrlength);
    ovwm_sim->ptgrNoise.SetSeed(PTGR_SEED);
    ovwm_sim->ptgrNoise.Initialize();

    int uniform_sigma_field;
    float uniform_sigma_value;

    config_list->WarnForMissingKeywords();
    if (! config_list->GetInt(UNIFORM_SIGMA_FIELD_KEYWORD,
        &uniform_sigma_field))
    {
        uniform_sigma_field=0;      // default value
    }
    ovwm_sim->uniformSigmaField=uniform_sigma_field;
    if (! config_list->GetFloat(UNIFORM_SIGMA_VALUE_KEYWORD,
        &uniform_sigma_value))
    {
        uniform_sigma_value=1.0;      // default value
    }
    ovwm_sim->uniformSigmaValue=uniform_sigma_value;

    int output_X_to_stdout;
    if (! config_list->GetInt(OUTPUT_X_TO_STDOUT_KEYWORD,
        &output_X_to_stdout))
    {
        output_X_to_stdout=0; // default value
    }
    ovwm_sim->outputXToStdout=output_X_to_stdout;

    int use_kfactor;
    if (! config_list->GetInt(USE_KFACTOR_KEYWORD, &use_kfactor))
        use_kfactor=0; // default value
    ovwm_sim->useKfactor=use_kfactor;

    int create_xtable;
    if (! config_list->GetInt(CREATE_XTABLE_KEYWORD, &create_xtable))
        create_xtable=0; // default value
    ovwm_sim->createXtable=create_xtable;

    int compute_xfactor;
    if (! config_list->GetInt(COMPUTE_XFACTOR_KEYWORD, &compute_xfactor))
        compute_xfactor=0; // default value
    ovwm_sim->computeXfactor=compute_xfactor;


    int range_gate_clipping;
    if (! config_list->GetInt(RANGE_GATE_CLIPPING_KEYWORD,
        &range_gate_clipping))
    {
        range_gate_clipping=0; // default value
    }
    ovwm_sim->rangeGateClipping=range_gate_clipping;

    int apply_doppler_error;
    if (! config_list->GetInt(APPLY_DOPPLER_ERROR_KEYWORD,
        &apply_doppler_error))
    {
        apply_doppler_error=0; // default value
    }
    ovwm_sim->applyDopplerError=apply_doppler_error;

    config_list->DoNothingForMissingKeywords();
    ovwm_sim->simVs1BCheckfile =
        config_list->Get(SIM_CHECKFILE_KEYWORD);
    // Remove any pre-existing check file
    if(ovwm_sim->simVs1BCheckfile!=NULL){
      FILE* fptr = fopen(ovwm_sim->simVs1BCheckfile,"w");
      if (fptr != NULL) fclose(fptr);
    }

    config_list->ExitForMissingKeywords();

    /****** Exactly one of these must be true ***/
    if (use_kfactor + compute_xfactor != 1)
    {
        fprintf(stderr,
            "ConfigOvwmSim:X computation incorrectly specified.\n");
        return(0);
    }
    else if (use_kfactor)
    {
        if (! ConfigXTable(&(ovwm_sim->kfactorTable),config_list,"r"))
            return(0);
    }

    if (create_xtable)
    {
        if (!ConfigXTable(&(ovwm_sim->xTable),config_list,"w"))
            return(0);
    }

    if (range_gate_clipping)
    {
        if (!compute_xfactor)
        {
            fprintf(stderr, "ConfigOvwmSim::");
            fprintf(stderr,
                "Range Gate Clipping requires computed X factor.\n");
            return(0);
        }
    }

    if(apply_doppler_error)
    {
        float doppler_bias;
        if (! config_list->GetFloat(DOPPLER_BIAS_KEYWORD, &doppler_bias))
        {
            return(0);
        }
        ovwm_sim->dopplerBias=doppler_bias*KHZ_TO_HZ;
    }

    //-------//
    // flags //
    //-------//

    int sim_land;
    if (! config_list->GetInt(SIM_LAND_FLAG_KEYWORD, &sim_land))
        return(0);
    ovwm_sim->simLandFlag = sim_land;

    int sim_kpc_flag;
    if (! config_list->GetInt(SIM_KPC_FLAG_KEYWORD, &sim_kpc_flag))
        return(0);
    ovwm_sim->simKpcFlag = sim_kpc_flag;

    int sim_corr_kpm_flag;
    if (! config_list->GetInt(SIM_CORR_KPM_FLAG_KEYWORD, &sim_corr_kpm_flag))
        return(0);
    ovwm_sim->simCorrKpmFlag = sim_corr_kpm_flag;

    int sim_uncorr_kpm_flag;
    if (! config_list->GetInt(SIM_UNCORR_KPM_FLAG_KEYWORD,&sim_uncorr_kpm_flag))
        return(0);
    ovwm_sim->simUncorrKpmFlag = sim_uncorr_kpm_flag;

    int sim_kpri_flag;
    if (! config_list->GetInt(SIM_KPRI_FLAG_KEYWORD, &sim_kpri_flag))
        return(0);
    ovwm_sim->simKpriFlag = sim_kpri_flag;

    double corr_kpmdB;
    if (! config_list->GetDouble(CORR_KPM_KEYWORD, &corr_kpmdB))
        return(0);
    // convert to real units and un-normalize
    ovwm_sim->correlatedKpm = pow(10.0, 0.1 * corr_kpmdB) - 1.0;

    // don't waste time generating zero variance rv's
    if (corr_kpmdB == 0.0)
        ovwm_sim->simCorrKpmFlag = 0;

    if (! config_list->GetInt(USE_HIGH_RES_SIM_KEYWORD,
			      &(ovwm_sim->simHiRes)))
    {
      return(0);
    }



    float integration_param; // km
    if(ovwm_sim->simHiRes){
      if (! config_list->GetFloat(INTEGRATION_STEP_SIZE_KEYWORD,
				  &integration_param))
	{
	  return(0);
	}

      ovwm_sim->integrationStepSize=integration_param;
// HACK FOR NOW until I check out the .h file
#define INTEGRATION_RANGE_WIDTH_FACTOR_KEYWORD "INTEGRATION_RANGE_WIDTH_FACTOR"
#define INTEGRATION_AZIMUTH_WIDTH_FACTOR_KEYWORD "INTEGRATION_AZIM_WIDTH_FACTOR"
#define SIM_MIN_ONEWAY_GAIN_KEYWORD "SIM_MIN_ONE_WAY_GAIN"
#define SIM_MIN_SIG_TO_AMB_RATIO_KEYWORD "SIM_MIN_SIG_TO_AMB"
      if (! config_list->GetFloat(INTEGRATION_RANGE_WIDTH_FACTOR_KEYWORD,
				  &integration_param))
	{
	  return(0);
	}

      ovwm_sim->integrationRangeWidthFactor=integration_param;

      if (! config_list->GetFloat(INTEGRATION_AZIMUTH_WIDTH_FACTOR_KEYWORD,
				  &integration_param))
	{
	  return(0);
	}

      ovwm_sim->integrationAzimuthWidthFactor=integration_param;
      ovwm_sim->AllocateIntermediateArrays();

      // Read Ambiguity and PointTargetResponse Tables
      if(!ConfigAmbigTable(&(ovwm_sim->ambigTable),config_list)) return(0);
      if(!ConfigPointTargetResponseTable(&(ovwm_sim->ptrTable),config_list))
	return(0);
    }
    float dbvalue;
    if (! config_list->GetFloat(SIM_MIN_ONEWAY_GAIN_KEYWORD,&dbvalue))
    {
      return(0);
    }
    ovwm_sim->minOneWayGain=pow(10.0,0.1*dbvalue);
    if (! config_list->GetFloat(SIM_MIN_SIG_TO_AMB_RATIO_KEYWORD,&dbvalue))
    {
      return(0);
    }
    ovwm_sim->minSignalToAmbigRatio=pow(10.0,0.1*dbvalue);


    //---------------
    //spot check keyword
    //---------------
    ovwm_sim ->spot_check_generate_map=false;// no map
    int beam_id, angle;
    if(config_list->GetInt(SIM_ONEFOOT_BEAM_KEYWORD, &beam_id) &&
       config_list->GetInt(SIM_ONEFOOT_SCAN_ANGLE_KEYWORD,&angle)){
      if(beam_id <1 || beam_id > 4 || angle<0 || angle >360)
	return(0);
      else{
	ovwm_sim->spot_check_beam_number = beam_id -1;//start from 0
	ovwm_sim->spot_check_scan_angle = angle ;
	ovwm_sim->spot_check_generate_map=true;
      }
    }

    return(1);
}

//----------------//
// AmbigTable     //
//----------------//

int ConfigAmbigTable(AmbigTable* atab,ConfigList* cfg_list){

  char* amb_index_file=cfg_list->Get(AMBIG_INDEX_FILE_KEYWORD); 
  char* amb_table_file=cfg_list->Get(AMBIG_TABLE_FILE_KEYWORD);
  if( ! atab->Read(amb_index_file, amb_table_file)) return(0);
  return(1);  
}

//------------------------------//
// PointTargetResponseTable     //
//------------------------------//

int ConfigPointTargetResponseTable(PointTargetResponseTable* ptrtab,
				    ConfigList* cfg_list){

  char keyword[1024];
  char number[8];

  for (int beam_idx = 0; beam_idx < NUMBER_OF_OVWM_BEAMS; beam_idx++) {

    int beam_number = beam_idx + 1;
    sprintf(number, "%d", beam_number);

    /* for aux file */
    substitute_string(PTRESPONSE_TABLE_AUX_FILE_BEAM_x_KEYWORD, "x",
                     number, keyword);
    char* ptrauxfile=cfg_list->Get(keyword);
    if( ! ptrtab->ReadAux(ptrauxfile, beam_number)) {
      cout << "TTT" << endl;
      return(0);
    }

    /* for data file */
    substitute_string(PTRESPONSE_TABLE_DATA_FILE_BEAM_x_KEYWORD, "x",
                     number, keyword);
    char* ptrdatafile=cfg_list->Get(keyword);
    if( ! ptrtab->ReadData(ptrdatafile, beam_number)) return(0);

  }

  return(1);  
}

//----------//
// L1A      //
//----------//

// Must be configured after OVWM object !!!!
int ConfigOvwmL1A(Ovwm* ovwm, L1A* l1a, ConfigList* config_list){
    //---------------------------//
    // configure the l1a product //
    //---------------------------//

    char* l1a_filename = config_list->Get(L1A_FILE_KEYWORD);
    if (l1a_filename == NULL)
        return(0);
    l1a->SetInputFilename(l1a_filename);
    l1a->SetOutputFilename(l1a_filename);

    int number_of_beams=NUMBER_OF_OVWM_BEAMS;
    int antenna_cycles_per_frame;
    if (! config_list->GetInt(L00_ANTENNA_CYCLES_PER_FRAME_KEYWORD,
        &antenna_cycles_per_frame))
    {
        return(0);
    }

    
    int total_pixels=ovwm->GetNumberOfPixels();

    //-------------------------//
    // configure the l1a frame //
    //-------------------------//

    if (! l1a->frame.Allocate(number_of_beams, antenna_cycles_per_frame,
        total_pixels))
    {
        return(0);
    }

    //-----------------------------------------//
    // use the frame size to allocate a buffer //
    //-----------------------------------------//

    if (! l1a->AllocateBuffer())
    {
        return(0);
    }

    
    return(1);
}
