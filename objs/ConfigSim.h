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

//======================================================================
// DESCRIPTION
//		These functions are used to initialize components and their
//		simulators.
//======================================================================

//------------//
// Spacecraft //
//------------//

#define SEMI_MAJOR_AXIS_KEYWORD			"SEMI_MAJOR_AXIS"
#define ECCENTRICITY_KEYWORD			"ECCENTRICITY"
#define INCLINATION_KEYWORD				"INCLINATION"
#define LONG_OF_ASC_NODE_KEYWORD		"LONG_OF_ASC_NODE"
#define ARGUMENT_OF_PERIGEE_KEYWORD		"ARGUMENT_OF_PERIGEE"
#define MEAN_ANOMALY_KEYWORD			"MEAN_ANOMALY"

#define EPHEMERIS_PERIOD_KEYWORD		"EPHEMERIS_PERIOD"

int ConfigSpacecraftSim(SpacecraftSim* spacecraft_sim,
	ConfigList* config_list);

//------------//
// Instrument //
//------------//

int ConfigInstrument(Instrument* instrument, ConfigList* config_list);

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
#define BEAM_x_TIME_OFFSET_KEYWORD		"BEAM_x_TIME_OFFSET"

int ConfigBeam(Beam* beam, int beam_number, ConfigList* config_list);

//-----//
// L00 //
//-----//

#define L00_FILE_KEYWORD					"L00_FILE"

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
