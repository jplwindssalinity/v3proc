//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef CONFIGSIM_H
#define CONFIGSIM_H

static const char rcs_id_configsim_h[] =
	"@(#) $Id$";

#include "SpacecraftSim.h"
#include "XTable.h"
#include "L00.h"
#include "L1A.h"
#include "L1AToL1B.h"
#include "L1B.h"
#include "L2A.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Wind.h"
#include "Grid.h"
#include "ConfigList.h"
#include "Tracking.h"
#include "ConfigSimDefs.h"
#include "Kp.h"

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
int ConfigBYUXTable(BYUXTable* BYUX,ConfigList* config_list);

//-----//
// L00 //
//-----//

int ConfigL00(L00* l00, ConfigList* config_list);

//-----//
// L1A //
//-----//

int ConfigL1A(L1A* l1a, ConfigList* config_list);

//-----//
// L1B //
//-----//

int ConfigL1B(L1B* l1b, ConfigList* config_list);

//--------//
// L1BHdf //
//--------//

int ConfigL1BHdf(L1BHdf* l1bHdf, ConfigList* config_list);

//----------//
// L1AToL1B //
//----------//

int ConfigL1AToL1B(L1AToL1B* l1atol1b, ConfigList* config_list);

//-----//
// L2A //
//-----//

int ConfigL2A(L2A* l2a, ConfigList* config_list);

//-----//
// L2B //
//-----//

int ConfigL2B(L2B* l2b, ConfigList* config_list);

//----------//
// L2AToL2B //
//----------//

int ConfigL2AToL2B(L2AToL2B* l2a_to_l2b, ConfigList* config_list);

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

//----//
// Kp //
//----//

int ConfigKp(Kp* kp, ConfigList* config_list);

//----------//
// KpmField //
//----------//

int ConfigKpmField(KpmField* kpmField, ConfigList* config_file);

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
