//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.8   21 Apr 1998 16:39:46   sally
//  for L2B
// 
//    Rev 1.7   20 Apr 1998 10:21:24   sally
// change for WindSwatch
// 
//    Rev 1.6   17 Apr 1998 16:48:42   sally
// add L2A and L2B parameter tables
// 
//    Rev 1.5   06 Apr 1998 16:28:02   sally
// merged with SVT
// 
//    Rev 1.4   30 Mar 1998 15:13:52   sally
// added L2A parameter table
// 
//    Rev 1.3   23 Mar 1998 15:35:00   sally
// adapt to derived science data
// 
//    Rev 1.2   20 Feb 1998 10:57:28   sally
// L1 to L1A
// 
//    Rev 1.1   17 Feb 1998 14:45:42   sally
// NOPM
// 
//    Rev 1.0   04 Feb 1998 14:15:48   daffer
// Initial checking
// Revision 1.5  1998/02/01 20:21:11  sally
// fixed 2D conversion
//
// Revision 1.4  1998/01/31 00:36:39  sally
// add scale factor
//
// Revision 1.3  1998/01/30 22:29:07  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#include <assert.h>
#include <time.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>

#include "L1AExtract.h"
#include "Parameter.h"
#include "Itime.h"

static const char rcs_id_L1AExtract_C[] = "@(#) $Header$";

#define LO_COUNT                4
#define CHANNEL_COUNT           4
#define RX_BEAM_CYCLE_LO2_MINUS 0
#define RX_BEAM_CYCLE_LO1_MINUS 1
#define RX_BEAM_CYCLE_LO2_PLUS  2
#define RX_BEAM_CYCLE_LO1_PLUS  3
#define RX_BEAM_CYCLE_ANY       -1
#define BAD_GAIN                -10.0
#define BAD_NOISE_FIGURE        10.0

static const float bk = 1.38e-23; // boltzman constant
static const float chBandwidth[]={ 194645.28, 105719.20, 54478.76, 49192.80 };

//======================================================================
// L1A DATA HEADER Functions
//
//   Input:     frame       pointer to a L1A DATA HEADER
//  Output:     data        pointer to the data destination
// Returns:     TRUE if the parameter has been extracted
//              FALSE otherwise
//======================================================================

#define LINE_LENGTH     80

#ifndef EXTRACT_GET_BIT
#define EXTRACT_GET_BIT(byte,pos) ((unsigned char)(((byte) >> (pos)) & 0x01))
#endif

#ifndef EXTRACT_GET_BITS
#define EXTRACT_GET_BITS(byte,leftpos,numBits) \
   (((byte) >> ((leftpos)+1-(numBits))) & ~(~0<<(numBits)))
#endif

//----------------------------------------------------------------------
// Function:    ExtractData1D ([])
// Extracts:    one dimensional data
//----------------------------------------------------------------------
int
ExtractData1D(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    return (l1File->GetDatasetData1D(sdsIDs[0], start,
                  stride, length, buffer) == HDF_SUCCEED ?  TRUE : FALSE);

}//ExtractData1D

//----------------------------------------------------------------------
// Function:    ExtractData1D_uint4_float ([])
// Extracts:    one dimensional data (int -> float)
//----------------------------------------------------------------------
int
ExtractData1D_uint4_float(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned int* tempBuffer =
             (unsigned int*) calloc(length, sizeof(unsigned int));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "ExtractData1D_uint4_float: Out of memory\n");
        exit(1);
    }

    // get the array of unsigned int
    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride, length,
                    (VOIDP)tempBuffer) != HDF_SUCCEED)
        return FALSE;

    // get the scale factor
    float64  scaleFactor;
    if (l1File->GetScaleFactor(sdsIDs[0], scaleFactor) == HDF_FAIL)
        return FALSE;

    // convert the integers to floats, and return
    float* floatP = (float*)buffer;
    for (int i=0; i < length; i++, floatP++)
    {
        *floatP = (float) (scaleFactor * tempBuffer[i]);
    }
    free((void*) tempBuffer);
    return(TRUE);

}//ExtractData1D_uint4_float

//----------------------------------------------------------------------
// Function:    ExtractData1D_uint2_float ([])
// Extracts:    one dimensional data (short -> float)
//----------------------------------------------------------------------
int
ExtractData1D_uint2_float(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold unsigned short integers
    unsigned short* tempBuffer =
             (unsigned short*) calloc(length, sizeof(unsigned short));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "ExtractData1D: Out of memory\n");
        exit(1);
    }

    // get the array of short integers
    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride, length,
                    (VOIDP)tempBuffer) != HDF_SUCCEED)
        return FALSE;

    // get the scale factor
    float64  scaleFactor;
    if (l1File->GetScaleFactor(sdsIDs[0], scaleFactor) == HDF_FAIL)
        return FALSE;

    // convert the short integers to floats, and return
    float* floatP = (float*)buffer;
    for (int i=0; i < length; i++, floatP++)
    {
        *floatP = (float) (scaleFactor * tempBuffer[i]);
    }
    free((void*) tempBuffer);
    return(TRUE);

}//ExtractData1D_uint2_float

