//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   14 Apr 1998 15:09:12   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#ifndef Hk2HdfMap_H
#define Hk2HdfMap_H

static const char Hk2HdfMap_h_id[] =
    "@(#) $Header$";

#include <mfhdf.h>

#include "Parameter.h"

#define EA_HK2_FRAME_SIZE    390


typedef int (*Hk2ReadFunc) (char* frame, int byteOffset, char* data);

struct Hk2HdfMapEntry
{
    char*           datasetName;    // dataset name in HDF
    char*           otherId;        // id or comments
    DataTypeE       eaDataType;     // data type stored in HDF
    double          scaleFactor;    // scale factor
    int             byteOffset;     // starting byte offset
    Hk2ReadFunc     readFunc;       // function for reading
};

extern const Hk2HdfMapEntry Hk2HdfMapTable[];
extern const int Hk2HdfTabSize;

#endif // Hk2HdfMap_H
