//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_qscatconfig_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <string.h>
#include "Qscat.h"
#include "ConfigList.h"
#include "QscatConfig.h"
#include "QscatConfigDefs.h"
#include "Misc.h"
#include "QscatSim.h"
#include "ConfigSimDefs.h"
#include "Constants.h"
#include "ConfigSim.h"

//----------------//
// ConfigQscatSas //
//----------------//

int
ConfigQscatSas(
    QscatSas*    qscat_sas,
    ConfigList*  config_list)
{
    //-----------------------//
    // configure the antenna //
    //-----------------------//

    if (! ConfigAntenna(&(qscat_sas->antenna), config_list))
        return(0);

    return(1);
}

//----------------//
// ConfigQscatSes //
//----------------//

int
ConfigQscatSes(
    QscatSes*    qscat_ses,
    ConfigList*  config_list)
{
    //-------------------------//
    // base transmit frequency //
    //-------------------------//

    float base_tx_frequency;   // GHz
    if (! config_list->GetFloat(BASE_TX_FREQUENCY_KEYWORD,
        &base_tx_frequency))
    {
        return(0);
    }
    qscat_ses->baseTxFrequency = base_tx_frequency * GHZ_TO_HZ;

    //----------------//
    // transmit power //
    //----------------//

    float transmit_power;    // W
    if (! config_list->GetFloat(TRANSMIT_POWER_KEYWORD, &transmit_power))
        return(0);
    qscat_ses->transmitPower = transmit_power;

    //----------------//
    // receiver gains //
    //----------------//

    float rx_gain_echo;    // dB
    if (! config_list->GetFloat(RX_GAIN_ECHO_KEYWORD, &rx_gain_echo))
        return(0);
    qscat_ses->rxGainEcho = (float)pow(10.0, 0.1 * rx_gain_echo);

    float rx_gain_noise;    // dB
    if (! config_list->GetFloat(RX_GAIN_NOISE_KEYWORD, &rx_gain_noise))
        return(0);
    qscat_ses->rxGainNoise = (float)pow(10.0, 0.1 * rx_gain_noise);

    //----------//
    // chirping //
    //----------//

    float chirp_rate;   // kHz/ms
    if (! config_list->GetFloat(CHIRP_RATE_KEYWORD, &chirp_rate))
        return(0);
    qscat_ses->chirpRate = chirp_rate * KHZ_PER_MS_TO_HZ_PER_S;

    float chirp_start_m;    // kHz/ms
    if (! config_list->GetFloat(CHIRP_START_M_KEYWORD, &chirp_start_m))
        return(0);
    qscat_ses->chirpStartM = chirp_start_m * KHZ_PER_MS_TO_HZ_PER_S;

    float chirp_rate_b;     // kHz
    if (! config_list->GetFloat(CHIRP_START_B_KEYWORD, &chirp_rate_b))
        return(0);
    qscat_ses->chirpStartB = chirp_rate_b * KHZ_TO_HZ;

    //--------//
    // slices //
    //--------//

    float s_bw;    // KHz
    if (! config_list->GetFloat(SCIENCE_SLICE_BANDWIDTH_KEYWORD, &s_bw))
        return(0);
    qscat_ses->scienceSliceBandwidth = s_bw * KHZ_TO_HZ;

    int s_count;
    if (! config_list->GetInt(SCIENCE_SLICES_PER_SPOT_KEYWORD, &s_count))
        return(0);
    qscat_ses->scienceSlicesPerSpot = s_count;

    float g_bw;    // KHz
    if (! config_list->GetFloat(GUARD_SLICE_BANDWIDTH_KEYWORD, &g_bw))
        return(0);
    qscat_ses->guardSliceBandwidth = g_bw * KHZ_TO_HZ;

    int g_count;
    if (! config_list->GetInt(GUARD_SLICES_PER_SIDE_KEYWORD, &g_count))
        return(0);
    qscat_ses->guardSlicesPerSide = g_count;

    //-----------------//
    // noise bandwidth //
    //-----------------//

    float noise_bandwidth;    // KHz
    if (! config_list->GetFloat(NOISE_BANDWIDTH_KEYWORD, &noise_bandwidth))
        return(0);
    qscat_ses->noiseBandwidth = noise_bandwidth * KHZ_TO_HZ;

    return(1);
}