//----------------------------------------------------------------------
// Function:    ExtractData1D_int2_float ([])
// Extracts:    one dimensional data (int2 -> float)
//----------------------------------------------------------------------
int
ExtractData1D_int2_float(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    short* tempBuffer = (short*) calloc(length, sizeof(short));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "ExtractData1D: Out of memory\n");
        exit(1);
    }

    // get the array of short integers
    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride, length,
                    (VOIDP)tempBuffer) != HDF_SUCCEED)
        return FALSE;

    // get the scale factor
    float64  scaleFactor;
    if (l1File->GetScaleFactor(sdsIDs[0], scaleFactor) == HDF_FAIL)
        return FALSE;

    // convert the short integers to floats, and return
    float* floatP = (float*)buffer;
    for (int i=0; i < length; i++, floatP++)
    {
        *floatP = (float) (scaleFactor * tempBuffer[i]);
    }
    free((void*) tempBuffer);
    return(TRUE);

}//ExtractData1D_int2_float

//----------------------------------------------------------------------
// Function:    ExtractData1D_uint1_float ([])
// Extracts:    one dimensional data (char -> float)
//----------------------------------------------------------------------
int
ExtractData1D_uint1_float(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned char* tmpBuffer =
             (unsigned char*) calloc(length, sizeof(unsigned char));
    if (tmpBuffer == NULL)
    {
        fprintf(stderr, "ExtractData1D: Out of memory\n");
        exit(1);
    }

    // get the array of short integers
    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride, length,
                    (VOIDP)tmpBuffer) != HDF_SUCCEED)
        return FALSE;

    // get the scale factor
    float64  scaleFactor;
    if (l1File->GetScaleFactor(sdsIDs[0], scaleFactor) == HDF_FAIL)
        return FALSE;

    // convert the short integers to floats, and return
    float* floatP = (float*)buffer;
    for (int i=0; i < length; i++, floatP++)
    {
        *floatP = (float) (scaleFactor * tmpBuffer[i]);
    }
    free((void*) tmpBuffer);
    return(TRUE);

}//ExtractData1D_uint1_float

//----------------------------------------------------------------------
// Function:    ExtractData1D_int_char3 ([])
// Extracts:    one dimensional data (int -> char3)
//----------------------------------------------------------------------
int
ExtractData1D_int_char3(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned int* tempBuffer =
             (unsigned int*) calloc(length, sizeof(unsigned int));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "ExtractData1D_int_char3: Out of memory\n");
        exit(1);
    }

    // get the array of integers
    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride, length,
                    (VOIDP)tempBuffer) != HDF_SUCCEED)
        return FALSE;

    // convert the integers to char3, and return
    char* charP = (char*)buffer;
    char* tempP = (char*)tempBuffer;
    for (int i=0; i < length; i++)
    {
        memcpy(charP, tempP + 1, 3);
        charP += 3;
        tempP += 4;
    }
    free((void*) tempBuffer);
    return(TRUE);

}//ExtractData1D_int_char3

//----------------------------------------------------------------------
// Function:    ExtractData2D_4 ([][4])
// Extracts:    two dimensional data
//----------------------------------------------------------------------
int
ExtractData2D_4(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    int32 sdStart[2], sdStride[2], sdEdge[2];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 4;

    if (stride == 1 || stride == 0)
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);
    else
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);

}//ExtractData2D_4

//----------------------------------------------------------------------
// Function:    ExtractData2D_5 ([][5])
// Extracts:    two dimensional data
//----------------------------------------------------------------------
int
ExtractData2D_5(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    int32 sdStart[2], sdStride[2], sdEdge[2];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 5;

    if (stride == 1 || stride == 0)
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);
    else
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);

}//ExtractData2D_5

//----------------------------------------------------------------------
// Function:    ExtractData3D_2_8 ([][2][8])
// Extracts:    three dimensional data
//----------------------------------------------------------------------
int
ExtractData3D_2_8(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    int32 sdStart[3], sdStride[3], sdEdge[3];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStart[2] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdStride[2] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 2;
    sdEdge[2] = 8;

    if (stride == 1 || stride == 0)
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);
    else
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);

}//ExtractData3D_2_8

//----------------------------------------------------------------------
// Function:    ExtractData2D_12 ([][12])
// Extracts:    two dimensional data
//----------------------------------------------------------------------
int
ExtractData2D_12(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    int32 sdStart[2], sdStride[2], sdEdge[2];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 12;

    if (stride == 1 || stride == 0)
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);
    else
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);

}//ExtractData2D_12

//----------------------------------------------------------------------
// Function:    ExtractData2D_13 ([][13])
// Extracts:    two dimensional data
//----------------------------------------------------------------------
int
ExtractData2D_13(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    int32 sdStart[2], sdStride[2], sdEdge[2];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 13;

    if (stride == 1 || stride == 0)
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);
    else
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);

}//ExtractData2D_13

//----------------------------------------------------------------------
// Function:    ExtractData2D_76 ([][76])
// Extracts:    two dimensional data
//----------------------------------------------------------------------
int
ExtractData2D_76(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    int32 sdStart[2], sdStride[2], sdEdge[2];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 76;

    if (stride == 1 || stride == 0)
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);
    else
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);

}//ExtractData2D_76

//----------------------------------------------------------------------
// Function:    ExtractData2D_49 ([][49])
// Extracts:    two dimensional data
//----------------------------------------------------------------------
int
ExtractData2D_49(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    int32 sdStart[2], sdStride[2], sdEdge[2];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 49;

    if (stride == 1 || stride == 0)
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);
    else
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);

}//ExtractData2D_49

