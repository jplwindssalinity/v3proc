#include <math.h>
#include <nlopt.hpp>
#include <vector>
#include "CAPGMF.h"
#include "Meas.h"
#include "Array.h"
#include "Constants.h"

CAP_ANC_L1B::CAP_ANC_L1B() : data(NULL) {
    return;
}

CAP_ANC_L1B::CAP_ANC_L1B(const char* filename) : data(NULL) {
    Read(filename);
    return;
}

CAP_ANC_L1B::~CAP_ANC_L1B() {
    if(data)
        free_array((void*)data, 3, narrays, nframes, nfootprints);

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
        sizeof(float), 3, narrays, nframes, nfootprints);
    if(!data) {
        fclose(ifp);
        return(0);
    }

    read_array(
        ifp, &data[0], sizeof(float), 3, narrays, nframes, nfootprints);
    fclose(ifp);
    return(1);
}

CAP_ANC_L2B::CAP_ANC_L2B() : data(NULL) {
    return;
}

CAP_ANC_L2B::CAP_ANC_L2B(const char* filename) : data(NULL) {
    Read(filename);
    return;
}

CAP_ANC_L2B::~CAP_ANC_L2B() {
    if(data)
        free_array((void*)data, 3, narrays, nati, ncti);

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

    data = (float***)make_array(sizeof(float), 3, narrays, nati, ncti);
    if(!data) {
        fclose(ifp);
        return(0);
    }

    read_array(ifp, &data[0], sizeof(float), 3, narrays, nati, ncti);
    fclose(ifp);
    return(1);
}

NCEP_ADJ::NCEP_ADJ() : table(NULL) {
    return;
}

NCEP_ADJ::NCEP_ADJ(const char* filename) : table(NULL) {
    Read(filename);
    return;
}

int NCEP_ADJ::Read(const char* filename) {

    FILE* ifp = fopen(filename, "r");
    fread(&spdmin, sizeof(float), 1, ifp);
    fread(&dspd, sizeof(float), 1, ifp);
    fread(&nspd, sizeof(int), 1, ifp);
    fread(&dtmin, sizeof(float), 1, ifp);
    fread(&ddt, sizeof(float), 1, ifp);
    fread(&ndt, sizeof(int), 1, ifp);

    table = (float**)make_array(sizeof(float), 2, ndt, nspd);
    read_array(ifp, &table[0], sizeof(float), 2, ndt, nspd);

    fclose(ifp);
    return(1);
}

float NCEP_ADJ::Get(float ncep_speed, float dt) {

    int ispd = floor(0.5+(ncep_speed - spdmin)/dspd);
    int idt = floor(0.5+(dt - dtmin)/ddt);
    float out_speed;

    if(ispd >= 0 && ispd < nspd && idt >= 0 & idt < ndt) {
        out_speed = ncep_speed + table[idt][ispd];
    } else {
        out_speed = ncep_speed;
    }
    return(out_speed);
}

NCEP_ADJ::~NCEP_ADJ() {
    if(table)
        free_array((void*)table, 2, ndt, nspd);

    return;
}

CAPGMF::CAPGMF() :_tbflat(NULL), _erough(NULL), _model_s0(NULL) {
    return;
}

CAPGMF::~CAPGMF() {
    _Deallocate();
    return;
}

int CAPGMF::BuildSolutionCurvesSpdOnly(
    MeasList* tb_ml, MeasList* s0_ml, float init_spd, float init_sss,
    float anc_spd, float anc_dir, float anc_sst, float anc_swh, float anc_rr,
    float anc_spd_std_prior, float active_weight, float passive_weight,
    CAPWVC* cap_wvc) {

    float start_speed = init_spd;
    float start_sss = init_sss;

    // best_spd, best_sss, best_obj are pointers to float[360] arrays.
    for(int iazi = 0; iazi < cap_wvc->n_azi; ++iazi) {
        float azi_spacing = 360 / (float)cap_wvc->n_azi;
        float this_angle = azi_spacing * (float)iazi * dtr;
        float spd, dir, sss, obj;

        Retrieve(
            tb_ml, s0_ml, start_speed, this_angle, init_sss, anc_spd, anc_dir,
            anc_sst, anc_swh, anc_rr, anc_spd_std_prior, active_weight,
            passive_weight, RETRIEVE_SPEED_ONLY, &spd, &dir, &sss, &obj);

        cap_wvc->best_spd[iazi] = spd;
        cap_wvc->best_sss[iazi] = init_sss;

        // Swap sign on objective function value
        cap_wvc->best_obj[iazi] = -obj;

    }
    return(1);
}


