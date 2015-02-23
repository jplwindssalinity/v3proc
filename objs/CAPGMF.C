#include <math.h>
#include "CAPGMF.h"
#include "Meas.h"
#include "Array.h"
#include "Constants.h"

CAPGMF::CAPGMF() :_tbflat(NULL), _erough(NULL) {
    return;
}

CAPGMF::~CAPGMF() {
    _Deallocate();
    return;
}

int CAPGMF::_MetToIndex(Meas::MeasTypeE met) {
    int idx = 0;
    switch(met) {
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

int CAPGMF::GetTBFlat(
    Meas::MeasTypeE met, float inc_in, float sst, float sss, float* tbflat) {

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

    return(0);
}

int CAPGMF::GetDTB(
        Meas::MeasTypeE met, float inc_in, float sst, float spd, float dir_in,
        float* dtb) {

    // returns the rough surface model emissivity from a look up table

    float inc = inc_in * 180/pi;
    float dir = dir_in * 180/pi;

    while(dir<0) dir+= 360;
    while(dir>=360) dir -= 360;

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

    float e_rough =
        dira * spda * inca * _erough[met_idx][idir0][ispd0][iinc0] +
        dira * spda * incb * _erough[met_idx][idir0][ispd0][iinc1] +
        dira * spdb * inca * _erough[met_idx][idir0][ispd1][iinc0] +
        dira * spdb * incb * _erough[met_idx][idir0][ispd1][iinc1] +
        dirb * spda * inca * _erough[met_idx][idir1][ispd0][iinc0] +
        dirb * spda * incb * _erough[met_idx][idir1][ispd0][iinc1] +
        dirb * spdb * inca * _erough[met_idx][idir1][ispd1][iinc0] +
        dirb * spdb * incb * _erough[met_idx][idir1][ispd1][iinc1];

    *dtb = sst * e_rough;

    return(0);
}

int CAPGMF::GetTB(
        Meas::MeasTypeE met, float inc, float sst, float sss, float spd,
        float dir, float* tb) {

    float tbflat, dtb;

    if(GetTBFlat(met, inc, sst, sss, &tbflat)||
       GetDTB(met, inc, sst, spd, dir, &dtb)) {
        return(1);
    }

    *tb = tbflat + dtb;
    return(0);
}

int CAPGMF::ReadFlat(const char*  filename) {

    if(!_tbflat)
        _AllocateFlat();

    FILE* ifp = fopen(filename, "r");

    for(int met_idx = 0; met_idx < _metCount; ++met_idx) {
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

int CAPGMF::_AllocateFlat() {
    _tbflat = (float ****)make_array(
        sizeof(float), 4, _metCount, _sssCount, _sstCount, _incCount);

    if(!_tbflat)
        return(0);

    return(1);
}

int CAPGMF::ReadRough(const char*  filename) {

    if(!_erough)
        _AllocateRough();

    FILE* ifp = fopen(filename, "r");

    for(int met_idx = 0; met_idx < _metCount; ++met_idx) {
        for(int dir_idx = 0; dir_idx < _dirCount; ++dir_idx) {
            for(int spd_idx = 0; spd_idx < _spdCount; ++spd_idx) {
                float values[_incCount];
                if(fread(&values, sizeof(float), _incCount, ifp) != _incCount) {
                    fclose(ifp);
                    return(0);
                }
                for(int inc_idx = 0; inc_idx < _incCount; ++inc_idx) {
                    _erough[met_idx][dir_idx][spd_idx][inc_idx] = values[inc_idx];
                }
            }
        }
    }
    fclose(ifp);
    return(1);
}

int CAPGMF::_AllocateRough() {
    _erough = (float ****)make_array(
        sizeof(float), 4, _metCount, _dirCount, _spdCount, _incCount);

    if(!_erough)
        return(0);

    return(1);
}

int CAPGMF::_Deallocate() {

    if (_tbflat) {
        free_array(
            (void *)_tbflat, 4, _metCount, _sssCount, _sstCount, _incCount);
        _tbflat = NULL;
    }

    if (_erough) {
        free_array(
            (void *)_erough, 4, _metCount, _dirCount, _spdCount, _incCount);
        _erough = NULL;
    }

    return(1);
}