//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_pscatconfig_c[] =
    "@(#) $Id$";

#include "PscatConfig.h"
#include "QscatConfig.h"
#include "ConfigSimDefs.h"
#include "PscatConfigDefs.h"
#include "ConfigSim.h"

//-------------//
// ConfigPscat //
//-------------//

int
ConfigPscat(
    Pscat*       pscat,
    ConfigList*  config_list)
{
    //------------------//
    // QSCAT base class //
    //------------------//

    if (! ConfigQscat(pscat, config_list))
        return(0);

    return(1);
}

//----------------//
// ConfigPscatSim //
//----------------//

int
ConfigPscatSim(
    PscatSim*    pscat_sim,
    ConfigList*  config_list)
{
    //-----------------//
    // time references //
    //-----------------//

    if (! config_list->GetDouble(ORBIT_EPOCH_KEYWORD, &(pscat_sim->epochTime)))
        return(0);
    pscat_sim->epochTimeString = config_list->Get(EPOCH_TIME_STRING_KEYWORD);

    //----------//
    // land map //
    //----------//

    int use_land_map;
    config_list->GetInt(USE_LANDMAP_KEYWORD, &use_land_map);
    if (use_land_map)
    {
        char* landfile = config_list->Get(LANDMAP_FILE_KEYWORD);
        if (! pscat_sim->landMap.Initialize(landfile, use_land_map))
        {
            fprintf(stderr, "Cannot Initialize Land Map\n");
            return(0);
        }
    }
    else
    {
        pscat_sim->landMap.Initialize(NULL, use_land_map);
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
    kp_ptgr = pow(10, 0.1*kp_ptgr) - 1.0;
    pscat_sim->ptgrNoise.SetVariance(kp_ptgr*kp_ptgr);
    ptgr_bias = pow(10, 0.1*ptgr_bias) - 1.0;
    pscat_sim->ptgrNoise.SetMean(ptgr_bias);
    pscat_sim->ptgrNoise.SetCorrelationLength(ptgr_corrlength);
    pscat_sim->ptgrNoise.SetSeed(PTGR_SEED);
    pscat_sim->ptgrNoise.Initialize();

    int uniform_sigma_field;

    config_list->WarnForMissingKeywords();
    if (! config_list->GetInt(UNIFORM_SIGMA_FIELD_KEYWORD,
        &uniform_sigma_field))
    {
        uniform_sigma_field=0;      // default value
    }
    pscat_sim->uniformSigmaField = uniform_sigma_field;

    int output_X_to_stdout;
    if (! config_list->GetInt(OUTPUT_X_TO_STDOUT_KEYWORD,
        &output_X_to_stdout))
    {
        output_X_to_stdout = 0; // default value
    }
    pscat_sim->outputXToStdout = output_X_to_stdout;

    int use_kfactor;
    if (! config_list->GetInt(USE_KFACTOR_KEYWORD, &use_kfactor))
        use_kfactor = 0; // default value
    pscat_sim->useKfactor = use_kfactor;

    int create_xtable;
    if (! config_list->GetInt(CREATE_XTABLE_KEYWORD, &create_xtable))
        create_xtable = 0; // default value
    pscat_sim->createXtable = create_xtable;

    int compute_xfactor;
    if (! config_list->GetInt(COMPUTE_XFACTOR_KEYWORD, &compute_xfactor))
        compute_xfactor = 0; // default value
    pscat_sim->computeXfactor = compute_xfactor;

    int use_BYU_xfactor;
    if (! config_list->GetInt(USE_BYU_XFACTOR_KEYWORD, &use_BYU_xfactor))
        use_BYU_xfactor = 0; // default value
    pscat_sim->useBYUXfactor = use_BYU_xfactor;

    int range_gate_clipping;
    if (! config_list->GetInt(RANGE_GATE_CLIPPING_KEYWORD,
        &range_gate_clipping))
    {
        range_gate_clipping = 0; // default value
    }
    pscat_sim->rangeGateClipping = range_gate_clipping;

    int apply_doppler_error;
    if (! config_list->GetInt(APPLY_DOPPLER_ERROR_KEYWORD,
        &apply_doppler_error))
    {
        apply_doppler_error = 0; // default value
    }

    config_list->DoNothingForMissingKeywords();
    pscat_sim->simVs1BCheckfile =
        config_list->Get(SIM_CHECKFILE_KEYWORD);
    // Remove any pre-existing check file
    if (pscat_sim->simVs1BCheckfile != NULL)
    {
      FILE* fptr = fopen(pscat_sim->simVs1BCheckfile,"w");
      if (fptr != NULL) fclose(fptr);
    }
    config_list->ExitForMissingKeywords();

    /****** Exactly one of these must be true ***/
    if (use_kfactor + compute_xfactor + use_BYU_xfactor != 1)
    {
        fprintf(stderr,
            "ConfigPscatSim:X computation incorrectly specified.\n");
        return(0);
    }
    if (compute_xfactor)
    {
        int num_look_steps;
        if (! config_list->GetInt(NUM_LOOK_STEPS_KEYWORD, &num_look_steps))
            return(0);
        pscat_sim->numLookStepsPerSlice = num_look_steps;

        float azimuth_integration_range;
        if (! config_list->GetFloat(AZIMUTH_INTEGRATION_RANGE_KEYWORD,
            &azimuth_integration_range))
        {
            return(0);
        }
        pscat_sim->azimuthIntegrationRange = azimuth_integration_range*dtr;

        float azimuth_step_size;
        if (! config_list->GetFloat(AZIMUTH_STEP_SIZE_KEYWORD,
            &azimuth_step_size))
        {
            return(0);
        }
        pscat_sim->azimuthStepSize = azimuth_step_size*dtr;
    }
    else if (use_kfactor)
    {
        if (! ConfigXTable(&(pscat_sim->kfactorTable), config_list,"r"))
            return(0);
    }
    else if (use_BYU_xfactor)
    {
        if (!ConfigBYUXTable(&(pscat_sim->BYUX), config_list))
            return(0);
    }

    if (create_xtable)
    {
        if (!ConfigXTable(&(pscat_sim->xTable), config_list,"w"))
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
        pscat_sim->dopplerBias = doppler_bias*KHZ_TO_HZ;
    }

    //-------//
    // flags //
    //-------//

    int sim_kpc_flag;
    if (! config_list->GetInt(SIM_KPC_FLAG_KEYWORD, &sim_kpc_flag))
        return(0);
    pscat_sim->simKpcFlag = sim_kpc_flag;

    int sim_corr_kpm_flag;
    if (! config_list->GetInt(SIM_CORR_KPM_FLAG_KEYWORD, &sim_corr_kpm_flag))
        return(0);
    pscat_sim->simCorrKpmFlag = sim_corr_kpm_flag;

    int sim_uncorr_kpm_flag;
    if (! config_list->GetInt(SIM_UNCORR_KPM_FLAG_KEYWORD,&sim_uncorr_kpm_flag))
        return(0);
    pscat_sim->simUncorrKpmFlag = sim_uncorr_kpm_flag;

    int sim_kpri_flag;
    if (! config_list->GetInt(SIM_KPRI_FLAG_KEYWORD, &sim_kpri_flag))
        return(0);
    pscat_sim->simKpriFlag = sim_kpri_flag;

    double corr_kpmdB;
    if (! config_list->GetDouble(CORR_KPM_KEYWORD, &corr_kpmdB))
        return(0);
    // convert to real units and un-normalize
    pscat_sim->correlatedKpm = pow(10.0, 0.1 * corr_kpmdB) - 1.0;

    // don't waste time generating zero variance rv's
    if (corr_kpmdB == 0.0)
        pscat_sim->simCorrKpmFlag = 0;

    return(1);
}

//----------------//
// ConfigPscatL1A //
//----------------//

int
ConfigPscatL1A(
    PscatL1A*    pscat_l1a,
    ConfigList*  config_list)
{
    //---------------------------------//
    // configure the PSCAT L1A product //
    //---------------------------------//

    char* l1a_filename = config_list->Get(L1A_FILE_KEYWORD);
    if (l1a_filename == NULL)
        return(0);
    pscat_l1a->SetInputFilename(l1a_filename);
    pscat_l1a->SetOutputFilename(l1a_filename);

    int number_of_beams;
    if (! config_list->GetInt(NUMBER_OF_BEAMS_KEYWORD, &number_of_beams))
        return(0);

    int antenna_cycles_per_frame;
    if (! config_list->GetInt(L00_ANTENNA_CYCLES_PER_FRAME_KEYWORD,
        &antenna_cycles_per_frame))
    {
        return(0);
    }

    int s_count;
    if (! config_list->GetInt(SCIENCE_SLICES_PER_SPOT_KEYWORD, &s_count))
        return(0);

    int g_count;
    if (! config_list->GetInt(GUARD_SLICES_PER_SIDE_KEYWORD, &g_count))
        return(0);

    int total_slices = s_count + 2 * g_count;

    //-------------------------//
    // configure the l1a frame //
    //-------------------------//

    if (! pscat_l1a->frame.Allocate(number_of_beams, antenna_cycles_per_frame,
        total_slices))
    {
        return(0);
    }

    //-----------------------------------------//
    // use the frame size to allocate a buffer //
    //-----------------------------------------//

    if (! pscat_l1a->AllocateBuffer())
    {
        return(0);
    }

    return(1);
}

//---------------------//
// ConfigPscatL1AToL1B //
//---------------------//

int
ConfigPscatL1AToL1B(
    PscatL1AToL1B*  l1a_to_l1b,
    ConfigList*     config_list)
{
    //-------------------------//
    // output simga0 to stdout //
    //-------------------------//

    config_list->DoNothingForMissingKeywords();
    int output_sigma0_to_stdout;
    if (! config_list->GetInt(OUTPUT_SIGMA0_TO_STDOUT_KEYWORD,
        &output_sigma0_to_stdout))
    {
        output_sigma0_to_stdout = 0;    // default value
    }
    l1a_to_l1b->outputSigma0ToStdout = output_sigma0_to_stdout;

    l1a_to_l1b->simVs1BCheckfile = config_list->Get(ONEB_CHECKFILE_KEYWORD);
    // Remove any pre-existing check file
    FILE* fptr = fopen(l1a_to_l1b->simVs1BCheckfile,"w");
    if (fptr != NULL)
        fclose(fptr);

    config_list->ExitForMissingKeywords();

    //--------------------------------------//
    // Read in the land map file            //
    //--------------------------------------//

    char* landfile=config_list->Get(LANDMAP_FILE_KEYWORD);
    int use_land;
    config_list->GetInt(USE_LANDMAP_KEYWORD, &use_land);
    if (! l1a_to_l1b->landMap.Initialize(landfile,use_land))
    {
        fprintf(stderr,"Cannot Initialize Land Map\n");
        exit(0);
    }

    //-------------------//
    // k-factor/x-factor //
    //-------------------//

    int use_kfactor;
    if (! config_list->GetInt(USE_KFACTOR_KEYWORD, &use_kfactor))
        use_kfactor = 0;    // default value
    l1a_to_l1b->useKfactor = use_kfactor;

    int use_BYU_xfactor;
    if (! config_list->GetInt(USE_BYU_XFACTOR_KEYWORD, &use_BYU_xfactor))
        use_BYU_xfactor = 0;    // default value
    l1a_to_l1b->useBYUXfactor = use_BYU_xfactor;

    /****** Exactly one of these must be true ***/
    if (use_kfactor + use_BYU_xfactor != 1)
    {
        fprintf(stderr,
            "ConfigL1AToL1B:X computation incorrectly specified.\n");
        return(0);
    }
    if (use_kfactor)
    {
        if (! ConfigXTable(&(l1a_to_l1b->kfactorTable),config_list,"r"))
            return(0);
    }
    else if (use_BYU_xfactor)
    {
        if (! ConfigBYUXTable(&(l1a_to_l1b->BYUX), config_list))
            return(0);
    }

    //------------------//
    // spot compositing //
    //------------------//

    int spot_comp;
    if (! config_list->GetInt(USE_SPOT_COMPOSITES_KEYWORD, &spot_comp))
        spot_comp = 0;
    l1a_to_l1b->useSpotCompositing = spot_comp;

    //---------------------------//
    // gain threshold for slices //
    //---------------------------//

    float gain;
    if (! config_list->GetFloat(SLICE_GAIN_THRESHOLD_KEYWORD, &gain))
        return(0);
    l1a_to_l1b->sliceGainThreshold = pow(10.0, 0.1 * gain);

    //-----------------------------------------------//
    // maximum number of slices to use in processing //
    //-----------------------------------------------//

    int max_slices;
    if (! config_list->GetInt(PROCESS_MAX_SLICES_KEYWORD, &max_slices))
        return(0);
    l1a_to_l1b->processMaxSlices = max_slices;

    return(1);
}
