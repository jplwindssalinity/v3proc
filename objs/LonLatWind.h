//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef LON_LAT_WIND_H
#define LON_LAT_WIND_H

static const char rcs_id_lon_lat_wind_h[] =
    "@(#) $Id$";

#include "LonLat.h"
#include "WindVector.h"

//======================================================================
// CLASSES
//    LonLatWind
//======================================================================

//======================================================================
// CLASS
//    LonLatWind
//
// DESCRIPTION
//    LonLatWind is a pure virtual class used to define an interface
//    for accessing wind vectors using latitude and longitude.
//======================================================================

class LonLatWind
{
public:
    //--------------//
    // construction //
    //--------------//

    // this is needed to prevent LonLatWind from being given a default
    // non-virtual destructor
    virtual ~LonLatWind() { };

    //--------//
    // access //
    //--------//

    virtual int NearestWindVector(LonLat lon_lat, WindVector* wv) = 0;
    virtual int InterpolatedWindVector(LonLat lon_lat, WindVector* wv) = 0;

    //----------//
    // tweaking //
    //----------//

protected:
};

#endif
