#include <math.h>
#include <nlopt.hpp>
#include <vector>
#include "CAPGMF.h"
#include "Meas.h"
#include "Array.h"
#include "Constants.h"

CAP_ANC_L1B::CAP_ANC_L1B(const char* filename) : data(NULL) {
    Read(filename);
    return;
}

CAP_ANC_L1B::~CAP_ANC_L1B() {
    if(data)
        free_array((void*)data, 3, nframes, nfootprints, narrays);

    return;
}

int CAP_ANC_L1B::Read(const char* filename) {
    FILE* ifp = fopen(filename, "r");
    fseek(ifp, 0, SEEK_END);
    size_t file_size = ftell(ifp);
    fseek(ifp, 0, SEEK_SET);
    fread(&nframes, sizeof(int), 1, ifp);
    fread(&nfootprints, sizeof(int), 1, ifp);

    narrays = (file_size-4*2)/(nframes*nfootprints*4);
    if(file_size-4*2 != narrays*nframes*nfootprints*4) {
        fprintf(
            stderr, "CAP_ANC_L1B::Read: Unexpected file size in %s\n",
            filename);
        fclose(ifp);
        return(0);
    }

    data = (float***)make_array(
        sizeof(float), 3, nframes, nfootprints, narrays);
    if(!data) {
        fclose(ifp);
        return(0);
    }

    read_array(
        ifp, &data[0], sizeof(float), 3, nframes, nfootprints, narrays);
    fclose(ifp);
    return(1);
}

CAP_ANC_L2B::CAP_ANC_L2B(const char* filename) : data(NULL) {
    Read(filename);
    return;
}

CAP_ANC_L2B::~CAP_ANC_L2B() {
    if(data)
        free_array((void*)data, 3, nati, ncti, narrays);

    return;
}

int CAP_ANC_L2B::Read(const char* filename) {

    FILE* ifp = fopen(filename, "r");
    fseek(ifp, 0, SEEK_END);
    size_t file_size = ftell(ifp);
    fseek(ifp, 0, SEEK_SET);

    narrays = file_size/(4*nati*ncti);
    if(file_size != 4*nati*ncti*narrays) {
        fprintf(
            stderr, "CAP_ANC_L1B::Read: Unexpected file size in %s\n",
            filename);
        fclose(ifp);
        return(0);
    }

    data = (float***)make_array(sizeof(float), 3, nati, ncti, narrays);
    if(!data) {
        fclose(ifp);
        return(0);
    }

    read_array(ifp, &data[0], sizeof(float), 3, nati, ncti, narrays);
    fclose(ifp);
    return(1);
}

CAPGMF::CAPGMF() :_tbflat(NULL), _erough(NULL), _model_s0(NULL) {
    return;
}

CAPGMF::~CAPGMF() {
    _Deallocate();
    return;
}

float CAPGMF::ObjectiveFunctionCAP(
    MeasList* tb_ml, MeasList* s0_ml, float trial_spd, float trial_dir,
    float trial_sss, float anc_swh, float anc_sst, float active_weight,
    float passive_weight) {

    float passive_obj = ObjectiveFunctionPassive(
        tb_ml, trial_spd, trial_dir, trial_sss, anc_swh, anc_sst);

    float active_obj = ObjectiveFunctionActive(
        s0_ml, trial_spd, trial_dir, anc_swh);

    return(passive_weight * passive_obj + active_weight * active_obj);
}

float CAPGMF::ObjectiveFunctionPassive(
    MeasList* tb_ml, float trial_spd, float trial_dir, float trial_sss,
    float anc_swh, float anc_sst) {

    float obj = 0;
    for(Meas* meas = tb_ml->GetHead(); meas; meas = tb_ml->GetNext()){

        float model_tb;
        float chi = trial_dir - meas->eastAzimuth + pi;

        GetTB(
            meas->measType, meas->incidenceAngle, anc_sst, trial_sss,
            trial_spd, chi, anc_swh, &model_tb);

        double var = meas->A;
        obj += pow(meas->value - model_tb, 2) / var;
    }
    return(obj);
}

float CAPGMF::ObjectiveFunctionActive(
    MeasList* s0_ml, float trial_spd, float trial_dir, float anc_swh) {

    float obj = 0;
    for(Meas* meas = s0_ml->GetHead(); meas; meas = s0_ml->GetNext()){

        // Compute model S0 (replace this stub!!!)
        float model_s0;
        float chi = trial_dir - meas->eastAzimuth + pi;

        GetModelS0(
            meas->measType, meas->incidenceAngle, trial_spd, chi, anc_swh,
            &model_s0);

        double var = (meas->A-1.0) * model_s0 * model_s0;
        obj += pow(meas->value - model_s0, 2) / var;
    }
    return(obj);
}

