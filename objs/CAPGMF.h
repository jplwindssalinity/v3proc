#ifndef CAPGMF_H
#define CAPGMF_H

#include "Meas.h"
#include "Wind.h"
#include "Array.h"
#include "CAPWind.h"

class CAP_ANC_L1B {
public:
    CAP_ANC_L1B(const char* filename);
    ~CAP_ANC_L1B();

    int Read(const char* filename);

    int nframes;
    int nfootprints;
    int narrays;

    float*** data;
};

class CAP_ANC_L2B {
public:
    CAP_ANC_L2B(const char* filename);
    ~CAP_ANC_L2B();

    int Read(const char* filename);

    static const int nati = 3248;
    static const int ncti = 152;
    int narrays;

    float*** data;
};

class CAPGMF {
public:
    CAPGMF();
    ~CAPGMF();

    enum CAPRetrievalMode {
        RETRIEVE_SPEED_ONLY, RETRIEVE_SPEED_DIRECTION,
        RETRIEVE_SPEED_DIRECTION_SALINITY, RETRIEVE_SPEED_SALINITY};

    int ReadFlat(const char* filename);
    int ReadRough(const char* filename);
    int ReadModelS0(const char* filename);

    double ObjectiveFunctionCAP(
        MeasList* tb_ml, MeasList* s0_ml, float trial_spd, float trial_dir,
        float trial_sss, float anc_swh, float anc_sst, float active_weight = 1,
        float passive_weight = 1);

    double ObjectiveFunctionActive(
        MeasList* s0_ml, float trial_spd, float trial_dir, float anc_swh);

    double ObjectiveFunctionPassive(
        MeasList* tb_ml, float trial_spd, float trial_dir, float trial_sss,
        float anc_swh, float anc_sst);

    int GetTBFlat(
        Meas::MeasTypeE met, float inc, float sst, float sss, float* tbflat);

    int GetDTB(
        Meas::MeasTypeE met, float inc, float sst, float spd, float dir,
        float swh, float* dtb);

    int GetModelS0(
        Meas::MeasTypeE met, float inc_in, float spd, float dir_in, float swh,
        float* model_s0);

    int GetTB(
        Meas::MeasTypeE met, float inc, float sst, float sss, float spd,
        float dir, float swh, float* tb_flat, float* dtb);

    int Retrieve(
        MeasList* tb_ml, MeasList* s0_ml, float init_spd, float init_dir,
        float init_sss, float anc_sst, float anc_swh, float anc_rr,
        float active_weight, float passive_weight, CAPRetrievalMode mode,
        float* spd, float* dir, float* sss, float* obj);

    int BuildSolutionCurves(
        MeasList* tb_ml, MeasList* s0_ml, float init_spd, float init_sss,
        float anc_sst, float anc_swh, float anc_rr, float active_weight,
        float passive_weight, CAPWVC* cap_wvc);

protected:

    int _AllocateFlat();
    int _AllocateRough();
    int _AllocateModelS0();
    int _Deallocate();

    int _MetToIndex(Meas::MeasTypeE met);
    int _IsActive(Meas::MeasTypeE met);
    int _IsPassive(Meas::MeasTypeE met);

    int _InterpolateTable(
        Meas::MeasTypeE met, float inc_in, float spd, float dir_in, float swh,
        float***** table, float* interpolated_value);

    static const int _incCount = 11;
    static const float _incMin = 35;
    static const float _incStep = 1.0;

    static const int _sstCount = 43;
    static const float _sstMin = -2.0;
    static const float _sstStep = 1.0;

    static const int _sssCount = 41;
    static const float _sssMin = 0.0;
    static const float _sssStep = 1.0;

    static const int _spdCount = 501;
    static const float _spdMin = 0.0;
    static const float _spdStep = 0.1;

    static const int _dirCount = 360;
    static const float _dirMin = 0.0;
    static const float _dirStep = 1.0;

    // Note 9 SWH bins in GMF tables, last is reserved for missing significant
    // wave height; uses GMF tables developed without any condition on SWH.
    static const int _swhCount = 8;
    static const float _swhMin = 0.5;
    static const float _swhStep = 1;

    static const int _metCountTB = 2;
    static const int _metCountS0 = 4;

    static const float kpm = 0.16;

    float**** _tbflat;
    float***** _erough;
    float***** _model_s0;
};

// For use with NLopt
typedef struct {
    MeasList* tb_ml;
    MeasList* s0_ml;
    CAPGMF* cap_gmf;
    double init_spd;
    double init_dir;
    double init_sss;
    double anc_sst;
    double anc_swh;
    double anc_rr;
    double active_weight;
    double passive_weight;
    CAPGMF::CAPRetrievalMode mode;
} CAPAncillary;

// For NLopt
double cap_obj_func(unsigned n, const double* x, double* grad, void* data);

#endif
