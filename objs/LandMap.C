//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_landmap_c[] =
    "@(#) $Id$";

#include <math.h>
#include "Array.h"
#include "Constants.h"
#include "LandMap.h"

//=========//
// LandMap //
//=========//

LandMap::LandMap()
:   _map(NULL), _pixelsPerDegree(12)
{
    _mapLatDim = 180 * _pixelsPerDegree;
    _mapLonDim = 45 * _pixelsPerDegree;
    return;
}

LandMap::~LandMap()
{
    if (_map)
        _Deallocate();
    _map=NULL;
    return;
}

//---------------------//
// LandMap::Initialize //
//---------------------//

int
LandMap::Initialize(
    char*  filename,
    int    use_map)
{
    _usemap = use_map;
    if (_usemap != 0)
    {
        if (! Read(filename))
            return(0);
    }
    return(1);
}

//---------------//
// LandMap::Read //
//---------------//

int
LandMap::Read(
    char*  filename)
{
    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "LandMap File Read Failed for file %s.\n", filename);
        exit(1);
    }
    if (! _Allocate())
        return(0);
    for (int c = 0; c < _mapLatDim; c++)
    {
        if (fread((void*)&_map[c][0], sizeof(unsigned char), _mapLonDim,
            ifp) != (unsigned)_mapLonDim)
        {
            return(0);
        }
    }
    fclose(ifp);
    return(1);
}

//-----------------//
// LandMap::IsLand //
//-----------------//

int
LandMap::IsLand(
    float  lon,
    float  lat)
{
    if (_usemap == 0)
        return(0);

    while (lon < 0)
        lon+=two_pi;

    int lon_idx = int(lon * rtd * _pixelsPerDegree + 0.5);
    int lat_idx = int((90.0 + lat * rtd) * _pixelsPerDegree + 0.5);
    int lon_byte_idx = lon_idx / 8;
    int lon_bit_idx = lon_idx % 8;
    if (lon_bit_idx == 0)
        lon_bit_idx = 8;

    lon_bit_idx = 8 - lon_bit_idx;
    if (lat_idx == _mapLatDim)
        return(0);

    if (lon_byte_idx == _mapLonDim)
        lon_byte_idx = 0;

    unsigned char byte = _map[lat_idx][lon_byte_idx];
    int bit = (int)(byte&(0x1 << lon_bit_idx));
    if (bit != 0)
        bit = 1;
    return(bit);
}

//-----------------//
// LandMap::IsLand //
//-----------------//

int
LandMap::IsLand(
    LonLat*  lon_lat)
{
    return(IsLand(lon_lat->longitude, lon_lat->latitude));
}

//--------------------//
// LandMap::_Allocate //
//--------------------//

int
LandMap::_Allocate()
{
    _map = (unsigned char**)make_array(sizeof(char), 2, _mapLatDim,
        _mapLonDim);
    if (! _map)
        return(0);
    return(1);
}

//----------------------//
// LandMap::_Deallocate //
//----------------------//

int
LandMap::_Deallocate()
{
    free_array((void*)_map, 2, _mapLatDim, _mapLonDim);
    return(1);
}

//===============//
// SimpleLandMap //
//===============//

SimpleLandMap::SimpleLandMap()
:   _map(NULL), _lonSamples(0), _latSamples(0), _lonResolution(0.0),
    _latResolution(0.0)
{
    return;
}

SimpleLandMap::~SimpleLandMap()
{
    if (_map)
        _Deallocate();
    return;
}

//---------------------//
// SimpleLandMap::Read //
//---------------------//

int
SimpleLandMap::Read(
    const char*  filename)
{
    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL)
        return(0);

    int lon_samples, lat_samples;
    if (fread((void *)&lon_samples, sizeof(int), 1, ifp) != 1 ||
        fread((void *)&lat_samples, sizeof(int), 1, ifp) != 1)
    {
        fclose(ifp);
        return(0);
    }

    _lonSamples = lon_samples;
    _latSamples = lat_samples;

    if (! _Allocate())
    {
        fclose(ifp);
        return(0);
    }

    int size = sizeof(char) * _latSamples;
    for (int lon_idx = 0; lon_idx < _lonSamples; lon_idx++)
    {
        if (fread((void *)*(_map + lon_idx), size, 1, ifp) != 1)
        {
            fclose(ifp);
            return(0);
        }
    }

    _lonResolution = two_pi / _lonSamples;
    _latResolution = pi / _latSamples;

    fclose(ifp);
    return(1);
}

//----------------------//
// SimpleLandMap::Write //
//----------------------//

int
SimpleLandMap::Write(
    const char*  filename)
{
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    if (fwrite((void *)&_lonSamples, sizeof(int), 1, ofp) != 1 ||
        fwrite((void *)&_latSamples, sizeof(int), 1, ofp) != 1)
    {
        fclose(ofp);
        return(0);
    }

    int size = sizeof(char) * _latSamples;
    for (int lon_idx = 0; lon_idx < _lonSamples; lon_idx++)
    {
        if (fwrite((void *)*(_map + lon_idx), size, 1, ofp) != 1)
        {
            fclose(ofp);
            return(0);
        }
    }

    fclose(ofp);
    return(1);
}

//-----------------------//
// SimpleLandMap::IsLand //
//-----------------------//
// lon must be between -pi and +two_pi radians
// lat must be between -pi/2 and +pi/2 radians

int
SimpleLandMap::IsLand(
    float  lon,
    float  lat)
{
    static const float pi_2 = pi / 2.0;

    lon += two_pi;    // to make sure it is in range
    int lon_idx = (int)(lon / _lonResolution);
    int lat_idx = (int)((lat + pi_2) / _latResolution);
    lon_idx = lon_idx % _lonSamples;
    if (lat_idx < 0)
        lat_idx = 0;
    if (lat_idx >= _latSamples)
        lat_idx = _latSamples;

    int flag = *(*(_map + lon_idx) + lat_idx);
    return(flag);
}

//-------------------------//
// SimpleLandMap::Allocate //
//-------------------------//

int
SimpleLandMap::Allocate(
    int  lon_samples,
    int  lat_samples)
{
    _lonSamples = lon_samples;
    _latSamples = lat_samples;
    return(_Allocate());
}

//---------------------//
// SimpleLandMap::Zero //
//---------------------//

int
SimpleLandMap::Zero()
{
    for (int i = 0; i < _lonSamples; i++)
    {
        for (int j = 0; j < _latSamples; j++)
        {
            *(*(_map + i) + j) = 0;
        }
    }
    return(1);
}

//--------------------------//
// SimpleLandMap::_Allocate //
//--------------------------//

int
SimpleLandMap::_Allocate()
{
    _map = (char**)make_array(sizeof(char), 2, _lonSamples,
        _latSamples);
    if (_map == NULL)
        return(0);
    return(1);
}

//----------------------------//
// SimpleLandMap::_Deallocate //
//----------------------------//

int
SimpleLandMap::_Deallocate()
{
    free_array((void*)_map, 2, _lonSamples, _latSamples);
    return(1);
}
