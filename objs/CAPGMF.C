#include <math.h>
#include "CAPGMF.h"
#include "Meas.h"
#include "Array.h"

// Will have two look up tables for passive:
// flat TB as a function of sss, sst, inc, pol
// excess surface emmisivity as a function of spd, dir, swh, inc, pol

CAPGMF::CAPGMF() :_tbflat(NULL) {
    return;
}

CAPGMF::~CAPGMF() {
    _Deallocate();
    return;
}

int CAPGMF::ReadFlat(const char*  filename) {

    if(!_tbflat)
        _Allocate();

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
                    _tbflat[inc_idx][sst_idx][sss_idx][met_idx] = values[inc_idx];
                }
            }
        }
    }
    fclose(ifp);
    return(1);
}

int CAPGMF::_Allocate() {
    _tbflat = (float ****)make_array(
        sizeof(float), 4, _metCount, _sssCount, _sstCount, _incCount);

    if(!_tbflat)
        return(0);

    return(1);
}

int CAPGMF::_Deallocate() {
    if (_tbflat) {
        free_array(
            (void *)_tbflat, 4, _metCount, _sssCount, _sstCount, _incCount);
        _tbflat = NULL;
    }
    return(1);
}