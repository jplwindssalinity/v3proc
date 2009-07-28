 //=============================================================//
// Copyright (C) 1997-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef GMF_H
#define GMF_H

static const char rcs_id_gmf_h[] =
    "@(#) $Id$";

#include "MiscTable.h"
#include "Wind.h"
#include "Meas.h"
#include "Constants.h"
#include "Kp.h"


//======================================================================
// CLASSES
//    GMF
//======================================================================

//======================================================================
// CLASS
//    GMF
//
// DESCRIPTION
//    The GMF object contains a geophysical model function in tabular
//    format.  It contains methods to access the model function as well
//    as methods for performing wind retrieval.
//======================================================================

#define DEFAULT_SPD_TOL            0.1
#define DEFAULT_SEP_ANGLE          5.0*dtr
#define DEFAULT_SMOOTH_ANGLE       10.0*dtr
#define DEFAULT_MAX_SOLUTIONS      4
#define DEFAULT_PHI_COUNT          360
//#define S2_DEBUG_INTERVAL        17
//#define S2_DETAILED_DEBUG        88
#define MINIMUM_WVC_MEASUREMENTS   4

class GMF : public MiscTable
{
public:
    //--------------//
    // construction //
    //--------------//

    GMF();
    ~GMF();

    //-----------------//
    // setting/getting //
    //-----------------//

    int  SetPhiCount(int phi_count);
    int  SetSpdTol(float spd_tol);
    int  SetCBandWeight(float wt);

    //--------------//
    // input/output //
    //--------------//

    // high level functions- use these    
    int  ReadOldStyle(const char* filename);
    int  ReadQScatStyle(const char*  filename);
    int  ReadHighWind(const char* filename);
    int  ReadKuAndC(const char* ku_filename,const char* c_filename);
    int  ReadCBand(const char* filename);
    int  ReadPolarimetric(const char* filename);

    // internal worker functions- don't use these directly
    int GMF::_ReadArrayFile(const char*  filename,
        bool mirrorChiValues = false, bool discardFirstVal = false);
    int GMF::_ReadArrayFileLoop(const char*  filename, 
        bool mirrorChiValues = false, bool discardFirstVal = false,
        int met_idx_start = -1, int n_met = -1);


    //---------//
    // analyze //
    //---------//

    int    GetCoefs(Meas::MeasTypeE met, float inc, float spd, float* A0,
               float* A1, float* A1_phase, float* A2, float* A2_phase,
               float* A3, float* A3_phase, float* A4, float* A4_phase);
    int    GetObjLimits(float* min_obj, float* max_obj);
    int    WriteSolutionCurves(FILE* ofp, MeasList* meas_list, Kp* kp);
    int    WritePdf(FILE* ofp, MeasList* meas_list, Kp* kp);
    int    WriteObjectiveCurve(FILE* ofp, float min_obj, float max_obj);
    int    WriteGSObjectiveCurve(FILE* ofp, float min_obj, float max_obj);
    int    AppendSolutions(FILE* ofp, WVC* wvc, float min_obj, float max_obj);
    int    WriteObjXmgr(char* basename, int panelcount, WVC* wvc);
    float  GetVariance(Meas* meas, float spd, float chi, float trial_sigma0,
               Kp* kp);

    //----------------//
    // wind retrieval //
    //----------------//

    int  CheckRetrieveCriteria(MeasList* meas_list);
    int  RetrieveWinds_PE(MeasList* meas_list, Kp* kp, WVC* wvc);
    int  RetrieveManyWinds(MeasList* meas_list, Kp* kp, WVC* wvc);
    int  RetrieveWindsWithPeakSplitting(MeasList* meas_list, Kp* kp,
             WVC* wvc, float one_peak_width,
             float two_peak_separation_threshold, float threshold,
             int max_num_ambigs);
    int  SolutionCurve(MeasList* meas_list, Kp* kp);
    int  Smooth();
    int  FindMaxima(WVC* wvc);
    int  FindMany(WVC* wvc);

    //------------------------//
    // special wind retrieval //
    //------------------------//

