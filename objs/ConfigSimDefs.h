//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef CONFIGSIMDEFS_H
#define CONFIGSIMDEFS_H

static const char rcs_id_configsimdefs_h[] =
	"@(#) $Id$";

//------------//
// Spacecraft //
//------------//

#define ORBIT_EPOCH_KEYWORD				"ORBIT_EPOCH"
#define SEMI_MAJOR_AXIS_KEYWORD			"SEMI_MAJOR_AXIS"
#define ECCENTRICITY_KEYWORD			"ECCENTRICITY"
#define INCLINATION_KEYWORD				"INCLINATION"
#define LONG_OF_ASC_NODE_KEYWORD		"LONG_OF_ASC_NODE"
#define ARGUMENT_OF_PERIGEE_KEYWORD		"ARGUMENT_OF_PERIGEE"
#define MEAN_ANOMALY_AT_EPOCH_KEYWORD	"MEAN_ANOMALY_AT_EPOCH"
#define ATTITUDE_ORDER_1_KEYWORD		"ATTITUDE_ORDER_1"
#define ATTITUDE_ORDER_2_KEYWORD		"ATTITUDE_ORDER_2"
#define ATTITUDE_ORDER_3_KEYWORD		"ATTITUDE_ORDER_3"

#define EPHEMERIS_PERIOD_KEYWORD		"EPHEMERIS_PERIOD"

#define SIM_KPRS_FLAG_KEYWORD			"SIM_KPRS_FLAG"

//------------------------------//
// Attitude Control Error Model //
//------------------------------//

#define ATTITUDE_CONTROL_MODEL_KEYWORD		"ATTITUDE_CONTROL_MODEL"

#define ROLL_CONTROL_MEAN_KEYWORD			"ROLL_CONTROL_MEAN"
#define PITCH_CONTROL_MEAN_KEYWORD			"PITCH_CONTROL_MEAN"
#define YAW_CONTROL_MEAN_KEYWORD			"YAW_CONTROL_MEAN"

#define ROLL_CONTROL_STD_KEYWORD			"ROLL_CONTROL_STD"
#define PITCH_CONTROL_STD_KEYWORD			"PITCH_CONTROL_STD"
#define YAW_CONTROL_STD_KEYWORD				"YAW_CONTROL_STD"

#define ROLL_CONTROL_CORRLENGTH_KEYWORD		"ROLL_CONTROL_CORRLENGTH"
#define PITCH_CONTROL_CORRLENGTH_KEYWORD	"PITCH_CONTROL_CORRLENGTH"
#define YAW_CONTROL_CORRLENGTH_KEYWORD		"YAW_CONTROL_CORRLENGTH"

//--------------------------------//
// Attitude Knowledge Error Model //
//--------------------------------//

#define ATTITUDE_KNOWLEDGE_MODEL_KEYWORD	"ATTITUDE_KNOWLEDGE_MODEL"

#define ROLL_KNOWLEDGE_MEAN_KEYWORD			"ROLL_KNOWLEDGE_MEAN"
#define PITCH_KNOWLEDGE_MEAN_KEYWORD		"PITCH_KNOWLEDGE_MEAN"
#define YAW_KNOWLEDGE_MEAN_KEYWORD			"YAW_KNOWLEDGE_MEAN"

#define ROLL_KNOWLEDGE_STD_KEYWORD			"ROLL_KNOWLEDGE_STD"
#define PITCH_KNOWLEDGE_STD_KEYWORD			"PITCH_KNOWLEDGE_STD"
#define YAW_KNOWLEDGE_STD_KEYWORD			"YAW_KNOWLEDGE_STD"

#define ROLL_KNOWLEDGE_CORRLENGTH_KEYWORD	"ROLL_KNOWLEDGE_CORRLENGTH"
#define PITCH_KNOWLEDGE_CORRLENGTH_KEYWORD	"PITCH_KNOWLEDGE_CORRLENGTH"
#define YAW_KNOWLEDGE_CORRLENGTH_KEYWORD	"YAW_KNOWLEDGE_CORRLENGTH"

//------------//
// Instrument //
//------------//

