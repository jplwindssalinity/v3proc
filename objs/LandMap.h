//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef LANDMAP_H
#define LANDMAP_H

static const char rcs_id_landmap_h[] =
    "@(#) $Id$";

#include "LandMap.h"
#include "LonLat.h"

//======================================================================
// CLASSES
//    LandMap
//======================================================================


//======================================================================
// CLASS
//    LandMap
//
// DESCRIPTION
//    The LandMap Object contains a longitude and latitude map of
//    where land occurs. It has a method which takes lon and lat
//    returning 0 for ocean or 1 for land.
//======================================================================

class LandMap
{
public:

    //--------------//
    // construction //
    //--------------//

    LandMap();
    ~LandMap();

    int  Initialize(char* filename, int use_map);
    int  Read(char* filename);

    int  IsLand(float lon, float lat);
    int  IsLand(LonLat* lon_lat);

protected:

    int  _Allocate();
    int  _Deallocate();

    //-----------//
    // variables //
    //-----------//
	
    unsigned char**  _map;
    int              _pixelsPerDegree; 
    int              _mapLatDim;
    int              _mapLonDim;
    int              _usemap;
};

#endif
