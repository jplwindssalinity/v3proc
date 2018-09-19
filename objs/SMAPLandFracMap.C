
#include <stdlib.h>
#include <vector>
#include <math.h>
#include "Meas.h"
#include "Constants.h"
#include "SMAPLandFracMap.h"
#include "EarthPosition.h"

SMAPLandTBNearMap::SMAPLandTBNearMap() {
    return;
}

SMAPLandTBNearMap::SMAPLandTBNearMap(const char* filename) {
    Read(filename);
    return;
}

SMAPLandTBNearMap::~SMAPLandTBNearMap() {
    return;
}

int SMAPLandTBNearMap::Read(const char* filename) {

    FILE* ifp = fopen(filename, "r");
    if(!ifp)
        return 0;

    _tb_near.resize(2);
    _tb_near[0].resize(_nlon*_nlat*_nmonths);
    _tb_near[1].resize(_nlon*_nlat*_nmonths);

    fread(&_tb_near[0][0], sizeof(float), _nlon*_nlat*_nmonths, ifp);
    fread(&_tb_near[1][0], sizeof(float), _nlon*_nlat*_nmonths, ifp);
    fclose(ifp);
    return 1;
}

int SMAPLandTBNearMap::Get(Meas* meas, int imonth, float* value) {

    double alt, lon, lat;
    if(!meas->centroid.GetAltLonGDLat(&alt, &lon, &lat))
        return 0;

    return Get(lon, lat, imonth, meas->measType, value);
}

int SMAPLandTBNearMap::Get(
    float lon, float lat, int imonth, Meas::MeasTypeE meas_type, float* value) {

    int c_month = imonth - 1;

    int ipol;
    if(meas_type == Meas::L_BAND_TBV_MEAS_TYPE)
        ipol = 0;
    else if(meas_type == Meas::L_BAND_TBH_MEAS_TYPE)
        ipol = 1;
    else {
        *value = 0;
        return 0;
    }

    lon *= rtd;
    lat *= rtd;

    if(lon < -180) lon += 360;
    if(lon >= 180) lon -= 360;

    int ilon = round((lon-_lon_0)/_delta);
    int ilat = round((lat-_lat_0)/_delta);

    if(ilon == _nlon) ilon = 0;

    if(ilon < 0 || ilon >= _nlon || ilat < 0 || ilat >= _nlat ||
       c_month < 0 || c_month >= 12) {
       *value = 0;
       return 0;
    }

    *value = _tb_near[ipol][ilon+(ilat+c_month*_nlat)*_nlon];
    return 1;
}

SMAPLandFracMap::SMAPLandFracMap() {
    return;
}

SMAPLandFracMap::SMAPLandFracMap(const char* filename) {
    Read(filename);
    return;
}

SMAPLandFracMap::~SMAPLandFracMap() {
    return;
}

int SMAPLandFracMap::Read(const char* filename) {

    FILE* ifp = fopen(filename, "r");
    if(!ifp)
        return(0);

    fread(&_delta, sizeof(float), 1, ifp);
    fread(&_dazi, sizeof(float), 1, ifp);

    _nlon = (int)round(360/_delta);
    _nlat = 1 + (int)round(180/_delta);
    _nazi = (int)round(360/_dazi);
    _lfmap.resize(_nlon*_nlat*_nazi);

    fread(&_lfmap[0], sizeof(float), _nlon*_nlat*_nazi, ifp);
    fclose(ifp);
    return 1;
}

int SMAPLandFracMap::Get(Meas* meas, float* land_frac) {

    // convert to clockwise from north
    float azi = (pi_over_two + two_pi) - meas->eastAzimuth;
    if(azi >= two_pi) azi -= two_pi;
    if(azi < 0) azi += two_pi;

    double alt, lon, lat;
    if(!meas->centroid.GetAltLonGDLat(&alt, &lon, &lat))
        return 0;

    return Get(lon, lat, azi, land_frac);
}

int SMAPLandFracMap::Get(float lon, float lat, float azi, float* land_frac) {

    lon *= rtd;
    lat *= rtd;
    azi *= rtd;

    int ilon0 = floor((lon-_lon_0)/_delta);
    int ilat0 = floor((lat-_lat_0)/_delta);
    int iazi0 = floor((azi-_azi_0)/_dazi);

    float lon0 = _lon_0 + (float)ilon0 * _delta;
    float flon = (lon-lon0)/_delta;

    float lat0 = _lat_0 + (float)ilat0 * _delta;
    float flat = (lat-lat0)/_delta;

    float azi0 = _azi_0 + (float)iazi0 * _dazi;
    float fazi = (azi-azi0)/_dazi;

    if(ilon0 == -1) ilon0 = _nlon - 1;
    if(ilon0 == _nlon) ilon0 = 0;

    if(iazi0 == -1) iazi0 = _nazi - 1;
    if(iazi0 == _nazi) iazi0 = 0;

    int ilon1 = ilon0 + 1;
    int ilat1 = ilat0 + 1;
    int iazi1 = iazi0 + 1;

    int idx000 = ilon0 + (ilat0 + iazi0 * _nlat) * _nlon;
    int idx001 = ilon0 + (ilat0 + iazi1 * _nlat) * _nlon;
    int idx010 = ilon0 + (ilat1 + iazi0 * _nlat) * _nlon;
    int idx011 = ilon0 + (ilat1 + iazi1 * _nlat) * _nlon;
    int idx100 = ilon1 + (ilat0 + iazi0 * _nlat) * _nlon;
    int idx101 = ilon1 + (ilat0 + iazi1 * _nlat) * _nlon;
    int idx110 = ilon1 + (ilat1 + iazi0 * _nlat) * _nlon;
    int idx111 = ilon1 + (ilat1 + iazi1 * _nlat) * _nlon;

    *land_frac = 
        (1-flon)*(1-flat)*(1-fazi)*_lfmap[idx000] + 
        (1-flon)*(1-flat)*   fazi *_lfmap[idx000] + 
        (1-flon)*   flat *(1-fazi)*_lfmap[idx000] + 
        (1-flon)*   flat *   fazi *_lfmap[idx000] + 
           flon *(1-flat)*(1-fazi)*_lfmap[idx000] + 
           flon *(1-flat)*   fazi *_lfmap[idx000] + 
           flon *   flat *(1-fazi)*_lfmap[idx000] + 
           flon *   flat *   fazi *_lfmap[idx000];

    return(1);
}
