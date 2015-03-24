#include <stdio.h>
#include <math.h>
#include <vector>
#include "CoastDistance.h"
#include "Constants.h"
#include "EarthPosition.h"

CoastDistance::CoastDistance() {
    // resize the array
    _distance.resize(_nlat);
    for(int ilat = 0; ilat<_nlat; ++ilat) {
        _distance[ilat].resize(_nlon);
    }
    return;
}

CoastDistance::~CoastDistance() {
    return;
}

int CoastDistance::Read(const char* filename) {
    FILE* ifp = fopen(filename, "r");
    fread(&_distance[0][0], sizeof(unsigned short), _nlon*_nlat, ifp);
    fclose(ifp);
    return(1);
}

int CoastDistance::Get(double lon, double lat, float* distance) {
    lon *= rtd;
    lat *= rtd;

    while(lon>=180) lon -= 360;
    while(lon<-180) lon += 360;

    int ilon = round((lon-_lon_0)/_dlon);
    int ilat = round((lat-_lat_0)/_dlat);

    if(ilat<0 || ilat>=_nlat || ilon<0 || ilon>=_nlon) {
        return(0);
    } else {
        *distance = (float)_distance[ilat][ilon];
        return(1);
    }
}

int CoastDistance::Get(EarthPosition* pos, float* distance) {
    double lon, lat, alt;
    pos->GetAltLonGDLat(&alt,&lon,&lat);
    return Get(lon, lat, distance);
}

