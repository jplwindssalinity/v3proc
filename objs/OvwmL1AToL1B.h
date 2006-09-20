//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef OVWML1ATOL1B_H
#define OVWML1ATOL1B_H

static const char rcs_id_pscatl1atol1b_h[] =
    "@(#) $Id$";

#include "Array.h"
#include "OvwmL1A.h"
#include "Spacecraft.h"
#include "Ephemeris.h"
#include "L1B.h"
#include "SchToXyz.h"
#include "Utils.h"
#include "LandMap.h"
#include "Topo.h"
#include "PointTargetResponseTable.h"
#include "AmbigTable.h"
#include "Ovwm.h"


//======================================================================
// CLASSES
//    OvwmL1AToL1B
//======================================================================

//======================================================================
// CLASS
//    OvwmL1AToL1B
//
// DESCRIPTION
//    The OvwmL1AToL1B object is used to convert between OVWM Level
//    1A data and Level 1B data.  It performs all of the engineering
//    unit conversions.
//======================================================================

class OvwmL1AToL1B
{
public:

    //--------------//
    // construction //
    //--------------//

    OvwmL1AToL1B();
    ~OvwmL1AToL1B();

    int AllocateIntermediateArrays();

    //------------//
    // conversion //
    //------------//

    int Convert(OvwmL1A* ovwm_l1a, Spacecraft* spacecraft, Ovwm* ovwm,
             Ephemeris* ephemeris, Topo* topo, Stable* stable, L1B* l1b);
    int ComputeSigma0(Ovwm* ovwm, Meas* meas, float Xfactor, float Esn_pixel,
                      float Esn_echo, float Esn_noise, float En_echo_load,
                      float En_noise_load, float* Es_pixel, float* En_pixel);


    //-----------//
    // variables //
    //-----------//

    unsigned long  pulseCount;  // cumulative counter
    //XTable         kfactorTable;
    //XTable         xTable;
    //BYUXTable      BYUX;
    LandMap        landMap;

    int    calXfactor;
    //int    useKfactor;            // read and use kfactor table
    //int    useBYUXfactor;         // read and use Xfactor table
    //int    useSpotCompositing;    // make spots by compositing slices
    int    outputSigma0ToStdout;  // output s0 values to stdout
    float  sliceGainThreshold;    // use to decide which slices to process
    int    processMaxSlices;      // maximum number of slices/spot to use
    char*  simVs1BCheckfile;      // holds cross check data

    float   Esn_echo_cal;    // Cal pulse data to be used.
    float   Esn_noise_cal;
    float   En_echo_load;
    float   En_noise_load;

    float   integrationStepSize;
    float   integrationRangeWidthFactor;
    float   integrationAzimuthWidthFactor;

    AmbigTable               ambigTable;
    PointTargetResponseTable ptrTable;

    float   minOneWayGain;         // use to decide which meas to process
    float   minSignalToAmbigRatio; // more ambiguous pixels omitted

    int     simLandFlag;           // 0 = ignore land, 1 = simulate land

protected:

    float** _ptr_array;
    int _max_int_range_bins;
    int _max_int_azim_bins;
};

#endif