//----------------------------------------------------------------------
// Function:    ExtractData2D_100 ([][100])
// Extracts:    two dimensional data
//----------------------------------------------------------------------
int
ExtractData2D_100(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    int32 sdStart[2], sdStride[2], sdEdge[2];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 100;

    if (stride == 1 || stride == 0)
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);
    else
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);

}//ExtractData2D_100

//----------------------------------------------------------------------
// Function:    ExtractData2D_810 ([][810])
// Extracts:    two dimensional data
//----------------------------------------------------------------------
int
ExtractData2D_810(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    int32 sdStart[2], sdStride[2], sdEdge[2];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 810;

    if (stride == 1 || stride == 0)
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);
    else
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);

}//ExtractData2D_810

//----------------------------------------------------------------------
// Function:    ExtractData2D_2_uint2_float ([][76])
// Extracts:    two dimensional data
//----------------------------------------------------------------------
int
ExtractData2D_76_uint2_float(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold unsigned short integers
    unsigned short* tempBuffer =
             (unsigned short*) calloc(length * 76, sizeof(unsigned short));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "ExtractData2D_76_uint2_float: Out of memory\n");
        exit(1);
    }
    int32 sdStart[2], sdStride[2], sdEdge[2];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 76;

    if (stride == 1 || stride == 0)
    {
        if (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, tempBuffer) != HDF_SUCCEED)
            return FALSE;
    }
    else
    {
        if (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, tempBuffer) != HDF_SUCCEED)
            return FALSE;
    }

    // get the scale factor
    float64  scaleFactor;
    if (l1File->GetScaleFactor(sdsIDs[0], scaleFactor) == HDF_FAIL)
        return FALSE;

    // convert the short integers to floats, and return
    float* floatP = (float*)buffer;
    unsigned short* ushortP = tempBuffer;
    for (int i=0; i < length; i++)
    {
        for (int j=0; j < 76; j++, floatP++, ushortP++)
        {
            
            *floatP = (float) (scaleFactor * (*ushortP));
        }
    }
    free((void*) tempBuffer);

    return TRUE;

}//ExtractData2D_76_uint2_float

//----------------------------------------------------------------------
// Function:    ExtractData2D_76_int2_float ([][76])
// Extracts:    two dimensional data
//----------------------------------------------------------------------
int
ExtractData2D_76_int2_float(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    short* tempBuffer = (short*) calloc(length * 76, sizeof(short));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "ExtractData2D_76_int2_float: Out of memory\n");
        exit(1);
    }
    int32 sdStart[2], sdStride[2], sdEdge[2];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 76;

    if (stride == 1 || stride == 0)
    {
        if (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, tempBuffer) != HDF_SUCCEED)
            return FALSE;
    }
    else
    {
        if (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, tempBuffer) != HDF_SUCCEED)
            return FALSE;
    }

    // get the scale factor
    float64  scaleFactor;
    if (l1File->GetScaleFactor(sdsIDs[0], scaleFactor) == HDF_FAIL)
        return FALSE;

    // convert the short integers to floats, and return
    float* floatP = (float*)buffer;
    short* ushortP = tempBuffer;
    for (int i=0; i < length; i++)
    {
        for (int j=0; j < 76; j++, floatP++, ushortP++)
        {
            
            *floatP = (float) (scaleFactor * (*ushortP));
        }
    }
    free((void*) tempBuffer);

    return TRUE;

}//ExtractData2D_76_int2_float

//----------------------------------------------------------------------
// Function:    ExtractData2D_810_uint2_float ([][810])
// Extracts:    two dimensional data
//----------------------------------------------------------------------
int
ExtractData2D_810_uint2_float(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold unsigned short integers
    unsigned short* tempBuffer =
             (unsigned short*) calloc(length * 810, sizeof(unsigned short));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "ExtractData2D_810_uint2_float: Out of memory\n");
        exit(1);
    }
    int32 sdStart[2], sdStride[2], sdEdge[2];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 810;

    if (stride == 1 || stride == 0)
    {
        if (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, tempBuffer) != HDF_SUCCEED)
            return FALSE;
    }
    else
    {
        if (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, tempBuffer) != HDF_SUCCEED)
            return FALSE;
    }

    // get the scale factor
    float64  scaleFactor;
    if (l1File->GetScaleFactor(sdsIDs[0], scaleFactor) == HDF_FAIL)
        return FALSE;

    // convert the short integers to floats, and return
    float* floatP = (float*)buffer;
    unsigned short* ushortP = tempBuffer;
    for (int i=0; i < length; i++)
    {
        for (int j=0; j < 810; j++, floatP++, ushortP++)
        {
            
            *floatP = (float) (scaleFactor * (*ushortP));
        }
    }
    free((void*) tempBuffer);

    return TRUE;

}//ExtractData2D_810_uint2_float

