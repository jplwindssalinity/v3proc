//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_sds_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <math.h>
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

//----------------------------//
// Attribute::ReplaceContents //
//----------------------------//

void
Attribute::ReplaceContents(
    const char*  contents)
{
    free(_contents);
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
    _frameSize = 1;
    for (int i = 0; i < rank; i++)
    {
        _start[i] = 0;
        _edges[i] = dim_sizes[i];
        _dimSizes[i] = dim_sizes[i];
        _dimNames[i] = strdup(dim_names[i]);
        if (i > 0)
        {
            _frameSize *= dim_sizes[i];
        }
    }

    // prepare to write one frame at a time
    _calibratedData = NULL;
    SetFrameCluster(1);

    _units = strdup(units);
    _cal = cal;
    _offset = offset;

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
        fprintf(stderr, "      SD Id: %d\n", (int)sd_id);
        fprintf(stderr, "   SDS Name: %s\n", _sdsName);
        fprintf(stderr, "  Data Type: %d\n", (int)_dataType);
        fprintf(stderr, "       Rank: %d\n", (int)_rank);
        fprintf(stderr, "       Dims:");
        for (int i = 0; i < _rank; i++)
        {
            fprintf(stderr, " %d", (int)_dimSizes[i]);
        }
        fprintf(stderr, "\n");
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
    return(0);
}

//==========//
// SdsUInt8 //
//==========//

SdsUInt8::SdsUInt8(
    const char*   sds_name,
    int32         rank,
    int32*        dim_sizes,
    const char*   units,
    float64       cal,
    float64       offset,
    const char**  dim_names,
    uint8         max,
    uint8         min)
:   Sds(sds_name, DFNT_UINT8, rank, dim_sizes, units, cal, offset, dim_names)
{
    // remember the max and min
    _max = max;
    _min = min;
    return;
}

//------------------------//
// SdsUInt8::SetMaxAndMin //
//------------------------//

int
SdsUInt8::SetMaxAndMin()
{
    if (SDsetrange(_sdsId, (void *)&_max, (void *)&_min) == FAIL)
        return(0);
    return(1);
}

//-------------------------------//
// SdsUInt8::SetWithUnsignedChar //
//-------------------------------//
// the calibration is ignored (assumed to be 1.0 and 0.0)

void
SdsUInt8::SetWithUnsignedChar(unsigned char* value)
{
    uint8* ptr = (uint8 *)_calibratedData;
    for (int i = 0; i < _frameCluster * _frameSize; i++)
    {
        *(ptr + i) = (uint8)(*(value + i));
    }
    return;
}

//===========//
// SdsUInt16 //
//===========//

SdsUInt16::SdsUInt16(
    const char*   sds_name,
    int32         rank,
    int32*        dim_sizes,
    const char*   units,
    float64       cal,
    float64       offset,
    const char**  dim_names,
    uint16        max,
    uint16        min)
:   Sds(sds_name, DFNT_UINT16, rank, dim_sizes, units, cal, offset, dim_names)
{
    // remember the max and min
    _max = max;
    _min = min;
    return;
}

//-------------------------//
// SdsUInt16::SetMaxAndMin //
//-------------------------//

int
SdsUInt16::SetMaxAndMin()
{
    if (SDsetrange(_sdsId, (void *)&_max, (void *)&_min) == FAIL)
        return(0);
    return(1);
}

//---------------------------------//
// SdsUInt16::SetWithUnsignedShort //
//---------------------------------//
// the calibration is ignored (assumed to be 1.0 and 0.0)

void
SdsUInt16::SetWithUnsignedShort(unsigned short* value)
{
    uint16* ptr = (uint16 *)_calibratedData;
    for (int i = 0; i < _frameCluster * _frameSize; i++)
    {
        *(ptr + i) = (uint16)(*(value + i));
    }
    return;
}

//-------------------------//
// SdsUInt16::SetFromFloat //
//-------------------------//

