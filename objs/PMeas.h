//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef PMEAS_H
#define PMEAS_H

static const char rcs_id_pscatmeas_h[] =
    "@(#) $Id$";

#include "Meas.h"

//======================================================================
// CLASSES
//    PMeas
//======================================================================

//======================================================================
// CLASS
//      PMeas
//
// DESCRIPTION
//      Subclassed off of Meas.  PMeas contains extra data needed
//      to handle correlation measurements.
//======================================================================

class PMeas : public Meas
{
public:

    //--------------//
    // construction //
    //--------------//

    PMeas();
    ~PMeas();

    //-----------//
    // variables //
    //-----------//

    float Snr;       // The true SNR.
    float Sigma0;    // The true sigma0.

};

#endif
