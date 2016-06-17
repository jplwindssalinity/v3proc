#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <Eigen/SparseCore>
#include "LCRESMap.h"
#include "Constants.h"
#include "EarthPosition.h"
#include "Misc.h"

LCRESMap::LCRESMap() {
    _sum_dX.resize(4*_nazi);
    _sum_dX_value.resize(4*_nazi);

    for(int iarray = 0; iarray < 4*_nazi; ++iarray) {
        _sum_dX[iarray].resize(_nlon, _nlat);
        _sum_dX_value[iarray].resize(_nlon, _nlat);
    }

    return;
}

LCRESMap::~LCRESMap() {
    return;
}

int LCRESMap::Read(const char* filename) {

    FILE* ifp = fopen(filename, "r");
    if(!ifp)
        return(0);

    for(int iarray = 0; iarray < 4*_nazi; ++iarray) {
        int num_entries;
        fread(&num_entries, sizeof(int), 1, ifp);

        if(!num_entries)
            continue;

        int ilons[num_entries], ilats[num_entries];
        float sum_dX[num_entries], sum_dX_value[num_entries];

        fread(&ilons[0], sizeof(int), num_entries, ifp);
        fread(&ilats[0], sizeof(int), num_entries, ifp);
        fread(&sum_dX[0], sizeof(float), num_entries, ifp);
        fread(&sum_dX_value[0], sizeof(float), num_entries, ifp);

        std::vector<Eigen::Triplet<float> > sum_dX_triplet;
        std::vector<Eigen::Triplet<float> > sum_dX_value_triplet;

        sum_dX_triplet.reserve(num_entries);
        sum_dX_value_triplet.reserve(num_entries);

        for(int ii=0; ii< num_entries; ++ii) {
            sum_dX_triplet.push_back(
                Eigen::Triplet<float>(ilons[ii], ilats[ii], sum_dX[ii]));

            sum_dX_value_triplet.push_back(
                Eigen::Triplet<float>(ilons[ii], ilats[ii], sum_dX_value[ii]));
        }

        _sum_dX[iarray].setFromTriplets(
            sum_dX_triplet.begin(), sum_dX_triplet.end());

        _sum_dX_value[iarray].setFromTriplets(
            sum_dX_value_triplet.begin(), sum_dX_value_triplet.end());
    }
    fclose(ifp);
    return(1);
}

int LCRESMap::Write(const char* filename) {
    FILE* ofp = fopen(filename, "w");
    if(!ofp)
        return(0);

    for(int iarray = 0; iarray < 4*_nazi; ++iarray) {

        std::vector<int> ilon, ilat;
        std::vector<float> sum_dX, sum_dX_value;

        for(int k=0; k<_sum_dX[iarray].outerSize(); ++k) {
            for(Eigen::SparseMatrix<float>::InnerIterator 
                it(_sum_dX[iarray], k); it; ++it) {

                ilon.push_back(it.row());
                ilat.push_back(it.col());
                sum_dX.push_back(it.value());
                sum_dX_value.push_back(
                    _sum_dX_value[iarray].coeffRef(it.row(), it.col()));
            }
        }

        int num_entries = ilon.size();

        fwrite(&num_entries, sizeof(int), 1, ofp);
        fwrite(&ilon[0], sizeof(int), num_entries, ofp);
        fwrite(&ilat[0], sizeof(int), num_entries, ofp);
        fwrite(&sum_dX[0], sizeof(float), num_entries, ofp);
        fwrite(&sum_dX_value[0], sizeof(float), num_entries, ofp);
    }

    return(1);
}

int LCRESMap::Add(EarthPosition* pos, float east_azi, float dX,
                  int ipol, int is_asc, float value) {

    // pos is EarthPosition of point with contributions dX to X int.
    // east_azi is east azimuth angle of this obs.
    // ipol == 0 or 1. (VV, HH)

    // Compute indicies into tables, check bounds
    int ilon, ilat, iazi;
    if(!_GetIdx(pos, east_azi, &iazi, &ilon, &ilat) || ipol<0 || ipol>1) {
        return(0);
    } else {
        int ipart = (is_asc) ? 0 : 1;
        int array_idx = iazi*4 + ipol*2 + ipart;

        _sum_dX[array_idx].coeffRef(ilon, ilat) += dX;
        _sum_dX_value[array_idx].coeffRef(ilon, ilat) += dX * value;
        return(1);
    }
}

