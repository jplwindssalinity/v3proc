//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef SIGMA0MAP_H
#define SIGMA0MAP_H

static const char rcs_id_sigma0map_h[] =
    "@(#) $Id$";

#include "Sigma0Map.h"
#include "LonLat.h"

//======================================================================
// CLASSES
//    Sigma0Map
//======================================================================


//======================================================================
// CLASS
//    Sigma0Map
//
// DESCRIPTION
//    The Sigma0Map object contains a longitude and latitude map of
//    average sigma-0 over the earth. It has a method which takes
//    lon and lat and returns the average sigma-0 value.
//======================================================================

class Sigma0Map
{
public:

    //--------------//
    // construction //
    //--------------//

    Sigma0Map();
    ~Sigma0Map();

    int    Read(const char* filename);
    int    Write(const char* filename);
    float  GetSigma0(float lon, float lat);

    int    Allocate(int lon_samples, int lat_samples);
    int    Fill(char value);

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
