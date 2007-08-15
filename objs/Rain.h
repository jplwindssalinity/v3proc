//==============================================================//
// Copyright (C) 2006, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef RAIN_H
#define RAIN_H

static const char rcs_id_rain_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include "Matrix.h"
#include "LonLat.h"
#include "Index.h"
#include "List.h"
#include "List.h"
#include "Meas.h"
#include "Spacecraft.h"
#include "CoordinateSwitch.h"

#define N_LAYERS 30  // for 15 km and layer separation 0.5 km
#define DZ_LAYER 0.5 // 0.5 km or 500 m
#define RAINCELL_DX 1.3 // in km
#define RAINCELL_DY 1.3 // in km
#define DIELECTRIC_WATER_CONST_SQ 0.93

class RainField 
{
public:

    //--------------//
    // construction //
    //--------------//

    RainField();
    ~RainField();

    //--------------//
    // input/output //
    //--------------//

    int  ReadSVBinary(char* file1, char* file2);
    int  ReadSV3DData(char* file1, char* file2);

    //--------//
    // access //
    //--------//

    int  NearestABLinear(LonLat lon_lat,float inc,float& atten, float& backscat);
    int  InterpolateABLinear(LonLat lon_lat,float inc,float& atten, float& backscat);
    
    int GetAttn(double alt, double lon, double lat, float* cellAttn);
    int ComputeAttn(Spacecraft* spacecraft, EarthPosition spot_centroid,
                    EarthPosition target, float incAngle,
                    CoordinateSwitch gc_to_rangeazim, float* attn);
    int GetRefl(double alt, double lon, double lat, float* cellRefl);
    int ComputeLoc(Spacecraft* spacecraft, Meas* meas, EarthPosition spot_centroid,
                    CoordinateSwitch gc_to_rangeazim, float** rainRngAz);
    int GetSplash(double lon, double lat, float inc, float* rainSpl);
    int ComputeAmbEs(Spacecraft* spacecraft, EarthPosition spot_centroid,
                     EarthPosition target, float incAngle,
                     CoordinateSwitch gc_to_rangeazim, float ambs0, float* combs0);

    float lat_min, lat_max;
    float lon_min, lon_max;
    int num_lats, num_lons;
    float inc_thresh;
    int flag_3d, num_hgts;
    float const_ZtoSigma;

protected:

    //--------------//
    // construction //
    //--------------//

    int  _Deallocate();
    int  _Allocate();

    //-----------//
    // variables //
    //-----------//

    Index  _lon;
    Index  _lat;

    int    _wrap;             // flag for longitude wrapping

    float***  A; //  _attenuationField;
    float***  vB; // _volumeBackscatterField;
    float***  sB; // _unattenuatedSplashField;
    int*** flag;

    float* hgtInc; // dz for each layer
    float*** A3;
    float*** vB3;
};

#endif
