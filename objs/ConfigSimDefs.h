//==============================================================//
// Copyright (C) 1997-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef CONFIGSIMDEFS_H
#define CONFIGSIMDEFS_H

static const char rcs_id_configsimdefs_h[] =
    "@(#) $Id$";

//------------//
// Spacecraft //
//------------//

#define ORBIT_EPOCH_KEYWORD            "ORBIT_EPOCH"
#define SEMI_MAJOR_AXIS_KEYWORD        "SEMI_MAJOR_AXIS"
#define ECCENTRICITY_KEYWORD           "ECCENTRICITY"
#define INCLINATION_KEYWORD            "INCLINATION"
#define LONG_OF_ASC_NODE_KEYWORD       "LONG_OF_ASC_NODE"
#define ARGUMENT_OF_PERIGEE_KEYWORD    "ARGUMENT_OF_PERIGEE"
#define MEAN_ANOMALY_AT_EPOCH_KEYWORD  "MEAN_ANOMALY_AT_EPOCH"
#define ATTITUDE_ORDER_1_KEYWORD       "ATTITUDE_ORDER_1"
#define ATTITUDE_ORDER_2_KEYWORD       "ATTITUDE_ORDER_2"
#define ATTITUDE_ORDER_3_KEYWORD       "ATTITUDE_ORDER_3"
#define EPHEMERIS_PERIOD_KEYWORD       "EPHEMERIS_PERIOD"
#define SIM_KPRS_FLAG_KEYWORD          "SIM_KPRS_FLAG"

//------------------------------//
// Attitude Control Error Model //
//------------------------------//

#define ATTITUDE_CONTROL_MODEL_KEYWORD    "ATTITUDE_CONTROL_MODEL"

#define ROLL_CONTROL_MEAN_KEYWORD         "ROLL_CONTROL_MEAN"
#define PITCH_CONTROL_MEAN_KEYWORD        "PITCH_CONTROL_MEAN"
#define YAW_CONTROL_MEAN_KEYWORD          "YAW_CONTROL_MEAN"

#define ROLL_CONTROL_STD_KEYWORD          "ROLL_CONTROL_STD"
#define PITCH_CONTROL_STD_KEYWORD         "PITCH_CONTROL_STD"
#define YAW_CONTROL_STD_KEYWORD           "YAW_CONTROL_STD"

#define ROLL_CONTROL_CORRLENGTH_KEYWORD   "ROLL_CONTROL_CORRLENGTH"
#define PITCH_CONTROL_CORRLENGTH_KEYWORD  "PITCH_CONTROL_CORRLENGTH"
#define YAW_CONTROL_CORRLENGTH_KEYWORD    "YAW_CONTROL_CORRLENGTH"

#define SIM_ROLL_BIAS_KEYWORD             "SIM_ROLL_BIAS"
#define SIM_PITCH_BIAS_KEYWORD            "SIM_PITCH_BIAS"
#define SIM_YAW_BIAS_KEYWORD              "SIM_YAW_BIAS"

#define ROLL_CONTROL_SEED_KEYWORD         "ROLL_CONTROL_SEED"
#define PITCH_CONTROL_SEED_KEYWORD        "PITCH_CONTROL_SEED"
#define YAW_CONTROL_SEED_KEYWORD          "YAW_CONTROL_SEED"

//--------------------------------//
// Attitude Knowledge Error Model //
//--------------------------------//

#define ATTITUDE_KNOWLEDGE_MODEL_KEYWORD    "ATTITUDE_KNOWLEDGE_MODEL"

#define ROLL_KNOWLEDGE_MEAN_KEYWORD         "ROLL_KNOWLEDGE_MEAN"
#define PITCH_KNOWLEDGE_MEAN_KEYWORD        "PITCH_KNOWLEDGE_MEAN"
#define YAW_KNOWLEDGE_MEAN_KEYWORD          "YAW_KNOWLEDGE_MEAN"

#define ROLL_KNOWLEDGE_STD_KEYWORD          "ROLL_KNOWLEDGE_STD"
#define PITCH_KNOWLEDGE_STD_KEYWORD         "PITCH_KNOWLEDGE_STD"
#define YAW_KNOWLEDGE_STD_KEYWORD           "YAW_KNOWLEDGE_STD"

#define ROLL_KNOWLEDGE_CORRLENGTH_KEYWORD   "ROLL_KNOWLEDGE_CORRLENGTH"
#define PITCH_KNOWLEDGE_CORRLENGTH_KEYWORD  "PITCH_KNOWLEDGE_CORRLENGTH"
#define YAW_KNOWLEDGE_CORRLENGTH_KEYWORD    "YAW_KNOWLEDGE_CORRLENGTH"

