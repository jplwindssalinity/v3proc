//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef LANDMAP_H
#define LANDMAP_H

static const char rcs_id_landmap_h[] =
    "@(#) $Id$";

#include "LandMap.h"
#include "LonLat.h"

//======================================================================
// CLASSES
//    LandMap, SimpleLandMap
//======================================================================


//======================================================================
// CLASS
//    LandMap
//
// DESCRIPTION
//    The LandMap object contains a longitude and latitude map of
//    where land occurs. It has a method which takes lon and lat
//    returning 0 for ocean or 1 for land.
//======================================================================
//*********************************************************************//
// YOU NEED TO CALL THE Initialize METHOD BEFORE USING THE LANDMAP     //
//*********************************************************************//
class LandMap
{
public:

    //--------------//
    // construction //
    //--------------//

    LandMap();
    ~LandMap();

    int  Initialize(char* filename, int use_map);

    int  IsLand(float lon, float lat);
    int  IsLand(LonLat* lon_lat);

protected:

    int  Read(char* filename);
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

//======================================================================
// CLASS
//    SimpleLandMap
//
// DESCRIPTION
//    The SimpleLandMap object contains a map of land and near-land
//    locations.  It has a method which takes lon and lat
//    returning 0 for ocean or 1 for land.  Generally, this is of
//    coarser resolution than LandMap.
//======================================================================

class SimpleLandMap
{
public:

    //--------------//
    // construction //
    //--------------//

    SimpleLandMap();
    ~SimpleLandMap();

    int  Read(const char* filename);
    int  Write(const char* filename);
    int  GetType(float lon, float lat);
    int  IsLand(float lon, float lat);

    int  Allocate(int lon_samples, int lat_samples);
    int  Fill(char value);

    char**  GetMap()         { return(_map); };
    int     GetLonSamples()  { return(_lonSamples); };
    int     GetLatSamples()  { return(_latSamples); };

protected:

    int  _Allocate();
    int  _Deallocate();

    //-----------//
    // variables //
    //-----------//

    char**  _map;
    int     _lonSamples;
    int     _latSamples;
    float   _lonResolution;
    float   _latResolution;
};

#endif
