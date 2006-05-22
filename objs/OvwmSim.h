//==============================================================//
// Copyright (C) 1998-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//
   
#ifndef OVWMSIM_H
#define OVWMSIM_H

static const char rcs_id_ovwmsim_h[] =
    "@(#) $Id$";

#include "Ovwm.h"
#include "LandMap.h"
#include "Distributions.h"
#include "XTable.h"
#include "BYUXTable.h"
#include "Wind.h"
#include "GMF.h"
#include "Meas.h"
#include "CheckFrame.h"
#include "L1AFrame.h"
#include "Sigma0Map.h"
#include "L1B.h"
#include "SchToXyz.h"
#include "Utils.h"
#include "AmbigTable.h"

//======================================================================
// CLASSES
//    OvwmSim
//======================================================================

//======================================================================
// CLASS
//    OvwmSim
//
// DESCRIPTION
//    The OvwmSim object contains the information necessary to
//    simulate the QuikSCAT instrument by operating on the subsystem
//    simulators.
//======================================================================

class OvwmSimBeamInfo
{
public:
    //--------------//
    // construction //
    //--------------//

    OvwmSimBeamInfo();
    ~OvwmSimBeamInfo();

    //-----------//
    // variables //
    //-----------//

    double  txDelay;
    double  rxTime;
    double  lastRxTime;
    double  txTime;
};

class OvwmSim
{
public:
    //--------------//
    // construction //
    //--------------//

    OvwmSim();
    ~OvwmSim();

    //--------------------//
    // simulation control //
    //--------------------//

    int  Initialize(Ovwm* ovwm);
    int  DetermineNextEvent(int spots_per_frame, Ovwm* ovwm,
             OvwmEvent* ovwm_event);
    int  L1AFrameInit(Spacecraft* spacecraft, Ovwm* ovwm,
             L1AFrame* l1aframe);
    int  ScatSim(Spacecraft* spacecraft, Ovwm* ovwm, WindField* windfield,
             Sigma0Map* inner_map, Sigma0Map* outer_map, GMF* gmf, Kp* kp,
             KpmField* kpmField, Topo* topo, Stable* stable,
             L1AFrame* l1a_frame, PointTargetResponseTable *ptrTable, AmbigTable* ambigTable, L1B* l1b=NULL);
    int  CheckTiming(Ovwm* ovwm);
    int  LoopbackSim(Spacecraft* spacecraft, Ovwm* ovwm,
             L1AFrame* l1a_frame);
    int  LoadSim(Spacecraft* spacecraft, Ovwm* ovwm, L1AFrame* l1a_frame);
    int  SetL1ASpacecraft(Spacecraft* spacecraft, L1AFrame* l1a_frame);
    int  SetMeasurements(Spacecraft* spacecraft, Ovwm* ovwm,
             MeasSpot* meas_spot, WindField* windfield, Sigma0Map* inner_map,
             Sigma0Map* outer_map, GMF* gmf, Kp* kp, KpmField* kpmField,
             Topo* topo, Stable* stable, CheckFrame* cf,
             PointTargetResponseTable *ptrTable, AmbigTable *ambigTable, int sim_l1b_direct);
    int  SetL1AScience(MeasSpot* meas_spot, CheckFrame* cf, Ovwm* ovwm,
             L1AFrame* l1a_frame);
    int  SetL1ALoopback(Ovwm* ovwm, L1AFrame* l1a_frame);
    int  SetL1ALoad(Ovwm* ovwm, L1AFrame* l1a_frame);
    int  ComputeXfactor(Spacecraft* spacecraft, Ovwm* ovwm, Meas* meas,
             float* X);
    int  MeasToEsnX(Ovwm* ovwm, Meas* meas, float X, float sigma0,
             float* Esn, float* Es, float* En, float* var_Esn);
    int  MeasToEsnK(Spacecraft* spacecraft, Ovwm* ovwm, Meas* meas,
             float K, float sigma0, float* Esn, float* Es, float* En,
             float* var_Esn, float* X);

