
#ifndef SMAPBEAM_H
#define SMAPBEAM_H

#include <stdlib.h>
#include <vector>

class SMAPBeam {

public:
    SMAPBeam();
    ~SMAPBeam();
    SMAPBeam(const char* filename);

    int Read(const char* filename);
    int Get(double theta, double phi, double* gain_h, double* gain_v);

protected:
    const static int _n_theta = 1801;
    const static int _n_phi = 3601;
    const static double _delta = 0.1;

    std::vector<std::vector<float> > _gainh, _gainv;

};

#endif
