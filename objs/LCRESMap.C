#include <stdio.h>
#include <math.h>
#include <vector>
#include "LCRESMap.h"
#include "Constants.h"
#include "EarthPosition.h"

LCRESMap::LCRESMap() {

    // Allocate the huge arrays for expected sigma0 maps
    _sum_dX.resize(_nazi);
    _sum_dX_value.resize(_nazi);

    for(int iazi=0; iazi<_nazi; ++iazi) {
        _sum_dX[iazi].resize(_nlat);
        _sum_dX_value[iazi].resize(_nlat);

        for(int ilat=0; ilat<_nlat; ++ilat) {
            _sum_dX[iazi][ilat].resize(_nlon);
            _sum_dX_value[iazi][ilat].resize(_nlon);

            for(int ilon=0; ilon<_nlon; ++ilon) {
                _sum_dX[iazi][ilat][ilon].resize(2);
                _sum_dX_value[iazi][ilat][ilon].resize(2);
            }
        }
    }
    return;
}

LCRESMap::~LCRESMap() {
    return;
}

int LCRESMap::Read(const char* filename) {
    FILE* ifp = fopen(filename, "r");
    fread(&_sum_dX[0][0][0][0], sizeof(float), _nlon*_nlat*_nazi*2, ifp);
    fread(&_sum_dX_value[0][0][0][0], sizeof(float), _nlon*_nlat*_nazi*2, ifp);
    fclose(ifp);
    return(1);
}

int LCRESMap::Write(const char* filename) {
    FILE* ofp = fopen(filename, "w");
    fwrite(&_sum_dX[0][0][0][0], sizeof(float), _nlon*_nlat*_nazi*2, ofp);
    fwrite(&_sum_dX_value[0][0][0][0], sizeof(float), _nlon*_nlat*_nazi*2, ofp);
    fclose(ofp);
    return(1);
}

int LCRESMap::Add(EarthPosition* pos, float east_azi, float dX,
                  int ipol, float value) {

    // pos is EarthPosition of point with contributions dX to X int.
    // east_azi is east azimuth angle of this obs.
    // ipol == 0 or 1. (VV, HH)

    // Compute indicies into tables, check bounds
    int ilon, ilat, iazi;
    if(!_GetIdx(pos, east_azi, &iazi, &ilon, &ilat) || ipol<0 || ipol>1) {
        return(0);
    } else {
        _sum_dX[iazi][ilat][ilon][ipol] += dX;
        _sum_dX_value[iazi][ilat][ilon][ipol] += dX * value;
        return(1);
    }
}

int LCRESMap::Get(EarthPosition* pos, float east_azi, int ipol, float* value) {

    int ilon, ilat, iazi;
    _GetIdx(pos, east_azi, &iazi, &ilon, &ilat);

    *value = 
        _sum_dX_value[iazi][ilat][ilon][ipol] / _sum_dX[iazi][ilat][ilon][ipol];

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


