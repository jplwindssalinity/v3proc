//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef WIND_SWATH_H
#define WIND_SWATH_H

static const char rcs_id_wind_swath_h[] =
    "@(#) $Id$";

#include "Wind.h"

#define NWP_SPEED_CORRECTION  0.84

//======================================================================
// CLASSES
//    WindSwath
//======================================================================

//======================================================================
// CLASS
//    WindSwath
//
// DESCRIPTION
//    The WindSwath object hold an ambiguous wind field gridded in
//    along track and cross track.
//======================================================================

class WindSwath : public LonLatWind
{
public:

    //--------------//
    // construction //
    //--------------//

    WindSwath();
    ~WindSwath();

    int  Allocate(int cross_track_bins, int along_track_bins);

    //----------//
    // building //
    //----------//

    int   Add(int cti, int ati, WVC* wvc);
    WVC*  Remove(int cti, int ati);

    //---------------------//
    // setting and getting //
    //---------------------//

    int   GetCrossTrackBins()  { return(_crossTrackBins); };
    int   GetAlongTrackBins()  { return(_alongTrackBins); };
    int   GetNumCellsSelected();
    int   GetNumCellsWithAmbiguities();
    int   GetMaxAmbiguityCount();
    WVC*  GetWVC(int cti, int ati);
    WVC*  GetGoodWVC(int cti, int ati);
    int   ReadFlagFile(const char* flag_file);

    //---------//
    // freeing //
    //---------//

    int  DeleteWVCs();
    int  DeleteEntireSwath();
    int  DeleteFlaggedData();
    int  DeleteFlaggedData(const char* flag_file, int use_thresh,
             float threshold_both, float threshold_outer);
    int  DeleteLatitudesOutside(float low_lat, float high_lat);
    int  DeleteDirectionOutliers(float max_dir_err, WindField* truth);
    int  DeleteSpeedOutliers(float max_spd_err, WindField* truth);
    int  DeleteLongitudesOutside(float low_lon, float high_lon);

    //--------------//
    // input/output //
    //--------------//

    int  WriteL2B(FILE* fp);
    int  ReadL2B(FILE* fp);
    int  ReadL2B(const char* filename);
    int  ReadNscatSwv25(const char* filename);
    int  GetSpdDirNumSel(float** spd, float** dir, int** num_ambig,
             int** selected);
    int  UpdateHdf(const char* filename, float** spd, float** dir,
             int** num_ambig, int** selected);
    int  WriteVctr(const char* filename, const int rank);
    int  WriteFlower(const char* filename);
    int  WriteAscii(const char* filename);
    int  WriteAscii(FILE* fp);

    //-----------//
    // filtering //
    //-----------//

    int    InitWithRank(int rank);
    int    InitWithNudge();
    int    InitRandom();
    int    HideSpeed(float min_speed, float max_speed);
    int    UnInitSpeed(float min_speed, float max_speed);
    int    GetNudgeVectors(WindField* nudge_field);
    int    GetNudgeVectors(float** spd, float** dir, int nati, int ncti);
    int    GetNudgeVectors(WindVectorField* nudge_field);
    int    GetHurricaneNudgeVectors(WindField* nudge_field,
               EarthPosition* center, float radius);
    int    Nudge(int min_rank);
    int    StreamNudge(float stream_thresh);
    int    HurricaneNudge(int min_rank, EarthPosition* center, float radius);
    int    S3Nudge();
    int    ThresNudge(int min_rank, float thres[2]);
    int    LoResNudge(WindVectorField* nudge_field, int min_rank);
    int    SmartNudge(WindField* nudge_field);
    int    MedianFilter(int window_size, int max_passes, int bound,
               int weight_flag = 0, int special = 0, int freeze = 0);
    int    MedianFilter_4Pass(int window_size, int max_passes, int bound,
               int weight_flag = 0, int special = 0, int freeze = 0);               
    int    BestKFilter(int window_size, int k);
    int    MedianFilterPass(int half_window, WindVectorPlus*** selected,
               char** change, int bound, int weight_flag = 0, int special = 0,
               int freeze = 0);
    int    MedianFilter4Pass_Pass(int half_window, WindVectorPlus*** selected,
               char** change, char** filter, char** influence, int bound, 
               int weight_flag = 0 );               
    int    BestKFilterPass(int half_window, int k,
               WindVectorPlus*** new_selected, float** prob, float* best_prob);
    int    BestKFilterSubPass(int half_window, WindVectorPlus*** new_selected,
               int* num_wrong, char** change);
    int    GetMedianBySorting(WindVectorPlus* wvp, int cti_min, int cti_max,
               int ati_min, int ati_max);
    float  GetMostProbableDir(WindVectorPlus* wvp, int cti, int ati,
               int cti_min, int cti_max, int ati_min, int ati_max);
    float  GetMostProbableAmbiguity(WindVectorPlus** wvp, int cti, int ati,
               int cti_min, int cti_max, int ati_min, int ati_max);
    int    GetWindowMean(WindVectorPlus* wvp, int cti_min, int cti_max,
               int ati_min, int ati_max);
    int    DiscardUnselectedRanges();
    int    Shotgun(int angle_window_size, int blast_window_size);

