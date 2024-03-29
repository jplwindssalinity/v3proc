#ifndef CAPGMF_H
#define CAPGMF_H

#include <vector>
#include "Meas.h"
#include "Wind.h"
#include "Array.h"
#include "CAPWind.h"

class CAP_ANC_L1B {
public:
    CAP_ANC_L1B();
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
    CAP_ANC_L2B();
    CAP_ANC_L2B(const char* filename);
    ~CAP_ANC_L2B();

    int Read(const char* filename);

    static const int nati = 3248;
    static const int ncti = 152;
    int narrays;

    float*** data;
};

class NCEP_ADJ {
public:
    NCEP_ADJ();
    NCEP_ADJ(const char* filename);
    ~NCEP_ADJ();

    int Read(const char* filename);
    float Get(float ncep_spd, float dt);
    float spdmin, dspd;
    float dtmin, ddt;
    int nspd, ndt;

    float** table;

};

class CAPGMF {
public:
    CAPGMF();
    ~CAPGMF();

    enum CAPRetrievalMode {
        RETRIEVE_SPEED_ONLY, RETRIEVE_SALINITY_ONLY, RETRIEVE_SPEED_DIRECTION,
        RETRIEVE_SPEED_DIRECTION_SALINITY, RETRIEVE_SPEED_SALINITY};

    int ReadFlat(const char* filename);
    int ReadRough(const char* filename);
    int ReadModelS0(const char* filename);

    double ObjectiveFunctionCAP(
        MeasList* tb_ml, MeasList* s0_ml, float trial_spd, float trial_dir,
        float trial_sss, float anc_spd, float anc_dir, float anc_swh,
        float anc_sst, float anc_spd_std_prior, float active_weight = 1,
        float passive_weight = 1);

    double ObjectiveFunctionActive(
        MeasList* s0_ml, float trial_spd, float trial_dir, float anc_swh,
        float anc_spd);

    double ObjectiveFunctionPassive(
        MeasList* tb_ml, float trial_spd, float trial_dir, float trial_sss,
        float anc_spd, float anc_dir, float anc_swh, float anc_sst,
        float anc_spd_std_prior);

    double SSSFWHM(
        MeasList* tb_ml, MeasList* s0_ml, float spd, float dir, float sss,
        float anc_spd, float anc_dir, float anc_swh, float anc_sst,
        float anc_spd_std_prior, float active_weight, float passive_weight);

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
        float init_sss, float anc_spd, float anc_dir, float anc_sst,
        float anc_swh, float anc_rr, float anc_spd_std_prior,
        float active_weight, float passive_weight, CAPRetrievalMode mode,
        float* spd, float* dir, float* sss, float* obj);

    int BuildSolutionCurves(
        MeasList* tb_ml, MeasList* s0_ml, float init_spd, float init_sss,
        float anc_spd, float anc_dir, float anc_sst, float anc_swh,
        float anc_rr, float anc_spd_std_prior, float active_weight,
        float passive_weight, CAPWVC* cap_wvc);

    int BuildSolutionCurvesSpdOnly(
        MeasList* tb_ml, MeasList* s0_ml, float init_spd, float init_sss,
        float anc_spd, float anc_dir, float anc_sst, float anc_swh,
        float anc_rr, float anc_spd_std_prior, float active_weight,
        float passive_weight, CAPWVC* cap_wvc);

    int BuildSolutionCurvesTwoStep(
        MeasList* tb_ml, MeasList* s0_ml, float init_spd, float init_sss,
        float anc_spd, float anc_dir, float anc_sst, float anc_swh,
        float anc_rr, float anc_spd_std_prior, float active_weight,
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

    static const int _sssCount = 46;
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

class TBGalCorr {

public:
    TBGalCorr();
    TBGalCorr(const char* filename);
    ~TBGalCorr();

    int Read(const char* filename);
    int Get(float ra, float dec, float spd, Meas::MeasTypeE met, float* dtg);

protected:
    std::vector<std::vector<float> > _dtb_gal;

    static const float _dspd = 1;
    static const float _delta = 1;
    static const int _nspd = 15;
    static const int _nra = 360;
    static const int _ndec = 180;

};

class TBVsLatDOYCorr {

public:
    TBVsLatDOYCorr();
    TBVsLatDOYCorr(const char* filename);
    ~TBVsLatDOYCorr();

    int Read(const char* filename);
    int Get(float lat, int doy, int is_asc, Meas::MeasTypeE met, float* dtb);

protected:
    std::vector<std::vector<float> > _dtb_table;
    static const int _ndays = 366;
    float _dlat;
    int _nlat;

};

class TBVsLatDOYAntAziCorr {

public:
    TBVsLatDOYAntAziCorr();
    TBVsLatDOYAntAziCorr(const char* filename);
    ~TBVsLatDOYAntAziCorr();

    int Read(const char* filename);
    int Get(
        float lat, int doy, int is_asc, float antazi, Meas::MeasTypeE met,
        float* dtb);

protected:
    std::vector<std::vector<float> > _dtb_table;
    static const int _ndays = 366;
    float _dlat;
    float _dazi;
    int _nlat;
    int _nazi;

};



// For use with NLopt
typedef struct {
    MeasList* tb_ml;
    MeasList* s0_ml;
    CAPGMF* cap_gmf;
    double init_spd;
    double init_dir;
    double init_sss;
    double anc_spd;
    double anc_dir;
    double anc_sst;
    double anc_swh;
    double anc_rr;
    double anc_spd_std_prior;
    double active_weight;
    double passive_weight;
    CAPGMF::CAPRetrievalMode mode;
} CAPAncillary;

// For NLopt
double cap_obj_func(unsigned n, const double* x, double* grad, void* data);

#endif
