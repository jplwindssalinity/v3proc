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
#include "WindField.h"

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

//-----------//
// Ephemeris //
//-----------//

#define EPHEMERIS_FILE_KEYWORD				"EPHEMERIS_FILE"

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

#endif