#define ROLL_KNOWLEDGE_SEED_KEYWORD         "ROLL_KNOWLEDGE_SEED"
#define PITCH_KNOWLEDGE_SEED_KEYWORD        "PITCH_KNOWLEDGE_SEED"
#define YAW_KNOWLEDGE_SEED_KEYWORD          "YAW_KNOWLEDGE_SEED"

//-----------------------//
// Kprc Noise Model      //
//-----------------------//

#define SIM_KPRC_KEYWORD      "SIM_KPRC"
#define KPRC_VALUE_KEYWORD    "KPRC_VALUE"
#define KPRC_SEED_KEYWORD      "KPRC_SEED"

//-------//
// seeds //
//-------//

// setting this non-zero causes the control and knowledge seeds to be
// randomly set
#define RANDOMIZE_SEEDS_KEYWORD  "RANDOMIZE_SEEDS"

//------------//
// Instrument //
//------------//

#define CHIRP_RATE_KEYWORD               "CHIRP_RATE"
#define SYSTEM_TEMPERATURE_KEYWORD       "SYSTEM_TEMPERATURE"
#define SCIENCE_SLICES_PER_SPOT_KEYWORD  "SCIENCE_SLICES_PER_SPOT"
#define SCIENCE_SLICE_BANDWIDTH_KEYWORD  "SCIENCE_SLICE_BANDWIDTH"
#define GUARD_SLICES_PER_SIDE_KEYWORD    "GUARD_SLICES_PER_SIDE"
#define GUARD_SLICE_BANDWIDTH_KEYWORD    "GUARD_SLICE_BANDWIDTH"
#define NOISE_BANDWIDTH_KEYWORD          "NOISE_BANDWIDTH"
#define TRANSMIT_POWER_KEYWORD           "TRANSMIT_POWER"
#define SYSTEM_LOSS_KEYWORD              "SYSTEM_LOSS"
#define PTGR_NOISE_KP_KEYWORD            "PTGR_NOISE_KP"
#define PTGR_NOISE_BIAS_KEYWORD          "PTGR_BIAS"
#define PTGR_NOISE_CORRLENGTH_KEYWORD    "PTGR_NOISE_CORRLENGTH"
#define UNIFORM_SIGMA_FIELD_KEYWORD      "UNIFORM_SIGMA_FIELD"
#define UNIFORM_SIGMA_VALUE_KEYWORD      "UNIFORM_SIGMA_VALUE"
#define OUTPUT_X_TO_STDOUT_KEYWORD       "OUTPUT_X_TO_STDOUT"
#define USE_KFACTOR_KEYWORD              "USE_KFACTOR"
#define COMPUTE_XFACTOR_KEYWORD          "COMPUTE_XFACTOR"
#define RANGE_GATE_CLIPPING_KEYWORD      "RANGE_GATE_CLIPPING"
#define APPLY_DOPPLER_ERROR_KEYWORD      "APPLY_DOPPLER_ERROR"
#define DOPPLER_BIAS_KEYWORD             "DOPPLER_BIAS"
#define CREATE_XTABLE_KEYWORD            "CREATE_XTABLE"
#define ORBIT_TICKS_PER_ORBIT_KEYWORD    "ORBIT_TICKS_PER_ORBIT"
#define CORR_KPM_KEYWORD                 "CORR_KPM"

#define SIM_KPC_FLAG_KEYWORD         "SIM_KPC_FLAG"
#define SIM_CORR_KPM_FLAG_KEYWORD    "SIM_CORR_KPM_FLAG"
#define SIM_UNCORR_KPM_FLAG_KEYWORD  "SIM_UNCORR_KPM_FLAG"
#define SIM_KPRI_FLAG_KEYWORD        "SIM_KPRI_FLAG"

#define TX_PULSE_WIDTH_KEYWORD       "TX_PULSE_WIDTH"

//--------------//
// BYU X factor //
//--------------//

#define USE_BYU_XFACTOR_KEYWORD          "USE_BYU_XFACTOR"
#define XFACTOR_INNER_BEAM_FILE_KEYWORD  "XFACTOR_INNER_BEAM_FILE"
#define XFACTOR_OUTER_BEAM_FILE_KEYWORD  "XFACTOR_OUTER_BEAM_FILE"

//----------------//
// Topography map //
//----------------//

