//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_sds_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include "Sds.h"
#include "mfhdf.h"

//===========//
// Attribute //
//===========//

Attribute::Attribute(
    const char*  name,
    const char*  type,
    const char*  size,
    const char*  contents)
{
    _name = strdup(name);
    _type = strdup(type);
    _size = strdup(size);
    _contents = strdup(contents);
    return;
}

//------------------//
// Attribute::Write //
//------------------//

int
Attribute::Write(
    int32  obj_id)
{
    char buffer[1024];
    sprintf(buffer, "%s\n%s\n%s\n", _type, _size, _contents);
    int length = strlen(buffer);
    if (SDsetattr(obj_id, _name, DFNT_CHAR8, length, (VOIDP)buffer) != SUCCEED)
    {
        return(0);
    }
    return(1);
}

//=====//
// Sds //
//=====//

Sds::Sds(
    const char*   sds_name,
    int32         data_type,
    int32         rank,
    int32*        dim_sizes,
    const char*   units,
    float64       cal,
    float64       offset,
    const char**  dim_names)
{
    _sdsId = FAIL;
    _sdsName = strdup(sds_name);
    _dataType = data_type;
    _rank = rank;
    _dimSizes = (int32 *)malloc(rank * sizeof(int32));
    _dimNames = (char **)malloc(rank * sizeof(char *));
    _start = (int32 *)malloc(rank * sizeof(int32));
    _edges = (int32 *)malloc(rank * sizeof(int32));
    for (int i = 0; i < rank; i++)
    {
        _start[i] = 0;
        _edges[i] = _dimSizes[i];
        _dimSizes[i] = dim_sizes[i];
        _dimNames[i] = strdup(dim_names[i]);
    }

    // prepare to write one frame at a time
    _calibratedData = NULL;
    SetFrameCluster(1);

    _units = strdup(units);
    _cal = 1.0;
    _offset = 0.0;

    return;
}

Sds::~Sds()
{
    free(_sdsName);
    free(_dimSizes);
    for (int i = 0; i < _rank; i++)
    {
        free(_dimNames[i]);
    }
    free(_dimNames);
    free(_start);
    free(_edges);
    return;
}

//----------------//
// Sds::FrameSize //
//----------------//
// returns the number of bytes per frame. A frame is defined by the
// first index

int
Sds::FrameSize()
{
    int size = DFKNTsize(_dataType);    // data type size

    // skip the first rank, it is the frame
    for (int i = 1; i < _rank; i++)
    {
        size *= _dimSizes[i];    // the dimensionality of each
    }
    return(size);
}

//----------------------//
// Sds::SetFrameCluster //
//----------------------//

int
Sds::SetFrameCluster(
    int32  frames)
{
    // determine the frame size
    int size = FrameSize();
    _calibratedData = realloc(_calibratedData, frames * size);
    if (_calibratedData == NULL)
        return(0);

    // set up edges for the desired number of frames
    _edges[0] = frames;

    _frameCluster = frames;
    return(1);
}

//--------//
// Create //
//--------//

int
Sds::Create(
    int32 sd_id)
{
    // create the sds
    _sdsId = SDcreate(sd_id, _sdsName, _dataType, _rank, _dimSizes);
    if (_sdsId == -1)
    {
        fprintf(stderr, "Sds::Create: error with SDcreate\n");
        return(0);
    }

    // set some string attributes
    if (SDsetdatastrs(_sdsId, _sdsName, _units, NULL, NULL) != SUCCEED)
    {
        fprintf(stderr, "Sds::Create: error with SDsetdatastrs\n");
        return(0);
    }

    // set the max and min
    if (! SetMaxAndMin())
    {
        fprintf(stderr, "Sds::Create: error with SetMaxAndMin\n");
        return(0);
    }

    // set the type (always single). This is a B. Weiss thing, not
    // an HDF thing
    if (SDsetattr(_sdsId, "SDS_type", DFNT_CHAR8, 6, (void *)"Single" )
        != SUCCEED)
    {
        fprintf(stderr, "Sds::Create: error with SDsetattr\n");
        return(0);
    }

    // set the calibration 
    if (SDsetcal(_sdsId, _cal, CAL_ERROR, _offset, OFFSET_ERROR,
        DATA_TYPE) != SUCCEED)
    {
        fprintf(stderr, "Sds::Create: error with SDsetcal\n");
        return(0);
    }

    // set the dimension names
    for (int i = 0; i < _rank; i++)
    {
        int32 dim_id = SDgetdimid(_sdsId, i);
        if (SDsetdimname(dim_id, _dimNames[i]) != SUCCEED)
        {
            fprintf(stderr, "Sds::Create: error with SDsetdimname\n");
            return(0);
        }
    }

// sds_index = SDnametoindex(sd_id, sds_name);

    return(1);
}