void
SdsUInt16::SetFromFloat(float* value)
{
    uint16* ptr = (uint16 *)_calibratedData;
    for (int i = 0; i < _frameCluster * _frameSize; i++)
    {
        *(ptr + i) = (uint16)(rint(((double)*(value + i) / _cal) + _offset));
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
    uint32* ptr = (uint32 *)_calibratedData;
    for (int i = 0; i < _frameCluster * _frameSize; i++)
    {
        *(ptr + i) = (uint32)(*(value + i));
    }
    return;
}

//=========//
// SdsInt8 //
//=========//

SdsInt8::SdsInt8(
    const char*   sds_name,
    int32         rank,
    int32*        dim_sizes,
    const char*   units,
    float64       cal,
    float64       offset,
    const char**  dim_names,
    int8          max,
    int8          min)
:   Sds(sds_name, DFNT_INT8, rank, dim_sizes, units, cal, offset, dim_names)
{
    // remember the max and min
    _max = max;
    _min = min;
    return;
}

//-----------------------//
// SdsInt8::SetMaxAndMin //
//-----------------------//

int
SdsInt8::SetMaxAndMin()
{
    if (SDsetrange(_sdsId, (void *)&_max, (void *)&_min) == FAIL)
        return(0);
    return(1);
}

//----------------------//
// SdsInt8::SetWithChar //
//----------------------//
// the calibration is ignored (assumed to be 1.0 and 0.0)

void
SdsInt8::SetWithChar(char* value)
{
    int8* ptr = (int8 *)_calibratedData;
    for (int i = 0; i < _frameCluster * _frameSize; i++)
    {
        *(ptr + i) = (int8)(*(value + i));
    }
    return;
}

//==========//
// SdsInt16 //
//==========//

SdsInt16::SdsInt16(
    const char*   sds_name,
    int32         rank,
    int32*        dim_sizes,
    const char*   units,
    float64       cal,
    float64       offset,
    const char**  dim_names,
    int16         max,
    int16         min)
:   Sds(sds_name, DFNT_INT16, rank, dim_sizes, units, cal, offset, dim_names)
{
    // remember the max and min
    _max = max;
    _min = min;
    return;
}

//------------------------//
// SdsInt16::SetMaxAndMin //
//------------------------//

int
SdsInt16::SetMaxAndMin()
{
    if (SDsetrange(_sdsId, (void *)&_max, (void *)&_min) == FAIL)
        return(0);
    return(1);
}

//------------------------//
// SdsInt16::SetWithInt16 //
//------------------------//
// the calibration is ignored (assumed to be 1.0 and 0.0)

void
SdsInt16::SetWithInt16(int16* value)
{
    int16* ptr = (int16 *)_calibratedData;
    for (int i = 0; i < _frameCluster * _frameSize; i++)
    {
        *(ptr + i) = *(value + i);
    }
    return;
}

//------------------------//
// SdsInt16::SetFromFloat //
//------------------------//

void
SdsInt16::SetFromFloat(float* value)
{
    int16* ptr = (int16 *)_calibratedData;
    for (int i = 0; i < _frameCluster * _frameSize; i++)
    {
        *(ptr + i) = (int16)(rint(((double)*(value + i) / _cal) + _offset));
    }

    return;
}

//============//
// SdsFloat32 //
//============//

SdsFloat32::SdsFloat32(
    const char*   sds_name,
    int32         rank,
    int32*        dim_sizes,
    const char*   units,
    float64       cal,
    float64       offset,
    const char**  dim_names,
    float32       max,
    float32       min)
:   Sds(sds_name, DFNT_FLOAT32, rank, dim_sizes, units, cal, offset, dim_names)
{
    // remember the max and min
    _max = max;
    _min = min;
    return;
}

//--------------------------//
// SdsFloat32::SetMaxAndMin //
//--------------------------//

int
SdsFloat32::SetMaxAndMin()
{
    if (SDsetrange(_sdsId, (void *)&_max, (void *)&_min) == FAIL)
        return(0);
    return(1);
}

//--------------------------//
// SdsFloat32::SetWithFloat //
//--------------------------//
// the calibration is ignored (assumed to be 1.0 and 0.0)

void
SdsFloat32::SetWithFloat(float* value)
{
    float32* ptr = (float32 *)_calibratedData;
    for (int i = 0; i < _frameCluster * _frameSize; i++)
    {
        *(ptr + i) = (float32)(*(value + i));
    }
    return;
}

//--------------------------//
// SdsFloat32::SetFromFloat //
//--------------------------//

void
SdsFloat32::SetFromFloat(float* value)
{
    float32* ptr = (float32 *)_calibratedData;
    for (int i = 0; i < _frameCluster * _frameSize; i++)
    {
        *(ptr + i) = (float32)(((double)*(value + i) / _cal) + _offset);
    }
    return;
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
    for (int i = 0; i < _frameCluster * _frameSize; i++)
    {
        *(ptr + i) = (float64)((*(value + i) / _cal) + _offset);
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
    for (int i = 0; i < _frameCluster * _frameSize; i++)
    {
        *(ptr + i) = (float64)(((double)*(value + i) / _cal) + _offset);
    }
    return;
}

//------------//
// SDnametoid //
//------------//

int32
SDnametoid(
    int32     sd_id,
    char*     sds_name,
    float64*  scale_factor)
{
    int32 sds_index = SDnametoindex(sd_id, sds_name);
    if (sds_index == FAIL)
        return(FAIL);
    int32 sds_id = SDselect(sd_id, sds_index);
    if (scale_factor != NULL)
    {
        float64 cal_error, offset, offset_error;
        int32 data_type;
        if (SDgetcal(sds_id, scale_factor, &cal_error, &offset,
            &offset_error, &data_type) == FAIL)
        {
            *scale_factor = 0.0;
        }
    }
    return (sds_id);
}

//--------------------//
// SDreaddata_or_exit //
//--------------------//

void
SDreaddata_or_exit(
    const char*  name,
    int32        sds_id,
    int32*       start,
    int32*       edges,
    void*        data)
{
    if (SDreaddata(sds_id, start, NULL, edges, data) == FAIL)
    {
        fprintf(stderr, "Error reading SD data (%s : %ld)\n", name, sds_id);
        exit(1);
    }
    return;
}

//-----------//
// SDattrint //
//-----------//
// read and parse an attribute to get its integer value.
// this function is specific to the GDS implementation of
// attributes as containing a multiline string. the real
// purpose of this function is for getting the actual frame
// count. it just happens to be a bit more generic than that.

int
SDattrint(
    int32  sd_id,
    char*  sds_name,
    int*   value)
{
    int32 attr_index = SDfindattr(sd_id, sds_name);
    char data[1024];
    if (SDreadattr(sd_id, attr_index, data) == FAIL)
    {
        fprintf(stderr, "SDattrint: error reading attribute for %s\n",
            sds_name);
        return(0);
    }
    int tmp_value = 0;
    if (sscanf(data, " %*[^\n] %*[^\n] %d", &tmp_value) != 1)
    {
        fprintf(stderr, "SDattrint: error parsing attribute %s\n",
            sds_name);
        fprintf(stderr, "%s\n", data);
        return(0);
    }
    *value = tmp_value;
    return(1);
}
