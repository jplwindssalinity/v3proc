//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef PSCATL1ATOL1B_H
#define PSCATL1ATOL1B_H

static const char rcs_id_pscatl1atol1b_h[] =
    "@(#) $Id$";

#include "PscatL1A.h"
#include "Spacecraft.h"
#include "Ephemeris.h"
#include "L1B.h"
#include "XTable.h"
#include "BYUXTable.h"
#include "LandMap.h"
#include "Pscat.h"


//======================================================================
// CLASSES
//    PscatL1AToL1B
//======================================================================

//======================================================================
// CLASS
//    PscatL1AToL1B
//
// DESCRIPTION
//    The PscatL1AToL1B object is used to convert between Pscat Level
//    1A data and Level 1B data.  It performs all of the engineering
//    unit conversions.
//======================================================================

class PscatL1AToL1B
{
public:

    //--------------//
    // construction //
    //--------------//

    PscatL1AToL1B();
    ~PscatL1AToL1B();

    //------------//
    // conversion //
    //------------//

    int  Convert(PscatL1A* pscat_l1a, Spacecraft* spacecraft, Pscat* pscat,
             Ephemeris* ephemeris, L1B* l1b);
    int  ComputeSigma0Corr(Pscat* qscat, Meas* meas, float Xfactor,
             float Esn_slice, float* Es_slice, float* En_slice);

    //-----------//
    // variables //
    //-----------//

    unsigned long  pulseCount;  // cumulative counter
    XTable         kfactorTable;
    XTable         xTable;
    BYUXTable      BYUX;
    LandMap        landMap;

    int    useKfactor;            // read and use kfactor table
    int    useBYUXfactor;         // read and use Xfactor table
    int    useSpotCompositing;    // make spots by compositing slices
    int    outputSigma0ToStdout;  // output s0 values to stdout
    float  sliceGainThreshold;    // use to decide which slices to process
    int    processMaxSlices;      // maximum number of slices/spot to use
    char*  simVs1BCheckfile;      // holds cross check data

    float   Esn_echo_cal;    // Cal pulse data to be used.
    float   Esn_noise_cal;
    float   En_echo_load;
    float   En_noise_load;
};

#endif