int CAPGMF::BuildSolutionCurvesTwoStep(
    MeasList* tb_ml, MeasList* s0_ml, float init_spd, float init_sss,
    float anc_spd, float anc_dir, float anc_sst, float anc_swh, float anc_rr,
    float anc_spd_std_prior, float active_weight, float passive_weight,
    CAPWVC* cap_wvc) {

    float start_speed = init_spd;
    float start_sss = init_sss;

    // best_spd, best_sss, best_obj are pointers to float[360] arrays.
    for(int iazi = 0; iazi < cap_wvc->n_azi; ++iazi) {
        float azi_spacing = 360 / (float)cap_wvc->n_azi;
        float this_angle = azi_spacing * (float)iazi * dtr;
        float spd, dir, sss, obj;

        // First do wind speed
        Retrieve(
            tb_ml, s0_ml, start_speed, this_angle, init_sss, anc_spd, anc_dir,
            anc_sst, anc_swh, anc_rr, anc_spd_std_prior, active_weight,
            passive_weight, RETRIEVE_SPEED_ONLY, &spd, &dir, &sss, &obj);

        cap_wvc->best_spd[iazi] = spd;
        cap_wvc->best_sss[iazi] = sss;

        // Swap sign on objective function value
        cap_wvc->best_obj[iazi] = -obj;

        start_speed = spd;

        float sss_active_weight = 0;
        float sss_passive_weight = 1;

        // Fix the wind speed and do TB-only SSS for that wind speed
        Retrieve(
            tb_ml, s0_ml, spd, this_angle, start_sss, anc_spd, anc_dir, anc_sst,
            anc_swh, anc_rr, anc_spd_std_prior, sss_active_weight,
            sss_passive_weight, RETRIEVE_SALINITY_ONLY, &spd, &dir, &sss, &obj);

        start_sss = sss;
        cap_wvc->best_sss[iazi] = sss;
    }
    return(1);
}

int CAPGMF::BuildSolutionCurves(
    MeasList* tb_ml, MeasList* s0_ml, float init_spd, float init_sss,
    float anc_spd, float anc_dir, float anc_sst, float anc_swh, float anc_rr,
    float anc_spd_std_prior, float active_weight, float passive_weight,
    CAPWVC* cap_wvc) {

    float start_speed = init_spd;

    // best_spd, best_sss, best_obj are pointers to float[360] arrays.
    for(int iazi = 0; iazi < cap_wvc->n_azi; ++iazi) {
        float azi_spacing = 360 / (float)cap_wvc->n_azi;
        float this_angle = azi_spacing * (float)iazi * dtr;
        float spd, dir, sss, obj;

        Retrieve(
            tb_ml, s0_ml, start_speed, this_angle, init_sss, anc_spd, anc_dir,
            anc_sst, anc_swh, anc_rr, anc_spd_std_prior, active_weight,
            passive_weight, RETRIEVE_SPEED_SALINITY, &spd, &dir, &sss, &obj);

        cap_wvc->best_spd[iazi] = spd;
        cap_wvc->best_sss[iazi] = sss;

        // Swap sign on objective function value
        cap_wvc->best_obj[iazi] = -obj;

        start_speed = spd;
    }
    return(1);
}

