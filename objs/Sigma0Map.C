//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_sigma0map_c[] =
    "@(#) $Id$";

#include <math.h>
#include "Array.h"
#include "Constants.h"
#include "Sigma0Map.h"

//===========//
// Sigma0Map //
//===========//

Sigma0Map::Sigma0Map()
:   _map(NULL), _lonSamples(0), _latSamples(0), _lonResolution(0.0),
    _latResolution(0.0)
{
    return;
}

Sigma0Map::~Sigma0Map()
{
    if (_map)
        _Deallocate();
    return;
}

//-----------------//
// Sigma0Map::Read //
//-----------------//

int
Sigma0Map::Read(
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

    int size = sizeof(float) * _latSamples;
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

//------------------//
// Sigma0Map::Write //
//------------------//

int
Sigma0Map::Write(
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

    int size = sizeof(float) * _latSamples;
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

//----------------------//
// Sigma0Map::GetSigma0 //
//----------------------//
// lon must be between 0 and +two_pi radians
// lat must be between -pi/2 and +pi/2 radians

float
Sigma0Map::GetSigma0(
    float  lon,
    float  lat)
{
    lon += two_pi;    // to make sure it is in range
    int lon_idx = (int)(lon / _lonResolution);
    int lat_idx = (int)((lat + pi_over_two) / _latResolution);
    lon_idx = lon_idx % _lonSamples;
    if (lat_idx < 0)
        lat_idx = 0;
    if (lat_idx >= _latSamples)
        lat_idx = _latSamples;

    float sigma0 = *(*(_map + lon_idx) + lat_idx);
    return(sigma0);
}

//---------------------//
// Sigma0Map::Allocate //
//---------------------//

int
Sigma0Map::Allocate(
    int  lon_samples,
    int  lat_samples)
{
    _lonSamples = lon_samples;
    _latSamples = lat_samples;

    _lonResolution = two_pi / _lonSamples;
    _latResolution = pi / _latSamples;

    return(_Allocate());
}

//----------------------//
// Sigma0Map::_Allocate //
//----------------------//

int
Sigma0Map::_Allocate()
{
    _map = (char**)make_array(sizeof(float), 2, _lonSamples,
        _latSamples);
    if (_map == NULL)
        return(0);
    return(1);
}

//------------------------//
// Sigma0Map::_Deallocate //
//------------------------//

int
Sigma0Map::_Deallocate()
{
    free_array((void*)_map, 2, _lonSamples, _latSamples);
    return(1);
}