//----------------//
// ConfigQscatCds //
//----------------//

int
ConfigQscatCds(
    QscatCds*    qscat_cds,
    ConfigList*  config_list)
{
    //-------------------//
    // use tracking flag //
    //-------------------//

    int use_tracking;
    if (! config_list->GetInt(USE_TRACKING_KEYWORD, &use_tracking))
        return(0);

    //--------------//
    // orbit period //
    //--------------//

    unsigned int orbit_ticks;
    if (! config_list->GetUnsignedInt(ORBIT_TICKS_PER_ORBIT_KEYWORD,
        &orbit_ticks))
    {
        return(0);
    }
    qscat_cds->orbitTicksPerOrbit = orbit_ticks;

    //------------------//
    // for each beam... //
    //------------------//

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        //---------------------------------------//
        // ...set up the beam number as a string //
        //---------------------------------------//

        char number[8];
        int beam_number = beam_idx + 1;
        sprintf(number, "%d", beam_number);

        char keyword[1024];
        if (use_tracking)
        {
            //----------------//
            // range tracking //
            //----------------//

            substitute_string(BEAM_x_RGC_FILE_KEYWORD, "x", number, keyword);
            char* rgc_file = config_list->Get(keyword);
            if (rgc_file == NULL)
                return(0);

            if (! qscat_cds->LoadRgc(beam_idx, rgc_file))
                return(0);

            //------------------//
            // Doppler tracking //
            //------------------//

            substitute_string(BEAM_x_DTC_FILE_KEYWORD, "x", number, keyword);
            char* dtc_file = config_list->Get(keyword);
            if (dtc_file == NULL)
                return(0);

            if (! qscat_cds->LoadDtc(beam_idx, dtc_file))
                return(0);
        }
    }

    return(1);
}

//-------------//
// ConfigQscat //
//-------------//

int
ConfigQscat(
    Qscat*       qscat,
    ConfigList*  config_list)
{
    //-----//
    // SAS //
    //-----//

    if (! ConfigQscatSas(&(qscat->sas), config_list))
        return(0);

    //-----//
    // SES //
    //-----//

    if (! ConfigQscatSes(&(qscat->ses), config_list))
        return(0);

    //-----//
    // CDS //
    //-----//

    if (! ConfigQscatCds(&(qscat->cds), config_list))
        return(0);

    //----------------------//
    // transmit pulse width //
    //----------------------//

    float tx_pulse_width;
    if (! config_list->GetFloat(TX_PULSE_WIDTH_KEYWORD, &tx_pulse_width))
        return(0);
    qscat->cds.CmdTxPulseWidthEu(tx_pulse_width, &(qscat->ses));

    //-----//
    // PRI //
    //-----//

    float pri;
    if (! config_list->GetFloat(PRI_KEYWORD, &pri))
        return(0);
    qscat->cds.CmdPriEu(pri, &(qscat->ses));

    //-------------//
    // system loss //
    //-------------//

    float system_loss;    // dB
    if (! config_list->GetFloat(SYSTEM_LOSS_KEYWORD, &system_loss))
        return(0);
    system_loss = (float)pow(10.0, 0.1*system_loss);
    qscat->systemLoss = system_loss;

    //--------------------//
    // system temperature //
    //--------------------//

    float system_temperature;    // K
    if (! config_list->GetFloat(SYSTEM_TEMPERATURE_KEYWORD,
        &system_temperature))
    {
        return(0);
    }
    qscat->systemTemperature = system_temperature;

    //--------------------------------------//
    // COMMANDS TO PASS TO OTHER SUBSYSTEMS //
    //--------------------------------------//

    //-----------//
    // spin rate //
    //-----------//

    float spin_rate;
    if (! config_list->GetFloat(SPIN_RATE_KEYWORD, &spin_rate))
        return(0);
    if (spin_rate == 18.0)
    {
        qscat->cds.CmdSpinRate(LOW_SPIN_RATE, &(qscat->sas));
    }
    else if (spin_rate == 19.8)
    {
        qscat->cds.CmdSpinRate(HIGH_SPIN_RATE, &(qscat->sas));
    }
    else
    {
        fprintf(stderr, "ConfigQscat: spin rate must be 18.0 or 19.8 (%g)\n",
            spin_rate);
        return(0);
    }

    //------------------//
    // for each beam... //
    //------------------//

    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
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
        float rx_gate_width;
        if (! config_list->GetFloat(keyword, &rx_gate_width))
            return(0);
        qscat->cds.CmdRxGateWidthEu(beam_idx, rx_gate_width, &(qscat->ses));
    }

    return(1);
}

