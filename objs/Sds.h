//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef SDS_H
#define SDS_H

static const char rcs_id_sds_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include "hdf.h"

//======================================================================
// CLASSES
//    Sds, Attribute
//======================================================================

#define CAL_ERROR     0.0
#define OFFSET_ERROR  0.0
#define DATA_TYPE     DFNT_FLOAT32    // assume uncalibrated data are floats

//======================================================================
// CLASS
//    Attribute
//
// DESCRIPTION
//======================================================================

class Attribute
{
public:
    Attribute(const char* name, const char* type, const char* size,
        const char* contents);
    void ReplaceContents(const char* contents);
    int Write(int32 obj_id);

protected:
    char*  _name;
    char*  _type;
    char*  _size;
    char*  _contents;
};

//======================================================================
// CLASS
//    Sds
//
// DESCRIPTION
//    The Sds class holds generic SDS information.
//======================================================================

class Sds
{
public:
    Sds(const char* sds_name, int32 data_type, int32 rank,
        int32* dim_sizes, const char* units, float64 cal, float64 offset,
        const char** dim_names);
    virtual ~Sds();

    const char* GetName() { return(_sdsName); };

    int FrameSize();
    int SetFrameCluster(int32 frames);
    int Create(int32 sds_id);
    int Write(int32 sd_id, int32 record_idx);
    int EndAccess();

    virtual int SetMaxAndMin();

protected:
    int32    _sdsId;
    char*    _sdsName;
    int32    _dataType;
    int32    _rank;
    int32*   _dimSizes;
    char*    _units;
    float64  _cal;
    float64  _offset;
    char**   _dimNames;
    int32*   _start;
    int32*   _edges;
    void*    _calibratedData;

    int      _frameSize;       // the number of elements per frame
    int      _frameCluster;    // the number of frames per i/o operation
};

//======================================================================
// CLASS
//    SdsUInt8
//
// DESCRIPTION
//    The Sds class holds SDS information for uint8 data
//======================================================================

class SdsUInt8 : public Sds
{
public:
    SdsUInt8(const char* sds_name, int32 rank, int32* dim_sizes,
        const char* units, float64 cal, float64 offset,
        const char** dim_names, uint8 max, uint8 min);

    int   SetMaxAndMin();
    void  SetWithUnsignedChar(unsigned char* value);

protected:
    uint8  _max;
    uint8  _min;
};

//======================================================================
// CLASS
//    SdsUInt16
//
// DESCRIPTION
//    The Sds class holds SDS information for uint16 data
//======================================================================

class SdsUInt16 : public Sds
{
public:
    SdsUInt16(const char* sds_name, int32 rank, int32* dim_sizes,
        const char* units, float64 cal, float64 offset,
        const char** dim_names, uint16 max, uint16 min);

    int   SetMaxAndMin();
    void  SetWithUnsignedShort(unsigned short* value);

protected:
    uint16  _max;
    uint16  _min;
};

//======================================================================
// CLASS
//    SdsUInt32
//
// DESCRIPTION
//    The Sds class holds SDS information for uint32 data
//======================================================================

class SdsUInt32 : public Sds
{
public:
    SdsUInt32(const char* sds_name, int32 rank, int32* dim_sizes,
        const char* units, float64 cal, float64 offset,
        const char** dim_names, uint32 max, uint32 min);

    int   SetMaxAndMin();
    void  SetWithUnsignedInt(unsigned int* value);

protected:
    uint32  _max;
    uint32  _min;
};

//======================================================================
// CLASS
//    SdsInt8
//
// DESCRIPTION
//    The Sds class holds SDS information for int8 data
//======================================================================

class SdsInt8 : public Sds
{
public:
    SdsInt8(const char* sds_name, int32 rank, int32* dim_sizes,
        const char* units, float64 cal, float64 offset,
        const char** dim_names, int8 max, int8 min);

    int   SetMaxAndMin();
    void  SetWithChar(char* value);

protected:
    int8  _max;
    int8  _min;
};

//======================================================================
// CLASS
//    SdsInt16
//
// DESCRIPTION
//    The Sds class holds SDS information for int16 data
//======================================================================

class SdsInt16 : public Sds
{
public:
    SdsInt16(const char* sds_name, int32 rank, int32* dim_sizes,
        const char* units, float64 cal, float64 offset,
        const char** dim_names, int16 max, int16 min);

    int   SetMaxAndMin();
    void  SetFromFloat(float* value);

protected:
    int16  _max;
    int16  _min;
};

//======================================================================
// CLASS
//    SdsFloat32
//
// DESCRIPTION
//    The Sds class holds SDS information for Float32 data
//======================================================================

class SdsFloat32 : public Sds
{
public:
    SdsFloat32(const char* sds_name, int32 rank, int32* dim_sizes,
        const char* units, float64 cal, float64 offset,
        const char** dim_names, float32 max, float32 min);

    int   SetMaxAndMin();
    void  SetWithFloat(float* value);
    void  SetFromFloat(float* value);

protected:
    float32  _max;
    float32  _min;
};

//======================================================================
// CLASS
//    SdsFloat64
//
// DESCRIPTION
//    The Sds class holds SDS information for Float64 data
//======================================================================

class SdsFloat64 : public Sds
{
public:
    SdsFloat64(const char* sds_name, int32 rank, int32* dim_sizes,
        const char* units, float64 cal, float64 offset,
        const char** dim_names, float64 max, float64 min);

    int   SetMaxAndMin();
    void  SetFromDouble(double* value);
    void  SetFromUnsignedInt(unsigned int* value);

protected:
    float64  _max;
    float64  _min;
};

//------------------//
// helper functions //
//------------------//

int32 SDnametoid(int32 sd_id, char* sds_name, float64* scale_factor = NULL);

#endif
