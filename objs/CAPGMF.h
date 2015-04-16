#include "Meas.h"
#include "Array.h"

class CAPGMF {
public:
    CAPGMF();
    ~CAPGMF();

    int ReadFlat(const char* filename);
    int ReadRough(const char* filename);
    int ReadModelS0(const char* filename);

    float ObjectiveFunctionCAP(
        MeasList* tb_ml, MeasList* s0_ml, float trial_spd, float trial_dir,
        float trial_sss, float anc_swh, float anc_sst, float active_weight = 1,
        float passive_weight = 1);

    float ObjectiveFunctionActive(
        MeasList* s0_ml, float trial_spd, float trial_dir, float anc_swh);

    float ObjectiveFunctionPassive(
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
        float dir, float swh, float* tb);

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

    static const int _swhCount = 8;
    static const float _swhMin = 0.5;
    static const float _swhStep = 1;

    static const int _metCountTB = 2;
    static const int _metCountS0 = 4;

    float**** _tbflat;
    float***** _erough;
    float***** _model_s0;
};