int CAPGMF::Retrieve(
    MeasList* tb_ml, MeasList* s0_ml, float init_spd, float init_dir,
    float init_sss, float anc_spd, float anc_dir, float anc_sst, float anc_swh,
    float anc_rr, float anc_spd_std_prior, float active_weight,
    float passive_weight, CAPRetrievalMode mode,
    float* spd, float* dir, float* sss, float* obj) {

    int n_dims;
    switch(mode) {
        case RETRIEVE_SPEED_ONLY:
            n_dims = 1;
            break;

        case RETRIEVE_SALINITY_ONLY:
            n_dims = 1;
            break;

        case RETRIEVE_SPEED_DIRECTION:
            n_dims = 2;
            break;

        case RETRIEVE_SPEED_SALINITY:
            n_dims = 2;
            break;

        case RETRIEVE_SPEED_DIRECTION_SALINITY:
            n_dims = 3;
            break;

        default:
            fprintf(stderr, "CAPGMF::Retrieve: Unknown CAPRetrievalMode: %d\n",
                    mode);
            return(0);
            break;
    }

    // Construct the optimization object
    nlopt::opt opt(nlopt::LN_COBYLA, n_dims);

    // Check bounds on initial guesses
    if(init_spd<0.5) init_spd = 0.5;
    if(init_spd>50) init_spd = 50;

    while(init_dir>two_pi) init_dir -= two_pi;
    while(init_dir<0) init_dir += two_pi;

    if(init_sss<20) init_sss = 20;
    if(init_sss>40) init_sss = 40;

    // Set contraints and initial guesses
    std::vector<double> lb(n_dims), ub(n_dims), x(n_dims);

    if(mode == RETRIEVE_SPEED_ONLY) {
        lb[0] = 0; ub[0] = 100;
        x[0] = init_spd;

    } else if (mode == RETRIEVE_SALINITY_ONLY) {
        lb[0] = 0; ub[0] = 40; // salinity
        x[0] = init_sss;

    } else if (mode == RETRIEVE_SPEED_DIRECTION) {
        lb[0] = 0; ub[0] = 100;
        lb[1] = 0; ub[1] = two_pi;

        x[0] = init_spd;
        x[1] = init_dir;

    } else if (mode == RETRIEVE_SPEED_SALINITY) {
        lb[0] = 0; ub[0] = 100;
        lb[1] = 0; ub[1] = 40; // salinity

        x[0] = init_spd;
        x[1] = init_sss;

    } else if (mode == RETRIEVE_SPEED_DIRECTION_SALINITY) {
        lb[0] = 0; ub[0] = 100;
        lb[1] = 0; ub[1] = two_pi;
        lb[0] = 0; ub[0] = 40; // salinity

        x[0] = init_spd;
        x[1] = init_dir;
        x[2] = init_sss;
    }

    CAPAncillary cap_anc;
    cap_anc.tb_ml = tb_ml;
    cap_anc.s0_ml = s0_ml;
    cap_anc.init_spd = init_spd;
    cap_anc.init_dir = init_dir;
    cap_anc.init_sss = init_sss;
    cap_anc.cap_gmf = this;
    cap_anc.anc_spd = anc_spd;
    cap_anc.anc_dir = anc_dir;
    cap_anc.anc_sst = anc_sst;
    cap_anc.anc_swh = anc_swh;
    cap_anc.anc_rr = anc_rr;
    cap_anc.mode = mode;
    cap_anc.anc_spd_std_prior = anc_spd_std_prior;
    cap_anc.active_weight = active_weight;
    cap_anc.passive_weight = passive_weight;

    // Config the optimization object for this problem
    opt.set_lower_bounds(lb);
    opt.set_upper_bounds(ub);
    opt.set_min_objective(cap_obj_func, &cap_anc);
    opt.set_xtol_rel(0.0001);

    // Solve it!
    double minf;
    nlopt::result result = opt.optimize(x, minf);

    // Copy outputs
    *obj = (float)minf;

    if(mode == RETRIEVE_SPEED_ONLY) {
        *spd = (float)x[0];
        *dir = init_dir;
        *sss = init_sss;

    } else if(mode == RETRIEVE_SALINITY_ONLY) {
        *spd = init_spd;
        *dir = init_dir;
        *sss = (float)x[0];

    } else if(mode == RETRIEVE_SPEED_DIRECTION) {
        *spd = (float)x[0];
        *dir = (float)x[1];
        *sss = init_sss;

    } else if(mode == RETRIEVE_SPEED_SALINITY) {
        *spd = (float)x[0];
        *dir = init_dir;
        *sss = (float)x[1];

    } else if(mode == RETRIEVE_SPEED_DIRECTION_SALINITY) {
        *spd = (float)x[0];
        *dir = (float)x[1];
        *sss = (float)x[2];
    }
    return(1);
}

double CAPGMF::ObjectiveFunctionCAP(
    MeasList* tb_ml, MeasList* s0_ml, float trial_spd, float trial_dir,
    float trial_sss, float anc_spd, float anc_dir, float anc_swh, float anc_sst,
    float anc_spd_std_prior, float active_weight, float passive_weight) {

    double passive_obj = 0;
    double active_obj = 0;

    if(tb_ml) {
        passive_obj = ObjectiveFunctionPassive(
            tb_ml, trial_spd, trial_dir, trial_sss, anc_spd, anc_dir, anc_swh,
            anc_sst, anc_spd_std_prior);
    }

    if(s0_ml) {
        active_obj = ObjectiveFunctionActive(
            s0_ml, trial_spd, trial_dir, anc_swh, anc_spd);
    }

    return(
        (double)passive_weight * passive_obj +
        (double)active_weight * active_obj);
}

