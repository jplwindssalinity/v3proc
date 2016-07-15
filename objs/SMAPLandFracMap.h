#ifndef SMAPLANDFRACMAP_H
#define SMAPLANDFRACMAP_H

#include <stdlib.h>
#include <vector>
#include "Meas.h"

class SMAPLandFracMap {

public:
    SMAPLandFracMap();
    SMAPLandFracMap(const char* filename);
    ~SMAPLandFracMap();

    int Read(const char* filename);
    int Get(float lon, float lat, float azi, float* land_frac);
    int Get(Meas* meas, float* land_frac);

protected:
    std::vector<float> _lfmap;

    float _delta, _dazi;
    static const float _lon_0 = -180;
    static const float _lat_0 = -90;
    static const float _azi_0 = 0;
    int _nlon, _nlat, _nazi;

};

#endif