//----------------//
// ConfigQscatSim //
//----------------//

int
ConfigQscatSim(
    QscatSim*    qscat_sim,
    ConfigList*  config_list)
{
    //----------//
    // land map //
    //----------//

    char* landfile=config_list->Get(LANDMAP_FILE_KEYWORD);
    int use_land;
    config_list->GetInt(USE_LANDMAP_KEYWORD, &use_land);
    if(! qscat_sim->landMap.Initialize(landfile, use_land))
    {
        fprintf(stderr,"Cannot Initialize Land Map\n");
        exit(0);
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
    qscat_sim->ptgrNoise.SetVariance(kp_ptgr*kp_ptgr);
    ptgr_bias=pow(10,0.1*ptgr_bias)-1.0;
    qscat_sim->ptgrNoise.SetMean(ptgr_bias);
    qscat_sim->ptgrNoise.SetCorrelationLength(ptgr_corrlength);
    qscat_sim->ptgrNoise.SetSeed(PTGR_SEED);
    qscat_sim->ptgrNoise.Initialize();

    int uniform_sigma_field;

    config_list->WarnForMissingKeywords();
    if (! config_list->GetInt(UNIFORM_SIGMA_FIELD_KEYWORD,
        &uniform_sigma_field))
    {
        uniform_sigma_field=0;      // default value
    }
    qscat_sim->uniformSigmaField=uniform_sigma_field;

    int output_X_to_stdout;
    if (! config_list->GetInt(OUTPUT_X_TO_STDOUT_KEYWORD,
        &output_X_to_stdout))
    {
        output_X_to_stdout=0; // default value
    }
    qscat_sim->outputXToStdout=output_X_to_stdout;

    int use_kfactor;
    if (! config_list->GetInt(USE_KFACTOR_KEYWORD, &use_kfactor))
        use_kfactor=0; // default value
    qscat_sim->useKfactor=use_kfactor;

    int create_xtable;
    if (! config_list->GetInt(CREATE_XTABLE_KEYWORD, &create_xtable))
        create_xtable=0; // default value
    qscat_sim->createXtable=create_xtable;

    int compute_xfactor;
    if (! config_list->GetInt(COMPUTE_XFACTOR_KEYWORD, &compute_xfactor))
        compute_xfactor=0; // default value
    qscat_sim->computeXfactor=compute_xfactor;

    int use_BYU_xfactor;
    if (! config_list->GetInt(USE_BYU_XFACTOR_KEYWORD, &use_BYU_xfactor))
        use_BYU_xfactor=0; // default value
    qscat_sim->useBYUXfactor=use_BYU_xfactor;

    int range_gate_clipping;
    if (! config_list->GetInt(RANGE_GATE_CLIPPING_KEYWORD,
        &range_gate_clipping))
    {
        range_gate_clipping=0; // default value
    }
    qscat_sim->rangeGateClipping=range_gate_clipping;

    int apply_doppler_error;
    if (! config_list->GetInt(APPLY_DOPPLER_ERROR_KEYWORD,
        &apply_doppler_error))
    {
        apply_doppler_error=0; // default value
    }
    qscat_sim->applyDopplerError=apply_doppler_error;

    config_list->DoNothingForMissingKeywords();
    qscat_sim->simVs1BCheckfile =
        config_list->Get(SIM_CHECKFILE_KEYWORD);

    config_list->ExitForMissingKeywords();

    /****** Exactly one of these must be true ***/
    if (use_kfactor + compute_xfactor + use_BYU_xfactor != 1)
    {
        fprintf(stderr,
            "ConfigQscatSim:X computation incorrectly specified.\n");
        return(0);
    }
    if (compute_xfactor)
    {
        int num_look_steps;
        if (! config_list->GetInt(NUM_LOOK_STEPS_KEYWORD, &num_look_steps))
            return(0);
        qscat_sim->numLookStepsPerSlice=num_look_steps;

        float azimuth_integration_range;
        if (! config_list->GetFloat(AZIMUTH_INTEGRATION_RANGE_KEYWORD,
            &azimuth_integration_range))
        {
            return(0);
        }
        qscat_sim->azimuthIntegrationRange=azimuth_integration_range*dtr;

        float azimuth_step_size;
        if (! config_list->GetFloat(AZIMUTH_STEP_SIZE_KEYWORD,
            &azimuth_step_size))
        {
            return(0);
        }
        qscat_sim->azimuthStepSize=azimuth_step_size*dtr;
    }
    else if (use_kfactor)
    {
        if (! ConfigXTable(&(qscat_sim->kfactorTable),config_list,"r"))
            return(0);
    }
    else if (use_BYU_xfactor)
    {
        if (!ConfigBYUXTable(&(qscat_sim->BYUX),config_list))
            return(0);
    }

    if (create_xtable)
    {
        if (!ConfigXTable(&(qscat_sim->xTable),config_list,"w"))
            return(0);
    }

    if (range_gate_clipping)
    {
        if (!compute_xfactor)
        {
            fprintf(stderr, "ConfigQscatSim::");
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
        qscat_sim->dopplerBias=doppler_bias*KHZ_TO_HZ;
    }

    //-------//
    // flags //
    //-------//

    int sim_kpc_flag;
    if (! config_list->GetInt(SIM_KPC_FLAG_KEYWORD, &sim_kpc_flag))
        return(0);
    qscat_sim->simKpcFlag = sim_kpc_flag;

    int sim_corr_kpm_flag;
    if (! config_list->GetInt(SIM_CORR_KPM_FLAG_KEYWORD, &sim_corr_kpm_flag))
        return(0);
    qscat_sim->simCorrKpmFlag = sim_corr_kpm_flag;

    int sim_uncorr_kpm_flag;
    if (! config_list->GetInt(SIM_UNCORR_KPM_FLAG_KEYWORD,&sim_uncorr_kpm_flag))
        return(0);
    qscat_sim->simUncorrKpmFlag = sim_uncorr_kpm_flag;

    int sim_kpri_flag;
    if (! config_list->GetInt(SIM_KPRI_FLAG_KEYWORD, &sim_kpri_flag))
        return(0);
    qscat_sim->simKpriFlag = sim_kpri_flag;

    double corr_kpmdB;
    if (! config_list->GetDouble(CORR_KPM_KEYWORD, &corr_kpmdB))
        return(0);
    // convert to real units and un-normalize
    qscat_sim->correlatedKpm = pow(10.0, 0.1 * corr_kpmdB) - 1.0;

    // don't waste time generating zero variance rv's
    if (corr_kpmdB == 0.0)
        qscat_sim->simCorrKpmFlag = 0;

    return(1);
}

//------------------------//
// ConfigQscatSimAccurate //
//------------------------//

int
ConfigQscatSimAccurate(
    QscatSimAccurate*  qscat_sim,
    ConfigList*        config_list)
{
    if (! ConfigQscatSim(qscat_sim, config_list))
        return(0);
    int num_look_steps;
    if (! config_list->GetInt(NUM_LOOK_STEPS_KEYWORD, &num_look_steps))
        return(0);
    qscat_sim->numLookStepsPerSlice=num_look_steps;

    float azimuth_integration_range;
    if (! config_list->GetFloat(AZIMUTH_INTEGRATION_RANGE_KEYWORD,
            &azimuth_integration_range))
    {
        return(0);
    }
    qscat_sim->azimuthIntegrationRange=azimuth_integration_range*dtr;

    float azimuth_step_size;
    if (! config_list->GetFloat(AZIMUTH_STEP_SIZE_KEYWORD,
            &azimuth_step_size))
    {
        return(0);
    }
    qscat_sim->azimuthStepSize=azimuth_step_size*dtr;

    return(1);
}