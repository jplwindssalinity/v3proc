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
#include "L00.h"
#include "L10.h"
#include "L15.h"
#include "L17.h"
#include "L20.h"
#include "L17ToL20.h"
#include "Wind.h"
#include "Grid.h"
#include "ConfigList.h"

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

#define EPHEMERIS_PERIOD_KEYWORD		"EPHEMERIS_PERIOD"

int ConfigSpacecraftSim(SpacecraftSim* spacecraft_sim,
	ConfigList* config_list);

//--------------------------------//
// Attitude Control Error Model   //
//--------------------------------//

#define ATTITUDE_CONTROL_MODEL_KEYWORD  "ATTITUDE_CONTROL_MODEL"
#define VELOCITY_SAMPLE_RATE_KEYWORD    "VELOCITY_SAMPLE_RATE"
#define ROLL_CONTROL_MEAN_KEYWORD 	"ROLL_CONTROL_MEAN"
#define PITCH_CONTROL_MEAN_KEYWORD 	"PITCH_CONTROL_MEAN"
#define YAW_CONTROL_MEAN_KEYWORD 	"YAW_CONTROL_MEAN"
#define ROLL_CONTROL_BOUND_KEYWORD 	"ROLL_CONTROL_BOUND"
#define PITCH_CONTROL_BOUND_KEYWORD 	"PITCH_CONTROL_BOUND"
#define YAW_CONTROL_BOUND_KEYWORD 	"YAW_CONTROL_BOUND"
#define ROLL_CONTROL_VARIANCE_KEYWORD 	"ROLL_CONTROL_VARIANCE"
#define PITCH_CONTROL_VARIANCE_KEYWORD 	"PITCH_CONTROL_VARIANCE"
#define YAW_CONTROL_VARIANCE_KEYWORD 	"YAW_CONTROL_VARIANCE"
#define ROLL_CONTROL_RADIUS_KEYWORD 	"ROLL_CONTROL_RADIUS"
#define PITCH_CONTROL_RADIUS_KEYWORD 	"PITCH_CONTROL_RADIUS"
#define YAW_CONTROL_RADIUS_KEYWORD 	"YAW_CONTROL_RADIUS"

int ConfigAttitudeControlModel(SpacecraftSim* spacecraft_sim,
	ConfigList* config_list);


//------------//
// Instrument //
//------------//

#define CHIRP_RATE_KEYWORD				"CHIRP_RATE"
#define CHIRP_START_M_KEYWORD			"CHIRP_START_M"
#define CHIRP_START_B_KEYWORD			"CHIRP_START_B"
#define SYSTEM_DELAY_KEYWORD			"SYSTEM_DELAY"
#define RECEIVER_GATE_WIDTH_KEYWORD		"RECEIVER_GATE_WIDTH"
#define BASE_TRANSMIT_FREQUENCY_KEYWORD	"BASE_TRANSMIT_FREQUENCY"
#define SLICE_BANDWIDTH_KEYWORD			"SLICE_BANDWIDTH"

int ConfigInstrument(Instrument* instrument, ConfigList* config_list);

#define INSTRUMENT_START_TIME_KEYWORD	"INSTRUMENT_START_TIME"
#define INSTRUMENT_END_TIME_KEYWORD		"INSTRUMENT_END_TIME"

int ConfigInstrumentSim(InstrumentSim* instrument_sim,
	ConfigList* config_list);

//---------//
// Antenna //
//---------//

#define NUMBER_OF_BEAMS_KEYWORD			"NUMBER_OF_BEAMS"
#define PRI_PER_BEAM_KEYWORD			"PRI_PER_BEAM"
#define NUMBER_OF_ENCODER_BITS_KEYWORD	"NUMBER_OF_ENCODER_BITS"
#define ANTENNA_PEDESTAL_ROLL_KEYWORD	"ANTENNA_PEDESTAL_ROLL"
#define ANTENNA_PEDESTAL_PITCH_KEYWORD	"ANTENNA_PEDESTAL_PITCH"
#define ANTENNA_PEDESTAL_YAW_KEYWORD	"ANTENNA_PEDESTAL_YAW"

int ConfigAntenna(Antenna* antenna, ConfigList* config_list);

#define SPIN_RATE_KEYWORD				"ANTENNA_SPIN_RATE"

int ConfigAntennaSim(AntennaSim* antenna_sim, ConfigList* config_list);

//------//
// Beam //
//------//

#define BEAM_x_LOOK_ANGLE_KEYWORD		"BEAM_x_LOOK_ANGLE"
#define BEAM_x_AZIMUTH_ANGLE_KEYWORD	"BEAM_x_AZIMUTH_ANGLE"
#define BEAM_x_POLARIZATION_KEYWORD		"BEAM_x_POLARIZATION"
#define BEAM_x_PULSE_WIDTH_KEYWORD		"BEAM_x_PULSE_WIDTH"
#define BEAM_x_TIME_OFFSET_KEYWORD		"BEAM_x_TIME_OFFSET"
#define BEAM_x_PATTERN_FILE_KEYWORD		"BEAM_x_PATTERN_FILE"

int ConfigBeam(Beam* beam, int beam_number, ConfigList* config_list);

//-----//
// L00 //
//-----//

#define L00_FILE_KEYWORD					"L00_FILE"
#define L00_BEAM_CYCLES_PER_FRAME_KEYWORD	"L00_BEAM_CYCLES_PER_FRAME"
#define L00_SLICES_PER_SPOT_KEYWORD			"L00_SLICES_PER_SPOT"

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
#define ALONGTRACK_START_TIME_KEYWORD		"ALONGTRACK_START_TIME"

int ConfigGrid(Grid* grid, ConfigList* config_list);

#endif
