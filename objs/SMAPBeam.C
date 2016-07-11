#include <stdio.h>
#include <math.h>
#include "Constants.h"
#include "hdf5.h"
#include "hdf5_hl.h"
#include "SMAPBeam.h"


SMAPBeam::SMAPBeam() {
    return;
}

SMAPBeam::~SMAPBeam() {
    return;
}

SMAPBeam::SMAPBeam(const char* filename) {
    Read(filename);
    return;
}

int SMAPBeam::Get(double theta, double phi, double* gain_h, double* gain_v) {

    // theta -- angle from boresight
    // phi -- azimutha angle about boresight, zero in direciton of increasing
    // incidence angle.

    theta *= rtd;
    phi *= rtd;

    if(phi<0) phi += 360;

    int itheta0 = floor(theta*10);
    int iphi0 = floor(phi*10);

    int itheta1 = itheta0 + 1;
    int iphi1 = iphi0 + 1;

    float dtheta = theta - 0.1*floor(theta*10);
    float dphi = phi - 0.1*floor(phi*10);

    if(itheta0 >= 0 && itheta1 < _n_theta && iphi0 >= 0 && iphi1 < _n_phi) {
        float g00h = _gainh[iphi0][itheta0];
        float g01h = _gainh[iphi0][itheta1];
        float g10h = _gainh[iphi1][itheta0];
        float g11h = _gainh[iphi1][itheta1];

        float g0h = g00h + (g01h-g00h)*dtheta/(double)0.1;
        float g1h = g10h + (g11h-g10h)*dtheta/(double)0.1;
        *gain_h = g0h + (g1h-g0h)*dphi/(double)0.1;

        float g00v = _gainv[iphi0][itheta0];
        float g01v = _gainv[iphi0][itheta1];
        float g10v = _gainv[iphi1][itheta0];
        float g11v = _gainv[iphi1][itheta1];

        float g0v = g00v + (g01v-g00v)*dtheta/(double)0.1;
        float g1v = g10v + (g11v-g10v)*dtheta/(double)0.1;
        *gain_v = g0v + (g1v-g0v)*dphi/(double)0.1;
        return 1;

    } else {

        *gain_h = 0;
        *gain_v = 0;
        return 0;
    }
}

int SMAPBeam::Read(const char* filename) {

    _gainh.resize(_n_phi);
    _gainv.resize(_n_phi);

    for(int iphi = 0; iphi < _n_phi; ++iphi) {
        _gainh[iphi].resize(_n_theta);
        _gainv[iphi].resize(_n_theta);
    }

    hid_t id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);

    std::vector<float> g1h, g2h, g3h, g4h;
    std::vector<float> g1v, g2v, g3v, g4v;

    hsize_t dims[2];
    H5T_class_t class_id;
    size_t type_size;

    g1h.resize(_n_theta*_n_phi);
    g2h.resize(_n_theta*_n_phi);
    g3h.resize(_n_theta*_n_phi);
    g4h.resize(_n_theta*_n_phi);

    g1v.resize(_n_theta*_n_phi);
    g2v.resize(_n_theta*_n_phi);
    g3v.resize(_n_theta*_n_phi);
    g4v.resize(_n_theta*_n_phi);

    H5LTread_dataset_float(id, "/Gain/G1h", &g1h[0]);
    H5LTread_dataset_float(id, "/Gain/G2h", &g2h[0]);
    H5LTread_dataset_float(id, "/Gain/G3h", &g3h[0]);
    H5LTread_dataset_float(id, "/Gain/G4h", &g4h[0]);

    H5LTread_dataset_float(id, "/Gain/G1v", &g1v[0]);
    H5LTread_dataset_float(id, "/Gain/G2v", &g2v[0]);
    H5LTread_dataset_float(id, "/Gain/G3v", &g3v[0]);
    H5LTread_dataset_float(id, "/Gain/G4v", &g4v[0]);

    for(int iphi = 0; iphi < _n_phi; ++iphi) {
        for(int itheta = 0; itheta < _n_theta; ++itheta) {

            int idx = itheta + iphi * _n_theta;

            _gainh[iphi][itheta] = 
                pow(g1h[idx], 2) + pow(g2h[idx], 2) +
                pow(g3h[idx], 2) + pow(g4h[idx], 2);

            _gainv[iphi][itheta] = 
                pow(g1v[idx], 2) + pow(g2v[idx], 2) +
                pow(g3v[idx], 2) + pow(g4v[idx], 2);

        }
    }


    H5Fclose(id);

    return 1;
}
