//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.25   10 Nov 1998 08:51:14   sally
// add delta instrument time because the instrument seems to skip cycle
// 
//    Rev 1.24   03 Nov 1998 16:00:10   sally
// add source sequence count
// 
//    Rev 1.23   29 Oct 1998 15:11:46   sally
// do a VSseek() before VSread()
// 
//    Rev 1.22   28 Oct 1998 15:01:58   sally
// add function for L1B Hdf
// Revision 1.2  1998/10/20 21:26:17  sally
// add L1B
//
// 
//    Rev 1.21   16 Oct 1998 09:04:36   sally
// added ExtractL1Time
// 
//    Rev 1.20   13 Oct 1998 15:32:58   sally
// added L1B file
// 
//    Rev 1.19   24 Sep 1998 15:54:40   sally
// add one extract function for full_frame
// 
//    Rev 1.18   18 Aug 1998 15:06:30   sally
// mv mWatts for transmit power to L1ADrvTab.C
// 
//    Rev 1.17   13 Aug 1998 16:26:28   sally
// 
//    Rev 1.16   04 Aug 1998 16:29:20   deliver
// pass polynomial table to ParameterList's HoldExtract
// 
//    Rev 1.15   04 Aug 1998 15:58:58   sally
// fixe L1AParTab so that dBm comes from polynomial table and
// mWatts will be calculated
// 
//    Rev 1.14   27 Jul 1998 14:00:10   sally
// passing polynomial table to extraction function
// 
//    Rev 1.13   23 Jul 1998 16:14:00   sally
// pass polynomial table to extractFunc()
// 
//    Rev 1.12   22 Jun 1998 15:25:22   sally
// change to incorporate Barry's update
// 
//    Rev 1.11   19 Jun 1998 14:02:18   sally
// add some bit extraction functions needed by HK2
// 
//    Rev 1.10   03 Jun 1998 10:09:46   sally
// change parameter names and types due to LP's changes
// 
//    Rev 1.9   06 May 1998 15:17:14   sally
// took out exit()
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
#include "PolyTable.h"
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
//  Input:     frame       pointer to a L1A DATA HEADER
//  Output:    data        pointer to the data destination
//  return:
//            num: num == length which user requests
//              1: 1 value
//              0: no value
//             -1: error occured
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
VOIDP       buffer,
PolynomialTable*)     // unused
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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned int* tempBuffer =
             (unsigned int*) calloc(length, sizeof(unsigned int));
    assert (tempBuffer != 0);

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold unsigned short integers
    unsigned short* tempBuffer =
             (unsigned short*) calloc(length, sizeof(unsigned short));
    assert(tempBuffer != 0);

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold short integers
    short* tempBuffer = (short*) calloc(length, sizeof(short));
    assert(tempBuffer != 0);

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
VOIDP      buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned char* tmpBuffer =
             (unsigned char*) calloc(length, sizeof(unsigned char));
    assert( tmpBuffer != 0);

    // get the array of short integers
    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride, length,
                    (VOIDP)tmpBuffer) != HDF_SUCCEED)
        return -1;

    // get the scale factor
    float64  scaleFactor;
    if (l1File->GetScaleFactor(sdsIDs[0], scaleFactor) == HDF_FAIL)
        return -1;

    // convert the short integers to floats, and return
    float* floatP = (float*)buffer;
    for (int i=0; i < length; i++, floatP++)
    {
        *floatP = (float) (scaleFactor * tmpBuffer[i]);
    }
    free((void*) tmpBuffer);
    return(length);

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned int* tempBuffer =
             (unsigned int*) calloc(length, sizeof(unsigned int));
    assert(tempBuffer != 0);

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
VOIDP       buffer,
PolynomialTable*)     // unused
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
VOIDP       buffer,
PolynomialTable*)     // unused
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
// Function:    ExtractData2D_8 ([][8])
// Extracts:    two dimensional data
//----------------------------------------------------------------------
int
ExtractData2D_8(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    int32 sdStart[2], sdStride[2], sdEdge[2];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 8;

    if (stride == 1 || stride == 0)
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  NULL, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);
    else
        return (l1File->GetDatasetDataMD(sdsIDs[0], sdStart,
                  sdStride, sdEdge, buffer) == HDF_SUCCEED ?  TRUE : FALSE);

}//ExtractData2D_8

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
VOIDP       buffer,
PolynomialTable*)     // unused
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
VOIDP       buffer,
PolynomialTable*)     // unused
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
VOIDP       buffer,
PolynomialTable*)     // unused
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
VOIDP       buffer,
PolynomialTable*)     // unused
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
VOIDP       buffer,
PolynomialTable*)     // unused
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
VOIDP       buffer,
PolynomialTable*)     // unused
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
VOIDP       buffer,
PolynomialTable*)     // unused
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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold unsigned short integers
    unsigned short* tempBuffer =
             (unsigned short*) calloc(length * 76, sizeof(unsigned short));
    assert(tempBuffer != 0);

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold short integers
    short* tempBuffer = (short*) calloc(length * 76, sizeof(short));
    assert(tempBuffer != 0);

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold unsigned short integers
    unsigned short* tempBuffer =
             (unsigned short*) calloc(length * 100, sizeof(unsigned short));
    assert(tempBuffer != 0);

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
// Function:    ExtractData2D_100_int2_float ([][100])
// Extracts:    two dimensional data
//----------------------------------------------------------------------
int
ExtractData2D_100_int2_float(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold signed short integers
    signed short* tempBuffer =
             (signed short*) calloc(length * 100, sizeof(signed short));
    assert(tempBuffer != 0);

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
    signed short* shortP = tempBuffer;
    for (int i=0; i < length; i++)
    {
        for (int j=0; j < 100; j++, floatP++, shortP++)
        {
            
            *floatP = (float) (scaleFactor * (*shortP));
        }
    }
    free((void*) tempBuffer);

    return TRUE;

}//ExtractData2D_100_int2_float

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold unsigned short integers
    unsigned short* tempBuffer =
             (unsigned short*) calloc(length * 810, sizeof(unsigned short));
    assert(tempBuffer != 0);

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold short integers
    short* tempBuffer = (short*) calloc(length * 810, sizeof(short));
    assert(tempBuffer != 0);

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold unsigned short integers
    unsigned short* tempBuffer =
             (unsigned short*) calloc(length * 304, sizeof(unsigned short));
    assert(tempBuffer != 0);

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold short integers
    short* tempBuffer = (short*) calloc(length * 304, sizeof(short));
    assert(tempBuffer != 0);

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
VOIDP       buffer,
PolynomialTable*)     // unused
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
// Function:    ExtractData3D_100_8_uint2_float ([][100][8])
// Extracts:    3 dimensional data
//----------------------------------------------------------------------
int
ExtractData3D_100_8_uint2_float(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold unsigned short integers
    unsigned short* tempBuffer =
             (unsigned short*) calloc(length * 800, sizeof(unsigned short));
    assert(tempBuffer != 0);

    int32 sdStart[3], sdStride[3], sdEdge[3];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStart[2] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdStride[2] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 100;
    sdEdge[2] = 8;

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
        for (int j=0; j < 800; j++, floatP++, ushortP++)
                *floatP = (float) (scaleFactor * (*ushortP));
    }
    free((void*) tempBuffer);

    return TRUE;

}//ExtractData3D_100_8_uint2_float

