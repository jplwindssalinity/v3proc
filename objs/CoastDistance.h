#ifndef COASTDISTANCE_H
#define COASTDISTANCE_H

#include <stdlib.h>
#include <vector>
#include "EarthPosition.h"

//
// Class for distance from coast files, generated from:
// http://oceancolor.gsfc.nasa.gov/cms/DOCS/DistFromCoast, converted to a simple
// binary file with one array int16 of size [36000, 18000]
// lon varying most quickly in file.  One int16 per 0.01 degree in lat, lon
//

class CoastDistance {

    public:
        CoastDistance();
        ~CoastDistance();

        int Read(const char* filename);
        int Get(double lon, double lat, double* distance);
        int Get(EarthPosition* pos, double* distance);

    protected:

        std::vector<std::vector<unsigned short> > _distance;

        static const float _lon_0 = -180;
        static const float _lat_0 = 90;
        static const float _dlon = 0.01;
        static const float _dlat = -0.01;
        static const int _nlon = 36000;
        static const int _nlat = 18000;
};

#endif