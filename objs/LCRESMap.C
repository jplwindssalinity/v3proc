#include <stdio.h>
#include <math.h>
#include <Eigen/SparseCore>
#include "LCRESMap.h"
#include "Constants.h"
#include "EarthPosition.h"

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


