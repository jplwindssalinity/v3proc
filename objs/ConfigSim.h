//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef CONFIGSIM_H
#define CONFIGSIM_H

static const char rcs_id_configsim_h[] =
	"@(#) $Id$";

#include "InstrumentSim.h"

//======================================================================
// DESCRIPTION
//		These functions are used to initialize the instrument
//		simulation and the corresponding sub-simulations.
//======================================================================

//--------------------------------//
// Instrument Simulation Keywords //
//--------------------------------//

#define PRI_PER_BEAM_KEYWORD			"PRI_PER_BEAM"
#define BEAM_B_TIME_OFFSET_KEYWORD		"BEAM_B_TIME_OFFSET"

int ConfigInstrumentSim(InstrumentSim* instrument_sim,
	ConfigList* config_list);

//---------------------------//
// Orbit Simulation Keywords //
//---------------------------//

#define SEMI_MAJOR_AXIS_KEYWORD			"SEMI_MAJOR_AXIS"
#define ECCENTRICITY_KEYWORD			"ECCENTRICITY"
#define INCLINATION_KEYWORD				"INCLINATION"
#define LONG_OF_ASC_NODE_KEYWORD		"LONG_OF_ASC_NODE"
#define ARGUMENT_OF_PERIGEE_KEYWORD		"ARGUMENT_OF_PERIGEE"
#define MEAN_ANOMALY_KEYWORD			"MEAN_ANOMALY"

int ConfigOrbitSim(OrbitSim* orbit_sim, ConfigList* config_list);

//-----------------------------//
// Antenna Simulation Keywords //
//-----------------------------//

#define SPIN_RATE_KEYWORD				"SPIN_RATE"

int ConfigAntennaSim(AntennaSim* antenna_sim, ConfigList* config_list);

//-------------//
// L0 Keywords //
//-------------//

#define L0_FILE_KEYWORD					"L0_FILE"

int ConfigL0(L0* l0, ConfigList* config_list);

#endif
