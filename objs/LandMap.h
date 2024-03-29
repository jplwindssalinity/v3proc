//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef LANDMAP_H
#define LANDMAP_H

static const char rcs_id_landmap_h[] =
    "@(#) $Id$";

#include <stdlib.h>
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

    int  Initialize(char* filename, int use_map, char* lmtype=NULL, float lonstart=0, float latstart=0);

    int  IsLand(float lon, float lat);
    int  IsLand(LonLat* lon_lat);
    int  IsCoastal(float lon, float lat, float thresh);
    
protected:

    int  IsLandUSGS(float lon, float lat);
    int  IsLandSimple(float lon, float lat);
    int  IsLandOld(float lon, float lat);
    int  ReadOld(char* filename);
    int  ReadSimple(char* filename);
    int  ReadUSGS(char* filename);
    int  ExpandUSGS(float lon, float lat); // expands map and
                                           // returns landflag value
    int  _Allocate();
    int  _Deallocate();

    //-----------//
    // variables //
    //-----------//

    char usgs_dir[100];
    unsigned char**  _map;
    int              _pixelsPerDegree;
    int              _mapLatDim;
    int              _mapLonDim;
    int              _usemap;
    double           _lat_start;
    double           _lon_start;
    double           _lonResolution;
    double           _latResolution;
    int landmap_type;  // 0 = LandMap, 1= SimpleLandMap, 2= USGS Landuse map
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

    int  Initialize(char* filename, int use_map);
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
    int     _usemap;
    int     _lonSamples;
    int     _latSamples;
    float   _lonResolution;
    float   _latResolution;
};

//======================================================================
// CLASS
//    QSLandMap
//
// DESCRIPTION
//    The QSLandMap object contains a map of land and near-land
//    locations.  LandMap from offical QS processing stream.
//======================================================================

class QSLandMap 
{
public:
	QSLandMap();
	~QSLandMap();
	
	int Read( const char* filename );
	int IsLand( float lon, float lat, int flagging_mode );

protected:

    int  _Allocate();
    int  _Deallocate();

    //-----------//
    // variables //
    //-----------//
    
    unsigned char** _map;
};

//======================================================================
// CLASS
//    QSIceMap
//
// DESCRIPTION
//    The QSIceMap object contains a map of ice 
//    locations.  Ice maps are from offical QS processing stream.
//======================================================================

class QSIceMap 
{
public:
	QSIceMap();
	~QSIceMap();
	
	int Read( const char* filename );
	int IsIce( float lon, float lat, int beam_idx );

protected:

    int  _Allocate();
    int  _Deallocate();

    //-----------//
    // variables //
    //-----------//
    
    unsigned char** _map;
};

class ICECMap {
public:
    ICECMap();
    ICECMap(const char* filename);
    ~ICECMap();

    int Read(const char* filename);
    int Get(float lon, float lat, float* ice_concentration);

protected:
    int  _Allocate();
    int  _Deallocate();

    //-----------//
    // variables //
    //-----------//
    float** _map;
};


#endif