//----------------------------------------------------------------------
// Function:    ExtractData3D_100_8_int2_float ([][100][8])
// Extracts:    3 dimensional data
//----------------------------------------------------------------------
int
ExtractData3D_100_8_int2_float(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold short integers
    short* tempBuffer = (short*) calloc(length * 800, sizeof(short));
    assert(tempBuffer != 0);

    int32 sdStart[3], sdStride[3], sdEdge[3];
    sdStart[0] = start;
    sdStart[1] = 0;
    sdStart[2] = 0;
    sdStride[0] = stride;
    sdStride[1] = 1;
    sdStride[2] = 1;
    sdEdge[0] = length;
    sdEdge[1] = 100;
    sdEdge[2] = 8;

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
        for (int j=0; j < 800; j++, floatP++, shortP++)
            *floatP = (float) (scaleFactor * (*shortP));
    }
    free((void*) tempBuffer);

    return TRUE;

}//ExtractData3D_100_8_int2_float

//----------------------------------------------------------------------
// Function:    Extract1of8Bits ([])
// Extracts:    one dimensional data (1 of 8 bits)
//----------------------------------------------------------------------
int
Extract1of8Bits(
TlmHdfFile*     l1File,
int32*          sdsIDs,
int32           start,
int32           stride,
int32           length,
unsigned char   whichBit,   // which bit to extract
VOIDP           buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0 && whichBit < 8);
    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, buffer) != HDF_SUCCEED)
        return(-1);

    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(charP[i], whichBit);
    }
    return 1;

}//Extract1of8Bits

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of8Bits(l1File,sdsIDs,start,stride,length, 0, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of8Bits(l1File,sdsIDs,start,stride,length, 1, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of8Bits(l1File,sdsIDs,start,stride,length, 2, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of8Bits(l1File,sdsIDs,start,stride,length, 3, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of8Bits(l1File,sdsIDs,start,stride,length, 4, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of8Bits(l1File,sdsIDs,start,stride,length, 5, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of8Bits(l1File,sdsIDs,start,stride,length, 6, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of8Bits(l1File,sdsIDs,start,stride,length, 7, buffer, 0));

}//Extract8Bit7

//----------------------------------------------------------------------
// Function:    Extract1of16Bits ([])
// Extracts:    one dimensional data (1 out of 16 bits)
//----------------------------------------------------------------------
int
Extract1of16Bits(
TlmHdfFile*    l1File,
int32*         sdsIDs,
int32          start,
int32          stride,
int32          length,
unsigned char  whichBit,
VOIDP          buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0 && whichBit < 16);
    // alloc space to hold long integers
    unsigned short* tmpBuffer =
              (unsigned short*) calloc(length, sizeof(unsigned short));
    assert(tmpBuffer != 0);

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                              length, tmpBuffer) != HDF_SUCCEED)
        return(-1);

    // extract 1 bit only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(tmpBuffer[i], whichBit);
    }
    free((void*) tmpBuffer);
    return 1;

}//Extract1of16Bits

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of16Bits(l1File, sdsIDs, start, stride, length,
                           0, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of16Bits(l1File, sdsIDs, start, stride, length,
                           1, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of16Bits(l1File, sdsIDs, start, stride, length,
                          2, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of16Bits(l1File, sdsIDs, start, stride, length,
                         3, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of16Bits(l1File, sdsIDs, start, stride, length,
                        4, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of16Bits(l1File, sdsIDs, start, stride, length,
                             5, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of16Bits(l1File, sdsIDs, start, stride, length,
                             6, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of16Bits(l1File, sdsIDs, start, stride, length,
                               7, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of16Bits(l1File, sdsIDs, start, stride, length,
                             8, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of16Bits(l1File, sdsIDs, start, stride, length,
                        9, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of16Bits(l1File, sdsIDs, start, stride, length,
                               10, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of16Bits(l1File, sdsIDs, start, stride, length,
                               11, buffer, 0));

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
VOIDP      buffer,
PolynomialTable*)     // unused
{
    return(Extract1of16Bits(l1File, sdsIDs, start, stride, length,
                             12, buffer, 0));

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
VOIDP      buffer,
PolynomialTable*)     // unused
{
    return(Extract1of16Bits(l1File, sdsIDs, start, stride, length,
                                 13, buffer, 0));

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
VOIDP      buffer,
PolynomialTable*)     // unused
{
    return(Extract1of16Bits(l1File, sdsIDs, start, stride, length,
                                 14, buffer, 0));

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
VOIDP      buffer,
PolynomialTable*)     // unused
{
    return(Extract1of16Bits(l1File, sdsIDs, start, stride, length,
                                15, buffer, 0));

}//Extract16Bit15

//----------------------------------------------------------------------
// Function:    Extract16Bit0_1 ([])
// Extracts:    one dimensional data (Bit 0-1 only)
//----------------------------------------------------------------------
int
Extract16Bit0_1(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* tempBuffer =
              (unsigned short*) calloc(length, sizeof(unsigned short));
    assert(tempBuffer != 0);

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

}//Extract16Bit0_1

//----------------------------------------------------------------------
// Function:    Extract16Bit2_3 ([])
// Extracts:    one dimensional data (Bit 2-3 only)
//----------------------------------------------------------------------
int
Extract16Bit2_3(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* tempBuffer =
              (unsigned short*) calloc(length, sizeof(unsigned short));
    assert(tempBuffer != 0);

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                              length, tempBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 2_3 only and return the buffer
    (void)memset(buffer, 0, length);
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BITS(tempBuffer[i], 3, 2);
    }
    free((void*) tempBuffer);
    return TRUE;

}//Extract16Bit2_3

//----------------------------------------------------------------------
// Function:    Extract16Bit0_3 ([])
// Extracts:    one dimensional data (Bit 0-3 only)
//----------------------------------------------------------------------
int
Extract16Bit0_3(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* tempBuffer =
              (unsigned short*) calloc(length, sizeof(unsigned short));
    assert(tempBuffer != 0);

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                              length, tempBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 0_3 only and return the buffer
    (void)memset(buffer, 0, length);
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BITS(tempBuffer[i], 3, 4);
    }
    free((void*) tempBuffer);
    return TRUE;

}//Extract16Bit0_3

//----------------------------------------------------------------------
// Function:    Extract16Bit0_13 ([])
// Extracts:    one dimensional data (Bit 0-13 only)
//----------------------------------------------------------------------
int
Extract16Bit0_13(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned short* tempBuffer =
              (unsigned short*) calloc(length, sizeof(unsigned short));
    assert(tempBuffer != 0);

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                              length, tempBuffer) != HDF_SUCCEED)
        return FALSE;

    // extract bit 0_14 only and return the buffer
    (void)memset(buffer, 0, length);
    unsigned short* shortP = (unsigned short*)buffer;
    for (int i=0; i < length; i++)
    {
        shortP[i] = EXTRACT_GET_BITS(tempBuffer[i], 13, 14);
    }
    free((void*) tempBuffer);
    return TRUE;

}//Extract16Bit0_13

//----------------------------------------------------------------------
// Function:    ExtractDeltaSrcSeqCnt ([])
// Extracts:    one dimensional data
//----------------------------------------------------------------------
int
ExtractDeltaSrcSeqCnt(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      ,
int32      length,
VOIDP      buffer,
PolynomialTable*)     // unused
{
    static unsigned short lastSrcSequenceCnt=0;
    if (length != 1) return 0;
    unsigned short newSrcSequenceCnt;
    if (Extract16Bit0_13(l1File, sdsIDs, start, 1, 1, &newSrcSequenceCnt) == 0)
        return 0;
    
    short* delta = (short*) buffer;
    *delta = newSrcSequenceCnt - lastSrcSequenceCnt;
    lastSrcSequenceCnt = newSrcSequenceCnt;

    return 1;

} // ExtractDeltaSrcSeqCnt

//----------------------------------------------------------------------
// Function:    ExtractSomeOf8Bits ([])
// Extracts:    one dimensional data (Bit 0-1 only)
//----------------------------------------------------------------------
int
ExtractSomeOf8Bits(
TlmHdfFile*     l1File,
int32*          sdsIDs,
int32           start,
int32           stride,
int32           length,
unsigned char   leftMostBit,
unsigned char   numBits,
VOIDP           buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned char* tempBuffer =
              (unsigned char*) calloc(length, sizeof(unsigned char));
    assert(tempBuffer != 0);

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                   length, tempBuffer) != HDF_SUCCEED)
        return(-1);

    // extract bits (leftMostBit + numBits) only and return the buffer
    (void)memset(buffer, 0, length);
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BITS(tempBuffer[i], leftMostBit, numBits);
    }
    free((void*) tempBuffer);
    return 1;

}//ExtractSomeOf8Bits

//----------------------------------------------------------------------
// Function:    Extract8Bit0_1 ([])
// Extracts:    one dimensional data (Bit 0-1 only)
//----------------------------------------------------------------------
int
Extract8Bit0_1(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer,
PolynomialTable*)     // unused
{
    return(ExtractSomeOf8Bits(l1File,sdsIDs,start,stride,length,1,
                                 2,buffer, 0));

}//Extract8Bit0_1

//----------------------------------------------------------------------
// Function:    Extract8Bit0_3 ([])
// Extracts:    one dimensional data (Bit 0-3 only)
//----------------------------------------------------------------------
int
Extract8Bit0_3(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer,
PolynomialTable*)     // unused
{
    return(ExtractSomeOf8Bits(l1File,sdsIDs,start,stride,length,3,
                                 4,buffer, 0));

}//Extract8Bit0_3

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
VOIDP      buffer,
PolynomialTable*)     // unused
{
    return(ExtractSomeOf8Bits(l1File,sdsIDs,start,stride,length,4,
                               5,buffer, 0));

}//Extract8Bit0_4

//----------------------------------------------------------------------
// Function:    Extract8Bit0_6 ([])
// Extracts:    one dimensional data (Bit 0-6 only)
//----------------------------------------------------------------------
int
Extract8Bit0_6(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer,
PolynomialTable*)     // unused
{
    return(ExtractSomeOf8Bits(l1File,sdsIDs,start,stride,length,6,
                               7,buffer, 0));

}//Extract8Bit0_6

//----------------------------------------------------------------------
// Function:    Extract8Bit1_2 ([])
// Extracts:    one dimensional data (Bit 1-2 only)
//----------------------------------------------------------------------
int
Extract8Bit1_2(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer,
PolynomialTable*)     // unused
{
    return(ExtractSomeOf8Bits(l1File,sdsIDs,start,stride,length,2,
                                  2,buffer, 0));

}//Extract8Bit1_2

//----------------------------------------------------------------------
// Function:    Extract8Bit2_3 ([])
// Extracts:    one dimensional data (Bit 2-3 only)
//----------------------------------------------------------------------
int
Extract8Bit2_3(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer,
PolynomialTable*)     // unused
{
    return(ExtractSomeOf8Bits(l1File,sdsIDs,start,stride,length,3,
                                 2,buffer, 0));

}//Extract8Bit2_3

//----------------------------------------------------------------------
// Function:    Extract8Bit3_7 ([])
// Extracts:    one dimensional data (Bit 4-5 only)
//----------------------------------------------------------------------
int
Extract8Bit3_7(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer,
PolynomialTable*)     // unused
{
    return(ExtractSomeOf8Bits(l1File,sdsIDs,start,stride,length,7,
                                   5,buffer, 0));

}//Extract8Bit3_7

//----------------------------------------------------------------------
// Function:    Extract8Bit4_5 ([])
// Extracts:    one dimensional data (Bit 4-5 only)
//----------------------------------------------------------------------
int
Extract8Bit4_5(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer,
PolynomialTable*)     // unused
{
    return(ExtractSomeOf8Bits(l1File,sdsIDs,start,stride,length,5,
                                   2,buffer, 0));

}//Extract8Bit4_5

//----------------------------------------------------------------------
// Function:    Extract8Bit4_6 ([])
// Extracts:    one dimensional data (Bit 4-6 only)
//----------------------------------------------------------------------
int
Extract8Bit4_6(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer,
PolynomialTable*)     // unused
{
    return(ExtractSomeOf8Bits(l1File,sdsIDs,start,stride,length,6,
                                   3,buffer, 0));

}//Extract8Bit4_6

//----------------------------------------------------------------------
// Function:    Extract8Bit5_6 ([])
// Extracts:    one dimensional data (Bit 5-6 only)
//----------------------------------------------------------------------
int
Extract8Bit5_6(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer,
PolynomialTable*)     // unused
{
    return(ExtractSomeOf8Bits(l1File,sdsIDs,start,stride,length,6,
                                  2,buffer, 0));

}//Extract8Bit5_6

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
VOIDP      buffer,
PolynomialTable*)     // unused
{
    return(ExtractSomeOf8Bits(l1File,sdsIDs,start,stride,length,7,3,buffer,0));

}//Extract8Bit5_7

//----------------------------------------------------------------------
// Function:    Extract8Bit6_7 ([])
// Extracts:    one dimensional data (Bit 6-7 only)
//----------------------------------------------------------------------
int
Extract8Bit6_7(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer,
PolynomialTable*)     // unused
{
    return(ExtractSomeOf8Bits(l1File,sdsIDs,start,stride,length,
                           7, 2,buffer, 0));

}//Extract8Bit6_7

//----------------------------------------------------------------------
// Function:    Extract1of32Bits ([])
// Extracts:    one dimensional data (1 out of 32 bits)
//----------------------------------------------------------------------
int
Extract1of32Bits(
TlmHdfFile*    l1File,
int32*         sdsIDs,
int32          start,
int32          stride,
int32          length,
unsigned char  whichBit,
VOIDP          buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0 && whichBit < 32);
    // alloc space to hold long integers
    unsigned int* tmpBuffer =
              (unsigned int*) calloc(length, sizeof(unsigned int));
    assert(tmpBuffer != 0);

    if (l1File->GetDatasetData1D(sdsIDs[0], start, stride,
                              length, tmpBuffer) != HDF_SUCCEED)
        return(-1);

    // extract 1 bit only and return the buffer
    unsigned char* charP = (unsigned char*)buffer;
    for (int i=0; i < length; i++)
    {
        charP[i] = EXTRACT_GET_BIT(tmpBuffer[i], whichBit);
    }
    free((void*) tmpBuffer);
    return 1;

}//Extract1of32Bits

