//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

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

//------------------------------//
// Attitude Control Error Model //
//------------------------------//

#define ATTITUDE_CONTROL_MODEL_KEYWORD	"ATTITUDE_CONTROL_MODEL"
#define CONTROL_SAMPLE_RATE_KEYWORD		"CONTROL_SAMPLE_RATE"
#define ROLL_CONTROL_MEAN_KEYWORD		"ROLL_CONTROL_MEAN"
#define PITCH_CONTROL_MEAN_KEYWORD		"PITCH_CONTROL_MEAN"
#define YAW_CONTROL_MEAN_KEYWORD		"YAW_CONTROL_MEAN"
#define ROLL_CONTROL_BOUND_KEYWORD		"ROLL_CONTROL_BOUND"
#define PITCH_CONTROL_BOUND_KEYWORD		"PITCH_CONTROL_BOUND"
#define YAW_CONTROL_BOUND_KEYWORD		"YAW_CONTROL_BOUND"
#define ROLL_CONTROL_VARIANCE_KEYWORD	"ROLL_CONTROL_VARIANCE"
#define PITCH_CONTROL_VARIANCE_KEYWORD	"PITCH_CONTROL_VARIANCE"
#define YAW_CONTROL_VARIANCE_KEYWORD	"YAW_CONTROL_VARIANCE"
#define ROLL_CONTROL_RADIUS_KEYWORD		"ROLL_CONTROL_RADIUS"
#define PITCH_CONTROL_RADIUS_KEYWORD	"PITCH_CONTROL_RADIUS"
#define YAW_CONTROL_RADIUS_KEYWORD		"YAW_CONTROL_RADIUS"

//--------------------------------//
// Attitude Knowledge Error Model //
//--------------------------------//

#define ATTITUDE_KNOWLEDGE_MODEL_KEYWORD	"ATTITUDE_KNOWLEDGE_MODEL"
#define KNOWLEDGE_SAMPLE_RATE_KEYWORD		"KNOWLEDGE_SAMPLE_RATE"
#define ROLL_KNOWLEDGE_MEAN_KEYWORD			"ROLL_KNOWLEDGE_MEAN"
#define PITCH_KNOWLEDGE_MEAN_KEYWORD		"PITCH_KNOWLEDGE_MEAN"
#define YAW_KNOWLEDGE_MEAN_KEYWORD			"YAW_KNOWLEDGE_MEAN"
#define ROLL_KNOWLEDGE_BOUND_KEYWORD		"ROLL_KNOWLEDGE_BOUND"
#define PITCH_KNOWLEDGE_BOUND_KEYWORD		"PITCH_KNOWLEDGE_BOUND"
#define YAW_KNOWLEDGE_BOUND_KEYWORD			"YAW_KNOWLEDGE_BOUND"
#define ROLL_KNOWLEDGE_VARIANCE_KEYWORD		"ROLL_KNOWLEDGE_VARIANCE"
#define PITCH_KNOWLEDGE_VARIANCE_KEYWORD	"PITCH_KNOWLEDGE_VARIANCE"
#define YAW_KNOWLEDGE_VARIANCE_KEYWORD		"YAW_KNOWLEDGE_VARIANCE"
#define ROLL_KNOWLEDGE_RADIUS_KEYWORD		"ROLL_KNOWLEDGE_RADIUS"
#define PITCH_KNOWLEDGE_RADIUS_KEYWORD		"PITCH_KNOWLEDGE_RADIUS"
#define YAW_KNOWLEDGE_RADIUS_KEYWORD		"YAW_KNOWLEDGE_RADIUS"

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
#define RECEIVER_GAIN_KEYWORD			"RECEIVER_GAIN"
#define SYSTEM_LOSS_KEYWORD				"SYSTEM_LOSS"
#define USE_KPC_KEYWORD					"USE_KPC"
#define USE_KPM_KEYWORD					"USE_KPM"
#define PTGR_NOISE_VARIANCE_KEYWORD		"PTGR_NOISE_VARIANCE"
#define PTGR_NOISE_MEAN_KEYWORD			"PTGR_NOISE_MEAN"
#define UNIFORM_SIGMA_FIELD_KEYWORD		"UNIFORM_SIGMA_FIELD"
#define OUTPUT_PR_TO_STDOUT_KEYWORD		"OUTPUT_PR_TO_STDOUT"
#define USE_KFACTOR_KEYWORD				"USE_KFACTOR"
#define CREATE_XTABLE_KEYWORD			"CREATE_XTABLE"

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
#define SPIN_RATE_KEYWORD				"ANTENNA_SPIN_RATE"

//------//
// Beam //
//------//

#define BEAM_x_LOOK_ANGLE_KEYWORD			"BEAM_x_LOOK_ANGLE"
#define BEAM_x_AZIMUTH_ANGLE_KEYWORD		"BEAM_x_AZIMUTH_ANGLE"
#define BEAM_x_POLARIZATION_KEYWORD			"BEAM_x_POLARIZATION"
#define BEAM_x_PULSE_WIDTH_KEYWORD			"BEAM_x_PULSE_WIDTH"
#define BEAM_x_RECEIVER_GATE_WIDTH_KEYWORD	"BEAM_x_RECEIVER_GATE_WIDTH"
#define BEAM_x_TIME_OFFSET_KEYWORD			"BEAM_x_TIME_OFFSET"
#define BEAM_x_PATTERN_FILE_KEYWORD			"BEAM_x_PATTERN_FILE"

//--------//
// XTable //
//--------//

#define XTABLE_FILENAME_KEYWORD				"XTABLE_FILENAME"

//-----//
// L00 //
//-----//

#define L00_FILE_KEYWORD						"L00_FILE"
#define L00_ANTENNA_CYCLES_PER_FRAME_KEYWORD	"L00_ANTENNA_CYCLES_PER_FRAME"

//-----//
// L10 //
//-----//

#define L10_FILE_KEYWORD					"L10_FILE"

//-----//
// L15 //
//-----//

#define L15_FILE_KEYWORD					"L15_FILE"

//----------//
// L10ToL15 //
//----------//

#define OUTPUT_SIGMA0_TO_STDOUT_KEYWORD		"OUTPUT_SIGMA0_TO_STDOUT"

//-----//
// L17 //
//-----//

#define L17_FILE_KEYWORD					"L17_FILE"

//----------//
// L17ToL20 //
//----------//

#define SPD_TOLERANCE_KEYWORD				"SPD_TOLERANCE"
#define PHI_STEP_KEYWORD					"PHI_STEP"
#define PHI_BUFFER_KEYWORD					"PHI_BUFFER"
#define PHI_MAX_SMOOTHING_KEYWORD			"PHI_MAX_SMOOTHING"

//-----//
// L20 //
//-----//

#define L20_FILE_KEYWORD					"L20_FILE"
#define MEDIAN_FILTER_WINDOW_SIZE_KEYWORD	"MEDIAN_FILTER_WINDOW_SIZE"
#define MEDIAN_FILTER_MAX_PASSES_KEYWORD	"MEDIAN_FILTER_MAX_PASSES"

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

//----------//
// Tracking //
//----------//

#define USE_RGC_KEYWORD		"USE_RGC"
#define USE_DTC_KEYWORD		"USE_DTC"
#define RGC_FILE_KEYWORD	"RGC_FILE"
#define DTC_FILE_KEYWORD	"DTC_FILE"

#endif