//----------------------------------------------------------------------
// Function:    ExtractData2D_810_int2_float ([][810])
// Extracts:    two dimensional data
//----------------------------------------------------------------------
int
ExtractData2D_810_int2_float(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    short* tempBuffer = (short*) calloc(length * 810, sizeof(short));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "ExtractData2D_810_int2_float: Out of memory\n");
        exit(1);
    }
    int32 sdStart[2], sdStride[2], sdEdge[2];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 810;

    if (stride == 1 || stride == 0)
    {
        if (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, tempBuffer) != HDF_SUCCEED)
            return FALSE;
    }
    else
    {
        if (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, tempBuffer) != HDF_SUCCEED)
            return FALSE;
    }

    // get the scale factor
    float64  scaleFactor;
    if (l1File->GetScaleFactor(sdsIDs[0], scaleFactor) == HDF_FAIL)
        return FALSE;

    // convert the short integers to floats, and return
    float* floatP = (float*)buffer;
    short* ushortP = tempBuffer;
    for (int i=0; i < length; i++)
    {
        for (int j=0; j < 810; j++, floatP++, ushortP++)
        {
            
            *floatP = (float) (scaleFactor * (*ushortP));
        }
    }
    free((void*) tempBuffer);

    return TRUE;

}//ExtractData2D_810_int2_float

//----------------------------------------------------------------------
// Function:    ExtractData2D_100_uint2_float ([][100])
// Extracts:    two dimensional data
//----------------------------------------------------------------------
int
ExtractData2D_100_uint2_float(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold unsigned short integers
    unsigned short* tempBuffer =
             (unsigned short*) calloc(length * 100, sizeof(unsigned short));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "ExtractData2D_100_uint2_float: Out of memory\n");
        exit(1);
    }
    int32 sdStart[2], sdStride[2], sdEdge[2];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 100;

    if (stride == 1 || stride == 0)
    {
        if (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, tempBuffer) != HDF_SUCCEED)
            return FALSE;
    }
    else
    {
        if (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, tempBuffer) != HDF_SUCCEED)
            return FALSE;
    }

    // get the scale factor
    float64  scaleFactor;
    if (l1File->GetScaleFactor(sdsIDs[0], scaleFactor) == HDF_FAIL)
        return FALSE;

    // convert the short integers to floats, and return
    float* floatP = (float*)buffer;
    unsigned short* ushortP = tempBuffer;
    for (int i=0; i < length; i++)
    {
        for (int j=0; j < 100; j++, floatP++, ushortP++)
        {
            
            *floatP = (float) (scaleFactor * (*ushortP));
        }
    }
    free((void*) tempBuffer);

    return TRUE;

}//ExtractData2D_100_uint2_float

//----------------------------------------------------------------------
// Function:    ExtractData3D_76_4_uint2_float ([][76][4])
// Extracts:    3 dimensional data
//----------------------------------------------------------------------
int
ExtractData3D_76_4_uint2_float(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold unsigned short integers
    unsigned short* tempBuffer =
             (unsigned short*) calloc(length * 304, sizeof(unsigned short));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "ExtractData3D_76_4_uint2_float: Out of memory\n");
        exit(1);
    }
    int32 sdStart[3], sdStride[3], sdEdge[3];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStart[2] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdStride[2] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 76;
    sdEdge[2] = 4;

    if (stride == 1 || stride == 0)
    {
        if (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, tempBuffer) != HDF_SUCCEED)
            return FALSE;
    }
    else
    {
        if (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, tempBuffer) != HDF_SUCCEED)
            return FALSE;
    }

    // get the scale factor
    float64  scaleFactor;
    if (l1File->GetScaleFactor(sdsIDs[0], scaleFactor) == HDF_FAIL)
        return FALSE;

    // convert the short integers to floats, and return
    float* floatP = (float*)buffer;
    unsigned short* ushortP = tempBuffer;
    for (int i=0; i < length; i++)
    {
        for (int j=0; j < 304; j++, floatP++, ushortP++)
                *floatP = (float) (scaleFactor * (*ushortP));
    }
    free((void*) tempBuffer);

    return TRUE;

}//ExtractData3D_76_4_uint2_float

//----------------------------------------------------------------------
// Function:    ExtractData3D_76_4_int2_float ([][76][4])
// Extracts:    3 dimensional data
//----------------------------------------------------------------------
int
ExtractData3D_76_4_int2_float(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    short* tempBuffer = (short*) calloc(length * 304, sizeof(short));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "ExtractData3D_76_4_int2_float: Out of memory\n");
        exit(1);
    }
    int32 sdStart[3], sdStride[3], sdEdge[3];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStart[2] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdStride[2] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 76;
    sdEdge[2] = 4;

    if (stride == 1 || stride == 0)
    {
        if (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, tempBuffer) != HDF_SUCCEED)
            return FALSE;
    }
    else
    {
        if (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, tempBuffer) != HDF_SUCCEED)
            return FALSE;
    }

    // get the scale factor
    float64  scaleFactor;
    if (l1File->GetScaleFactor(sdsIDs[0], scaleFactor) == HDF_FAIL)
        return FALSE;

    // convert the short integers to floats, and return
    float* floatP = (float*)buffer;
    short* shortP = tempBuffer;
    for (int i=0; i < length; i++)
    {
        for (int j=0; j < 304; j++, floatP++, shortP++)
            *floatP = (float) (scaleFactor * (*shortP));
    }
    free((void*) tempBuffer);

    return TRUE;

}//ExtractData3D_76_4_int2_float

//----------------------------------------------------------------------
// Function:    ExtractData3D_100_12 ([][100][12])
// Extracts:    three dimensional data
//----------------------------------------------------------------------
int
ExtractData3D_100_12(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    int32 sdStart[3], sdStride[3], sdEdge[3];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStart[2] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdStride[2] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 100;
    sdEdge[2] = 12;

    if (stride == 1 || stride == 0)
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);
    else
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);

}//ExtractData3D_100_12


