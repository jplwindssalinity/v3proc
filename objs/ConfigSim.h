//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef CONFIGSIM_H
#define CONFIGSIM_H

static const char rcs_id_configsim_h[] =
	"@(#) $Id$";

#include "SpacecraftSim.h"
#include "InstrumentSim.h"
#include "InstrumentSimAccurate.h"
#include "XTable.h"
#include "L00.h"
#include "L10.h"
#include "L10ToL15.h"
#include "L15.h"
#include "L17.h"
#include "L20.h"
#include "L17ToL20.h"
#include "Wind.h"
#include "Grid.h"
#include "ConfigList.h"
#include "Tracking.h"

//======================================================================
// DESCRIPTION
//		These functions are used to initialize components and their
//		simulators.
//======================================================================

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

int ConfigSpacecraft(Spacecraft* spacecraft, ConfigList* config_list);
int ConfigSpacecraftSim(SpacecraftSim* spacecraft_sim,
		ConfigList* config_list);

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

int ConfigAttitudeControlModel(SpacecraftSim* spacecraft_sim,
	ConfigList* config_list);

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

int ConfigAttitudeKnowledgeModel(SpacecraftSim* spacecraft_sim,
	ConfigList* config_list);

//------------------------------//
// Generic Noise Model Config	//
// Routines						//
//------------------------------//

Gaussian*	ConfigGaussian(const char* variance_keyword,
	const char* mean_keyword,
	ConfigList* config_list);

Uniform* ConfigUniform(const char* radius_keyword,
	const char* mean_keyword,
	ConfigList* config_list);

RandomVelocity* ConfigGaussianRandomVelocity(const char*
	samprate_keyword,
	const char* bound_keyword, const char* mean_keyword,
	const char* variance_keyword,
	ConfigList* config_list);

RandomVelocity* ConfigUniformRandomVelocity(const char*
	samprate_keyword,
	const char* bound_keyword, const char* mean_keyword,
	const char* radius_keyword,
	ConfigList* config_list);


//------------//
// Instrument //
//------------//

#define CHIRP_RATE_KEYWORD				"CHIRP_RATE"
#define CHIRP_START_M_KEYWORD			"CHIRP_START_M"
#define CHIRP_START_B_KEYWORD			"CHIRP_START_B"
#define SYSTEM_DELAY_KEYWORD			"SYSTEM_DELAY"
#define SYSTEM_TEMPERATURE_KEYWORD		"SYSTEM_TEMPERATURE"
#define XMIT_PULSE_WIDTH_KEYWORD		"XMIT_PULSE_WIDTH"
#define RECEIVER_GATE_WIDTH_KEYWORD		"RECEIVER_GATE_WIDTH"
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
#define PTGR_NOISE_VARIANCE_KEYWORD		"PTGR_NOISE_VARIANCE"
#define PTGR_NOISE_MEAN_KEYWORD			"PTGR_NOISE_MEAN"
#define UNIFORM_SIGMA_FIELD_KEYWORD		"UNIFORM_SIGMA_FIELD"
#define OUTPUT_PR_TO_STDOUT_KEYWORD		"OUTPUT_PR_TO_STDOUT"
#define USE_KFACTOR_KEYWORD				"USE_KFACTOR"
#define CREATE_XTABLE_KEYWORD			"CREATE_XTABLE"

int ConfigInstrument(Instrument* instrument, ConfigList* config_list);
int ConfigInstrumentSim(InstrumentSim* instrument_sim,
		ConfigList* config_list);

//-----------------------//
// InstrumentSimAccurate //
//-----------------------//

#define NUM_LOOK_STEPS_KEYWORD			"NUM_INTEGRATION_LOOK_STEPS_PER_SLICE"
#define AZIMUTH_INTEGRATION_RANGE_KEYWORD	"AZIMUTH_INTEGRATION_RANGE"
#define AZIMUTH_STEP_SIZE_KEYWORD			"AZIMUTH_INTEGRATION_STEP_SIZE"


int ConfigInstrumentSimAccurate(InstrumentSimAccurate* instrument_sim,
	ConfigList* config_list);

//------------//
// AntennaSim //
//------------//

#define ANTENNA_START_AZIMUTH_KEYWORD	"ANTENNA_START_AZIMUTH"
#define ANTENNA_START_TIME_KEYWORD		"ANTENNA_START_TIME"

int ConfigAntennaSim(AntennaSim* antenna_sim, ConfigList* config_list);

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

int ConfigAntenna(Antenna* antenna, ConfigList* config_list);

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

int ConfigBeam(Beam* beam, int beam_number, ConfigList* config_list);


//--------//
// XTable //
//--------//

#define XTABLE_FILENAME_KEYWORD				"XTABLE_FILENAME"
int ConfigXTable(XTable* xTable, ConfigList* config_list, char* read_write);