#define USE_TOPOMAP_KEYWORD     "USE_TOPOMAP"
#define TOPOMAP_FILE_KEYWORD    "TOPOMAP_FILE"
#define STABLE_FILE_KEYWORD     "STABLE_FILE"
#define STABLE_MODE_ID_KEYWORD  "STABLE_MODE_ID"

//-----------//
// Fbb Table //
//-----------//

#define FBB_INNER_BEAM_FILE_KEYWORD  "FBB_INNER_BEAM_FILE"
#define FBB_OUTER_BEAM_FILE_KEYWORD  "FBB_OUTER_BEAM_FILE"

//---------//
// LandMap //
//---------//

#define LANDMAP_FILE_KEYWORD            "LANDMAP_FILE"
#define USE_LANDMAP_KEYWORD             "USE_LANDMAP"
#define LAND_SIGMA0_INNER_BEAM_KEYWORD  "LAND_SIGMA0_INNER_BEAM"  // linear
#define LAND_SIGMA0_OUTER_BEAM_KEYWORD  "LAND_SIGMA0_OUTER_BEAM"  // linear

// this sort of doesn't belong here...
#define SIMPLE_LANDMAP_FILE_KEYWORD     "SIMPLE_LANDMAP_FILE"

//---------------//
// InstrumentSim //
//---------------//

#define SIM_CHECKFILE_KEYWORD  "SIM_CHECKFILE"

//-----------------------//
// InstrumentSimAccurate //
//-----------------------//

#define NUM_LOOK_STEPS_KEYWORD           "NUM_INTEGRATION_LOOK_STEPS_PER_SLICE"
#define AZIMUTH_INTEGRATION_RANGE_KEYWORD  "AZIMUTH_INTEGRATION_RANGE"
#define AZIMUTH_STEP_SIZE_KEYWORD          "AZIMUTH_INTEGRATION_STEP_SIZE"

//---------//
// Antenna //
//---------//

#define NUMBER_OF_BEAMS_KEYWORD         "NUMBER_OF_BEAMS"
#define ANTENNA_PEDESTAL_ROLL_KEYWORD   "ANTENNA_PEDESTAL_ROLL"
#define ANTENNA_PEDESTAL_PITCH_KEYWORD  "ANTENNA_PEDESTAL_PITCH"
#define ANTENNA_PEDESTAL_YAW_KEYWORD    "ANTENNA_PEDESTAL_YAW"
#define ANTENNA_START_AZIMUTH_KEYWORD   "ANTENNA_START_AZIMUTH"
#define ANTENNA_START_TIME_KEYWORD      "ANTENNA_START_TIME"

//------//
// Beam //
//------//

#define USE_RGC_KEYWORD                "USE_RGC"
#define USE_DTC_KEYWORD                "USE_DTC"
#define MECH_LOOK_ANGLE_KEYWORD        "MECH_LOOK_ANGLE"
#define MECH_AZIMUTH_ANGLE_KEYWORD     "MECH_AZIMUTH_ANGLE"
#define SILENT_BEAMS_KEYWORD           "SILENT_BEAMS"
#define BEAM_x_PEAK_GAIN_KEYWORD       "BEAM_x_PEAK_GAIN"
#define BEAM_x_WAVEGUIDE_LOSS_KEYWORD  "BEAM_x_WAVEGUIDE_LOSS"
#define BEAM_x_LOOK_ANGLE_KEYWORD      "BEAM_x_LOOK_ANGLE"
#define BEAM_x_AZIMUTH_ANGLE_KEYWORD   "BEAM_x_AZIMUTH_ANGLE"
#define BEAM_x_POLARIZATION_KEYWORD    "BEAM_x_POLARIZATION"
#define BEAM_x_PATTERN_FILE_KEYWORD    "BEAM_x_PATTERN_FILE"
#define BEAM_x_RGC_FILE_KEYWORD        "BEAM_x_RGC_FILE"
#define BEAM_x_DTC_FILE_KEYWORD        "BEAM_x_DTC_FILE"

//--------//
// XTable //
//--------//

#define XTABLE_FILENAME_KEYWORD         "XTABLE_FILENAME"
#define XTABLE_NUM_AZIMUTHS_KEYWORD     "XTABLE_NUM_AZIMUTHS"
#define XTABLE_NUM_ORBIT_STEPS_KEYWORD  "XTABLE_NUM_ORBIT_STEPS"

//-----//
// L00 //
//-----//

#define L00_FILE_KEYWORD                      "L00_FILE"
#define L00_ANTENNA_CYCLES_PER_FRAME_KEYWORD  "L00_ANTENNA_CYCLES_PER_FRAME"

