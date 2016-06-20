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

        float delta;
        float lon0, lat0;
        int nlon;
        int nlat;

    protected:

        std::vector<std::vector<short> > _distance;
};

#endif
