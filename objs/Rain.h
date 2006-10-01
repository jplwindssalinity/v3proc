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

    //--------//
    // access //
    //--------//

    int  NearestABLinear(LonLat lon_lat,float inc,float& atten, float& backscat);
    int  InterpolateABLinear(LonLat lon_lat,float inc,float& atten, float& backscat);
    

    float lat_min, lat_max;
    float lon_min, lon_max;
    int num_lats, num_lons;
    float inc_thresh;

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
};

#endif