//----------------------------------------------------------------------
// Function:    Extract32Bit0 ([])
// Extracts:    one dimensional data (Bit 0 only)
//----------------------------------------------------------------------
int
Extract32Bit0(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                 0, buffer, 0));

}//Extract32Bit0

//----------------------------------------------------------------------
// Function:    Extract32Bit1 ([])
// Extracts:    one dimensional data (Bit 1 only)
//----------------------------------------------------------------------
int
Extract32Bit1(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                     1, buffer, 0));

}//Extract32Bit1

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                 2, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                3, buffer, 0));

}//Extract32Bit3

//----------------------------------------------------------------------
// Function:    Extract32Bit4 ([])
// Extracts:    one dimensional data (Bit 4 only)
//----------------------------------------------------------------------
int
Extract32Bit4(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                 4, buffer, 0));
 
}//Extract32Bit4

//----------------------------------------------------------------------
// Function:    Extract32Bit5 ([])
// Extracts:    one dimensional data (Bit 5 only)
//----------------------------------------------------------------------
int
Extract32Bit5(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                  5, buffer, 0));
 
}//Extract32Bit5

//----------------------------------------------------------------------
// Function:    Extract32Bit6 ([])
// Extracts:    one dimensional data (Bit 6 only)
//----------------------------------------------------------------------
int
Extract32Bit6(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                   6, buffer, 0));
 
}//Extract32Bit6

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                 7, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                  8, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                       9, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                      10, buffer, 0));

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                        11, buffer, 0));

}//Extract32Bit11