//----------------------------------------------------------------------
// Function:    Extract8Bit0 ([])
// Extracts:    one dimensional data (Bit 0 only)
//----------------------------------------------------------------------
int
Extract8Bit0(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, buffer) != HDF_SUCCEED)
        return FALSE;

    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(charP[i], 0);
    }
    return TRUE;

}//Extract8Bit0

//----------------------------------------------------------------------
// Function:    Extract8Bit1 ([])
// Extracts:    one dimensional data (Bit 1 only)
//----------------------------------------------------------------------
int
Extract8Bit1(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, buffer) != HDF_SUCCEED)
        return FALSE;

    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(charP[i], 1);
    }
    return TRUE;

}//Extract8Bit1

//----------------------------------------------------------------------
// Function:    Extract8Bit2 ([])
// Extracts:    one dimensional data (Bit 1 only)
//----------------------------------------------------------------------
int
Extract8Bit2(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, buffer) != HDF_SUCCEED)
        return FALSE;

    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(charP[i], 2);
    }
    return TRUE;

}//Extract8Bit2

//----------------------------------------------------------------------
// Function:    Extract8Bit3 ([])
// Extracts:    one dimensional data (Bit 1 only)
//----------------------------------------------------------------------
int
Extract8Bit3(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, buffer) != HDF_SUCCEED)
        return FALSE;

    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(charP[i], 3);
    }
    return TRUE;

}//Extract8Bit3

//----------------------------------------------------------------------
// Function:    Extract8Bit4 ([])
// Extracts:    one dimensional data (Bit 1 only)
//----------------------------------------------------------------------
int
Extract8Bit4(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, buffer) != HDF_SUCCEED)
        return FALSE;

    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(charP[i], 4);
    }
    return TRUE;

}//Extract8Bit4

//----------------------------------------------------------------------
// Function:    Extract8Bit5 ([])
// Extracts:    one dimensional data (Bit 1 only)
//----------------------------------------------------------------------
int
Extract8Bit5(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, buffer) != HDF_SUCCEED)
        return FALSE;

    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(charP[i], 5);
    }
    return TRUE;

}//Extract8Bit5

//----------------------------------------------------------------------
// Function:    Extract8Bit6 ([])
// Extracts:    one dimensional data (Bit 1 only)
//----------------------------------------------------------------------
int
Extract8Bit6(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, buffer) != HDF_SUCCEED)
        return FALSE;

    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(charP[i], 6);
    }
    return TRUE;

}//Extract8Bit6

//----------------------------------------------------------------------
// Function:    Extract8Bit7 ([])
// Extracts:    one dimensional data (Bit 1 only)
//----------------------------------------------------------------------
int
Extract8Bit7(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, buffer) != HDF_SUCCEED)
        return FALSE;

    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(charP[i], 7);
    }
    return TRUE;

}//Extract8Bit7

