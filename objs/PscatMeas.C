//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_pscat_c[] =
    "@(#) $Id$";

#include "Pscat.h"
#include "Distributions.h"

//===========//
// PscatMeas //
//===========//

PscatMeas::PscatMeas()
:   Snr(0.0), Sigma0(0.0)
{
    return;
}

PscatMeas::~PscatMeas()
{
    return;
}
