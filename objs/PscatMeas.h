//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef PSCATMEAS_H
#define PSCATMEAS_H

static const char rcs_id_pscatmeas_h[] =
    "@(#) $Id$";

#include "Meas.h"

//======================================================================
// CLASSES
//    PscatMeas
//======================================================================

//======================================================================
// CLASS
//      PscatMeas
//
// DESCRIPTION
//      Subclassed off of Meas.  PscatMeas contains extra data needed
//      to handle correlation measurements.
//======================================================================

class PscatMeas : public Meas
{
public:

    //--------------//
    // construction //
    //--------------//

    PscatMeas();
    ~PscatMeas();

    //-----------//
    // variables //
    //-----------//

    float Snr;       // The true SNR.
    float Sigma0;    // The true sigma0.

};

#endif