    int  GetSpotNumber()  { return (_spotNumber); };
    int  AllocateIntermediateArrays();
    
    //-----------//
    // variables //
    //-----------//

    unsigned long            pulseCount; // cumulative counter
    double                   epochTime;  // used for setting time strings
    char*                    epochTimeString;
    double                   epochOffset; //epochtimestring, added to epoch time    
    double                   latMin;
    double                   latMax;
    double                   lonMin;
    double                   lonMax;

    double                   startTime;
    OvwmSimBeamInfo          beamInfo[NUMBER_OF_OVWM_BEAMS];
    OvwmEvent::OvwmEventE    lastEventType;
    unsigned short           lastEventIdealEncoder;
    LandMap                  landMap;
    int                      simCoast;
    TimeCorrelatedGaussian   ptgrNoise;
    int                      numLookStepsPerSlice;
    float                    azimuthIntegrationRange;
    float                    azimuthStepSize;
    XTable                   kfactorTable;
    BYUXTable                BYUX;
    XTable                   xTable;
    AmbigTable               ambigTable;
    PointTargetResponseTable ptrTable;
    float                    dopplerBias;
    double                   correlatedKpm;
    float                    landSigma0[NUMBER_OF_OVWM_BEAMS];

    char*  simVs1BCheckfile;  // output data for cross check with 1B

    //----------//
    // feedback //
    //----------//

    int    spotLandFlag;    // 0=ocean, 1=mixed, 2=land
    float  maxSigma0;       // for the spot

    //-------//
    // flags //
    //-------//

    int    simLandFlag;        // 0 = ignore land, 1 = simulate land
    int    uniformSigmaField;  // set all sigma0 values to a constant value
    float  uniformSigmaValue;  // the sigma0 when uniformSigmaField is set
    int    outputXToStdout;    // write X value to stdout
    int    useKfactor;         // read and use K-factor table
    int    createXtable;       // create an X table
    int    computeXfactor;     // compute X-factor
    int    useBYUXfactor;      // read and use Xfactor table
    int    rangeGateClipping;  // simulate range gate clipping
    int    applyDopplerError;  // simulate doppler tracking error
    int    l1aFrameReady;      // indicates a level 0 frame is ready
    int    simKpcFlag;         // 0 = no Kpc, 1 = Kpc
    int    simCorrKpmFlag;     // 0 = no Kpm, 1 = correllated Kpm
    int    simUncorrKpmFlag;   // 0 = no Kpm, 1 = uncorrellated Kpm
    int    simKpriFlag;        // 0 = no Kpri, 1 = Kpri
    int    simHiRes;           // 0 = quickSim, 1 = high resolution sim
    float    integrationStepSize; // step size of high res sim km
    float    integrationRangeWidthFactor; // multiple of range width to 
                                          // integrate over
    float    integrationAzimuthWidthFactor; // multiple of range width to 
                                            // integrate over
    float    minOneWayGain;    // lower gain pixels omitted for L1A/L1B
    float    minSignalToAmbigRatio; // more ambiguous pixels omitted 
                                    // from L1A/L1B
    int    spot_check_beam_number ;// spot check beam number
    int    spot_check_scan_angle;// spot check scan angle
    bool     spot_check_generate_map;//true when  map generation is requested

protected:
    int  _spotNumber;
    int  _spinUpPulses;      // first two pulses just do tracking
    int  _calPending;        // A cal pulse is waiting to execute.
    float** _ptr_array;      // point target response array
    int  _max_int_range_bins; // first dim of _ptr_array
    int  _max_int_azim_bins; // second dim of _ptr_array

    float** amb_map_ ; //amb ratio map inside beam foot print
    float** X_map_; //X map inside beam foot print
    float** kpc_map_;//kpc map
    float** gain_map_; //gain map inside beam foot print
   
};

#endif
