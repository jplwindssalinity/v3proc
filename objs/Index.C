//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_index_c[] =
    "@(#) $Id$";

#include <stdlib.h>
#include <math.h>
#include "Index.h"

//=======//
// Index //
//=======//

Index::Index()
:   _min(0.0), _max(0.0), _bins(0), _step(0.0)
{
    return;
}

Index::~Index()
{
    return;
}

//---------------------//
// Index::SpecifyEdges //
//---------------------//

int
Index::SpecifyEdges(
    float  min,
    float  max,
    int    bins)
{
    _step = (max - min) / (float)bins;
    _min = min + _step / 2.0;
    _max = max - _step / 2.0;
    _bins = bins;
    return(1);
}

//-----------------------//
// Index::SpecifyCenters //
//-----------------------//

int
Index::SpecifyCenters(
    float  min,
    float  max,
    int    bins)
{
    _min = min;
    _max = max;
    _bins = bins;
    _step = (max - min) / (float)(bins - 1);
    return(1);
}

//------------------------------//
// Index::SpecifyWrappedCenters //
//------------------------------//

int
Index::SpecifyWrappedCenters(
    float  min,
    float  max,
    int    bins)
{
    _step = (max - min) / (float)bins;
    _min = min;
    _max = max - _step;
    _bins = bins;
    return(1);
}

//-----------------------//
// Index::SpecifyNewBins //
//-----------------------//

int
Index::SpecifyNewBins(
    Index*  index,
    int     bins)
{
    float edge_min = index->_min - index->_step / 2.0;
    float edge_max = index->_max + index->_step / 2.0;

    if (! SpecifyEdges(edge_min, edge_max, bins))
        return(0);

    return(1);
}

//---------------------//
// Index::MakeIntArray //
//---------------------//

int*
Index::MakeIntArray()
{
    int* ptr = (int*)malloc(sizeof(int)*_bins);
    if (ptr == NULL) return(NULL);
    for (int i = 0; i < _bins; i++)
    {
        ptr[i] = 0;
    }

    return(ptr);
}

//-----------------------//
// Index::MakeFloatArray //
//-----------------------//

float*
Index::MakeFloatArray()
{
    float* ptr = (float*)malloc(sizeof(float)*_bins);
    if (ptr == NULL) return(NULL);
    for (int i = 0; i < _bins; i++)
    {
        ptr[i] = 0.0;
    }

    return(ptr);
}

//-------------//
// Index::Read //
//-------------//

int
Index::Read(
    FILE*  fp)
{
    if (fread((void *)&_min, sizeof(float), 1, fp) != 1 ||
        fread((void *)&_max, sizeof(float), 1, fp) != 1 ||
        fread((void *)&_bins, sizeof(int), 1, fp) != 1 ||
        fread((void *)&_step, sizeof(float), 1, fp) != 1)
    {
        return(0);
    }
    return(1);
}

//------------------//
// Index::ReadAscii //
//------------------//

int
Index::ReadAscii(
    FILE*  fp)
{
    if (fscanf(fp, " %g %g %d %g", &_min, &_max, &_bins, &_step) != 4)
        return(0);
    return(1);
}

//--------------//
// Index::Write //
//--------------//

int
Index::Write(
    FILE*  fp)
{
    if (fwrite((void *)&_min, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&_max, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&_bins, sizeof(int), 1, fp) != 1 ||
        fwrite((void *)&_step, sizeof(float), 1, fp) != 1)
    {
        return(0);
    }
    return(1);
}

//-------------------//
// Index::WriteAscii //
//-------------------//

int
Index::WriteAscii(
    FILE*  fp)
{
    fprintf(fp, "%g %g %d %g\n", _min, _max, _bins, _step);
    return(1);
}

//-----------------------------//
// Index::GetLinearCoefsStrict //
//-----------------------------//
// IF value is between _min and _max
//   sets interpolation indicies/coefficients
// ELSE
//   returns 0

int
Index::GetLinearCoefsStrict(
    float  value,
    int    idx[2],
    float  coef[2])
{
    // check range
    if (value < _min || value > _max)
        return(0);

    // calculate floating point index
    float fidx = (value - _min) / _step;

    // calculate indicies in range
    idx[0] = (int)fidx;
    idx[1] = idx[0] + 1;
    if (idx[1] == _bins)
    {
        // at upper edge, shift down
        idx[0]--;
        idx[1]--;
    }

    // calculate coefficients
    coef[0] = (float)idx[1] - fidx;
    coef[1] = fidx - (float)idx[0];

    return(1);
}

//------------------------------//
// Index::GetLinearCoefsWrapped //
//------------------------------//
// Always returns valid interpolation indicies/coefficients by wrapping.

int
Index::GetLinearCoefsWrapped(
    float  value,
    int    idx[2],
    float  coef[2])
{
    // calculate floating point index
    float fidx = (value - _min) / _step;

    // calculate indicies (don't worry about range)
    idx[0] = (int)fidx;
    idx[1] = idx[0] + 1;

    // calculate coefficients
    coef[0] = (float)idx[1] - fidx;
    coef[1] = fidx - (float)idx[0];

    // wrap indicies into range
    idx[0] %= _bins;
    idx[1] %= _bins;

    return(1);
}

//------------------------------//
// Index::GetLinearCoefsClipped //
//------------------------------//
// IF value is less than _min
//   returns idx = {0, 0}, coef = {1.0, 0.0}
// IF value is greater than _max
//   returns idx = {_bins-1, 0}, coef = {1.0, 0.0}
// IF value is between _min and _max
//   returns interpolation indicies/coefficients

int
Index::GetLinearCoefsClipped(
    float  value,
    int    idx[2],
    float  coef[2])
{
    // check for less than min case
    if (value < _min)
    {
        // these will calculate table[0]
        idx[0] = 0;
        idx[1] = 0;
        coef[0] = 1.0;
        coef[1] = 0.0;
        return(1);
    }

    // check for greater than max case
    if (value > _max)
    {
        // these will calculate table[_bins-1]
        idx[0] = _bins - 1;
        idx[1] = 0;
        coef[0] = 1.0;
        coef[1] = 0.0;
        return(1);
    }

    // calculate floating point index
    float fidx = (value - _min) / _step;

    // calculate indicies in range
    idx[0] = (int)fidx;
    idx[1] = idx[0] + 1;
    if (idx[1] == _bins)
    {
        // at upper edge, shift down
        idx[0]--;
        idx[1]--;
    }

    // calculate coefficients
    coef[0] = (float)idx[1] - fidx;
    coef[1] = fidx - (float)idx[0];

    return(1);
}

//------------------------//
// Index::GetNearestIndex //
//------------------------//
// IF value is between _min - _step/2 and _max + _step/2
//   returns nearest index

int
Index::GetNearestIndex(
    float  value,
    int*   idx)
{
    if (value < _min - _step / 2.0 ||
        value > _max + _step / 2.0)
    {
        return(0);
    }

    *idx = (int)((value - _min) / _step + 0.5);
    return(1);
}

//-------------------------------//
// Index::GetNearestWrappedIndex //
//-------------------------------//

int
Index::GetNearestWrappedIndex(
    float  value,
    int*   idx)
{
    int index = (int)(floor((value - _min) / _step + 0.5) + 0.5);
    while (index < 0)
        index += _bins;
    *idx = index % _bins;

    return(1);
}

//---------------------//
// Index::IndexToValue //
//---------------------//

int
Index::IndexToValue(
    int     idx,
    float*  value)
{
    if (idx < 0 || idx >= _bins)
        return(0);

    *value = _min + (float)idx * _step;
    return(1);
}
