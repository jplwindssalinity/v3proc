//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef MODU_H
#define MODU_H

static const char rcs_id_modu_h[] =
    "@(#) $Id$";

#include <stdio.h>

//======//
// Modu //
//======//

#define AZIMUTH_BINS  90

class Modu
{
public:
    Modu();

    int           Write(FILE* fp);
    int           Read(FILE* fp);
    float         sigma0[AZIMUTH_BINS];
    unsigned int  count[AZIMUTH_BINS];
};

//==========//
// ModuGrid //
//==========//

class ModuGrid
{
public:
    int      Allocate(float lon_min, float lon_max, float lon_step_size,
                 float lat_min, float lat_max, float lat_step_size);
    int      Allocate(int lon_bins, int lat_bins);
    int      Deallocate();

    int      Accumulate(float lon, float lat, float azimuth, float sigma0);
    int      Add(ModuGrid* other_grid);

    int      Write(const char* filename);
    int      Read(const char* filename);

    Modu***  grid;
    float    lonMin;
    float    lonMax;
    float    lonStep;
    int      lonBins;
    float    latMin;
    float    latMax;
    float    latStep;
    int      latBins;
};

//------------//
// Modu::Modu //
//------------//

Modu::Modu()
{
    for (int i = 0; i < AZIMUTH_BINS; i++)
    {
        sigma0[i] = 0.0;
        count[i] = 0;
    }
    return;
}

//--------------------//
// ModuGrid::Allocate //
//--------------------//

int
ModuGrid::Allocate(
    float  lon_min,
    float  lon_max,
    float  lon_step_size,
    float  lat_min,
    float  lat_max,
    float  lat_step_size)
{
    lonBins = (int)((lon_max - lon_min) / lon_step_size + 0.5);
    lonStep = (lon_max - lon_min) / (float)lonBins;
    latBins = (int)((lat_max - lat_min) / lat_step_size + 0.5);
    latStep = (lat_max - lat_min) / (float)latBins;

    if (! Allocate(lonBins, latBins))
        return(0);

    return(1);
}

//--------------------//
// ModuGrid::Allocate //
//--------------------//

int
ModuGrid::Allocate(
    int  lon_bins,
    int  lat_bins)
{
    grid = (Modu ***)malloc(sizeof(Modu **) * lon_bins);
    if (grid == NULL)
        return(0);
    for (int lon_idx = 0; lon_idx < lon_bins; lon_idx++)
    {
        *(grid + lon_idx) = (Modu **)malloc(sizeof(Modu *) * lat_bins);
        if (*(grid + lon_idx) == NULL)
            return(0);
        for (int lat_idx = 0; lat_idx < lat_bins; lat_idx++)
        {
            *(*(grid + lon_idx) + lat_idx) = new Modu;
            if (*(*(grid + lon_idx) + lat_idx) == NULL)
                return(0);
        }
    }
    return(1);
}

//----------------------//
// ModuGrid::Deallocate //
//----------------------//

int
ModuGrid::Deallocate()
{
    for (int lon_idx = 0; lon_idx < lonBins; lon_idx++)
    {
        for (int lat_idx = 0; lat_idx < latBins; lat_idx++)
        {
            Modu* modu = *(*(grid + lon_idx) + lat_idx);
            delete modu;
        }
        Modu** modup = *(grid + lon_idx);
        free(modup);
    }
    Modu*** modupp = grid;
    free(modupp);

    lonMin = 0.0;
    lonMax = 0.0;
    lonStep = 0.0;
    lonBins = 0;
    latMin = 0.0;
    latMax = 0.0;
    latStep = 0.0;
    latBins = 0;
    grid = NULL;

    return(1);
}

//----------------------//
// ModuGrid::Accumulate //
//----------------------//

