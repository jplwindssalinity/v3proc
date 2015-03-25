#ifndef LCRESMap_H
#define LCRESMap_H

#include <stdlib.h>
#include <vector>
#include "EarthPosition.h"

class LCRESMap {

    public:
        LCRESMap();
        ~LCRESMap();

        int Read(const char* filename);
        int Write(const char* filename);

        int Add(EarthPosition* pos, float east_azi, float dX, int ipol,
                float value);

        int Get(EarthPosition* pos, float east_azi, int ipol, float* value);

    protected:
        int _GetIdx(EarthPosition* pos, float east_azi, int* iazi, int* ilon,
                    int* ilat);

        std::vector<std::vector<std::vector<std::vector<float> > > > _sum_dX_value;
        std::vector<std::vector<std::vector<std::vector<float> > > > _sum_dX;

        static const float _lat_min = -90.0;
        static const int _nlat = 3600;
        static const float _dlat = 0.05;

        static const float _lon_min = -180.0;
        static const int _nlon = 7200;
        static const float _dlon = 0.05;

        static const float _dazi = 10.0;
        static const int _nazi = 36;
};

#endif