//----------------------------------------------------------------------
// Function:    Extract16Bit0 ([])
// Extracts:    one dimensional data (Bit 0 only)
//----------------------------------------------------------------------
int
Extract16Bit0(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* shortBuffer =
              (unsigned short*) calloc(length, sizeof(short));
    if (shortBuffer == NULL)
    {
        fprintf(stderr, "Extract16Bit0: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, shortBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 0 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(shortBuffer[i], 0);
    }
    free((void*) shortBuffer);
    return TRUE;

}//Extract16Bit0

//----------------------------------------------------------------------
// Function:    Extract16Bit1 ([])
// Extracts:    one dimensional data (Bit 1 only)
//----------------------------------------------------------------------
int
Extract16Bit1(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* shortBuffer =
              (unsigned short*) calloc(length, sizeof(short));
    if (shortBuffer == NULL)
    {
        fprintf(stderr, "Extract16Bit1: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, shortBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 1 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(shortBuffer[i], 1);
    }
    free((void*) shortBuffer);
    return TRUE;

}//Extract16Bit1

//----------------------------------------------------------------------
// Function:    Extract16Bit2 ([])
// Extracts:    one dimensional data (Bit 2 only)
//----------------------------------------------------------------------
int
Extract16Bit2(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* shortBuffer =
              (unsigned short*) calloc(length, sizeof(short));
    if (shortBuffer == NULL)
    {
        fprintf(stderr, "Extract16Bit2: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, shortBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 2 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(shortBuffer[i], 2);
    }
    free((void*) shortBuffer);
    return TRUE;

}//Extract16Bit2

//----------------------------------------------------------------------
// Function:    Extract16Bit3 ([])
// Extracts:    one dimensional data (Bit 3 only)
//----------------------------------------------------------------------
int
Extract16Bit3(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* shortBuffer =
              (unsigned short*) calloc(length, sizeof(short));
    if (shortBuffer == NULL)
    {
        fprintf(stderr, "Extract16Bit3: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, shortBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 3 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(shortBuffer[i], 3);
    }
    free((void*) shortBuffer);
    return TRUE;

}//Extract16Bit3

//----------------------------------------------------------------------
// Function:    Extract16Bit4 ([])
// Extracts:    one dimensional data (Bit 4 only)
//----------------------------------------------------------------------
int
Extract16Bit4(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* shortBuffer =
              (unsigned short*) calloc(length, sizeof(short));
    if (shortBuffer == NULL)
    {
        fprintf(stderr, "Extract16Bit4: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, shortBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 4 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(shortBuffer[i], 4);
    }
    free((void*) shortBuffer);
    return TRUE;

}//Extract16Bit4

//----------------------------------------------------------------------
// Function:    Extract16Bit5 ([])
// Extracts:    one dimensional data (Bit 5 only)
//----------------------------------------------------------------------
int
Extract16Bit5(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* shortBuffer =
              (unsigned short*) calloc(length, sizeof(short));
    if (shortBuffer == NULL)
    {
        fprintf(stderr, "Extract16Bit5: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, shortBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 5 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(shortBuffer[i], 5);
    }
    free((void*) shortBuffer);
    return TRUE;

}//Extract16Bit5

//----------------------------------------------------------------------
// Function:    Extract16Bit6 ([])
// Extracts:    one dimensional data (Bit 6 only)
//----------------------------------------------------------------------
int
Extract16Bit6(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* shortBuffer =
              (unsigned short*) calloc(length, sizeof(short));
    if (shortBuffer == NULL)
    {
        fprintf(stderr, "Extract16Bit6: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, shortBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 6 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(shortBuffer[i], 6);
    }
    free((void*) shortBuffer);
    return TRUE;

}//Extract16Bit6

//----------------------------------------------------------------------
// Function:    Extract16Bit7 ([])
// Extracts:    one dimensional data (Bit 7 only)
//----------------------------------------------------------------------
int
Extract16Bit7(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* shortBuffer =
              (unsigned short*) calloc(length, sizeof(short));
    if (shortBuffer == NULL)
    {
        fprintf(stderr, "Extract16Bit7: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, shortBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 7 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(shortBuffer[i], 7);
    }
    free((void*) shortBuffer);
    return TRUE;

}//Extract16Bit7

//----------------------------------------------------------------------
// Function:    Extract16Bit8 ([])
// Extracts:    one dimensional data (Bit 8 only)
//----------------------------------------------------------------------
int
Extract16Bit8(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* shortBuffer =
              (unsigned short*) calloc(length, sizeof(short));
    if (shortBuffer == NULL)
    {
        fprintf(stderr, "Extract16Bit8: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, shortBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 8 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(shortBuffer[i], 8);
    }
    free((void*) shortBuffer);
    return TRUE;

}//Extract16Bit8

//----------------------------------------------------------------------
// Function:    Extract16Bit9 ([])
// Extracts:    one dimensional data (Bit 9 only)
//----------------------------------------------------------------------
int
Extract16Bit9(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* shortBuffer =
              (unsigned short*) calloc(length, sizeof(short));
    if (shortBuffer == NULL)
    {
        fprintf(stderr, "Extract16Bit9: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, shortBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 9 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(shortBuffer[i], 9);
    }
    free((void*) shortBuffer);
    return TRUE;

}//Extract16Bit9

//----------------------------------------------------------------------
// Function:    Extract16Bit10 ([])
// Extracts:    one dimensional data (Bit 10 only)
//----------------------------------------------------------------------
int
Extract16Bit10(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* shortBuffer =
              (unsigned short*) calloc(length, sizeof(short));
    if (shortBuffer == NULL)
    {
        fprintf(stderr, "Extract16Bit10: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, shortBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 10 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(shortBuffer[i], 10);
    }
    free((void*) shortBuffer);
    return TRUE;

}//Extract16Bit10

//----------------------------------------------------------------------
// Function:    Extract16Bit11 ([])
// Extracts:    one dimensional data (Bit 11 only)
//----------------------------------------------------------------------
int
Extract16Bit11(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* shortBuffer =
              (unsigned short*) calloc(length, sizeof(short));
    if (shortBuffer == NULL)
    {
        fprintf(stderr, "Extract16Bit11: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, shortBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 11 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(shortBuffer[i], 11);
    }
    free((void*) shortBuffer);
    return TRUE;

}//Extract16Bit11

//----------------------------------------------------------------------
// Function:    Extract16Bit12 ([])
// Extracts:    one dimensional data (Bit 12 only)
//----------------------------------------------------------------------
int
Extract16Bit12(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* shortBuffer =
              (unsigned short*) calloc(length, sizeof(short));
    if (shortBuffer == NULL)
    {
        fprintf(stderr, "Extract16Bit12: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, shortBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 12 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(shortBuffer[i], 12);
    }
    free((void*) shortBuffer);
    return TRUE;

}//Extract16Bit12

//----------------------------------------------------------------------
// Function:    Extract16Bit13 ([])
// Extracts:    one dimensional data (Bit 13 only)
//----------------------------------------------------------------------
int
Extract16Bit13(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* shortBuffer =
              (unsigned short*) calloc(length, sizeof(short));
    if (shortBuffer == NULL)
    {
        fprintf(stderr, "Extract16Bit13: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, shortBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 13 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(shortBuffer[i], 13);
    }
    free((void*) shortBuffer);
    return TRUE;

}//Extract16Bit13

//----------------------------------------------------------------------
// Function:    Extract16Bit14 ([])
// Extracts:    one dimensional data (Bit 14 only)
//----------------------------------------------------------------------
int
Extract16Bit14(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* shortBuffer =
              (unsigned short*) calloc(length, sizeof(short));
    if (shortBuffer == NULL)
    {
        fprintf(stderr, "Extract16Bit14: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, shortBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 14 only and return the buffer
    (void)memset(buffer, 0, length);
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(shortBuffer[i], 14);
    }
    free((void*) shortBuffer);
    return TRUE;

}//Extract16Bit14

//----------------------------------------------------------------------
// Function:    Extract16Bit15 ([])
// Extracts:    one dimensional data (Bit 15 only)
//----------------------------------------------------------------------
int
Extract16Bit15(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* shortBuffer =
              (unsigned short*) calloc(length, sizeof(short));
    if (shortBuffer == NULL)
    {
        fprintf(stderr, "Extract16Bit15: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, shortBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 15 only and return the buffer
    (void)memset(buffer, 0, length);
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(shortBuffer[i], 15);
    }
    free((void*) shortBuffer);
    return TRUE;

}//Extract16Bit15

//----------------------------------------------------------------------
// Function:    Extract8Bit0_4 ([])
// Extracts:    one dimensional data (Bit 0-4 only)
//----------------------------------------------------------------------
int
Extract8Bit0_4(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned char* tempBuffer =
              (unsigned char*) calloc(length, sizeof(unsigned char));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "Extract8Bit0_4: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, tempBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 0_4 only and return the buffer
    (void)memset(buffer, 0, length);
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BITS(tempBuffer[i], 4, 5);
    }
    free((void*) tempBuffer);
    return TRUE;

}//Extract8Bit0_4

//----------------------------------------------------------------------
// Function:    Extract8Bit5_7 ([])
// Extracts:    one dimensional data (Bit 5-7 only)
//----------------------------------------------------------------------
int
Extract8Bit5_7(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned char* tempBuffer =
              (unsigned char*) calloc(length, sizeof(unsigned char));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "Extract8Bit5_7: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, tempBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 5_7 only and return the buffer
    (void)memset(buffer, 0, length);
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BITS(tempBuffer[i], 7, 3);
    }
    free((void*) tempBuffer);
    return TRUE;

}//Extract8Bit5_7

//----------------------------------------------------------------------
// Function:    Extract32Bit2 ([])
// Extracts:    one dimensional data (Bit 2 only)
//----------------------------------------------------------------------
int
Extract32Bit2(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold long integers
    unsigned int* tmpBuffer =
              (unsigned int*) calloc(length, sizeof(unsigned int));
    if (tmpBuffer == NULL)
    {
        fprintf(stderr, "Extract32Bit2: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                              length, tmpBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 2 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(tmpBuffer[i], 2);
    }
    free((void*) tmpBuffer);
    return TRUE;

}//Extract32Bit2

//----------------------------------------------------------------------
// Function:    Extract32Bit3 ([])
// Extracts:    one dimensional data (Bit 3 only)
//----------------------------------------------------------------------
int
Extract32Bit3(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold long integers
    unsigned int* tmpBuffer =
              (unsigned int*) calloc(length, sizeof(unsigned int));
    if (tmpBuffer == NULL)
    {
        fprintf(stderr, "Extract32Bit3: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                              length, tmpBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 3 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(tmpBuffer[i], 3);
    }
    free((void*) tmpBuffer);
    return TRUE;

}//Extract32Bit3

//----------------------------------------------------------------------
// Function:    Extract32Bit7 ([])
// Extracts:    one dimensional data (Bit 7 only)
//----------------------------------------------------------------------
int
Extract32Bit7(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold long integers
    unsigned int* tmpBuffer =
              (unsigned int*) calloc(length, sizeof(unsigned int));
    if (tmpBuffer == NULL)
    {
        fprintf(stderr, "Extract32Bit7: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                              length, tmpBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 7 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(tmpBuffer[i], 7);
    }
    free((void*) tmpBuffer);
    return TRUE;

}//Extract32Bit7

//----------------------------------------------------------------------
// Function:    Extract32Bit8 ([])
// Extracts:    one dimensional data (Bit 8 only)
//----------------------------------------------------------------------
int
Extract32Bit8(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold long integers
    unsigned int* tmpBuffer =
              (unsigned int*) calloc(length, sizeof(unsigned int));
    if (tmpBuffer == NULL)
    {
        fprintf(stderr, "Extract32Bit8: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                              length, tmpBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 8 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(tmpBuffer[i], 8);
    }
    free((void*) tmpBuffer);
    return TRUE;

}//Extract32Bit8

//----------------------------------------------------------------------
// Function:    Extract32Bit9 ([])
// Extracts:    one dimensional data (Bit 9 only)
//----------------------------------------------------------------------
int
Extract32Bit9(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold long integers
    unsigned int* tmpBuffer =
              (unsigned int*) calloc(length, sizeof(unsigned int));
    if (tmpBuffer == NULL)
    {
        fprintf(stderr, "Extract32Bit9: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                              length, tmpBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 9 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(tmpBuffer[i], 9);
    }
    free((void*) tmpBuffer);
    return TRUE;

}//Extract32Bit9

//----------------------------------------------------------------------
// Function:    Extract32Bit10 ([])
// Extracts:    one dimensional data (Bit 10 only)
//----------------------------------------------------------------------
int
Extract32Bit10(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold long integers
    unsigned int* tmpBuffer =
              (unsigned int*) calloc(length, sizeof(unsigned int));
    if (tmpBuffer == NULL)
    {
        fprintf(stderr, "Extract32Bit10: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                              length, tmpBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 10 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(tmpBuffer[i], 10);
    }
    free((void*) tmpBuffer);
    return TRUE;

}//Extract32Bit10

//----------------------------------------------------------------------
// Function:    Extract32Bit11 ([])
// Extracts:    one dimensional data (Bit 11 only)
//----------------------------------------------------------------------
int
Extract32Bit11(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold long integers
    unsigned int* tmpBuffer =
              (unsigned int*) calloc(length, sizeof(unsigned int));
    if (tmpBuffer == NULL)
    {
        fprintf(stderr, "Extract32Bit11: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                              length, tmpBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 11 only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(tmpBuffer[i], 11);
    }
    free((void*) tmpBuffer);
    return TRUE;

}//Extract32Bit11

//----------------------------------------------------------------------
// Function:    Extract32Bit0_1 ([])
// Extracts:    one dimensional data (Bit 0-1 only)
//----------------------------------------------------------------------
int
Extract32Bit0_1(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned int* tempBuffer =
              (unsigned int*) calloc(length, sizeof(unsigned int));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "Extract32Bit0_1: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                              length, tempBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 0_1 only and return the buffer
    (void)memset(buffer, 0, length);
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BITS(tempBuffer[i], 1, 2);
    }
    free((void*) tempBuffer);
    return TRUE;

}//Extract32Bit0_1

//----------------------------------------------------------------------
// Function:    Extract32Bit4_6 ([])
// Extracts:    one dimensional data (Bit 4-6 only)
//----------------------------------------------------------------------
int
Extract32Bit4_6(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer)
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned int* tempBuffer =
              (unsigned int*) calloc(length, sizeof(unsigned int));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "Extract32Bit5_7: Out of memory\n");
        exit(1);
    }

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, tempBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 5_7 only and return the buffer
    (void)memset(buffer, 0, length);
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BITS(tempBuffer[i], 6, 3);
    }
    free((void*) tempBuffer);
    return TRUE;

}//Extract32Bit4_6

//----------------------------------------------------------------------
// Function:    ExtractCodaATime ([])
// Extracts:    Coda A time string
//----------------------------------------------------------------------
int
ExtractTaiTime(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer)
{
    assert(l1File != 0);

    // alloc space to hold short integers
    double* tempBuffer = (double*) calloc(length, sizeof(double));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "ExtractTaiTime: Out of memory\n");
        exit(1);
    }

    // get the array of double
    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride, length,
                    (VOIDP)tempBuffer) != HDF_SUCCEED)
        return FALSE;

    //----------------------------------------------------
    // convert the doubles to Itime, add to the base,
    // then convert to Char6 and return
    //----------------------------------------------------
    char* ptr = (char*) buffer;
    for (int i=0; i < length; i++)
    {
#if 0
fprintf(stdout, "\nTai Time (double)= %lf\n", tempBuffer[i]);
#endif
        Itime l1TaiTime(tempBuffer[i] + ITIME_DEFAULT_SEC);
#if 0
fprintf(stdout, "New Time (sec)= %d\n", l1TaiTime.sec);
int output_string[L1_TIME_LEN];
l1TaiTime.ItimeToL1(output_string);
fprintf(stdout, "%s\n", output_string);
#endif
        // return Char6 in buffer
        memcpy(ptr,     (char *)&(l1TaiTime.sec), sizeof(int));
        memcpy(ptr + 4, (char *)&(l1TaiTime.ms), sizeof(short));
        ptr += 6;
    }
    free((void*) tempBuffer);

    return TRUE;

}//ExtractTaiTime

//----------------------------------------------------------------------
// Function:    ExtractData1D_m_km
// Extracts:    float: meter => kilometer
//----------------------------------------------------------------------
int
ExtractData1D_m_km(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0);
    // alloc space to hold floats
    float* tempBuffer =
             (float*) calloc(length, sizeof(float));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "ExtractData1D_m_km: Out of memory\n");
        exit(1);
    }

    // get the array of unsigned int
    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride, length,
                    (VOIDP)tempBuffer) != HDF_SUCCEED)
        return FALSE;

    // convert the meter to kilometer, and return
    float* floatP = (float*)buffer;
    for (int i=0; i < length; i++, floatP++)
    {
        *floatP = tempBuffer[i] / 1000.0;
    }
    free((void*) tempBuffer);
    return(TRUE);

} //ExtractData1D_m_km

#if 0
//----------------------------------------------------------------------
// Function:    ExtractXmitPowerdBm
// Extracts:    float
//----------------------------------------------------------------------
int
ExtractXmitPowerAdBm(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    assert(l1File != 0 && length > 0);

    // alloc space to hold floats
    float* tempBuffer =
             (float*) calloc(length, sizeof(float));
    if (tempBuffer == NULL)
    {
        fprintf(stderr, "ExtractData1D_m_km: Out of memory\n");
        exit(1);
    }

    if (ExtractData1D_uint1_float(l1File, sdsIDs, start, stride,
                          length, tempBuffer) == 0)
        return 0;

    // need to manually apply the polynomial here
    // too complicated to get it from polynomial table

#if 0
    // create a polynomial table and keep it
    static PolynomialTable* polyTable=0;
    if (polyTable == 0)
    {
    }

    const Polynomial* polynomial = polyTable->SelectPolynomial(
                                 "transmit_power_a", "mWatts");
    if (polynomial == 0) return 0;
    polynomial->ApplyReplaceArray(tempBuffer, length);
#endif


    memcpy(buffer, tempBuffer, length * sizeof(float));

    free((void*) tempBuffer);
    return(TRUE);

} //ExtractXmitPowerdBm
#endif
