/*=========================================================
** Copyright  (C)1996, California Institute of Technology. 
** U.S. Government sponsorship under 
** NASA Contract NAS7-1260 is acknowledged
**
**
** CM Log
** $Log$
// 
//    Rev 1.0   18 Aug 1998 11:04:12   sally
// Initial revision.
** 
**    Rev 1.2   20 Jul 1998 14:14:58   sally
** pass file descriptor to read function, instead of frame buffer
** 
**    Rev 1.1   01 May 1998 14:45:06   sally
** added HK2 file
** 
**    Rev 1.0   14 Apr 1998 15:09:12   sally
** Initial revision.
** 
** $Date$
** $Revision$
** $Author$
**
=========================================================*/

#ifndef Hk2HdfMap_H
#define Hk2HdfMap_H

static const char Hk2HdfMap_h_id[] =
    "@(#) $Header$";

#include <mfhdf.h>

#define EA_HK2_FRAME_SIZE    398
#define EA_HK2_TIME_SIZE     8

typedef enum
{
    DATA_UNKNOWN,
    DATA_UINT1,
    DATA_INT1,
    DATA_UINT2,
    DATA_INT2,
    DATA_UINT3,
    DATA_UINT4,
    DATA_INT4,
    DATA_FLOAT4,
    DATA_FLOAT8,
    DATA_ITIME,
    DATA_CHAR1
} DataTypeE;

typedef int (*Hk2ReadFunc) (int infd, char* data);

typedef struct
{
    char*           datasetName;    /* dataset name in HDF */
    char*           otherId;        /* id or comments */
    DataTypeE       eaDataType;     /* data type stored in HDF */
    double          scaleFactor;    /* scale factor */
    int             byteOffset;     /* starting byte offset */
    Hk2ReadFunc     readFunc;       /* function for reading */
} Hk2HdfMapEntry;

extern const Hk2HdfMapEntry Hk2HdfMapTable[];
extern const int Hk2HdfTabSize;

#endif /* Hk2HdfMap_H */