//----------------------------------------------------------------------
// Function:    Extract32Bit12 ([])
// Extracts:    one dimensional data (Bit 12 only)
//----------------------------------------------------------------------
int
Extract32Bit12(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                           12, buffer, 0));

}//Extract32Bit12

//----------------------------------------------------------------------
// Function:    Extract32Bit13 ([])
// Extracts:    one dimensional data (Bit 13 only)
//----------------------------------------------------------------------
int
Extract32Bit13(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                13, buffer, 0));
 
}//Extract32Bit13

//----------------------------------------------------------------------
// Function:    Extract32Bit14 ([])
// Extracts:    one dimensional data (Bit 14 only)
//----------------------------------------------------------------------
int
Extract32Bit14(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                               14, buffer, 0));
 
}//Extract32Bit14

//----------------------------------------------------------------------
// Function:    Extract32Bit15 ([])
// Extracts:    one dimensional data (Bit 15 only)
//----------------------------------------------------------------------
int
Extract32Bit15(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                 15, buffer, 0));
 
}//Extract32Bit15

//----------------------------------------------------------------------
// Function:    Extract32Bit16 ([])
// Extracts:    one dimensional data (Bit 16 only)
//----------------------------------------------------------------------
int
Extract32Bit16(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                 16, buffer, 0));
 
}//Extract32Bit16

