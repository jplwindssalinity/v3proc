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
#include "ConfigSimDefs.h"

//======================================================================
// DESCRIPTION
//		These functions are used to initialize components and their
//		simulators.
//======================================================================

//------------//
// Spacecraft //
//------------//

int ConfigSpacecraft(Spacecraft* spacecraft, ConfigList* config_list);
int ConfigSpacecraftSim(SpacecraftSim* spacecraft_sim,
		ConfigList* config_list);

//------------------------------//
// Attitude Control Error Model //
//------------------------------//

int ConfigAttitudeControlModel(AttDist* attctnl,
	ConfigList* config_list);

//--------------------------------//
// Attitude Knowledge Error Model //
//--------------------------------//

int ConfigAttitudeKnowledgeModel(AttDist* attknow,
	ConfigList* config_list);


//------------//
// Instrument //
//------------//

int ConfigInstrument(Instrument* instrument, ConfigList* config_list);
int ConfigInstrumentSim(InstrumentSim* instrument_sim,
		ConfigList* config_list);

//-----------------------//
// InstrumentSimAccurate //
//-----------------------//

int ConfigInstrumentSimAccurate(InstrumentSimAccurate* instrument_sim,
	ConfigList* config_list);

//------------//
// AntennaSim //
//------------//

int ConfigAntennaSim(AntennaSim* antenna_sim, ConfigList* config_list);

//---------//
// Antenna //
//---------//

int ConfigAntenna(Antenna* antenna, ConfigList* config_list);

//------//
// Beam //
//------//

int ConfigBeam(Beam* beam, int beam_number, ConfigList* config_list);

//--------//
// XTable //
//--------//

int ConfigXTable(XTable* xTable, ConfigList* config_list, char* read_write);

//-----//
// L00 //
//-----//

int ConfigL00(L00* l00, ConfigList* config_list);

//-----//
// L10 //
//-----//

int ConfigL10(L10* l10, ConfigList* config_list);

//-----//
// L15 //
//-----//

int ConfigL15(L15* l15, ConfigList* config_list);

//----------//
// L10ToL15 //
//----------//

int ConfigL10ToL15(L10ToL15* l10tol15, ConfigList* config_list);

//-----//
// L17 //
//-----//

int ConfigL17(L17* l17, ConfigList* config_list);

//----------//
// L17ToL20 //
//----------//

int ConfigL17ToL20(L17ToL20* l17_to_l20, ConfigList* config_list);

//-----//
// L20 //
//-----//

int ConfigL20(L20* l20, ConfigList* config_list);

//-----------//
// Ephemeris //
//-----------//

int ConfigEphemeris(Ephemeris* ephemeris, ConfigList* config_list);

//-----------//
// WindField //
//-----------//

int ConfigWindField(WindField* windfield, ConfigList* config_list);

//-----//
// GMF //
//-----//

int ConfigGMF(GMF* gmf, ConfigList* config_file);

//------//
// Grid //
//------//

int ConfigGrid(Grid* grid, ConfigList* config_list);

//---------------//
// Control Stuff //
//---------------//

int		ConfigControl(SpacecraftSim* spacecraft_sim, ConfigList* config_list,
			double* grid_start_time, double* grid_end_time,
			double* instrument_start_time, double* instrument_end_time,
			double* spacecraft_start_time, double* spacecraft_end_time);

#endif
