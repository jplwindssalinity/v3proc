//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef QSCATSIM_H
#define QSCATSIM_H

static const char rcs_id_qscatsim_h[] =
    "@(#) $Id$";

#include "Qscat.h"
#include "LandMap.h"
#include "Distributions.h"
#include "XTable.h"
#include "BYUXTable.h"
#include "Wind.h"
#include "GMF.h"
#include "Meas.h"
#include "CheckFrame.h"

//======================================================================
// CLASSES
//      QscatEvent, QscatSim
//======================================================================

//======================================================================
// CLASS
//      QscatEvent
//
// DESCRIPTION
//      The QscatEvent object contains a QSCAT event time and ID.
//======================================================================

class QscatEvent
{
public:

    //-------//
    // enums //
    //-------//

    enum QscatEventE { NONE, SCATTEROMETER_MEASUREMENT };

    //--------------//
    // construction //
    //--------------//

    QscatEvent();
    ~QscatEvent();

    //-----------//
    // variables //
    //-----------//

    double       time;
    QscatEventE  eventId;
    int          beamIdx;
};

//======================================================================
// CLASS
//      QscatSim
//
// DESCRIPTION
//      The QscatSim object contains the information necessary to
//      simulate the QuikSCAT instrument by operating on the subsystem
//      simulators.
//======================================================================

class QscatSimBeamInfo
{
public:
    //--------------//
    // construction //
    //--------------//

    QscatSimBeamInfo();
    ~QscatSimBeamInfo();

    //-----------//
    // variables //
    //-----------//

    double  txTime;
};

class QscatSim
{
public:
    //--------------//
    // construction //
    //--------------//

    QscatSim();
    ~QscatSim();

    //--------------------//
    // simulation control //
    //--------------------//

    int  Initialize(Qscat* qscat);
    int  DetermineNextEvent(Qscat* qscat, QscatEvent* qscat_event);
    int  ScatSim(Spacecraft* spacecraft, Qscat* qscat, WindField* windfield,
             GMF* gmf, Kp* kp, KpmField* kpmField, L00Frame* l00_frame);
    int  SetL00Spacecraft(Spacecraft* spacecraft, L00Frame* l00_frame);
    int  SetMeasurements(Spacecraft* spacecraft, Qscat* qscat,
             MeasSpot* meas_spot, CheckFrame* cf, WindField* windfield,
             GMF* gmf, Kp* kp, KpmField* kpmField);
    int  SetL00Science(MeasSpot* meas_spot, CheckFrame* cf, Qscat* qscat,
             L00Frame* l00_frame);
    int  ComputeXfactor(Spacecraft* spacecraft, Qscat* qscat, Meas* meas,
             float* X);

    //-----------//
    // variables //
    //-----------//

    double                  startTime;
    QscatSimBeamInfo        beamInfo[NUMBER_OF_QSCAT_BEAMS];
    LandMap                 landMap;
    TimeCorrelatedGaussian  ptgrNoise;
    int                     numLookStepsPerSlice;
    float                   azimuthIntegrationRange;
    float                   azimuthStepSize;
    XTable                  kfactorTable;
    BYUXTable               BYUX;
    XTable                  xTable;
    float                   dopplerBias;
    double                  correlatedKpm;

    char*  simVs1BCheckfile;  // output data for cross check with 1B

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
    int  applyDopplerError;  // simulate doppler tracking error
    int  l00FrameReady;      // indicates a level 0 frame is ready
    int  simKpcFlag;         // 0 = no Kpc, 1 = Kpc
    int  simCorrKpmFlag;     // 0 = no Kpm, 1 = correllated Kpm
    int  simUncorrKpmFlag;   // 0 = no Kpm, 1 = uncorrellated Kpm
    int  simKpriFlag;        // 0 = no Kpri, 1 = Kpri

protected:
    int  _spotNumber;
};

#endif