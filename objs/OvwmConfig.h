//==============================================================//
// Copyright (C) 1998-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//
   
#ifndef OVWMCONFIG_H
#define OVWMCONFIG_H

static const char rcs_id_ovwmconfig_h[] =
    "@(#) $Id$";

#include "Ovwm.h"
#include "ConfigList.h"
#include "OvwmSim.h"
#include "AmbigTable.h"
#include "PointTargetResponseTable.h"
#include "OvwmL1A.h"
#include "OvwmL1AToL1B.h"

//======================================================================
// DESCRIPTION
//      These functions are used to initialize OVWM components.
//======================================================================

//----------//
// OvwmSas //
//----------//

int ConfigOvwmSas(OvwmSas* ovwm_sas, ConfigList* config_list);

//----------//
// OvwmSes //
//----------//

int ConfigOvwmSes(OvwmSes* ovwm_ses, ConfigList* config_list);

//----------//
// OvwmCds //
//----------//

int ConfigOvwmCds(OvwmCds* ovwm_cds, ConfigList* config_list);

//-------//
// Ovwm //
//-------//

int ConfigOvwm(Ovwm* ovwm, ConfigList* config_list);


//----------//
// OvwmSim //
//----------//

int ConfigOvwmSim(OvwmSim* ovwm_sim, ConfigList* config_list);

//----------------//
// AmbigTable     //
//----------------//

int ConfigAmbigTable(AmbigTable* atab,ConfigList* config_list);

//------------------------------//
// PointTargetResponseTable     //
//------------------------------//

int ConfigPointTargetResponseTable(PointTargetResponseTable* ptrtab,
				    ConfigList* config_list);

//-------------------------------//
// L1A the OVWM way              //
//-------------------------------//
int ConfigOvwmL1A(Ovwm* ovwm, OvwmL1A* l1a, ConfigList* config_list);

//-------------------------------//
// L1A to L1B for OVWM           //
//-------------------------------//

int ConfigOvwmL1AToL1B(OvwmL1AToL1B* l1a_to_l1b, ConfigList* config_list);

#endif