    //------------//
    // evaluation //
    //------------//

    void  operator-=(const WindSwath& w);
    int   DifferenceFromTruth(LonLatWind* truth);
    int   ExcessDifference(WindField* truth);
    int   CtdArray(float cross_track_res, float* ctd_array);
    int   DirArray(int number_of_bins, float* dir_array);

    float  RmsSpdErr(WindField* truth);
    float  RmsDirErr(WindField* truth);
    int    WriteMaxDirErrIndices(WindField* truth, FILE* ofp);
    int    WriteDirErrMap(WindField* truth, FILE* ofp);
    float  Skill(WindField* truth);
    float  SpdBias(WindField* truth);
    int    DirectionDensity(WindField* truth,
               unsigned int* swath_density_array,
               unsigned int* field_density_array, float low_speed,
               float high_speed, int direction_count);

    int    SelectNearest(WindField* truth);
    int    SelectTruth(WindField* truth);
    int    SelectNudge();
    int    MatchSelected(WindSwath* source);
    int    GetProbabilityArray( WindField* truth, float*** prob,
               int** num_samples, float** widths, int true_dir_bins,
               int delta_dir_bins);
    int    AvgNambigVsCti(WindField* truth, float* avg_nambig,
               float low_speed, float high_speed);
    int    RmsSpdErrVsCti(WindField* truth, float* rms_spd_err_array,
               float* std_dev_array, float* std_err_array,
               float* spd_bias_array, int* count_array, float low_speed,
               float high_speed);
    int    RmsDirErrVsCti(WindField* truth, float* rms_dir_err_array,
               float* std_dev_array, float* std_err_array,
               float* dir_bias_array, int* count_array, float low_speed,
               float high_speed);
    int    SkillVsCti(WindField* truth, float* skill_array, int* count_array,
               float low_speed, float high_speed);
    int    WithinVsCti(WindField* truth, float* within_array,
               int* count_array, float low_speed, float high_speed,
               float within_angle);
    int    DirectionDensityVsCti(WindField* truth,
               unsigned int** swath_density_array,
               unsigned int** field_density_array, float low_speed,
               float high_speed, int direction_count);
    int    VectorCorrelationVsCti(WindField* truth, float* vc_array,
               int* count_array, float low_speed, float high_speed);
    int    ComponentCovarianceVsCti(WindField* truth, float* cc_array,
               int* count_array, float low_speed, float high_speed,
               COMPONENT_TYPE component1, COMPONENT_TYPE component2);
    int    Streamosity(WindField* truth, float* stream_array,
               float* good_stream_array, float low_speed, float high_speed);
    int    FractionNAmbigs(WindField* truth, float* frac_1amb_array,
               float* frac_2amb_array, float* frac_3amb_array,
               float* frac_4amb_array, float low_speed, float high_speed);
    int    NudgeOverrideVsCti(WindField* truth, float* correction_rate_array,
               float* change_incorrect_rate_array,
               float* bad_nudge_rate_array, float low_speed,
               float high_speed);

    //----------------------//
    // LonLatWind interface //
    //----------------------//

    int  NearestWindVector(LonLat lon_lat, WindVector* wv);
    int  InterpolatedWindVector(LonLat lon_lat, WindVector* wv);

    //-----------//
    // variables //
    //-----------//

    WVC***  swath;
    int     useNudgeVectorsAsTruth;
    int     nudgeVectorsRead;

protected:

    //--------------//
    // construction //
    //--------------//

    int  _Allocate();
    int  _Deallocate();

    //---------------------------//
    // LonLatWind interface help //
    //---------------------------//

    int  _Interpolate(LonLat& lon_lat, int low_cti, int low_ati,
             WindVector* wv);

    //-----------//
    // variables //
    //-----------//

    int  _crossTrackBins;
    int  _alongTrackBins;
    int  _validCells;
};

#endif