int LCRESMap::Get(
    EarthPosition* pos, float east_azi, int ipol, int is_asc, float* value) {

    int ilon, ilat, iazi;
    _GetIdx(pos, east_azi, &iazi, &ilon, &ilat);

    int ipart = (is_asc) ? 0 : 1;
    int array_idx = iazi*4 + ipol*2 + ipart;

    *value = 
        _sum_dX_value[array_idx].coeff(ilon, ilat) /
        _sum_dX[array_idx].coeff(ilon, ilat);

    return(1);
}

int LCRESMap::_GetIdx(
    EarthPosition* pos, float east_azi, int* iazi, int* ilon, int* ilat){

    double lon, lat, alt;
    pos->GetAltLonGDLat(&alt,&lon,&lat);

    lon *= rtd;
    lat *= rtd;
    east_azi *= rtd;

    while(lon > 180) lon -= 360;
    while(lon <= -180) lon += 360;

    int ilat_ = round((lat-_lat_min)/_dlat);
    int ilon_ = round((lon-_lon_min)/_dlon);
    int iazi_ = round(east_azi/_dazi);

    if(iazi_ == _nazi)
        iazi_ == 0;

    if(ilat_<0 || ilat_>=_nlat || ilon_<0 || ilon_>=_nlon || iazi_<0 ||
       iazi_>=_nazi) {
        return(0);
    } else {
        *iazi = iazi_;
        *ilon = ilon_;
        *ilat = ilat_;
        return(1);
    }
}



LCRESMapTile::LCRESMapTile(const char* filename) {
    _sum_dX.resize(4*_nazi);
    _sum_dX_value.resize(4*_nazi);

    for(int iarray = 0; iarray < 4*_nazi; ++iarray) {
        _sum_dX[iarray].resize(_nlon, _nlat);
        _sum_dX_value[iarray].resize(_nlon, _nlat);
    }

    if(filename)
        Read(filename);

    return;
}

LCRESMapTile::~LCRESMapTile() {
    return;
};

int LCRESMapTile::Read(const char* filename) {

    FILE* ifp = fopen(filename, "r");
    if(!ifp)
        return(0);

    fread(&_lon_min, sizeof(float), 1, ifp);
    fread(&_lat_min, sizeof(float), 1, ifp);

    for(int iarray = 0; iarray < 4*_nazi; ++iarray) {
        int num_entries;
        fread(&num_entries, sizeof(int), 1, ifp);

        if(!num_entries)
            continue;

        int ilons[num_entries], ilats[num_entries];
        float sum_dX[num_entries], sum_dX_value[num_entries];

        fread(&ilons[0], sizeof(int), num_entries, ifp);
        fread(&ilats[0], sizeof(int), num_entries, ifp);
        fread(&sum_dX[0], sizeof(float), num_entries, ifp);
        fread(&sum_dX_value[0], sizeof(float), num_entries, ifp);

        std::vector<Eigen::Triplet<float> > sum_dX_triplet;
        std::vector<Eigen::Triplet<float> > sum_dX_value_triplet;

        sum_dX_triplet.reserve(num_entries);
        sum_dX_value_triplet.reserve(num_entries);

        for(int ii=0; ii< num_entries; ++ii) {
            sum_dX_triplet.push_back(
                Eigen::Triplet<float>(ilons[ii], ilats[ii], sum_dX[ii]));

            sum_dX_value_triplet.push_back(
                Eigen::Triplet<float>(ilons[ii], ilats[ii], sum_dX_value[ii]));
        }

        _sum_dX[iarray].setFromTriplets(
            sum_dX_triplet.begin(), sum_dX_triplet.end());

        _sum_dX_value[iarray].setFromTriplets(
            sum_dX_value_triplet.begin(), sum_dX_value_triplet.end());
    }
    fclose(ifp);
    return(1);
}

int LCRESMapTile::Write(const char* filename) {
    FILE* ofp = fopen(filename, "w");
    if(!ofp)
        return(0);

    fwrite(&_lon_min, sizeof(float), 1, ofp);
    fwrite(&_lat_min, sizeof(float), 1, ofp);

    for(int iarray = 0; iarray < 4*_nazi; ++iarray) {

        std::vector<int> ilon, ilat;
        std::vector<float> sum_dX, sum_dX_value;

        for(int k=0; k<_sum_dX[iarray].outerSize(); ++k) {
            for(Eigen::SparseMatrix<float>::InnerIterator
                it(_sum_dX[iarray], k); it; ++it) {

                ilon.push_back(it.row());
                ilat.push_back(it.col());
                sum_dX.push_back(it.value());
                sum_dX_value.push_back(
                    _sum_dX_value[iarray].coeffRef(it.row(), it.col()));
            }
        }

        int num_entries = ilon.size();

        fwrite(&num_entries, sizeof(int), 1, ofp);
        fwrite(&ilon[0], sizeof(int), num_entries, ofp);
        fwrite(&ilat[0], sizeof(int), num_entries, ofp);
        fwrite(&sum_dX[0], sizeof(float), num_entries, ofp);
        fwrite(&sum_dX_value[0], sizeof(float), num_entries, ofp);
    }

    return(1);
}