//----------------------------------------------------------------------
// Function:    Extract32Bit17 ([])
// Extracts:    one dimensional data (Bit 17 only)
//----------------------------------------------------------------------
int
Extract32Bit17(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                 17, buffer, 0));
 
}//Extract32Bit17

//----------------------------------------------------------------------
// Function:    Extract32Bit18 ([])
// Extracts:    one dimensional data (Bit 18 only)
//----------------------------------------------------------------------
int
Extract32Bit18(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                  18, buffer, 0));
 
}//Extract32Bit18

//----------------------------------------------------------------------
// Function:    Extract32Bit19 ([])
// Extracts:    one dimensional data (Bit 19 only)
//----------------------------------------------------------------------
int
Extract32Bit19(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                  19, buffer, 0));
 
}//Extract32Bit19

//----------------------------------------------------------------------
// Function:    Extract32Bit20 ([])
// Extracts:    one dimensional data (Bit 20 only)
//----------------------------------------------------------------------
int
Extract32Bit20(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                 20, buffer, 0));
 
}//Extract32Bit20

//----------------------------------------------------------------------
// Function:    Extract32Bit21 ([])
// Extracts:    one dimensional data (Bit 21 only)
//----------------------------------------------------------------------
int
Extract32Bit21(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                    21, buffer, 0));
 
}//Extract32Bit21