#define CHIRP_RATE_KEYWORD				"CHIRP_RATE"
#define CHIRP_START_M_KEYWORD			"CHIRP_START_M"
#define CHIRP_START_B_KEYWORD			"CHIRP_START_B"
#define SYSTEM_TEMPERATURE_KEYWORD		"SYSTEM_TEMPERATURE"
#define BASE_TRANSMIT_FREQUENCY_KEYWORD	"BASE_TRANSMIT_FREQUENCY"
#define SCIENCE_SLICES_PER_SPOT_KEYWORD	"SCIENCE_SLICES_PER_SPOT"
#define SCIENCE_SLICE_BANDWIDTH_KEYWORD	"SCIENCE_SLICE_BANDWIDTH"
#define GUARD_SLICES_PER_SIDE_KEYWORD	"GUARD_SLICES_PER_SIDE"
#define GUARD_SLICE_BANDWIDTH_KEYWORD	"GUARD_SLICE_BANDWIDTH"
#define NOISE_BANDWIDTH_KEYWORD			"NOISE_BANDWIDTH"
#define TRANSMIT_POWER_KEYWORD			"TRANSMIT_POWER"
#define ECHO_RECEIVER_GAIN_KEYWORD		"ECHO_RECEIVER_GAIN"
#define NOISE_RECEIVER_GAIN_KEYWORD		"NOISE_RECEIVER_GAIN"
#define SYSTEM_LOSS_KEYWORD				"SYSTEM_LOSS"
#define PTGR_NOISE_KP_KEYWORD			"PTGR_NOISE_KP"
#define PTGR_NOISE_BIAS_KEYWORD			"PTGR_BIAS"
#define PTGR_NOISE_CORRLENGTH_KEYWORD	"PTGR_NOISE_CORRLENGTH"
#define UNIFORM_SIGMA_FIELD_KEYWORD		"UNIFORM_SIGMA_FIELD"
#define OUTPUT_X_TO_STDOUT_KEYWORD		"OUTPUT_X_TO_STDOUT"
#define USE_KFACTOR_KEYWORD				"USE_KFACTOR"
#define COMPUTE_KFACTOR_KEYWORD			"COMPUTE_KFACTOR"
#define RANGE_GATE_CLIPPING_KEYWORD		"RANGE_GATE_CLIPPING"
#define CREATE_XTABLE_KEYWORD			"CREATE_XTABLE"
#define ORBIT_TICKS_PER_ORBIT_KEYWORD	"ORBIT_TICKS_PER_ORBIT"
#define CORR_KPM_KEYWORD				"CORR_KPM"

#define SIM_KPC_FLAG_KEYWORD			"SIM_KPC_FLAG"
#define SIM_CORR_KPM_FLAG_KEYWORD		"SIM_CORR_KPM_FLAG"
#define SIM_UNCORR_KPM_FLAG_KEYWORD		"SIM_UNCORR_KPM_FLAG"
#define SIM_KPRI_FLAG_KEYWORD			"SIM_KPRI_FLAG"

//-----------------------//
// InstrumentSimAccurate //
//-----------------------//

#define NUM_LOOK_STEPS_KEYWORD			"NUM_INTEGRATION_LOOK_STEPS_PER_SLICE"
#define AZIMUTH_INTEGRATION_RANGE_KEYWORD	"AZIMUTH_INTEGRATION_RANGE"
#define AZIMUTH_STEP_SIZE_KEYWORD			"AZIMUTH_INTEGRATION_STEP_SIZE"

//------------//
// AntennaSim //
//------------//

#define ANTENNA_START_AZIMUTH_KEYWORD	"ANTENNA_START_AZIMUTH"
#define ANTENNA_START_TIME_KEYWORD		"ANTENNA_START_TIME"

//---------//
// Antenna //
//---------//

