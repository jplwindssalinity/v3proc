#include <stdio.h>
#include <math.h>
#include <vector>
#include "CoastDistance.h"
#include "Constants.h"
#include "EarthPosition.h"

CoastDistance::CoastDistance() {
    return;
}

CoastDistance::~CoastDistance() {
    return;
}

int CoastDistance::Read(const char* filename) {

    FILE* ifp = fopen(filename, "r");
    fseek(ifp, 0, SEEK_END);
    size_t file_size = ftell(ifp);
    fseek(ifp, 0, SEEK_SET);

    if(file_size == 1296000000) {
        delta = 0.01;
        nlat = 180*100;
        nlon = 360*100;

    } else if(file_size == 324000000) {
        delta = 0.02;
        nlat = 180*50;
        nlon = 360*50;
    }

    lat0 = 90-delta/2;
    lon0 = -180+delta/2;

    // resize the array
    _distance.resize(nlat);
    for(int ilat = 0; ilat<nlat; ++ilat) {
        _distance[ilat].resize(nlon);
        fread(&_distance[ilat][0], sizeof(short), nlon, ifp);
    }
    fclose(ifp);
    return(1);
}

int CoastDistance::Get(double lon, double lat, double* distance) {
    lon *= rtd;
    lat *= rtd;

    while(lon>=180) lon -= 360;
    while(lon<-180) lon += 360;

    int ilon = round((lon-lon0)/delta);
    int ilat = round((lat-lat0)/-delta);

    if(ilon == -1) ilon = nlon-1;
    if(ilon == nlon) ilon = 0;

    if(ilat<0 || ilat>=nlat || ilon<0 || ilon>=nlon) {
        return(0);
    } else {
        *distance = (double)_distance[ilat][ilon];
        return(1);
    }
}

int CoastDistance::Get(EarthPosition* pos, double* distance) {
    double lon, lat, alt;
    pos->GetAltLonGDLat(&alt,&lon,&lat);
    return Get(lon, lat, distance);
}

