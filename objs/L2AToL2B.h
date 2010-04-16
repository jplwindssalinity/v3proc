//==============================================================//
// Copyright (C) 1997-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef L2ATOL2B_H
#define L2ATOL2B_H

static const char rcs_id_l2atol2b_h[] =
    "@(#) $Id$";

#include "L2A.h"
#include "L2B.h"
#include "GMF.h"
#include "MLPData.h"
#include "MLP.h"


class MLPDataArray;

#define DESIRED_SOLUTIONS  4

//======================================================================
// CLASSES
//     L2AToL2B
//======================================================================

//======================================================================
// CLASS
//      L2AToL2B
//
// DESCRIPTION
//      The L2AToL2B object is used to convert between Level 2A data
//      and Level 2B data.  It performs wind retrieval.
//======================================================================

class L2AToL2B
{
public:

    //------//
    // enum //
    //------//

    enum WindRetrievalMethodE { GS, GS_FIXED, H1, H2, H3, S1, S2, S3, S4,
				POLAR_SPECIAL, CHEAT , S3RAIN, CoastSpecial,
                                CoastSpecialGS, HurrSp1};

    //--------------//
    // construction //
    //--------------//

    L2AToL2B();
    ~L2AToL2B();

    //---------//
    // setting //
    //---------//

    int  SetWindRetrievalMethod(const char* wr_method);

    //------------//
    // conversion //
    //------------//
    int ReadNudgeArray(char* filename);
    float  NeuralNetRetrieve(L2A* l2a, L2B* l2b, MLPDataArray* spdnet, MLPDataArray* dirnet, GMF* gmf, Kp* kp, int need_all_looks);
    float  HybridNeuralNetRetrieve(L2A* l2a, L2B* l2b, MLPDataArray* spdnet, MLPDataArray* dirnet, GMF* gmf, Kp* kp, int need_all_looks);
    int convertMeasToMLP_IOType(Meas* meas, char *type, char *out_buf);
    int MakeAmbigsFromDirectionArrays(WVC* wvc, float diroff);
    int BuildDirectionRanges(WVC* wvc, float thresh);
    float GetNeuralDirectionOffset(L2A* l2a);
    float GetSpacecraftVelocityAngle(float atd, float ctd);
    int  ConvertAndWrite(L2A* l2a, GMF* gmf, Kp* kp, L2B* l2b);
    int  InitAndFilter(L2B* l2b);
    int  InitFilterAndFlush(L2B* l2b);

    //------------------------------------------//
    // Routine for outputting the Nudge Field   //
    // wind vector                              //
    //------------------------------------------//

    int  Cheat(MeasList* meas_list, WVC* wvc);
    int  HurrSp1Top(GMF* gmf, Kp* kp, MeasList* meas_list, WVC* wvc);

    //-----------//
    // debugging //
    //-----------//

    int  WriteSolutionCurves(L2A* l2a, GMF* gmf, Kp* kp,
             const char* output_file);

    //----------------------//
    // processing variables //
    //----------------------//

    int  medianFilterWindowSize;
    int  medianFilterMaxPasses;
    int  maxRankForNudging;

    //----------------------------//
    // Kprc noise distribution    //
    //----------------------------//

    Gaussian kprc;
    //-------//
    // flags //
    //-------//

    int                   useManyAmbiguities;
    int                   useAmbiguityWeights;
    int                   useNudging;
    int                   smartNudgeFlag;
    WindRetrievalMethodE  wrMethod;
    int                   useNudgeThreshold;
    int                   useNMF;
    int                   useRandomInit;
    int                   useNudgeStream;

     
    //-----------------------------------------//
    // Parameters for Peak Splitting Algorithm //
    //-----------------------------------------//

    float  onePeakWidth;
    float  twoPeakSep;
    float  probThreshold;

    //-------------------//
    // nudging variables //
    //-------------------//

    WindField        nudgeField;
    WindVectorField  nudgeVctrField;
    float            nudgeThresholds[2];
    float            streamThreshold;

    //----------------------------------------------//
    // Auxiliary Variables For Hurricane Processing //
    //----------------------------------------------//

    int            useHurricaneNudgeField;
    WindField      hurricaneField;
    float          hurricaneRadius;  // km
    EarthPosition  hurricaneCenter;
    MLP            s0corr_mlp; // multilayer perceptron, used for the 
                        // hybrid ann correction of sigma0
    MLP            errEst_mlp; // for estimating the error of the retrieval

    int                   ann_train_ati;
    float                   ann_train_diroff;
    char                  *ann_sigma0_corr_file, *ann_error_est_file;
    int                   useSigma0Weights;
    float                 sigma0WeightCorrLength;
    float                 atdToNadirLat[360];
    float                 orbitInclination;
    float                 groundTrackLength;
    int arrayNudgeFlag;
 protected:
    float computeGroundTrackParameters();
    int _phiCount;

    float** arrayNudgeSpd;
    float** arrayNudgeDir;
    int arrayNudgeNati;
    int arrayNudgeNcti;
};

#endif
