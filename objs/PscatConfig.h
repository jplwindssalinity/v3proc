//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef PSCATCONFIG_H
#define PSCATCONFIG_H

static const char rcs_id_pscatconfig_h[] =
    "@(#) $Id$";

#include "Pscat.h"
#include "PscatSim.h"
#include "PscatConfig.h"
#include "PscatL1A.h"
#include "PscatL1AToL1B.h"
#include "ConfigList.h"

//======================================================================
// DESCRIPTION
//      These functions are used to initialize PSCAT components.
//======================================================================

int  ConfigPscat(Pscat* pscat, ConfigList* config_list);
int  ConfigPscatSim(PscatSim* pscat_sim, ConfigList* config_list);
int  ConfigPscatL1A(PscatL1A* pscat_l1a, ConfigList* config_list);
int  ConfigPscatL1AToL1B(PscatL1AToL1B* l1a_to_l1b, ConfigList* config_list);

#endif