//----------------------------------------------------------------------
// Function:    Extract32Bit22 ([])
// Extracts:    one dimensional data (Bit 22 only)
//----------------------------------------------------------------------
int
Extract32Bit22(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                              22, buffer, 0));
 
}//Extract32Bit22

//----------------------------------------------------------------------
// Function:    Extract32Bit23 ([])
// Extracts:    one dimensional data (Bit 23 only)
//----------------------------------------------------------------------
int
Extract32Bit23(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                 23, buffer, 0));
 
}//Extract32Bit23

//----------------------------------------------------------------------
// Function:    Extract32Bit24 ([])
// Extracts:    one dimensional data (Bit 24 only)
//----------------------------------------------------------------------
int
Extract32Bit24(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                24, buffer, 0));
 
}//Extract32Bit24

//----------------------------------------------------------------------
// Function:    Extract32Bit25 ([])
// Extracts:    one dimensional data (Bit 25 only)
//----------------------------------------------------------------------
int
Extract32Bit25(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                  25, buffer, 0));
 
}//Extract32Bit25

//----------------------------------------------------------------------
// Function:    Extract32Bit26 ([])
// Extracts:    one dimensional data (Bit 26 only)
//----------------------------------------------------------------------
int
Extract32Bit26(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                                26, buffer, 0));
 
}//Extract32Bit26

//----------------------------------------------------------------------
// Function:    Extract32Bit27 ([])
// Extracts:    one dimensional data (Bit 27 only)
//----------------------------------------------------------------------
int
Extract32Bit27(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                             27, buffer, 0));
 
}//Extract32Bit27

//----------------------------------------------------------------------
// Function:    Extract32Bit28 ([])
// Extracts:    one dimensional data (Bit 28 only)
//----------------------------------------------------------------------
int
Extract32Bit28(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    return(Extract1of32Bits(l1File, sdsIDs, start, stride, length,
                             28, buffer, 0));
 
}//Extract32Bit28

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
VOIDP      buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned int* tempBuffer =
              (unsigned int*) calloc(length, sizeof(unsigned int));
    assert(tempBuffer != 0);

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
VOIDP      buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold short integers
    unsigned int* tempBuffer =
              (unsigned int*) calloc(length, sizeof(unsigned int));
    assert(tempBuffer != 0);

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
// Function:    ExtractTaiTime ([])
// Extracts:    Coda A TAI time string
//----------------------------------------------------------------------
int
ExtractTaiTime(
TlmHdfFile* l1File,
int32*     sdsIDs,
int32      start,
int32      stride,
int32      length,
VOIDP      buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);

    // alloc space to hold short integers
    double* tempBuffer = (double*) calloc(length, sizeof(double));
    assert(tempBuffer != 0);

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
// Function:    ExtractL1Time ([])
// Extracts:    Coda A time string
//----------------------------------------------------------------------
int
ExtractL1Time(
TlmHdfFile*,
int32*     vdIDs,
int32      start,
int32,
int32      length,
VOIDP      buffer,
PolynomialTable*)     // unused
{
    // alloc space to hold short integers
    char* tempBuffer =(char*) calloc(L1_TIME_LEN * length, sizeof(char));
    assert(tempBuffer != 0);

    if (VSseek(vdIDs[0], start) == FAIL)
        return FALSE;

    // get the L1 time strings
    if (VSread(vdIDs[0], (unsigned char*)tempBuffer,
                       length, FULL_INTERLACE) == FAIL)
        return FALSE;

    //----------------------------------------------------
    // convert the L1 time string to Itime, add to the base,
    // then convert to Char6 and return
    //----------------------------------------------------
    char* ptr = (char*) buffer;
    char* l1TimeString = tempBuffer;
    for (int i=0; i < length; i++)
    {
        Itime itime;
        if ( ! itime.L1ToItime(l1TimeString))
            return FALSE;
        l1TimeString += L1_TIME_LEN;
        memcpy(ptr,     (char *)&(itime.sec), sizeof(int));
        memcpy(ptr + 4, (char *)&(itime.ms), sizeof(short));
        ptr += 6;
    }
    free((void*) tempBuffer);

    return TRUE;

}//ExtractL1Time

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
VOIDP       buffer,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // alloc space to hold floats
    float* tempBuffer =
             (float*) calloc(length, sizeof(float));
    assert(tempBuffer != 0);

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