int CAPGMF::GetModelS0(
        Meas::MeasTypeE met, float inc_in, float spd, float dir_in, float swh,
        float* model_s0) {

    // check meas type
    if(!_IsActive(met)) {
        fprintf(stderr, "Invalid MeasType: %d in CAPGMF::GetModelS0\n", met);
        return(0);
    }

    _InterpolateTable(met, inc_in, spd, dir_in, swh, _model_s0, model_s0);
    return(1);
}

int CAPGMF::GetTB(
        Meas::MeasTypeE met, float inc, float sst, float sss, float spd,
        float dir, float swh, float* tb) {

    float tbflat, dtb;

    if(GetTBFlat(met, inc, sst, sss, &tbflat)||
       GetDTB(met, inc, sst, spd, dir, swh, &dtb)) {
        return(0);
    }

    *tb = tbflat + dtb;
    return(1);
}

int CAPGMF::GetDTB(
        Meas::MeasTypeE met, float inc_in, float sst, float spd, float dir_in,
        float swh, float* dtb) {

    // check meas type
    if(!_IsPassive(met)) {
        fprintf(stderr, "Invalid MeasType: %d\n in CAPGMF::GetDTB", met);
        return(0);
    }

    float erough;
    _InterpolateTable(met, inc_in, spd, dir_in, swh, _erough, &erough);
    *dtb = sst * erough;
    return(1);
}

int CAPGMF::GetTBFlat(
    Meas::MeasTypeE met, float inc_in, float sst, float sss, float* tbflat) {

    // check meas type
    if(!_IsPassive(met)) {
        fprintf(stderr, "Invalid MeasType: %d\n in CAPGMF::GetTBFlat", met);
        return(0);
    }

    // returns the flat surface brightness temp
    float inc = inc_in * 180/pi;
    int met_idx = _MetToIndex(met);

    int isst0 = floor((sst-_sstMin)/_sstStep);
    if(isst0<0)
        isst0 = 0;
    if(isst0>_sstCount-2)
        isst0 = _sstCount-2;
    int isst1 = isst0 + 1;

    float sst0 = _sstMin + _sstStep*(float)isst0;
    float sst1 = _sstMin + _sstStep*(float)isst1;
    float ssta = 1-(sst-sst0)/(sst1-sst0);
    float sstb = 1-ssta;

    int isss0 = floor((sss-_sssMin)/_sssStep);
    if(isss0<0)
        isss0 = 0;
    if(isss0>_sssCount-2)
        isss0 = _sssCount-2;
    int isss1 = isss0 + 1;

    float sss0 = _sssMin + _sssStep*(float)isss0;
    float sss1 = _sssMin + _sssStep*(float)isss1;
    float sssa = 1-(sss-sss0)/(sss1-sss0);
    float sssb = 1-sssa;

    int iinc0 = floor((inc-_incMin)/_incStep);
    if(iinc0<0)
        iinc0 = 0;
    if(iinc0>_incCount-2)
        iinc0 = _incCount-2;
    int iinc1 = iinc0 + 1;

    float inc0 = _incMin + _incStep*(float)iinc0;
    float inc1 = _incMin + _incStep*(float)iinc1;
    float inca = 1-(inc-inc0)/(inc1-inc0);
    float incb = 1-inca;

    *tbflat =
        ssta * sssa * inca * _tbflat[met_idx][isss0][isst0][iinc0] +
        ssta * sssa * incb * _tbflat[met_idx][isss0][isst0][iinc1] +
        ssta * sssb * inca * _tbflat[met_idx][isss0][isst1][iinc0] +
        ssta * sssb * incb * _tbflat[met_idx][isss0][isst1][iinc1] +
        sstb * sssa * inca * _tbflat[met_idx][isss1][isst0][iinc0] +
        sstb * sssa * incb * _tbflat[met_idx][isss1][isst0][iinc1] +
        sstb * sssb * inca * _tbflat[met_idx][isss1][isst1][iinc0] +
        sstb * sssb * incb * _tbflat[met_idx][isss1][isst1][iinc1];

    return(1);
}

