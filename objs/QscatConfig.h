//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef QSCATCONFIG_H
#define QSCATCONFIG_H

static const char rcs_id_qscatconfig_h[] =
    "@(#) $Id$";

#include "Qscat.h"
#include "ConfigList.h"
#include "QscatSim.h"
#include "QscatSimAccurate.h"

//======================================================================
// DESCRIPTION
//      These functions are used to initialize QSCAT components.
//======================================================================

//----------//
// QscatSas //
//----------//

int ConfigQscatSas(QscatSas* qscat_sas, ConfigList* config_list);

//----------//
// QscatSes //
//----------//

int ConfigQscatSes(QscatSes* qscat_ses, ConfigList* config_list);

//----------//
// QscatCds //
//----------//

int ConfigQscatCds(QscatCds* qscat_cds, ConfigList* config_list);

//-------//
// Qscat //
//-------//

int ConfigQscat(Qscat* qscat, ConfigList* config_list);

//----------//
// QscatSim //
//----------//

int ConfigQscatSim(QscatSim* qscat_sim, ConfigList* config_list);

//------------------//
// QscatSimAccurate //
//------------------//

int ConfigQscatSimAccurate(QscatSimAccurate* qscat_sim,
    ConfigList* config_list);

#endif