#define NUMBER_OF_BEAMS_KEYWORD			"NUMBER_OF_BEAMS"
#define PRI_PER_BEAM_KEYWORD			"PRI_PER_BEAM"
#define NUMBER_OF_ENCODER_BITS_KEYWORD	"NUMBER_OF_ENCODER_BITS"
#define ANTENNA_PEDESTAL_ROLL_KEYWORD	"ANTENNA_PEDESTAL_ROLL"
#define ANTENNA_PEDESTAL_PITCH_KEYWORD	"ANTENNA_PEDESTAL_PITCH"
#define ANTENNA_PEDESTAL_YAW_KEYWORD	"ANTENNA_PEDESTAL_YAW"
#define COMMANDED_SPIN_RATE_KEYWORD		"COMMANDED_SPIN_RATE"
#define ACTUAL_SPIN_RATE_KEYWORD		"ACTUAL_SPIN_RATE"
#define ENCODER_A_OFFSET_KEYWORD		"ENCODER_A_OFFSET"
#define ENCODER_DELAY_KEYWORD			"ENCODER_DELAY"

//------//
// Beam //
//------//

#define USE_RGC_KEYWORD						"USE_RGC"
#define USE_DTC_KEYWORD						"USE_DTC"
#define MECH_LOOK_ANGLE_KEYWORD				"MECH_LOOK_ANGLE"
#define MECH_AZIMUTH_ANGLE_KEYWORD			"MECH_AZIMUTH_ANGLE"
#define BEAM_x_LOOK_ANGLE_KEYWORD			"BEAM_x_LOOK_ANGLE"
#define BEAM_x_AZIMUTH_ANGLE_KEYWORD		"BEAM_x_AZIMUTH_ANGLE"
#define BEAM_x_POLARIZATION_KEYWORD			"BEAM_x_POLARIZATION"
#define BEAM_x_PULSE_WIDTH_KEYWORD			"BEAM_x_PULSE_WIDTH"
#define BEAM_x_RECEIVER_GATE_WIDTH_KEYWORD	"BEAM_x_RECEIVER_GATE_WIDTH"
#define BEAM_x_TIME_OFFSET_KEYWORD			"BEAM_x_TIME_OFFSET"
#define BEAM_x_PATTERN_FILE_KEYWORD			"BEAM_x_PATTERN_FILE"
#define BEAM_x_RGC_FILE_KEYWORD				"BEAM_x_RGC_FILE"
#define BEAM_x_DTC_FILE_KEYWORD				"BEAM_x_DTC_FILE"

// Tracking
#define BEAM_x_PEAK_OFFSET_DN_KEYWORD		"BEAM_x_PEAK_OFFSET_DN"

//--------//
// XTable //
//--------//

#define XTABLE_FILENAME_KEYWORD				"XTABLE_FILENAME"
#define XTABLE_NUM_AZIMUTHS_KEYWORD			"XTABLE_NUM_AZIMUTHS"
#define XTABLE_NUM_ORBIT_STEPS_KEYWORD		"XTABLE_NUM_ORBIT_STEPS"

//-----//
// L00 //
//-----//

#define L00_FILE_KEYWORD						"L00_FILE"
#define L00_ANTENNA_CYCLES_PER_FRAME_KEYWORD	"L00_ANTENNA_CYCLES_PER_FRAME"

//-----//
// L1A //
//-----//

#define L1A_FILE_KEYWORD		"L1A_FILE"

//-----//
// L1B //
//-----//

#define L1B_FILE_KEYWORD		"L1B_FILE"

//----------//
// L1AToL1B //
//----------//

#define OUTPUT_SIGMA0_TO_STDOUT_KEYWORD		"OUTPUT_SIGMA0_TO_STDOUT"
#define USE_SPOT_COMPOSITES_KEYWORD			"USE_SPOT_COMPOSITES"
#define SLICE_GAIN_THRESHOLD_KEYWORD		"SLICE_GAIN_THRESHOLD"
#define PROCESS_MAX_SLICES_KEYWORD			"PROCESS_MAX_SLICES"

//-----//
// L2A //
//-----//

#define L2A_FILE_KEYWORD		"L2A_FILE"

//-----//
// L2B //
//-----//

#define L2B_FILE_KEYWORD					"L2B_FILE"

//----------//
// L2AToL2B //
//----------//