int CAPGMF::_InterpolateTable(
        Meas::MeasTypeE met, float inc_in, float spd, float dir_in, float swh,
        float***** table, float* interpolated_value) {

    // returns the rough surface model for TB or sigma0 from a look up table

    float inc = inc_in * 180/pi;
    float dir = dir_in * 180/pi;

    while(dir<0) dir+= 360;
    while(dir>=360) dir -= 360;

    // determine the measurement flavor (TB or sigma0)
    int met_idx = _MetToIndex(met);

    int idir0 = floor((dir-_dirMin)/_dirStep);
    if(idir0<0)
        idir0 = 0;
    if(idir0>_dirCount-2)
        idir0 = _dirCount-2;
    int idir1 = idir0 + 1;

    float dir0 = _dirMin + _dirStep*(float)idir0;
    float dir1 = _dirMin + _dirStep*(float)idir1;
    float dira = 1-(dir-dir0)/(dir1-dir0);
    float dirb = 1-dira;

    int ispd0 = floor((spd-_spdMin)/_spdStep);
    if(ispd0<0)
        ispd0 = 0;
    if(ispd0>_spdCount-2)
        ispd0 = _spdCount-2;
    int ispd1 = ispd0 + 1;

    float spd0 = _spdMin + _spdStep*(float)ispd0;
    float spd1 = _spdMin + _spdStep*(float)ispd1;
    float spda = 1-(spd-spd0)/(spd1-spd0);
    float spdb = 1-spda;

    int iinc0 = floor((inc-_incMin)/_incStep);
    if(iinc0<0)
        iinc0 = 0;
    if(iinc0>_incCount-2)
        iinc0 = _incCount-2;
    int iinc1 = iinc0 + 1;

    float inc0 = _incMin + _incStep*(float)iinc0;
    float inc1 = _incMin + _incStep*(float)iinc1;
    float inca = 1-(inc-inc0)/(inc1-inc0);
    float incb = 1-inca;

    int iswh0 = floor((swh-_swhMin)/_swhStep);
    if(iswh0<0)
        iswh0 = 0;
    if(iswh0>_spdCount-2)
        iswh0 = _spdCount-2;
    int iswh1 = iswh0 + 1;

    float swh0 = _swhMin + _swhStep*(float)iswh0;
    float swh1 = _swhMin + _swhStep*(float)iswh1;
    float swha = 1-(swh-swh0)/(swh1-swh0);
    float swhb = 1-swha;

    float value =
        swha * dira * spda * inca * table[met_idx][iswh0][idir0][ispd0][iinc0] +
        swha * dira * spda * incb * table[met_idx][iswh0][idir0][ispd0][iinc1] +
        swha * dira * spdb * inca * table[met_idx][iswh0][idir0][ispd1][iinc0] +
        swha * dira * spdb * incb * table[met_idx][iswh0][idir0][ispd1][iinc1] +
        swha * dirb * spda * inca * table[met_idx][iswh0][idir1][ispd0][iinc0] +
        swha * dirb * spda * incb * table[met_idx][iswh0][idir1][ispd0][iinc1] +
        swha * dirb * spdb * inca * table[met_idx][iswh0][idir1][ispd1][iinc0] +
        swha * dirb * spdb * incb * table[met_idx][iswh0][idir1][ispd1][iinc1] +
        swhb * dira * spda * inca * table[met_idx][iswh1][idir0][ispd0][iinc0] +
        swhb * dira * spda * incb * table[met_idx][iswh1][idir0][ispd0][iinc1] +
        swhb * dira * spdb * inca * table[met_idx][iswh1][idir0][ispd1][iinc0] +
        swhb * dira * spdb * incb * table[met_idx][iswh1][idir0][ispd1][iinc1] +
        swhb * dirb * spda * inca * table[met_idx][iswh1][idir1][ispd0][iinc0] +
        swhb * dirb * spda * incb * table[met_idx][iswh1][idir1][ispd0][iinc1] +
        swhb * dirb * spdb * inca * table[met_idx][iswh1][idir1][ispd1][iinc0] +
        swhb * dirb * spdb * incb * table[met_idx][iswh1][idir1][ispd1][iinc1];

    *interpolated_value = value;
    return(1);
}