//-------//
// Write //
//-------//
int
Sds::Write(
    int32  sd_id,
    int32  record_idx)
{
    // fill in the first dimension of start with the record index
    _start[0] = record_idx;
    if (SDwritedata(_sdsId, _start, NULL, _edges, _calibratedData) == SUCCEED)
        return(1);
    return(0);
}

//----------------//
// Sds::EndAccess //
//----------------//

int
Sds::EndAccess()
{
    if (SDendaccess(_sdsId) != SUCCEED)
        return(0);

    _sdsId = 0;
    return(1);
}

//-------------------//
// Sds::SetMaxAndMin //
//-------------------//

int
Sds::SetMaxAndMin()
{
    fprintf(stderr, "Calling the base SetMaxAndMin is evil.\n");
    exit(1);
}

//============//
// SdsFloat64 //
//============//

SdsFloat64::SdsFloat64(
    const char*   sds_name,
    int32         rank,
    int32*        dim_sizes,
    const char*   units,
    float64       cal,
    float64       offset,
    const char**  dim_names,
    float64       max,
    float64       min)
:   Sds(sds_name, DFNT_FLOAT64, rank, dim_sizes, units, cal, offset, dim_names)
{
    // remember the max and min
    _max = max;
    _min = min;
    return;
}

//--------------------------//
// SdsFloat64::SetMaxAndMin //
//--------------------------//

int
SdsFloat64::SetMaxAndMin()
{
    if (SDsetrange(_sdsId, (void *)&_max, (void *)&_min) == FAIL)
        return(0);
    return(1);
}

//---------------------------//
// SdsFloat64::SetFromDouble //
//---------------------------//

void
SdsFloat64::SetFromDouble(double* value)
{
    float64* ptr = (float64 *)_calibratedData;
    for (int i = 0; i < _frameCluster; i++)
    {
        *(ptr + i) = (*(value + i) / _cal) + _offset;
    }
    return;
}

//--------------------------------//
// SdsFloat64::SetFromUnsignedInt //
//--------------------------------//

void
SdsFloat64::SetFromUnsignedInt(unsigned int* value)
{
    float64* ptr = (float64 *)_calibratedData;
    for (int i = 0; i < _frameCluster; i++)
    {
        *(ptr + i) = ((double)*(value + i) / _cal) + _offset;
    }
    return;
}

//============//
// SdsUInt32 //
//============//

SdsUInt32::SdsUInt32(
    const char*   sds_name,
    int32         rank,
    int32*        dim_sizes,
    const char*   units,
    float64       cal,
    float64       offset,
    const char**  dim_names,
    uint32        max,
    uint32        min)
:   Sds(sds_name, DFNT_UINT32, rank, dim_sizes, units, cal, offset, dim_names)
{
    // remember the max and min
    _max = max;
    _min = min;
    return;
}

//-------------------------//
// SdsUInt32::SetMaxAndMin //
//-------------------------//

int
SdsUInt32::SetMaxAndMin()
{
    if (SDsetrange(_sdsId, (void *)&_max, (void *)&_min) == FAIL)
        return(0);
    return(1);
}

//-------------------------------//
// SdsUInt32::SetWithUnsignedInt //
//-------------------------------//
// the calibration is ignored (assumed to be 1.0 and 0.0)

void
SdsUInt32::SetWithUnsignedInt(unsigned int* value)
{
    float64* ptr = (float64 *)_calibratedData;
    for (int i = 0; i < _frameCluster; i++)
    {
        *(ptr + i) = *(value + i);
    }
    return;
}