    int    GetHHBiasUsingVV(MeasList* meas_list, Kp* kp, float* bias);
    int    RetrieveWinds_H1(MeasList* meas_list, Kp* kp, WVC* wvc);
    int    RetrieveWinds_H2(MeasList* meas_list, Kp* kp, WVC* wvc,
               int h3_and_s1_flag = 0);
    int    RetrieveWinds_S2(MeasList* meas_list, Kp* kp, WVC* wvc);
    int    RetrieveWinds_S3(MeasList* meas_list, Kp* kp, WVC* wvc,
			    int s4_flag = 0,float prior_dir=0);
    int    RetrieveWinds_CoastSpecial(MeasList* meas_list, Kp* kp, WVC* wvc,
				      int s4_flag = 0, int dirth_flag=1);
    int    RetrieveWinds_S3Rain(MeasList* meas_list, Kp* kp, WVC* wvc,float prior_dir=0);
    int    RetrieveWinds_HurrSp1(MeasList* meas_list, Kp* kp, WVC* wvc);
    int    BuildDirectionRanges(WVC* wvc, float threshold);
    int    BuildDirectionRangesByMSE(WVC* wvc, float threshold);
    int    BruteForceGetMinEstimateMSE(float* peak_dir, int num_peaks,
               float* mse, int level = 0, float* tmp_peak_dir = NULL);
    int    GetMinEstimateMSE(float* peak_dir, int num_peaks, float* mse,
               int num = 0);
    int    DeleteBadPeaks(WVC* wvc, float* peak_dir, int* num_peaks,
               float mse);
    float  EstimateDirMSE(float* peak_dir, int num_peaks);
    float  EstimateDirMSE(AngleIntervalListPlus* alp);
    int    ConvertObjToPdf();
    int    SolutionCurve_H1(MeasList* meas_list, Kp* kp);
    int    FindBestSpeed(MeasList* meas_list, Kp* kp, float dir,
               float low_speed, float high_speed, float* best_speed,
               float* best_obj);

    //-------------------//
    // GS wind retrieval //
    //-------------------//

    int  RetrieveWinds_GS(MeasList* meas_list, Kp* kp, WVC* wvc,
			  int polar_special=0, float prior_dir=0);
    int  Calculate_Init_Wind_Solutions(MeasList* meas_list, Kp* kp, WVC* wvc,
				       int polar_special=0, float prior_dir=0);
    int  FindMultiSpeedRidge(MeasList* meas_list, Kp* kp, int dir_idx,
			     float* max_sep, float* min_sep, float prior_dir=0);
    int  RemoveBadCopol(MeasList* meas_list, Kp* kp);
    int  Optimize_Wind_Solutions(MeasList* meas_list, Kp* kp, WVC* wvc, float prior_dir=0);
    int  CopyBuffersGSToPE();

    //-----------------------//
    // Brute Force Retrieval   //
    //-----------------------//
    int  RetrieveWinds_BruteForce(MeasList* meas_list, Kp* kp, WVC* wvc,
				  int polar_special=0, float spdmin=-1,
				  float spdmax=-1,float prior_dir=0);
    void CalculateSigma0Weights(MeasList* meas_list);
    

    //-------//
    // flags //
    //-------//

    int  retrieveUsingKpcFlag;
    int  retrieveUsingKpmFlag;
    int  retrieveUsingKpriFlag;
    int  retrieveUsingKprsFlag;
    int  retrieveUsingLogVar;
    int  retrieveOverIce;
    int  retrieveOverCoast;
    int  smartNudgeFlag;
    int  retrieveUsingCriteriaFlag;

    float minimumAzimuthDiversity;

    float cBandWeight;
    float kuBandWeight;

    int  objectiveFunctionMethod;

    //protected:

    //----------------//
    // wind retrieval //
    //----------------//

    float  _ObjectiveFunction(MeasList* meas_list, float u, float phi, Kp* kp, float phi_prior=0.0);
    float  _ObjectiveFunctionOld(MeasList* meas_list, float u, float phi, Kp* kp);
    float  _ObjectiveFunctionNew(MeasList* meas_list, float u, float phi, Kp* kp);

    float  _ObjectiveFunctionDirPrior(MeasList* meas_list, float u, float phi, Kp* kp, float phi_prior=0);

    float  _ObjectiveFunctionMeasVar(MeasList* meas_list, float u, float phi, Kp* kp);
    float  _ObjectiveFunctionMeasVarWt(MeasList* meas_list, float u, float phi, Kp* kp);

    float  _ObjectiveFunctionFixedTrial(MeasList* meas_list, float u, float phi, Kp* kp, float fixed_sigma0);
    int    _ObjectiveToProbability(float scale, int radius);

    //-----------//
    // variables //
    //-----------//

    int    _phiCount;         // number of angles for wind retrieval
    float  _phiStepSize;      // step size of angles
    float  _spdTol;           // speed tolerance for golden section search
    float  _sepAngle;         // minimum angle between solutions
    float  _smoothAngle;      // widest angle of smoothing obj
    int    _maxSolutions;     // the maximum number of solutions

    float*  _bestSpd;    // array to hold best speed for each direction
    float*  _bestObj;    // array to hold best objective for each direction
    float*  _copyObj;    // storage for a copy of obj

    //----------------------//
    // GS related variables //
    //----------------------//

    float*  _speed_buffer;        // array to hold best speed for each dir
    float*  _objective_buffer;    // array to hold best objective for each dir
    int*    _dir_mle_maxima;      // storage for a copy of obj
};

#endif
