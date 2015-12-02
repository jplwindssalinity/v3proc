#ifndef L2BTBONLY_H
#define L2BTBONLY_H

#include <vector>
#include "hdf5.h"
#include "hdf5_hl.h"

class L2BTBOnly {
public:
    L2BTBOnly(const char* filename);
    ~L2BTBOnly();

    int nati;
    int ncti;

    std::vector<float> anc_dir;
    std::vector<float> anc_spd;
    std::vector<float> anc_sss;
    std::vector<float> anc_sst;
    std::vector<float> anc_swh;
    std::vector<float> azi_aft;
    std::vector<float> azi_fore;
    std::vector<float> inc_aft;
    std::vector<float> inc_fore;
    std::vector<float> lat;
    std::vector<float> lon;
    std::vector<unsigned char> n_h_aft;
    std::vector<unsigned char> n_h_fore;
    std::vector<unsigned char> n_v_aft;
    std::vector<unsigned char> n_v_fore;
    std::vector<float> nedt_h_aft;
    std::vector<float> nedt_h_fore;
    std::vector<float> nedt_v_aft;
    std::vector<float> nedt_v_fore;
    std::vector<unsigned short> quality_flag;
    std::vector<float> tb_h_aft;
    std::vector<float> tb_h_fore;
    std::vector<float> tb_v_aft;
    std::vector<float> tb_v_fore;
    std::vector<float> smap_spd;
    std::vector<float> smap_sss;

protected:
    int _Allocate();

};

#endif