#define MEDIAN_FILTER_WINDOW_SIZE_KEYWORD		"MEDIAN_FILTER_WINDOW_SIZE"
#define MEDIAN_FILTER_MAX_PASSES_KEYWORD		"MEDIAN_FILTER_MAX_PASSES"
#define USE_MANY_AMBIGUITIES_KEYWORD			"USE_MANY_AMBIGUITIES"
#define USE_AMBIGUITY_WEIGHTS_KEYWORD			"USE_AMBIGUITY_WEIGHTS"
#define USE_PEAK_SPLITTING_KEYWORD				"USE_PEAK_SPLITTING"
#define USE_NUDGING_KEYWORD						"USE_PEAK_SPLITTING"
#define NUDGE_WINDFIELD_FILE_KEYWORD			"NUDGE_WINDFIELD_FILE"
#define NUDGE_WINDFIELD_TYPE_KEYWORD			"NUDGE_WINDFIELD_TYPE"
#define ONE_PEAK_WIDTH_KEYWORD					"ONE_PEAK_WIDTH"
#define TWO_PEAK_SEPARATION_THRESHOLD_KEYWORD	"TWO_PEAK_SEPARATION_THRESHOLD"
#define SCALED_PROBABILITY_THRESHOLD_KEYWORD	"SCALED_PROBABILITY_THRESHOLD"

//-----------//
// Ephemeris //
//-----------//

#define EPHEMERIS_FILE_KEYWORD				"EPHEMERIS_FILE"

//-----------//
// WindField //
//-----------//

#define WINDFIELD_TYPE_KEYWORD				"WINDFIELD_TYPE"
#define WINDFIELD_FILE_KEYWORD				"WINDFIELD_FILE"

//-----//
// GMF //
//-----//

#define GMF_FILE_KEYWORD					"GMF_FILE"
#define GMF_PHI_COUNT_KEYWORD				"GMF_PHI_COUNT"
#define RETRIEVE_USING_KPC_FLAG_KEYWORD		"RETRIEVE_USING_KPC_FLAG"
#define RETRIEVE_USING_KPM_FLAG_KEYWORD		"RETRIEVE_USING_KPM_FLAG"
#define RETRIEVE_USING_KPRI_FLAG_KEYWORD	"RETRIEVE_USING_KPRI_FLAG"
#define RETRIEVE_USING_KPRS_FLAG_KEYWORD	"RETRIEVE_USING_KPRS_FLAG"

//-----//
// Kpm //
//-----//

#define KPM_FILE_KEYWORD					"KPM_FILE"

//------//
// Kprs //
//------//

#define KPRS_FILE_KEYWORD					"KPRS_FILE"

//----------//
// KpmField //
//----------//

#define KPM_FIELD_FILE_KEYWORD				"KPM_FIELD_FILE"

//------//
// Grid //
//------//

#define ALONGTRACK_RESOLUTION_KEYWORD		"ALONGTRACK_RESOLUTION"
#define CROSSTRACK_RESOLUTION_KEYWORD		"CROSSTRACK_RESOLUTION"

//---------------//
// Control Stuff //
//---------------//

#define TIME_IN_REV_KEYWORD				"TIME_IN_REV"
#define GRID_LATITUDE_RANGE_KEYWORD		"GRID_LATITUDE_RANGE"
#define GRID_TIME_RANGE_KEYWORD			"GRID_TIME_RANGE"

#define INSTRUMENT_START_TIME_KEYWORD	"INSTRUMENT_START_TIME"
#define INSTRUMENT_END_TIME_KEYWORD		"INSTRUMENT_END_TIME"
#define SPACECRAFT_START_TIME_KEYWORD	"SPACECRAFT_START_TIME"
#define SPACECRAFT_END_TIME_KEYWORD		"SPACECRAFT_END_TIME"

#define INSTRUMENT_TIME_BUFFER_KEYWORD	"INSTRUMENT_TIME_BUFFER"

//-------------------------------//
// Random Number Generator Seeds //
//-------------------------------//

#define ROLL_CONTROL_SEED	1034
#define PITCH_CONTROL_SEED	45299
#define YAW_CONTROL_SEED	1999

#define ROLL_KNOWLEDGE_SEED		5661
#define PITCH_KNOWLEDGE_SEED	78965
#define YAW_KNOWLEDGE_SEED		486

#define PTGR_SEED 944

#endif