//----------------------------------------------------------------------
// Function:    ExtractEuTodB
// Extracts:    uint1, apply polynomial to DN to EU, then do 10 * log10
//----------------------------------------------------------------------
int
Extract_uint1_eu_dB(
TlmHdfFile*      l1File,
int32*           sdsIDs,
int32            start,
int32            stride,
int32            length,
VOIDP            buffer,
const char*      sdsName,
const char*      euUnitName,
PolynomialTable* polyTable)
{
    assert(l1File != 0 && length > 0);
    if (polyTable == 0) return -1;

    // alloc space to hold floats
    float* tempBuffer = (float*) calloc(length, sizeof(float));
    assert(tempBuffer != 0);

    int rc = ExtractData1D_uint1_float(l1File, sdsIDs, start, stride,
                          length, tempBuffer);
    if (rc <= 0)
        return rc;

    const Polynomial* polynomial = polyTable->SelectPolynomial(
                                       sdsName, euUnitName);
    if (polynomial == 0) return -1;
    polynomial->ApplyReplaceArray(tempBuffer, length);

    // now convert it to dB
    float* floatP = (float*)buffer;
    for (int i=0; i < length; i++)
    {
        float dBValue = (float) 10 * log10((double)tempBuffer[i]);
        (void)memcpy(floatP, &dBValue, sizeof(float));
        floatP++;
    }

    free((void*) tempBuffer);
    return(length);

} // Extract_uint1_eu_dB
#if 0
//----------------------------------------------------------------------
// Function:    ExtractXmitPowerAdBm
// Extracts:    float
//----------------------------------------------------------------------
int
ExtractXmitPowerAdBm(
TlmHdfFile*      l1File,
int32*           sdsIDs,
int32            start,
int32            stride,
int32            length,
VOIDP            buffer,
PolynomialTable* polyTable)
{
    return(Extract_uint1_eu_dB(l1File, sdsIDs, start, stride, length,
                          buffer, "transmit_power_a", "mWatts",
                          polyTable));
} // ExtractXmitPowerAdBm

//----------------------------------------------------------------------
// Function:    ExtractXmitPowerBdBm
// Extracts:    float
//----------------------------------------------------------------------
int
ExtractXmitPowerBdBm(
TlmHdfFile* l1File,
int32*           sdsIDs,
int32            start,
int32            stride,
int32            length,
VOIDP            buffer,
PolynomialTable* polyTable)
{
    return(Extract_uint1_eu_dB(l1File, sdsIDs, start, stride, length,
                          buffer, "transmit_power_b", "mWatts",
                          polyTable));
} // ExtractXmitPowerBdBm

//----------------------------------------------------------------------
// Function:    ExtractXmitPowerAmWatts
// Extracts:    float
//----------------------------------------------------------------------
int
ExtractXmitPowerAmWatts(
TlmHdfFile*      l1File,
int32*           sdsIDs,
int32            start,
int32            stride,
int32            length,
VOIDP            buffer,
PolynomialTable* polyTable)
{
    return(Extract_uint1_eu_mWatts(l1File, sdsIDs, start, stride, length,
                          buffer, "transmit_power_a", "dBm",
                          polyTable));
} // ExtractXmitPowerAmWatts

//----------------------------------------------------------------------
// Function:    ExtractXmitPowerBmWatts
// Extracts:    float
//----------------------------------------------------------------------
int
ExtractXmitPowerBmWatts(
TlmHdfFile* l1File,
int32*           sdsIDs,
int32            start,
int32            stride,
int32            length,
VOIDP            buffer,
PolynomialTable* polyTable)
{
    return(Extract_uint1_eu_mWatts(l1File, sdsIDs, start, stride, length,
                          buffer, "transmit_power_b", "dBm",
                          polyTable));
} // ExtractXmitPowerBmWatts

//----------------------------------------------------------------------
// Function:    Extract_uint1_eu_mWatts
// Extracts:    uint1, apply polynomial to DN to EU, then do pow(10,dBm/10)
//----------------------------------------------------------------------
int
Extract_uint1_eu_mWatts(
TlmHdfFile*      l1File,
int32*           sdsIDs,
int32            start,
int32            stride,
int32            length,
VOIDP            buffer,
const char*      sdsName,
const char*      euUnitName,
PolynomialTable* polyTable)
{
    assert(l1File != 0 && length > 0);
    if (polyTable == 0) return -1;

    // alloc space to hold floats
    float* tempBuffer = (float*) calloc(length, sizeof(float));
    assert(tempBuffer != 0);

    int rc = ExtractData1D_uint1_float(l1File, sdsIDs, start, stride,
                          length, tempBuffer);
    if (rc <= 0)
        return rc;

    const Polynomial* polynomial = polyTable->SelectPolynomial(
                                       sdsName, euUnitName);
    if (polynomial == 0) return -1;
    polynomial->ApplyReplaceArray(tempBuffer, length);

    // now convert it to mWatts
    float* floatP = (float*)buffer;
    for (int i=0; i < length; i++)
    {
        float mWattsValue = (float) pow( (double) 10.0,
                                      (double) tempBuffer[i] / 10.0 );
        (void)memcpy(floatP, &mWattsValue, sizeof(float));
        floatP++;
    }

    free((void*) tempBuffer);
    return(length);

} // Extract_uint1_eu_mWatts
#endif

