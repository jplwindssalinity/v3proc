//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef PSCATSIM_H
#define PSCATSIM_H

static const char rcs_id_pscatsim_h[] =
    "@(#) $Id$";

#include "Pscat.h"
#include "LandMap.h"
#include "Distributions.h"
#include "XTable.h"
#include "BYUXTable.h"
#include "Wind.h"
#include "GMF.h"
#include "Meas.h"
#include "PscatL1A.h"

//======================================================================
// CLASSES
//      PscatSim
//======================================================================

//======================================================================
// CLASS
//      PscatSim
//
// DESCRIPTION
//      The PscatSim object contains the information necessary to
//      simulate the PolSCAT instrument by operating on the subsystem
//      simulators.
//======================================================================

class PscatSim
{
public:
    //--------------//
    // construction //
    //--------------//

    PscatSim();
    ~PscatSim();

    //--------------------//
    // simulation control //
    //--------------------//

    int  Initialize(Pscat* pscat);
    int  DetermineNextEvent(Pscat* pscat, PscatEvent* pscat_event);
    int  L1AFrameInit(Spacecraft* spacecraft, Pscat* pscat,
             PscatL1AFrame* l1aframe);
    int  ScatSim(Spacecraft* spacecraft, Pscat* pscat,
             PscatEvent* pscat_event, WindField* windfield, GMF* gmf, Kp* kp,
             KpmField* kpmField, PscatL1AFrame* pscat_l1a_frame);
    int  SetL1ASpacecraft(Spacecraft* spacecraft,
             PscatL1AFrame* pscat_l1a_frame);
    int  SetMeasTypes(PscatEvent* pscat_event, MeasSpot* meas_spot);
    int  SetMeasurements(Spacecraft* spacecraft, Pscat* pscat,
             PscatEvent* pscat_event, MeasSpot* meas_spot,
             WindField* windfield, GMF* gmf, Kp* kp, KpmField* kpmField);
    int  SetL1AScience(MeasSpot* meas_spot, Pscat* pscat,
             PscatEvent* pscat_event, PscatL1AFrame* pscat_l1a_frame);
    int  ComputeXfactor(Spacecraft* spacecraft, Pscat* pscat, Meas* meas,
             float* X);

    //-----------//
    // variables //
    //-----------//

    double                   epochTime;  // used for setting time strings
    char*                    epochTimeString;
    double                   startTime;
    // beamInfo contains the eventTime of the next event, but the eventId
    // of the current (previous event)
    PscatEvent               beamInfo[NUMBER_OF_QSCAT_BEAMS];  // hybrid
    unsigned short           lastEventIdealEncoder;
    LandMap                  landMap;
    TimeCorrelatedGaussian   ptgrNoise;
    int                      numLookStepsPerSlice;
    float                    azimuthIntegrationRange;
    float                    azimuthStepSize;
    XTable                   kfactorTable;
    BYUXTable                BYUX;
    XTable                   xTable;
    float                    dopplerBias;
    double                   correlatedKpm;

    //-------//
    // flags //
    //-------//

    int  uniformSigmaField;  // set all sigma0 values to 1.0
    int  outputXToStdout;    // write X value to stdout
    int  useKfactor;         // read and use K-factor table
    int  createXtable;       // create an X table
    int  computeXfactor;     // compute X-factor
    int  useBYUXfactor;      // read and use Xfactor table
    int  rangeGateClipping;  // simulate range gate clipping
    int  l1aFrameReady;      // indicates a level 0 frame is ready
    int  simKpcFlag;         // 0 = no Kpc, 1 = Kpc
    int  simCorrKpmFlag;     // 0 = no Kpm, 1 = correllated Kpm
    int  simUncorrKpmFlag;   // 0 = no Kpm, 1 = uncorrellated Kpm
    int  simKpriFlag;        // 0 = no Kpri, 1 = Kpri

protected:
    int  _spotNumber;
    int  _spinUpPulses;      // first two pulses just do tracking
};

#endif