int LCRESMapTile::Get(
    EarthPosition* pos, float east_azi, int ipol, int is_asc, float* value) {

    int ilon, ilat, iazi;
    if(!_GetIdx(pos, east_azi, &iazi, &ilon, &ilat))
        return(0);

    int ipart = (is_asc) ? 0 : 1;
    int array_idx = iazi*4 + ipol*2 + ipart;

    *value =
        _sum_dX_value[array_idx].coeff(ilon, ilat) /
        _sum_dX[array_idx].coeff(ilon, ilat);

    return(1);
}

int LCRESMapTile::_GetIdx(
    EarthPosition* pos, float east_azi, int* iazi, int* ilon, int* ilat){

    double lon, lat, alt;
    pos->GetAltLonGDLat(&alt, &lon, &lat);

    lon *= rtd;
    lat *= rtd;
    east_azi *= rtd;

    while(lon > 180) lon -= 360;
    while(lon <= -180) lon += 360;

    int ilat_ = (int)round((lat-_lat_min)/_dlat);
    int ilon_ = (int)round((lon-_lon_min)/_dlon);
    int iazi_ = (int)round(east_azi/_dazi);

    if(iazi_ == _nazi)
        iazi_ = 0;

    if(ilat_<0 || ilat_>=_nlat || ilon_<0 || ilon_>=_nlon || iazi_<0 ||
       iazi_>=_nazi) {
        return(0);
    } else {
        *iazi = iazi_;
        *ilon = ilon_;
        *ilat = ilat_;
        return(1);
    }
}

LCRESMapTileList::LCRESMapTileList(const char* tile_directory, int max_tiles) {
    num_tiles = max_tiles;
    directory = tile_directory;
    tiles.resize(0);
    return;
};

LCRESMapTileList::~LCRESMapTileList() {
    return;
};

int LCRESMapTileList::Get(
    EarthPosition* pos, float east_azi, int ipol, int is_asc, float* value) {

    if(!_GetIfLoaded(pos, east_azi, ipol, is_asc, value)) {
        _FindAndLoadTile(pos);
        if(!_GetIfLoaded(pos, east_azi, ipol, is_asc, value))
            return(0);
    }
    return(1);
}

int LCRESMapTileList::_GetIfLoaded(
    EarthPosition* pos, float east_azi, int ipol, int is_asc, float* value) {

    // Returns the value if tile is loaded into tiles
    int which_tile = -1;
    for(int i_tile = 0; i_tile < tiles.size(); ++i_tile) {
        if(tiles[i_tile].Get(pos, east_azi, ipol, is_asc, value)) {
            which_tile = i_tile;
            break;
        }
    }

    // If this tile is next to get flushed put it back on top.
    if(which_tile == num_tiles-1)
        std::swap(tiles[which_tile], tiles[0]);

    // Return 0 if failed to find it in tiles.
    if(which_tile == -1)
        return(0);
    else
        return(1);
}

int LCRESMapTileList::_FindAndLoadTile(EarthPosition* pos) {

    // Construct filename for the tile containing EarthPosition "pos".
    double lon, lat, alt;
    pos->GetAltLonGDLat(&alt,&lon,&lat);

    lon *= rtd;
    lat *= rtd;

    while(lon > 180) lon -= 360;
    while(lon <= -180) lon += 360;

    int tile_ll_lon = floor((lon+180)/30) * 30 - 180;
    int tile_ll_lat = floor((lat+90)/30) * 30 - 90;

    char lon_dir = 'E';
    if(tile_ll_lon < 0)
        lon_dir = 'W';

    char lat_dir = 'N';
    if(tile_ll_lat < 0)
        lat_dir = 'S';

    char filename[2048];
    sprintf(
        filename, "%s/lcres_%c%3.3d_%c%2.2d.dat", directory, lon_dir,
        (int)fabs(tile_ll_lon), lat_dir, (int)fabs(tile_ll_lat));


    // If tiles already at num_tiles flush last tile in list.
    if(tiles.size() == num_tiles) {
        tiles.pop_back();
    }

    // Put this tile on top of the list of tiles.
    tiles.insert(tiles.begin(), LCRESMapTile(filename));

    return(1);
}