int CAPGMF::ReadFlat(const char*  filename) {

    if(!_tbflat)
        _AllocateFlat();

    FILE* ifp = fopen(filename, "r");

    for(int met_idx = 0; met_idx < _metCountTB; ++met_idx) {
        for(int sss_idx = 0; sss_idx < _sssCount; ++sss_idx) {
            for(int sst_idx = 0; sst_idx < _sstCount; ++sst_idx) {
                float values[_incCount];
                if(fread(&values, sizeof(float), _incCount, ifp) != _incCount) {
                    fclose(ifp);
                    return(0);
                }
                for(int inc_idx = 0; inc_idx < _incCount; ++inc_idx) {
                    _tbflat[met_idx][sss_idx][sst_idx][inc_idx] = values[inc_idx];
                }
            }
        }
    }
    fclose(ifp);
    return(1);
}

int CAPGMF::ReadRough(const char*  filename) {

    if(!_erough)
        _AllocateRough();

    FILE* ifp = fopen(filename, "r");

    for(int met_idx = 0; met_idx < _metCountTB; ++met_idx) {
        for(int swh_idx = 0; swh_idx < _swhCount; ++swh_idx) {
            for(int dir_idx = 0; dir_idx < _dirCount; ++dir_idx) {
                for(int spd_idx = 0; spd_idx < _spdCount; ++spd_idx) {
                    float values[_incCount];
                    if(fread(&values, sizeof(float), _incCount, ifp) != 
                        _incCount) {
                        fclose(ifp);
                        return(0);
                    }
                    for(int inc_idx = 0; inc_idx < _incCount; ++inc_idx) {
                        _erough[met_idx][swh_idx][dir_idx][spd_idx][inc_idx] =
                            values[inc_idx];
                    }
                }
            }
        }
    }
    fclose(ifp);
    return(1);
}

int CAPGMF::ReadModelS0(const char*  filename) {

    if(!_erough)
        _AllocateRough();

    FILE* ifp = fopen(filename, "r");

    for(int met_idx = 0; met_idx < _metCountS0; ++met_idx) {
        for(int swh_idx = 0; swh_idx < _swhCount; ++swh_idx) {
            for(int dir_idx = 0; dir_idx < _dirCount; ++dir_idx) {
                for(int spd_idx = 0; spd_idx < _spdCount; ++spd_idx) {
                    float values[_incCount];
                    if(fread(&values, sizeof(float), _incCount, ifp) != 
                        _incCount) {
                        fclose(ifp);
                        return(0);
                    }
                    for(int inc_idx = 0; inc_idx < _incCount; ++inc_idx) {
                        _model_s0[met_idx][swh_idx][dir_idx][spd_idx][inc_idx] =
                            values[inc_idx];
                    }
                }
            }
        }
    }
    fclose(ifp);
    return(1);
}

int CAPGMF::_MetToIndex(Meas::MeasTypeE met) {
    int idx = 0;
    switch(met) {
        case Meas::VV_MEAS_TYPE:
            idx = 0;
            break;
        case Meas::HH_MEAS_TYPE:
            idx = 1;
            break;
        case Meas::VH_MEAS_TYPE:
            idx = 2;
            break;
        case Meas::HV_MEAS_TYPE:
            idx = 3;
            break;
        case Meas::L_BAND_TBV_MEAS_TYPE:
            idx = 0;
            break;
        case Meas::L_BAND_TBH_MEAS_TYPE:
            idx = 1;
            break;
        default:
            fprintf(stderr, "CAPGMF::_MetToIndex: invalid meas type %d\n", met);
            exit(1);
    }
    return(idx);
}

int CAPGMF::_IsPassive(Meas::MeasTypeE met) {
    if(met == Meas::L_BAND_TBV_MEAS_TYPE || met == Meas::L_BAND_TBH_MEAS_TYPE)
        return(1);
    else
        return(0);
}

int CAPGMF::_IsActive(Meas::MeasTypeE met) {
    if(met == Meas::VV_MEAS_TYPE || met == Meas::HH_MEAS_TYPE &&
       met == Meas::VH_MEAS_TYPE || met == Meas::HV_MEAS_TYPE)
        return(1);
    else
        return(0);
}

int CAPGMF::_AllocateFlat() {
    _tbflat = (float ****)make_array(
        sizeof(float), 4, _metCountTB, _sssCount, _sstCount, _incCount);

    if(!_tbflat)
        return(0);

    return(1);
}

int CAPGMF::_AllocateRough() {
    _erough = (float *****)make_array(
        sizeof(float), 5, _metCountTB, _swhCount, _dirCount, _spdCount,
        _incCount);

    if(!_erough)
        return(0);

    return(1);
}