//-----//
// L00 //
//-----//

#define L00_FILE_KEYWORD						"L00_FILE"
#define L00_ANTENNA_CYCLES_PER_FRAME_KEYWORD	"L00_ANTENNA_CYCLES_PER_FRAME"

int ConfigL00(L00* l00, ConfigList* config_list);

//-----//
// L10 //
//-----//

#define L10_FILE_KEYWORD					"L10_FILE"

int ConfigL10(L10* l10, ConfigList* config_list);

//-----//
// L15 //
//-----//

#define L15_FILE_KEYWORD					"L15_FILE"

int ConfigL15(L15* l15, ConfigList* config_list);

//----------//
// L10ToL15 //
//----------//

#define OUTPUT_SIGMA0_TO_STDOUT_KEYWORD		"OUTPUT_SIGMA0_TO_STDOUT"

int ConfigL10ToL15(L10ToL15* l10tol15, ConfigList* config_list);

//-----//
// L17 //
//-----//

#define L17_FILE_KEYWORD					"L17_FILE"

int ConfigL17(L17* l17, ConfigList* config_list);

//----------//
// L17ToL20 //
//----------//

#define SPD_TOLERANCE_KEYWORD				"SPD_TOLERANCE"
#define PHI_STEP_KEYWORD					"PHI_STEP"
#define PHI_BUFFER_KEYWORD					"PHI_BUFFER"
#define PHI_MAX_SMOOTHING_KEYWORD			"PHI_MAX_SMOOTHING"

int ConfigL17ToL20(L17ToL20* l17_to_l20, ConfigList* config_list);

//-----//
// L20 //
//-----//

#define L20_FILE_KEYWORD					"L20_FILE"
#define MEDIAN_FILTER_WINDOW_SIZE_KEYWORD	"MEDIAN_FILTER_WINDOW_SIZE"
#define MEDIAN_FILTER_MAX_PASSES_KEYWORD	"MEDIAN_FILTER_MAX_PASSES"

int ConfigL20(L20* l20, ConfigList* config_list);

//-----------//
// Ephemeris //
//-----------//

#define EPHEMERIS_FILE_KEYWORD				"EPHEMERIS_FILE"

int ConfigEphemeris(Ephemeris* ephemeris, ConfigList* config_list);

//-----------//
// WindField //
//-----------//

#define WINDFIELD_FILE_KEYWORD				"WINDFIELD_FILE"

int ConfigWindField(WindField* windfield, ConfigList* config_list);

//-----//
// GMF //
//-----//

#define GMF_FILE_KEYWORD					"GMF_FILE"

int ConfigGMF(GMF* gmf, ConfigList* config_file);

//------//
// Grid //
//------//

#define ALONGTRACK_RESOLUTION_KEYWORD		"ALONGTRACK_RESOLUTION"
#define CROSSTRACK_RESOLUTION_KEYWORD		"CROSSTRACK_RESOLUTION"

int ConfigGrid(Grid* grid, ConfigList* config_list);

//---------------//
// Control Stuff //
//---------------//

#define GRID_START_TIME_KEYWORD			"GRID_START_TIME"
#define GRID_END_TIME_KEYWORD			"GRID_END_TIME"
#define INSTRUMENT_START_TIME_KEYWORD	"INSTRUMENT_START_TIME"
#define INSTRUMENT_END_TIME_KEYWORD		"INSTRUMENT_END_TIME"
#define SPACECRAFT_START_TIME_KEYWORD	"SPACECRAFT_START_TIME"
#define SPACECRAFT_END_TIME_KEYWORD		"SPACECRAFT_END_TIME"

#define APPROXIMATE_START_TIME_KEYWORD	"APPROXIMATE_START_TIME"
#define GRID_START_ARG_OF_LAT_KEYWORD	"GRID_START_ARG_OF_LAT"
#define GRID_LATITUDE_RANGE_KEYWORD		"GRID_LATITUDE_RANGE"
#define GRID_TIME_RANGE_KEYWORD			"GRID_TIME_RANGE"
#define INSTRUMENT_TIME_BUFFER_KEYWORD	"INSTRUMENT_TIME_BUFFER"

int		ConfigControl(SpacecraftSim* spacecraft_sim, ConfigList* config_list,
			double* grid_start_time, double* grid_end_time,
			double* instrument_start_time, double* instrument_end_time,
			double* spacecraft_start_time, double* spacecraft_end_time);

//----------//
// Tracking //
//----------//

#define RGC_FILE_KEYWORD	"RGC_FILE"
#define DTC_FILE_KEYWORD	"DTC_FILE"

int		ConfigRangeTracker(RangeTracker* range_tracker,
			ConfigList* config_list);
int		ConfigDopplerTracker(DopplerTracker* doppler_tracker,
			ConfigList* config_list);

#endif
