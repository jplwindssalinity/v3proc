//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef CONFIGSIM_H
#define CONFIGSIM_H

static const char rcs_id_configsim_h[] =
	"@(#) $Id$";

#include "InstrumentSim.h"
#include "L0.h"
#include "L1.h"
#include "L15.h"

//======================================================================
// DESCRIPTION
//		These functions are used to initialize the instrument
//		simulation and the corresponding sub-simulations.
//======================================================================

//------------//
// Instrument //
//------------//

int ConfigInstrument(Instrument* instrument, ConfigList* config_list);

//---------//
// Antenna //
//---------//

#define NUMBER_OF_BEAMS_KEYWORD			"NUMBER_OF_BEAMS"

int ConfigAntenna(Antenna* antenna, ConfigList* config_list);

//------//
// Beam //
//------//

#define BEAM_x_LOOK_ANGLE_KEYWORD		"BEAM_x_LOOK_ANGLE"
#define BEAM_x_AZIMUTH_ANGLE_KEYWORD	"BEAM_x_AZIMUTH_ANGLE"

int ConfigBeam(Beam* beam, int beam_number, ConfigList* config_list);

//-----------------------//
// Instrument Simulation //
//-----------------------//

#define PRI_PER_BEAM_KEYWORD			"PRI_PER_BEAM"
#define BEAM_B_TIME_OFFSET_KEYWORD		"BEAM_B_TIME_OFFSET"

int ConfigInstrumentSim(InstrumentSim* instrument_sim,
	ConfigList* config_list);

//-----------------------//
// Spacecraft Simulation //
//-----------------------//

#define SEMI_MAJOR_AXIS_KEYWORD			"SEMI_MAJOR_AXIS"
#define ECCENTRICITY_KEYWORD			"ECCENTRICITY"
#define INCLINATION_KEYWORD				"INCLINATION"
#define LONG_OF_ASC_NODE_KEYWORD		"LONG_OF_ASC_NODE"
#define ARGUMENT_OF_PERIGEE_KEYWORD		"ARGUMENT_OF_PERIGEE"
#define MEAN_ANOMALY_KEYWORD			"MEAN_ANOMALY"

int ConfigSpacecraftSim(SpacecraftSim* spacecraft_sim,
	ConfigList* config_list);

//--------------------//
// Antenna Simulation //
//--------------------//

#define SPIN_RATE_KEYWORD				"ANTENNA_SPIN_RATE"

int ConfigAntennaSim(AntennaSim* antenna_sim, ConfigList* config_list);

//----//
// L0 //
//----//

#define L0_FILE_KEYWORD					"L0_FILE"

int ConfigL0(L0* l0, ConfigList* config_list);

//----//
// L1 //
//----//

#define L1_FILE_KEYWORD					"L1_FILE"

int ConfigL1(L1* l1, ConfigList* config_list);

//-----//
// L15 //
//-----//

#define L15_FILE_KEYWORD				"L15_FILE"

int ConfigL15(L15* l15, ConfigList* config_list);

#endif