//-----//
// L1A //
//-----//

#define L1A_FILE_KEYWORD         "L1A_FILE"
#define L1A_FILE_FORMAT_KEYWORD  "L1A_FILE_FORMAT"
#define L1A_HDF_FILE_KEYWORD     "L1A_HDF_FILE"

//-----//
// L1B //
//-----//

#define L1B_FILE_KEYWORD         "L1B_FILE"
#define L1B_HDF_FILE_KEYWORD     "L1B_HDF_FILE"
#define L1B_FILE_FORMAT_KEYWORD  "L1B_FILE_FORMAT"

//----------//
// L1AToL1B //
//----------//

#define OUTPUT_SIGMA0_TO_STDOUT_KEYWORD  "OUTPUT_SIGMA0_TO_STDOUT"
#define USE_SPOT_COMPOSITES_KEYWORD      "USE_SPOT_COMPOSITES"
#define SLICE_GAIN_THRESHOLD_KEYWORD     "SLICE_GAIN_THRESHOLD"
#define PROCESS_MAX_SLICES_KEYWORD       "PROCESS_MAX_SLICES"
#define ONEB_CHECKFILE_KEYWORD           "ONEB_CHECKFILE"

//-----//
// L2A //
//-----//

#define L2A_FILE_KEYWORD      "L2A_FILE"
#define L2A_HDF_FILE_KEYWORD  "L2A_HDF_FILE"

//-----//
// L2B //
//-----//

#define L2B_FILE_KEYWORD      "L2B_FILE"
#define L2B_HDF_FILE_KEYWORD  "L2B_HDF_FILE"

//----------//
// L2AToL2B //
//----------//

#define MEDIAN_FILTER_WINDOW_SIZE_KEYWORD      "MEDIAN_FILTER_WINDOW_SIZE"
#define MEDIAN_FILTER_MAX_PASSES_KEYWORD       "MEDIAN_FILTER_MAX_PASSES"
#define MAX_RANK_FOR_NUDGING_KEYWORD           "MAX_RANK_FOR_NUDGING"
#define WIND_RETRIEVAL_METHOD_KEYWORD          "WIND_RETRIEVAL_METHOD"
#define REQUIRED_AZIMUTH_DIVERSITY_KEYWORD     "REQUIRED_AZIMUTH_DIVERSITY"
#define USE_MANY_AMBIGUITIES_KEYWORD           "USE_MANY_AMBIGUITIES"
#define USE_AMBIGUITY_WEIGHTS_KEYWORD          "USE_AMBIGUITY_WEIGHTS"
#define USE_NUDGING_KEYWORD                    "USE_NUDGING"
#define USE_STREAM_NUDGING_KEYWORD             "USE_STREAM_NUDGING"
#define STREAM_THRESHOLD_KEYWORD               "STREAM_THRESHOLD"
#define SMART_NUDGE_FLAG_KEYWORD               "SMART_NUDGE_FLAG"
#define NUDGE_WINDFIELD_FILE_KEYWORD           "NUDGE_WINDFIELD_FILE"
#define NUDGE_WINDFIELD_TYPE_KEYWORD           "NUDGE_WINDFIELD_TYPE"
#define USE_HURRICANE_NUDGE_KEYWORD            "USE_HURRICANE_NUDGE"
#define HURRICANE_WINDFIELD_FILE_KEYWORD       "HURRICANE_WINDFIELD_FILE"
#define HURRICANE_RADIUS_KEYWORD               "HURRICANE_RADIUS"
#define HURRICANE_CENTER_LATITUDE_KEYWORD      "HURRICANE_CENTER_LATITUDE"
#define HURRICANE_CENTER_LONGITUDE_KEYWORD     "HURRICANE_CENTER_LONGITUDE"
#define ONE_PEAK_WIDTH_KEYWORD                 "ONE_PEAK_WIDTH"
#define TWO_PEAK_SEPARATION_THRESHOLD_KEYWORD  "TWO_PEAK_SEPARATION_THRESHOLD"
#define SCALED_PROBABILITY_THRESHOLD_KEYWORD   "SCALED_PROBABILITY_THRESHOLD"
#define USE_NUDGING_THRESHOLD_KEYWORD          "USE_NUDGING_THRESHOLD"
#define NEAR_SWATH_NUDGE_THRESHOLD_KEYWORD     "NEAR_SWATH_NUDGE_THRESHOLD"
#define FAR_SWATH_NUDGE_THRESHOLD_KEYWORD      "FAR_SWATH_NUDGE_THRESHOLD"
#define USE_NARROW_MEDIAN_FILTER_KEYWORD       "USE_NARROW_MEDIAN_FILTER"
#define USE_RANDOM_RANK_INIT_KEYWORD           "USE_RANDOM_RANK_INIT"