double CAPGMF::ObjectiveFunctionPassive(
    MeasList* tb_ml, float trial_spd, float trial_dir, float trial_sss,
    float anc_spd, float anc_dir, float anc_swh, float anc_sst,
    float anc_spd_std_prior) {

    double obj = 0;
    for(Meas* meas = tb_ml->GetHead(); meas; meas = tb_ml->GetNext()){

        float tb_flat, dtb, model_tb;
        float chi = trial_dir - meas->eastAzimuth + pi;

        GetTB(
            meas->measType, meas->incidenceAngle, anc_sst, trial_sss,
            trial_spd, chi, anc_swh, &tb_flat, &dtb);

        model_tb = tb_flat + dtb;

        double var = meas->A + pow(kpm * dtb, 2);
        obj += pow((double)(meas->value - model_tb), 2) / var;
    }

    if(trial_spd > 0)
        obj += pow((trial_spd - anc_spd)/anc_spd_std_prior, 2);

    return(obj);
}

double CAPGMF::ObjectiveFunctionActive(
    MeasList* s0_ml, float trial_spd, float trial_dir, float anc_swh,
    float anc_spd) {

    double obj = 0;
    for(Meas* meas = s0_ml->GetHead(); meas; meas = s0_ml->GetNext()){

        // Compute model S0 (replace this stub!!!)
        float model_s0;
        float chi = trial_dir - meas->eastAzimuth + pi;

        GetModelS0(
            meas->measType, meas->incidenceAngle, trial_spd, chi, anc_swh,
            &model_s0);

        double kp_tot = (1 + pow(kpm, 2)) * meas->A - 1;
        double var = kp_tot * model_s0 * model_s0;

        double weight = 1;

        if(meas->measType == Meas::VH_MEAS_TYPE ||
           meas->measType == Meas::HV_MEAS_TYPE) {

            if(anc_spd < 15) {
                weight = 0;
            } else if(anc_spd < 20) {
                weight = 1-(20-anc_spd)/5;
            } else {
                weight = 1;
            }
        }

        obj += weight * pow((double)(meas->value - model_s0), 2) / var;
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
        float dir, float swh, float* tb_flat, float* dtb) {

    float tbflat_, dtb_;

    if(!GetTBFlat(met, inc, sst, sss, &tbflat_)||
       !GetDTB(met, inc, sst, spd, dir, swh, &dtb_)) {
        return(0);
    }
    *dtb = dtb_;
    *tb_flat = tbflat_;
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

    sst = sst - 273.16;

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
        sssa * ssta * inca * _tbflat[met_idx][isss0][isst0][iinc0] +
        sssa * ssta * incb * _tbflat[met_idx][isss0][isst0][iinc1] +
        sssa * sstb * inca * _tbflat[met_idx][isss0][isst1][iinc0] +
        sssa * sstb * incb * _tbflat[met_idx][isss0][isst1][iinc1] +
        sssb * ssta * inca * _tbflat[met_idx][isss1][isst0][iinc0] +
        sssb * ssta * incb * _tbflat[met_idx][isss1][isst0][iinc1] +
        sssb * sstb * inca * _tbflat[met_idx][isss1][isst1][iinc0] +
        sssb * sstb * incb * _tbflat[met_idx][isss1][isst1][iinc1];

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

    int iswh0, iswh1;
    float swh0, swh1, swha, swhb;
    if (swh < 0) {
        // The last swh index for missing swh, uses GMFs without significant
        // wave height.
        iswh0 = _swhCount;
        iswh1 = _swhCount;

        swha = 1;
        swhb = 0;

    } else {
        iswh0 = floor((swh-_swhMin)/_swhStep);
        if(iswh0<0)
            iswh0 = 0;
        if(iswh0>_swhCount-2)
            iswh0 = _swhCount-2;
        iswh1 = iswh0 + 1;

        swh0 = _swhMin + _swhStep*(float)iswh0;
        swh1 = _swhMin + _swhStep*(float)iswh1;
        swha = 1-(swh-swh0)/(swh1-swh0);
        swhb = 1-swha;
    }

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
        for(int swh_idx = 0; swh_idx < _swhCount+1; ++swh_idx) {
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

    if(!_model_s0)
        _AllocateModelS0();

    FILE* ifp = fopen(filename, "r");

    for(int met_idx = 0; met_idx < _metCountS0; ++met_idx) {
        for(int swh_idx = 0; swh_idx < _swhCount+1; ++swh_idx) {
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
    if(met == Meas::VV_MEAS_TYPE || met == Meas::HH_MEAS_TYPE ||
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
        sizeof(float), 5, _metCountTB, _swhCount+1, _dirCount, _spdCount,
        _incCount);

    if(!_erough)
        return(0);

    return(1);
}

int CAPGMF::_AllocateModelS0() {
    _model_s0 = (float *****)make_array(
        sizeof(float), 5, _metCountS0, _swhCount+1, _dirCount, _spdCount,
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
            (void *)_erough, 5, _metCountTB, _swhCount+1, _dirCount, _spdCount,
            _incCount);
        _erough = NULL;
    }

    if (_model_s0) {
        free_array(
            (void *)_model_s0, 5, _metCountS0, _swhCount+1, _dirCount, _spdCount,
            _incCount);
        _model_s0 = NULL;
    }

    return(1);
}

TBGalCorr::TBGalCorr() {
    return;
}

TBGalCorr::TBGalCorr(const char* filename) {
    Read(filename);
    return;
}

TBGalCorr::~TBGalCorr() {
    return;
}


int TBGalCorr::Read(const char* filename) {

    _dtb_gal.resize(2);
    _dtb_gal[0].resize(_nspd*_nra*_ndec);
    _dtb_gal[1].resize(_nspd*_nra*_ndec);

    FILE* ifp = fopen(filename, "r");
    fread(&_dtb_gal[0][0], _nspd*_nra*_ndec, sizeof(float), ifp);
    fread(&_dtb_gal[1][0], _nspd*_nra*_ndec, sizeof(float), ifp);
    fclose(ifp);
    return(1);
}

int TBGalCorr::Get(
    float ra, float dec, float spd, Meas::MeasTypeE met, float* dtg) {

    int ipol;
    if(met == Meas::L_BAND_TBV_MEAS_TYPE)
        ipol = 0;
    else if(met == Meas::L_BAND_TBH_MEAS_TYPE)
        ipol = 1;
    else {
        fprintf(stderr, "TBGalCorr::Get: Invalid measType\n");
        return(0);
    }

    float spdmin = _dspd/2;
    float ramin = _delta/2;
    float decmin = -90 + _delta/2;

    int ispd = round((spd-spdmin)/_dspd);
    int ira = round((ra-ramin)/_delta);
    int idec = round((dec-decmin)/_delta);

    if(ira == -1) ira = _nra - 1;
    if(ira == _nra) ira = 0;

    if(ispd >= _nspd) ispd = _nspd - 1;
    if(ispd < 0) ispd = 0;

    if(ira < 0 || ira >= _nra || idec < 0 || idec >= _ndec)
        return(0);
    else {
        int idx = ira + _nra * (idec + _ndec * ispd);
        *dtg = _dtb_gal[ipol][idx];
        return(1);
    }
}

double cap_obj_func(unsigned n, const double* x, double* grad, void* data) {
    // Need this call structure for NLopt library call

    CAPAncillary* cap_anc = (CAPAncillary*)data;

    float trial_spd, trial_dir, trial_sss;
    if(cap_anc->mode == CAPGMF::RETRIEVE_SPEED_ONLY) {
        trial_spd = (float)x[0];
        trial_dir = cap_anc->init_dir;
        trial_sss = cap_anc->init_sss;

    } else if(cap_anc->mode == CAPGMF::RETRIEVE_SALINITY_ONLY) {
        trial_spd = cap_anc->init_spd;
        trial_dir = cap_anc->init_dir;
        trial_sss = (float)x[0];

    } else if(cap_anc->mode == CAPGMF::RETRIEVE_SPEED_DIRECTION) {
        trial_spd = (float)x[0];
        trial_dir = (float)x[1];
        trial_sss = cap_anc->init_sss;

    } else if(cap_anc->mode == CAPGMF::RETRIEVE_SPEED_SALINITY) {
        trial_spd = (float)x[0];
        trial_dir = cap_anc->init_dir;
        trial_sss = (float)x[1];

    } else if(cap_anc->mode == CAPGMF::RETRIEVE_SPEED_DIRECTION_SALINITY) {
        trial_spd = (float)x[0];
        trial_dir = (float)x[1];
        trial_sss = (float)x[2];
    }

    float active_weight = (float)cap_anc->active_weight;
    float passive_weight = (float)cap_anc->passive_weight;

    if(trial_spd!=trial_spd || trial_dir!=trial_dir || trial_sss!=trial_sss)
        return HUGE_VAL;

    double obj = (double)cap_anc->cap_gmf->ObjectiveFunctionCAP(
        cap_anc->tb_ml, cap_anc->s0_ml, trial_spd, trial_dir, trial_sss,
        cap_anc->anc_spd, cap_anc->anc_dir, cap_anc->anc_swh, cap_anc->anc_sst,
        cap_anc->anc_spd_std_prior, active_weight, passive_weight);

    return(obj);
}
