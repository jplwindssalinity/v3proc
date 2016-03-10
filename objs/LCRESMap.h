#ifndef LCRESMap_H
#define LCRESMap_H

#include <stdlib.h>
#include <vector>
#include <Eigen/SparseCore>
#include "EarthPosition.h"

class LCRESMap {

    public:
        LCRESMap();
        ~LCRESMap();

        int Read(const char* filename);
        int Write(const char* filename);

        int Add(
            EarthPosition* pos, float east_azi, float dX, int ipol, int is_asc,
                float value);

        int Get(
            EarthPosition* pos, float east_azi, int ipol, int is_asc,
            float* value);

    protected:
        int _GetIdx(EarthPosition* pos, float east_azi, int* iazi, int* ilon,
                    int* ilat);

        static const float _lat_min = -90.0;
        static const int _nlat = 3200;
        static const float _dlat = 0.05;

        static const float _lon_min = -180.0;
        static const int _nlon = 7200;
        static const float _dlon = 0.05;

        static const float _dazi = 10.0;
        static const int _nazi = 36;

        // Ascending VV, Descending VV, Ascending HH, Descending HH
        std::vector<Eigen::SparseMatrix<float> > _sum_dX;
        std::vector<Eigen::SparseMatrix<float> > _sum_dX_value;

};

#endif
