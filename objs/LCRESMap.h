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
        static const int _nlat = 3600;
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

class LCRESMapTile {

    public:
        LCRESMapTile(const char* filename = NULL);
        ~LCRESMapTile();

        int Read(const char* filename);
        int Write(const char* filename);

        int Get(
            EarthPosition* pos, float east_azi, int ipol, int is_asc,
            float* value);

    protected:
        int _GetIdx(EarthPosition* pos, float east_azi, int* iazi, int* ilon,
                    int* ilat);

        float _lat_min;
        static const int _nlat = 600;
        static const float _dlat = 0.05;

        float _lon_min;
        static const int _nlon = 600;
        static const float _dlon = 0.05;

        static const float _dazi = 10.0;
        static const int _nazi = 36;

        // Ascending VV, Descending VV, Ascending HH, Descending HH
        std::vector<Eigen::SparseMatrix<float> > _sum_dX;
        std::vector<Eigen::SparseMatrix<float> > _sum_dX_value;

};

class LCRESMapTileList{

    public:

        const char* directory;
        int num_tiles;

        LCRESMapTileList(const char* tile_directory, int max_tiles = 4);
        ~LCRESMapTileList();

        int Get(
            EarthPosition* pos, float east_azi, int ipol, int is_asc,
            float* value);

    protected:

        std::vector<LCRESMapTile> tiles;

        int _GetIfLoaded(
            EarthPosition* pos, float east_azi, int ipol, int is_asc,
            float* value);

        int _FindAndLoadTile(EarthPosition* pos);

};

#endif