int
ModuGrid::Accumulate(
    float  lon,
    float  lat,
    float  azimuth,
    float  sigma0)
{
    int lon_idx = (int)((lon - lonMin) / lonStep + 0.5);
    if (lon_idx < 0 || lon_idx >= lonBins)
        return(1);
    int lat_idx = (int)((lat - latMin) / latStep + 0.5);
    if (lat_idx < 0 || lat_idx >= latBins)
        return(1);

    Modu* modu = *(*(grid + lon_idx) + lat_idx);
    int az_idx = (int)(AZIMUTH_BINS * azimuth / two_pi + 0.5);
    az_idx %= AZIMUTH_BINS;
    modu->sigma0[az_idx] += sigma0;
    modu->count[az_idx]++;

    return(1);
}

//---------------//
// ModuGrid::Add //
//---------------//

int
ModuGrid::Add(
    ModuGrid*  other_grid)
{
    for (int lon_idx = 0; lon_idx < lonBins; lon_idx++)
    {
        for (int lat_idx = 0; lat_idx < latBins; lat_idx++)
        {
            Modu* modu = *(*(grid + lon_idx) + lat_idx);
            Modu* other_modu = *(*(other_grid->grid + lon_idx) + lat_idx);
            for (int az_idx = 0; az_idx < AZIMUTH_BINS; az_idx++)
            {
                modu->count[az_idx] += other_modu->count[az_idx];
                modu->sigma0[az_idx] += other_modu->sigma0[az_idx];
            }
        }
    }
    return(1);
}

//-----------------//
// ModuGrid::Write //
//-----------------//

int
ModuGrid::Write(
    const char*  filename)
{
    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
        return(0);

    if (fwrite((void *)&lonMin, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&lonMax, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&lonStep, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&lonBins, sizeof(int), 1, fp) != 1 ||
        fwrite((void *)&latMin, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&latMax, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&latStep, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&latBins, sizeof(int), 1, fp) != 1)
    {
        return(0);
    }

    for (int lon_idx = 0; lon_idx < lonBins; lon_idx++)
    {
        for (int lat_idx = 0; lat_idx < latBins; lat_idx++)
        {
            Modu* modu = *(*(grid + lon_idx) + lat_idx);
            if (! modu->Write(fp))
                return(0);
        }
    }
    return(1);
}

//----------------//
// ModuGrid::Read //
//----------------//

int
ModuGrid::Read(
    const char*  filename)
{
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return(0);

    if (fread((void *)&lonMin, sizeof(float), 1, fp) != 1 ||
        fread((void *)&lonMax, sizeof(float), 1, fp) != 1 ||
        fread((void *)&lonStep, sizeof(float), 1, fp) != 1 ||
        fread((void *)&lonBins, sizeof(int), 1, fp) != 1 ||
        fread((void *)&latMin, sizeof(float), 1, fp) != 1 ||
        fread((void *)&latMax, sizeof(float), 1, fp) != 1 ||
        fread((void *)&latStep, sizeof(float), 1, fp) != 1 ||
        fread((void *)&latBins, sizeof(int), 1, fp) != 1)
    {
        return(0);
    }

    if (! Allocate(lonBins, latBins))
        return(0);

    for (int lon_idx = 0; lon_idx < lonBins; lon_idx++)
    {
        for (int lat_idx = 0; lat_idx < latBins; lat_idx++)
        {
            Modu* modu = *(*(grid + lon_idx) + lat_idx);
            if (! modu->Read(fp))
                return(0);
        }
    }
    return(1);
}

//-------------//
// Modu::Write //
//-------------//

int
Modu::Write(
    FILE*  fp)
{
    if (fwrite((void *)sigma0, sizeof(float) * AZIMUTH_BINS, 1, fp) != 1 ||
        fwrite((void *)count, sizeof(unsigned int) * AZIMUTH_BINS, 1, fp) != 1)
    {
        return(0);
    }
    return(1);
}

//------------//
// Modu::Read //
//------------//

int
Modu::Read(
    FILE*  fp)
{
    if (fread((void *)sigma0, sizeof(float) * AZIMUTH_BINS, 1, fp) != 1 ||
        fread((void *)count, sizeof(unsigned int) * AZIMUTH_BINS, 1, fp) != 1)
    {
        return(0);
    }
    return(1);
}

#endif
