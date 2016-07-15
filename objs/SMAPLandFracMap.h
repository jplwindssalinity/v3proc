#ifndef SMAPLANDFRACMAP_H
#define SMAPLANDFRACMAP_H

#include <stdlib.h>
#include <vector>
#include "Meas.h"

class SMAPLandTBNearMap {

public:

    SMAPLandTBNearMap();
    SMAPLandTBNearMap(const char* filename);
    ~SMAPLandTBNearMap();

    int Read(const char* filename);
    int Get(
        float lon, float lat, int imonth, Meas::MeasTypeE meas_type,
        float* value);

    int Get(Meas* meas, int imonth, float* value);

protected:

    std::vector<float> _tb_near[2];

    static const float _delta = 0.25;
    static const float _lon_0 = -180 + 0.25/2;
    static const float _lat_0 = -90 + 0.25/2;
    static const int _nlon = 1440;
    static const int _nlat = 720;
    static const int _nmonths = 12;
};

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