//------------------//
// Spacecraft State //
//------------------//

#define EPHEMERIS_FILE_KEYWORD  "EPHEMERIS_FILE"
#define ATTITUDE_FILE_KEYWORD   "ATTITUDE_FILE"

//-----------//
// WindField //
//-----------//

#define WINDFIELD_TYPE_KEYWORD         "WINDFIELD_TYPE"
#define WINDFIELD_FILE_KEYWORD         "WINDFIELD_FILE"
#define WINDFIELD_FIXED_SPEED_KEYWORD  "WINDFIELD_FIXED_SPEED"
#define WINDFIELD_SPEED_MULTIPLIER_KEYWORD "WINDFIELD_SPEED_MULTIPLIER"

//-----//
// GMF //
//-----//

#define GMF_FILE_KEYWORD                  "GMF_FILE"
#define GMF_FILE_FORMAT_KEYWORD           "GMF_FILE_FORMAT"
#define GMF_PHI_COUNT_KEYWORD             "GMF_PHI_COUNT"
#define RETRIEVE_USING_KPC_FLAG_KEYWORD   "RETRIEVE_USING_KPC_FLAG"
#define RETRIEVE_USING_KPM_FLAG_KEYWORD   "RETRIEVE_USING_KPM_FLAG"
#define RETRIEVE_USING_KPRI_FLAG_KEYWORD  "RETRIEVE_USING_KPRI_FLAG"
#define RETRIEVE_USING_KPRS_FLAG_KEYWORD  "RETRIEVE_USING_KPRS_FLAG"
#define RETRIEVE_USING_LOGVAR_KEYWORD     "RETRIEVE_USING_LOGVAR"
#define RETRIEVE_OVER_ICE_KEYWORD         "RETRIEVE_OVER_ICE"

//-----//
// Kpm //
//-----//

#define KPM_FILE_KEYWORD  "KPM_FILE"

//------//
// Kprs //
//------//

#define KPRS_FILE_KEYWORD  "KPRS_FILE"

//----------//
// KpmField //
//----------//

#define KPM_FIELD_FILE_KEYWORD  "KPM_FIELD_FILE"

//------//
// Grid //
//------//

#define ALONGTRACK_RESOLUTION_KEYWORD        "ALONGTRACK_RESOLUTION"
#define CROSSTRACK_RESOLUTION_KEYWORD        "CROSSTRACK_RESOLUTION"
#define GRID_WINDOW_CROSSTRACK_SIZE_KEYWORD  "GRID_WINDOW_CROSSTRACK_SIZE"
#define GRID_WINDOW_ALONGTRACK_SIZE_KEYWORD  "GRID_WINDOW_ALONGTRACK_SIZE"

//---------------//
// Control Stuff //
//---------------//

#define TIME_IN_REV_KEYWORD             "TIME_IN_REV"
#define GRID_LATITUDE_RANGE_KEYWORD     "GRID_LATITUDE_RANGE"
#define GRID_TIME_RANGE_KEYWORD         "GRID_TIME_RANGE"

#define INSTRUMENT_START_TIME_KEYWORD   "INSTRUMENT_START_TIME"
#define INSTRUMENT_END_TIME_KEYWORD     "INSTRUMENT_END_TIME"
#define SPACECRAFT_START_TIME_KEYWORD   "SPACECRAFT_START_TIME"
#define SPACECRAFT_END_TIME_KEYWORD     "SPACECRAFT_END_TIME"

#define INSTRUMENT_TIME_BUFFER_KEYWORD  "INSTRUMENT_TIME_BUFFER"

//-------------------------//
// geodetic vs. geocentric //
//-------------------------//

#define ATTITUDE_KEYWORD  "ATTITUDE"

//-------------------------------//
// Random Number Generator Seeds //
//-------------------------------//

#define DEFAULT_ROLL_CONTROL_SEED     1034
#define DEFAULT_PITCH_CONTROL_SEED    45299
#define DEFAULT_YAW_CONTROL_SEED      1999

#define DEFAULT_ROLL_KNOWLEDGE_SEED   5661
#define DEFAULT_PITCH_KNOWLEDGE_SEED  78965
#define DEFAULT_YAW_KNOWLEDGE_SEED    486

#define PTGR_SEED             944

#define DEFAULT_KPRC_SEED             11456

#endif