//----------------------------------------------------------------------
// Function:    ExtractData1D_int2_float_dtr
// Extracts:    float: degrees => radians
//----------------------------------------------------------------------
int
ExtractData1D_int2_float_dtr(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    if (ExtractData1D_int2_float(l1File, sdsIDs, start, stride,
              length, buffer) == 0)
        return 0;
    // convert degrees to radians, and return
    float* floatP = (float*)buffer;
    for (int i=0; i < length; i++, floatP++)
        *floatP *= EA_CONST_DEGREES_TO_RADIANS;

    return(TRUE);

} //ExtractData1D_int2_float_dtr

//----------------------------------------------------------------------
// Function:    ExtractData1D_float_dtr
// Extracts:    float: degrees => radians
//----------------------------------------------------------------------
int
ExtractData1D_float_dtr(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    if (ExtractData1D(l1File, sdsIDs, start, stride,
              length, buffer) == 0)
        return 0;

    // convert degrees to radians, and return
    float* floatP = (float*)buffer;
    for (int i=0; i < length; i++, floatP++)
    {
        *floatP *= EA_CONST_DEGREES_TO_RADIANS;
    }
    return(TRUE);

} //ExtractData1D_float_dtr

//----------------------------------------------------------------------
// Function:    ExtractData2D_100_float_dtr ([][100])
// Extracts:    float[100]: degrees => radians
//----------------------------------------------------------------------
int
ExtractData2D_100_float_dtr(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    if (ExtractData2D_100(l1File, sdsIDs, start, stride,
              length, buffer) == 0)
        return 0;

    float* floatP = (float*)buffer;
    for (int j=0; j < length; j++)
         for (int i=0; i < 100; i++, floatP++)
             *floatP *= EA_CONST_DEGREES_TO_RADIANS;

    return TRUE;

}//ExtractData2D_100_float_dtr

//----------------------------------------------------------------------
// Function:    ExtractData2D_100_uint2_float_dtr ([][100])
// Extracts:    float[100]: degrees => radians
//----------------------------------------------------------------------
int
ExtractData2D_100_uint2_float_dtr(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    if (ExtractData2D_100_uint2_float(l1File, sdsIDs, start, stride,
              length, buffer) == 0)
        return 0;

    float* floatP = (float*)buffer;
    for (int j=0; j < length; j++)
         for (int i=0; i < 100; i++, floatP++)
            *floatP *= EA_CONST_DEGREES_TO_RADIANS;
    return TRUE;

}//ExtractData2D_100_uint2_float_dtr

//----------------------------------------------------------------------
// Function:    ExtractData2D_100_int2_float_dtr ([][100])
// Extracts:    float[100]: degrees => radians
//----------------------------------------------------------------------
int
ExtractData2D_100_int2_float_dtr(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    if (ExtractData2D_100_int2_float(l1File, sdsIDs, start, stride,
              length, buffer) == 0)
        return 0;

    float* floatP = (float*)buffer;
    for (int j=0; j < length; j++)
         for (int i=0; i < 100; i++, floatP++)
        *floatP *= EA_CONST_DEGREES_TO_RADIANS;

    return TRUE;

}//ExtractData2D_100_int2_float_dtr

//----------------------------------------------------------------------
// Function:    ExtractData3D_100_8_int2_float_dtr ([][100][8])
// Extracts:    3 dimensional data
//----------------------------------------------------------------------
int
ExtractData3D_100_8_int2_float_dtr(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    if (ExtractData3D_100_8_int2_float(l1File, sdsIDs, start, stride,
              length, buffer) == 0)
        return 0;

    // convert the degree to radians
    float* floatP = (float*)buffer;
    for (int i=0; i < length; i++)
    {
        for (int j=0; j < 800; j++, floatP++)
            *floatP *= EA_CONST_DEGREES_TO_RADIANS;
    }

    return TRUE;

}//ExtractData3D_100_8_int2_float_dtr

//----------------------------------------------------------------------
// Function:    ExtractData3D_100_8_uint2_float_dtr ([][100][8])
// Extracts:    3 dimensional data
//----------------------------------------------------------------------
int
ExtractData3D_100_8_uint2_float_dtr(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    if (ExtractData3D_100_8_uint2_float(l1File, sdsIDs, start, stride,
              length, buffer) == 0)
        return 0;

    // convert the degree to radians
    float* floatP = (float*)buffer;
    for (int i=0; i < length; i++)
    {
        for (int j=0; j < 800; j++, floatP++)
            *floatP *= EA_CONST_DEGREES_TO_RADIANS;
    }

    return TRUE;

}//ExtractData3D_100_8_uint2_float_dtr

//----------------------------------------------------------------------
// Function:    ExtractDeltaInstTime
//              get the Delta of instrument time, this is to check if
//              any skipping in the instrument data
// Extracts:    double[]
//----------------------------------------------------------------------
int
ExtractDeltaInstTime(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    static double lastInstTime=0.0;
    if (length != 1) return 0;
    double newInstTime;

    if (ExtractData1D(l1File, sdsIDs, start, 1, 1, &newInstTime) == 0)
        return 0;

    double* delta = (double*) buffer;
    *delta = newInstTime - lastInstTime;
    lastInstTime = newInstTime;

    return TRUE;

}//ExtractDeltaInstTime