int CAPGMF::_AllocateModelS0() {
    _model_s0 = (float *****)make_array(
        sizeof(float), 5, _metCountS0, _swhCount, _dirCount, _spdCount,
        _incCount);

    if(!_erough)
        return(0);

    return(1);
}

int CAPGMF::_Deallocate() {

    if (_tbflat) {
        free_array(
            (void *)_tbflat, 4, _metCountTB, _sssCount, _sstCount, _incCount);
        _tbflat = NULL;
    }

    if (_erough) {
        free_array(
            (void *)_erough, 5, _metCountTB, _swhCount, _dirCount, _spdCount,
            _incCount);
        _erough = NULL;
    }

    if (_model_s0) {
        free_array(
            (void *)_model_s0, 5, _metCountS0, _swhCount, _dirCount, _spdCount,
            _incCount);
        _model_s0 = NULL;
    }

    return(1);
}

double cap_obj_func(unsigned n, const double* x, double* grad, void* data) {

    CAPAncillary* cap_anc = (CAPAncillary*)data;

    float trial_spd = (float)x[0];
    float trial_dir = cap_anc->s0_wvc->selected->dir;
    float trial_sss = cap_anc->anc_sss;

    if(cap_anc->mode>0)
        trial_dir = (float)x[1];
    if(cap_anc->mode>1)
        trial_sss = (float)x[2];

    float active_weight = 1;
    float passive_weight = 1;

    if(trial_spd!=trial_spd || trial_dir!=trial_dir || trial_sss!=trial_sss)
        return HUGE_VAL;

    double obj = cap_anc->cap_gmf->ObjectiveFunctionCAP(
        cap_anc->tb_ml, cap_anc->s0_ml, trial_spd, trial_dir, trial_sss,
        cap_anc->anc_swh, cap_anc->anc_sst, active_weight, passive_weight);

    return(obj);
}

int retrieve_cap(CAPAncillary* cap_anc, double* spd, double* dir, double* sss,
                 double* min_obj) {

    int n_dims = cap_anc->mode + 1;

    // Construct the optimization object
    nlopt::opt opt(nlopt::LN_COBYLA, n_dims);

    // Check bounds on initial guesses
    double init_spd = cap_anc->s0_wvc->selected->spd;
    if(init_spd<0.5) init_spd = 0.5;
    if(init_spd>50) init_spd = 50;

    double init_dir = cap_anc->s0_wvc->selected->dir;
    while(init_dir>two_pi) init_dir -= two_pi;
    while(init_dir<0) init_dir += two_pi;

    double init_sss = cap_anc->anc_sss;
    if(init_sss<30) init_sss = 30;
    if(init_sss>40) init_sss = 40;

    // Set contraints and initial guesses
    std::vector<double> lb(n_dims), ub(n_dims), x(n_dims);
    if(cap_anc->mode == SPEED_ONLY) {
        lb[0] = 0; ub[0] = 100;
        x[0] = init_spd;

    } else if (cap_anc->mode == SPEED_DIRECTION) {
        lb[0] = 0; ub[0] = 100;
        lb[1] = 0; ub[1] = two_pi;

        x[0] = init_spd;
        x[1] = init_dir;

    } else if (cap_anc->mode == SPEED_DIRECTION_SALINITY) {
        lb[0] = 0; ub[0] = 100;
        lb[1] = 0; ub[1] = two_pi;
        lb[2] = 28; ub[2] = 42; // salinity

        x[0] = init_spd;
        x[1] = init_dir;
        x[2] = init_sss;
    }

    // Config the optimization object for this problem
    opt.set_lower_bounds(lb);
    opt.set_upper_bounds(ub);
    opt.set_min_objective(cap_obj_func, cap_anc);
    opt.set_xtol_rel(0.01);

    // Solve it!
    double minf;
    nlopt::result result = opt.optimize(x, minf);

    // Copy outputs
    *min_obj = minf;

    if(cap_anc->mode == SPEED_ONLY) {
        *spd = x[0];
        *dir = cap_anc->s0_wvc->selected->dir;
        *sss = cap_anc->anc_sss;

    } else if(cap_anc->mode == SPEED_DIRECTION) {
        *spd = x[0];
        *dir = x[1];
        *sss = cap_anc->anc_sss;

    } else if(cap_anc->mode == SPEED_DIRECTION_SALINITY) {
        *spd = x[0];
        *dir = x[1];
        *sss = x[2];
    }
    return(1);
}

