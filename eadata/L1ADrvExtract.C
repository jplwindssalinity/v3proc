//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.25   09 Jun 1999 16:35:32   sally
// need to allocate space for dataBuf for local DerivedExtractResult
// otherwise, "bad address alignment"
// 
//    Rev 1.24   08 Jun 1999 16:25:26   sally
// make mWatts = 0.0, dB = -1000 when dn = 0.0
// 
//    Rev 1.23   27 Apr 1999 12:56:50   sally
// if true_cal_pulse_pos is even => Beam A, else Beam B
// 
//    Rev 1.22   23 Apr 1999 15:51:24   sally
// typo on load_cal_B_power, and fix the array size for tempsdsIDs
// 
//    Rev 1.21   05 Apr 1999 13:46:28   sally
// fix for receiver gain
// 
//    Rev 1.20   23 Dec 1998 16:30:34   sally
// move "Orbit Period" and "Antenna Spin Rate" from derived L1A to L1A,
// because it returns one single value only, not 100 pulses of values.
// 
//    Rev 1.19   08 Dec 1998 14:11:12   sally
// ROM is same as WOM
// 
//    Rev 1.18   08 Dec 1998 11:50:46   sally
// initialize the total with the first slice values first
// 
//    Rev 1.17   07 Dec 1998 15:40:56   sally
// move EA_DN_TO_DB() to CommonDefs.h
// 
//    Rev 1.16   04 Dec 1998 16:33:26   sally
// ExtractDataxxxx() functions return number of data or -1 not TRUE or FALSE.
// 
//    Rev 1.15   20 Nov 1998 16:02:58   sally
// change some data types and limit check arrays
// 
//    Rev 1.14   09 Nov 1998 11:24:36   sally
// 
//    Rev 1.13   11 Sep 1998 10:28:54   sally
// add mWatts for all dBm units
// 
//    Rev 1.12   18 Aug 1998 15:06:02   sally
// mv mWatts for transmit power to L1ADrvTab.C
// 
//    Rev 1.11   18 Aug 1998 10:56:36   sally
// make L1ADrvExtract return any number of values
// 
//    Rev 1.10   28 Jul 1998 10:48:34   sally
// added pulse pattern for CBM
// 
//    Rev 1.9   27 Jul 1998 13:59:30   sally
// passing polynomial table to extraction function
// 
//    Rev 1.8   23 Jul 1998 16:13:46   sally
// pass polynomial table to extractFunc()
// 
//    Rev 1.7   22 Jun 1998 09:25:00   sally
// took out some compile errors and warnings for GNU GCC
// 
//    Rev 1.6   19 Jun 1998 16:53:46   sally
// added "Orbit Period" in L1A Derived Data 
// 
//    Rev 1.5   15 Jun 1998 11:28:50   sally
// ExtractTotalLoadPowerdB() should call ExtractTotalLoadPowerDN()
// instead of ExtractBeamAPowerDN()
// 
//    Rev 1.4   06 May 1998 15:17:08   sally
// took out exit()
// 
//    Rev 1.3   06 Apr 1998 16:27:12   sally
// merged with SVT
// 
//    Rev 1.2   30 Mar 1998 15:13:32   sally
// added L2A parameter table
// 
//    Rev 1.1   27 Mar 1998 09:58:44   sally
// added L1A Derived data
// 
//    Rev 1.0   24 Mar 1998 16:02:28   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#include <assert.h>
#include <time.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>

#include "L1AExtract.h"
#include "L1ADrvExtract.h"
#include "Parameter.h"
#include "Itime.h"
#include "PolyTable.h"
#include "Polynomial.h"

static const char rcs_id_L1ADrvExtract_C[] = "@(#) $Header$";

int32*  _rgACurrentIndex=0;
int*    _rgALastDataIndex=0;
float*  _rgABuf=0;
int32*  _rgBCurrentIndex=0;
int*    _rgBLastDataIndex=0;
float*  _rgBBuf=0;

int32   _rgADNCurrentIndex=0;
int     _rgADNLastDataIndex=0;
float   _rgADNBuf[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int32   _rgADBCurrentIndex=0;
int     _rgADBLastDataIndex=0;
float   _rgADBBuf[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int32   _rgBDNCurrentIndex=0;
int     _rgBDNLastDataIndex=0;
float   _rgBDNBuf[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int32   _rgBDBCurrentIndex=0;
int     _rgBDBLastDataIndex=0;
float   _rgBDBBuf[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

unsigned char _onlyOneMap[] =
{
// 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
 
   1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

unsigned char _beamAMap[] =
{
// 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
 
   0,  1,  0,  0,  0,  1,  0,  0,  0,  1,
   0,  0,  0,  1,  0,  0,  0,  1,  0,  0,
   0,  1,  0,  0,  0,  1,  0,  0,  0,  1,
   0,  0,  0,  1,  0,  0,  0,  1,  0,  0,
   0,  1,  0,  0,  0,  1,  0,  0,  0,  1,
   0,  0,  0,  1,  0,  0,  0,  1,  0,  0,
   0,  1,  0,  0,  0,  1,  0,  0,  0,  1,
   0,  0,  0,  1,  0,  0,  0,  1,  0,  0,
   0,  1,  0,  0,  0,  1,  0,  0,  0,  0,
   0,  0,  0,  1,  0,  0,  0,  0,  0,  0
};

unsigned char _beamBMap[] =
{
// 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,

   0,  0,  1,  0,  0,  0,  1,  0,  0,  0,
   1,  0,  0,  0,  1,  0,  0,  0,  1,  0,
   0,  0,  1,  0,  0,  0,  1,  0,  0,  0,
   1,  0,  0,  0,  1,  0,  0,  0,  1,  0,
   0,  0,  1,  0,  0,  0,  1,  0,  0,  0,
   1,  0,  0,  0,  1,  0,  0,  0,  1,  0,
   0,  0,  1,  0,  0,  0,  1,  0,  0,  0,
   1,  0,  0,  0,  1,  0,  0,  0,  1,  0,
   0,  0,  1,  0,  0,  0,  1,  0,  0,  0,
   0,  0,  0,  0,  1,  0,  0,  0,  0,  0
};

unsigned char _noiseLoadMap[] =
{
// 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  1,  1,  0,  0,  1,  1,  0,  0,  1,
   1,  0,  0,  1,  1,  0,  0,  1,  1,  0,
   0,  1,  1,  0,  0,  1,  1,  0,  0,  1,
   1,  0,  0,  1,  1,  0,  0,  1,  1,  0,
   0,  1,  1,  0,  0,  1,  1,  1,  1,  0
};

#define EA_MIN_BANDWIDTH_FRAMES    400

inline int
_oneUint2TodB(
unsigned short*         uint2P,
DerivedExtractResult*   extractResults)
{
    float* floatP = (float*)extractResults->dataBuf;
    *floatP = (float) EA_DN_TO_DB(*uint2P);
    return 1;
} // _oneUint2TodB

#if 0
static int
_allUint2TodB(
unsigned short*         uint2P,
DerivedExtractResult*   extractResults)
{
    unsigned short*  tempUint2P;
    float*           floatP;
    int              numValidValues = 0;

    for (int i=0; i < MAX_NUM_DERIVED_VALUES; i++)
    {
        if (extractResults->validDataMap[i])
        {
            tempUint2P = uint2P + i;
            floatP = (float*)(extractResults->dataBuf + i * sizeof(float));
            *floatP = (float)EA_DN_TO_DB(*tempUint2P);
            numValidValues++;
        }
    }
    return (numValidValues);
} // _allUint2TodB
#endif

inline int
_oneUint4TodB(
DerivedExtractResult*   extractResults)
{
    float* floatP = (float*)extractResults->dataBuf;
    *floatP = (float)EA_DN_TO_DB(*((unsigned int*)extractResults->dataBuf));
    return 1;
} // _oneUint4TodB

static int
_allUint4TodB(
DerivedExtractResult*   extractResults)
{
    unsigned int*   uintP;
    float*          floatP;
    int             numValidValues = 0;

    for (int i=0; i < MAX_NUM_DERIVED_VALUES; i++)
    {
        if (extractResults->validDataMap[i])
        {
            uintP = (unsigned int*)(extractResults->dataBuf +
                                       i * sizeof(unsigned int));
            floatP = (float*)(extractResults->dataBuf + i * sizeof(float));
            *floatP = (float)EA_DN_TO_DB(*uintP);
            numValidValues++;
        }
    }
    return (numValidValues);
} // _allUint4TodB

inline int
_oneFloat4TodB(
DerivedExtractResult*   extractResults)
{
    float* floatP = (float*)extractResults->dataBuf;
    *floatP = (float) EA_DN_TO_DB(*((float*)extractResults->dataBuf));
    return 1;
} // _oneFloat4TodB

static int
_allFloat4TodB(
DerivedExtractResult*   extractResults)
{
    float*          floatP;
    int             numValidValues = 0;
 
    for (int i=0; i < MAX_NUM_DERIVED_VALUES; i++)
    {
        if (extractResults->validDataMap[i])
        {
            floatP = (float*)extractResults->dataBuf + i;
            *floatP = (float)EA_DN_TO_DB(*floatP);
            numValidValues++;
        }
 
    }
    return (numValidValues);
} // _allFloat4TodB


//----------------------------------------------------------------------
// Function:    ExtractBeamANoiseDN
// Extracts:    UINT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamANoiseDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0 && p_extractResults != 0);

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 1, 5, 9, ....
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from noise_dn
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        int rc = ExtractData2D_100(l1File, tempsdsIDs, start,
                          1, 1, (VOIDP)(extractResults->dataBuf));
        if (rc <= 0) return(rc);

        (void)memcpy(extractResults->validDataMap,
                           _beamAMap, MAX_NUM_DERIVED_VALUES);
        return (23);
    }
    else if (mode == L1_MODE_WOM || mode == L1_MODE_ROM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be even number (Beam A)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and even
        if (status <= 0 || status % 2 != 0)
        {
            (void)memset(extractResults->validDataMap, 0,
                                             MAX_NUM_DERIVED_VALUES);
            return 0;
        }

        // get it from loop_back_cal_noise
        if (l1File->GetDatasetData1D(sdsIDs[3], start, 1, 1,
                        (VOIDP)(extractResults->dataBuf)) != HDF_SUCCEED)
            return (-1);
   
        (void)memcpy(extractResults->validDataMap, _onlyOneMap,
                                             MAX_NUM_DERIVED_VALUES);
        return 1;
    }
    (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
    return 0;

} //ExtractBeamANoiseDN

//----------------------------------------------------------------------
// Function:    ExtractBeamANoisedB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamANoisedB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    int rc = ExtractBeamANoiseDN(l1File, sdsIDs,
                           start, stride, length, p_extractResults);
    switch (rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamANoisedB

//----------------------------------------------------------------------
// Function:    ExtractBeamBNoiseDN
// Extracts:    UINT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamBNoiseDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0 && p_extractResults != 0);

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 2, 6, 10 ...
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from noise_dn
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        int rc = ExtractData2D_100(l1File, tempsdsIDs, start,
                            1, 1, (VOIDP)(extractResults->dataBuf));
        if (rc <= 0) return(rc);

        (void)memcpy(extractResults->validDataMap,
                           _beamBMap, MAX_NUM_DERIVED_VALUES);
        return (23);
    }
    else if (mode == L1_MODE_WOM || mode == L1_MODE_ROM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam B)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and odd
        if (status <= 0 || status % 2 == 0)
        {
            (void)memset((void*)extractResults->validDataMap, 0,
                                             MAX_NUM_DERIVED_VALUES);
            return 0;
        }

        // get it from loop_back_cal_noise
        if (l1File->GetDatasetData1D(sdsIDs[3], start, 1, 1,
                        (VOIDP)(extractResults->dataBuf)) != HDF_SUCCEED)
            return (-1);
   
        (void)memcpy((void*)extractResults->validDataMap, (void*)_onlyOneMap,
                                             MAX_NUM_DERIVED_VALUES);
        return 1;
    }
    (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
    return 0;

} //ExtractBeamBNoiseDN

//----------------------------------------------------------------------
// Function:    ExtractBeamBNoisedB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamBNoisedB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    int rc = ExtractBeamBNoiseDN(l1File, sdsIDs,
                           start, stride, length, p_extractResults);
   
    switch (rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamBNoisedB

//----------------------------------------------------------------------
// Function:    _extractBeamAOneSliceDN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
static int
_extractBeamAOneSliceDN(
TlmHdfFile*         l1File,
int32*              sdsIDs,
int32               start,
int32               sliceIndex,  // slice no, start from 0
unsigned int*       dnValues,
unsigned char*      validDataMap)
{
    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract all 100 pulses
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned int allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        int rc = ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer);
        if (rc <= 0) return(rc);

        unsigned int* uintP = dnValues;
        for (int i=0; i < 100; i++)
            *uintP++ = allBuffer[i][sliceIndex];

        // copy the beam A valid data map
        (void)memcpy(validDataMap, _beamAMap, MAX_NUM_DERIVED_VALUES);
        return (23);
    }
    else if (mode == L1_MODE_WOM || mode == L1_MODE_ROM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be even number (Beam A)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                                               != HDF_SUCCEED)
            return(-1);

        // true cal pulse pos has to be >0 and even
        if (status <= 0 || status % 2 != 0)
        {
            (void)memset(validDataMap, 0, MAX_NUM_DERIVED_VALUES);
            return 0;
        }

        // get it from loop_back_cal_power
        unsigned int allBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        int rc = ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                                      (VOIDP)allBuffer);
        if (rc <= 0) return(rc);

        unsigned int* uintP = dnValues;
        *uintP = allBuffer[sliceIndex];
        // copy the one valid data map
        (void)memcpy(validDataMap, _onlyOneMap, MAX_NUM_DERIVED_VALUES);

        return 1;
    }
    (void)memset(validDataMap, 0, MAX_NUM_DERIVED_VALUES);
    return 0;

} //_extractBeamAOneSliceDN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice1DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice1DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamAOneSliceDN(l1File, sdsIDs, start, 0,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamASlice1DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice1dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice1dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamASlice1DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamASlice1dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice2DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice2DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamAOneSliceDN(l1File, sdsIDs, start, 1,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamASlice2DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice2dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice2dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamASlice2DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamASlice2dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice3DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice3DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamAOneSliceDN(l1File, sdsIDs, start, 2,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamASlice3DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice3dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice3dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamASlice3DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamASlice3dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice4DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice4DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamAOneSliceDN(l1File, sdsIDs, start, 3,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamASlice4DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice4dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice4dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamASlice4DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamASlice4dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice5DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice5DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamAOneSliceDN(l1File, sdsIDs, start, 4,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamASlice5DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice5dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice5dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamASlice5DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamASlice5dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice6DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice6DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamAOneSliceDN(l1File, sdsIDs, start, 5,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamASlice6DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice6dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice6dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamASlice6DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamASlice6dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice7DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice7DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamAOneSliceDN(l1File, sdsIDs, start, 6,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamASlice7DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice7dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice7dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamASlice7DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamASlice7dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice8DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice8DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamAOneSliceDN(l1File, sdsIDs, start, 7,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamASlice8DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice8dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice8dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamASlice8DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamASlice8dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice9DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice9DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamAOneSliceDN(l1File, sdsIDs, start, 8,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamASlice9DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice9dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice9dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamASlice9DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamASlice9dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice10DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice10DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamAOneSliceDN(l1File, sdsIDs, start, 9,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamASlice10DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice10dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice10dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamASlice10DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamASlice10dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice11DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice11DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamAOneSliceDN(l1File, sdsIDs, start, 10,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamASlice11DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice11dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice11dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamASlice11DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamASlice11dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice12DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice12DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamAOneSliceDN(l1File, sdsIDs, start, 11,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamASlice12DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice12dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamASlice12dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamASlice12DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamASlice12dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBOneSliceDN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
static int
_extractBeamBOneSliceDN(
TlmHdfFile*         l1File,
int32*              sdsIDs,
int32               start,
int32               sliceIndex,  // slice no, start from 0
unsigned int*       dnValues,
unsigned char*      validDataMap)
{
    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract all 100 pulses
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned int allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        int rc = ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer);
        if (rc <= 0) return(rc);

        unsigned int* uintP = dnValues;
        for (int i=0; i < 100; i++)
            *uintP++ = allBuffer[i][sliceIndex];

        // copy the beam B valid data map
        (void)memcpy(validDataMap, _beamBMap, MAX_NUM_DERIVED_VALUES);
        return (23);
    }
    else if (mode == L1_MODE_WOM || mode == L1_MODE_ROM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam B)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                                               != HDF_SUCCEED)
            return(-1);

        // true cal pulse pos has to be >0 and odd
        if (status <= 0 || status % 2 == 0)
        {
            (void)memset(validDataMap, 0, MAX_NUM_DERIVED_VALUES);
            return 0;
        }

        // get it from loop_back_cal_power
        unsigned int allBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        int rc = ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                                      (VOIDP)allBuffer);
        if (rc <= 0) return(rc);

        unsigned int* uintP = dnValues;
        *uintP = allBuffer[sliceIndex];
        // copy the one valid data map
        (void)memcpy(validDataMap, _onlyOneMap, MAX_NUM_DERIVED_VALUES);

        return 1;
    }
    (void)memset(validDataMap, 0, MAX_NUM_DERIVED_VALUES);
    return 0;

} //_extractBeamBOneSliceDN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice1DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice1DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamBOneSliceDN(l1File, sdsIDs, start, 0,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamBSlice1DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice1dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice1dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamBSlice1DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamBSlice1dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice2DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice2DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamBOneSliceDN(l1File, sdsIDs, start, 1,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamBSlice2DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice2dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice2dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamBSlice2DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamBSlice2dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice3DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice3DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamBOneSliceDN(l1File, sdsIDs, start, 2,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamBSlice3DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice3dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice3dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamBSlice3DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamBSlice3dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice4DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice4DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamBOneSliceDN(l1File, sdsIDs, start, 3,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamBSlice4DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice4dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice4dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamBSlice4DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamBSlice4dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice5DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice5DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamBOneSliceDN(l1File, sdsIDs, start, 4,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamBSlice5DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice5dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice5dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamBSlice5DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamBSlice5dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice6DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice6DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamBOneSliceDN(l1File, sdsIDs, start, 5,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamBSlice6DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice6dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice6dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamBSlice6DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamBSlice6dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice7DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice7DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamBOneSliceDN(l1File, sdsIDs, start, 6,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamBSlice7DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice7dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice7dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamBSlice7DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamBSlice7dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice8DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice8DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamBOneSliceDN(l1File, sdsIDs, start, 7,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamBSlice8DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice8dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice8dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamBSlice8DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamBSlice8dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice9DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice9DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamBOneSliceDN(l1File, sdsIDs, start, 8,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamBSlice9DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice9dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice9dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamBSlice9DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamBSlice9dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice10DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice10DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamBOneSliceDN(l1File, sdsIDs, start, 9,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamBSlice10DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice10dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice10dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamBSlice10DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamBSlice10dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice11DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice11DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamBOneSliceDN(l1File, sdsIDs, start, 10,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamBSlice11DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice11dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice11dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamBSlice11DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamBSlice11dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice12DN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice12DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractBeamBOneSliceDN(l1File, sdsIDs, start, 11,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractBeamBSlice12DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice12dB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamBSlice12dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractBeamBSlice12DN(l1File, sdsIDs,
                           start, 0, 1, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamBSlice12dB

//----------------------------------------------------------------------
// Function:    ExtractBeamAPowerDN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamAPowerDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    unsigned int uintBuffer[100];

    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;

    // get the first slice to find out number of values is returned
    int rc = _extractBeamAOneSliceDN(l1File, sdsIDs, start, 0,
                                  uintBuffer, extractResults->validDataMap);
    switch(rc)
    {
        case 0:
        case -1:
            return rc;
        case 1:
        {
            // every slice should only contain 1 value
            unsigned int * uint4P = (unsigned int*)extractResults->dataBuf;
            *uint4P = uintBuffer[0];
            int nextrc;
            for (int sliceIndex=1; sliceIndex < 12; sliceIndex++)
            {
                nextrc = _extractBeamAOneSliceDN(l1File, sdsIDs, start,
                                  sliceIndex, uintBuffer,
                                  extractResults->validDataMap);
                if (nextrc == 1)
                    *uint4P += uintBuffer[0];
            }
            return rc;
        }
        default:
        {
            // every slice contains more than 1 value
            unsigned int * uint4P = (unsigned int*)extractResults->dataBuf;
            unsigned int* tempUint4P = 0;
            for (int j=0; j < MAX_NUM_DERIVED_VALUES; j++)
            {
                if (extractResults->validDataMap[j])
                {
                    tempUint4P = uint4P + j;
                    *tempUint4P = uintBuffer[j];
                }
            }
            int i;
            for (int sliceIndex=1; sliceIndex < 12; sliceIndex++)
            {
                int nextrc = _extractBeamAOneSliceDN(l1File, sdsIDs, start,
                                          sliceIndex, uintBuffer,
                                          extractResults->validDataMap);
                if (nextrc > 1)
                {
                    tempUint4P = 0;
                    for (i=0; i < MAX_NUM_DERIVED_VALUES; i++)
                    {
                        if (extractResults->validDataMap[i])
                        {
                            tempUint4P = uint4P + i;
                            *tempUint4P += uintBuffer[i];
                        }
                    }
                }
                else
                {
                    (void)memset(extractResults->validDataMap, 0,
                                             MAX_NUM_DERIVED_VALUES);
                    return -1;
                }
            }
            return rc;
        }
    }
    return -1;

} //ExtractBeamAPowerDN

//----------------------------------------------------------------------
// Function:    ExtractBeamAPowerdB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamAPowerdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    int rc = ExtractBeamAPowerDN(l1File, sdsIDs,
                           start, stride, length, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamAPowerdB

//----------------------------------------------------------------------
// Function:    ExtractBeamBPowerDN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractBeamBPowerDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    unsigned int uintBuffer[100];

    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;

    // get the first slice to find out number of values is returned
    int rc = _extractBeamBOneSliceDN(l1File, sdsIDs, start, 0,
                                  uintBuffer, extractResults->validDataMap);
    switch(rc)
    {
        case 0:
        case -1:
            return rc;
        case 1:
        {
            // every slice should only contain 1 value
            unsigned int * uint4P = (unsigned int*)extractResults->dataBuf;
            *uint4P = uintBuffer[0];
            int nextrc;
            for (int sliceIndex=1; sliceIndex < 12; sliceIndex++)
            {
                nextrc = _extractBeamBOneSliceDN(l1File, sdsIDs, start,
                                        sliceIndex, uintBuffer,
                                        extractResults->validDataMap);
                if (nextrc == 1)
                    *uint4P += uintBuffer[0];
            }
            return rc;
        }
        default:
        {
            // every slice contains more than 1 value
            unsigned int * uint4P = (unsigned int*)extractResults->dataBuf;
            unsigned int* tempUint4P = 0;
            for (int j=0; j < MAX_NUM_DERIVED_VALUES; j++)
            {
                if (extractResults->validDataMap[j])
                {
                    tempUint4P = uint4P + j;
                    *tempUint4P = uintBuffer[j];
                }
            }
            int i;
            for (int sliceIndex=1; sliceIndex < 12; sliceIndex++)
            {
                int nextrc = _extractBeamBOneSliceDN(l1File, sdsIDs, start,
                                            sliceIndex, uintBuffer,
                                            extractResults->validDataMap);
                if (nextrc > 1)
                {
                    tempUint4P = 0;
                    for (i=0; i < MAX_NUM_DERIVED_VALUES; i++)
                    {
                        if (extractResults->validDataMap[i])
                        {
                            tempUint4P = uint4P + i;
                            *tempUint4P += uintBuffer[i];
                        }
                    }
                }
                else
                {
                    (void)memset(extractResults->validDataMap, 0,
                                             MAX_NUM_DERIVED_VALUES);
                    return -1;
                }
            }
            return rc;
        }
    }
    return -1;

} //ExtractBeamBPowerDN

//----------------------------------------------------------------------
// Function:    ExtractBeamBPowerdB
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractBeamBPowerdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    int rc = ExtractBeamBPowerDN(l1File, sdsIDs,
                           start, stride, length, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractBeamBPowerdB

//----------------------------------------------------------------------
// Function:    ExtractNoiseLoadDN
// Extracts:    UINT4
//----------------------------------------------------------------------
int
ExtractNoiseLoadDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;
    assert(l1File != 0 && p_extractResults != 0);

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from noise_dn
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        int rc = ExtractData2D_100(l1File, tempsdsIDs, start,
                              1, 1, (VOIDP)(extractResults->dataBuf));
        if (rc <= 0) return(rc);

        (void)memcpy(extractResults->validDataMap,
                           _noiseLoadMap, MAX_NUM_DERIVED_VALUES);
        return(26);
    }
    else if (mode == L1_MODE_WOM || mode == L1_MODE_ROM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0
        if (status <= 0)
        {
            (void)memset(extractResults->validDataMap, 0,
                                             MAX_NUM_DERIVED_VALUES);
            return 0;
        }

        // get it from load_cal_noise
        if (l1File->GetDatasetData1D(sdsIDs[3], start, 1, 1,
                        (VOIDP)(extractResults->dataBuf)) != HDF_SUCCEED)
            return (-1);
   
        (void)memcpy(extractResults->validDataMap, _onlyOneMap,
                                             MAX_NUM_DERIVED_VALUES);
        return 1;
    }
    (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
    return 0;

} //ExtractNoiseLoadDN

//----------------------------------------------------------------------
// Function:    ExtractNoiseLoaddB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractNoiseLoaddB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    int rc = ExtractNoiseLoadDN(l1File, sdsIDs,
                           start, stride, length, p_extractResults);
    switch (rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;
} // ExtractNoiseLoaddB

//----------------------------------------------------------------------
// Function:    ExtractSlice1LoadPowerDN
// Extracts:    UINT2[]
//----------------------------------------------------------------------
static int
_extractLoadPowerOneSliceDN(
TlmHdfFile*         l1File,
int32*              sdsIDs,
int32               start,
int32               sliceIndex,  // slice no, start from 0
unsigned int*       dnValues,
unsigned char*      validDataMap)
{
    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
    {
        (void)memset(validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        return -1;
    }

    // CBM: extract all 100 pulses
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned int allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        int rc = ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer);
        if (rc <= 0)
        {
            (void)memset(validDataMap, 0, MAX_NUM_DERIVED_VALUES);
            return rc;
        }

        unsigned int* uintP = dnValues;
        for (int i=0; i < 100; i++)
            *uintP++ = allBuffer[i][sliceIndex];

        // copy the noise load valid data map
        (void)memcpy(validDataMap, _noiseLoadMap, MAX_NUM_DERIVED_VALUES);
        return (26);
    }
    else if (mode == L1_MODE_WOM || mode == L1_MODE_ROM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
        {
            (void)memset(validDataMap, 0, MAX_NUM_DERIVED_VALUES);
            return -1;
        }
        // true cal pulse pos has to be >0
        if (status <= 0)
        {
            (void)memset(validDataMap, 0, MAX_NUM_DERIVED_VALUES);
            return 0;
        }

        // get it from load_cal_power
        unsigned int allBuffer[12];
        int32 tempsdsIDs[1];

        // even => A, odd => B
        if (status % 2 == 0)
            tempsdsIDs[0] = sdsIDs[3];
        else
            tempsdsIDs[0] = sdsIDs[4];
        int rc = ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)allBuffer);
        if (rc <= 0)
        {
            (void)memset(validDataMap, 0, MAX_NUM_DERIVED_VALUES);
            return rc;
        }
   
        unsigned int* uintP = dnValues;
        *uintP = allBuffer[sliceIndex];
        // copy the one valid data map
        (void)memcpy(validDataMap, _onlyOneMap, MAX_NUM_DERIVED_VALUES);
        return 1;
    }
    (void)memset(validDataMap, 0, MAX_NUM_DERIVED_VALUES);
    return 0;

} // _extractLoadPowerOneSliceDN

//----------------------------------------------------------------------
// Function:    ExtractSlice1LoadPowerDN
// Extracts:    UINT2[]
//----------------------------------------------------------------------
int
ExtractSlice1LoadPowerDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractLoadPowerOneSliceDN(l1File, sdsIDs, start, 0,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractSlice1LoadPowerDN

//----------------------------------------------------------------------
// Function:    ExtractSlice1LoadPowerdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractSlice1LoadPowerdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractSlice1LoadPowerDN(l1File, sdsIDs,
                           start, 0, length, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractSlice1LoadPowerdB

//----------------------------------------------------------------------
// Function:    ExtractSlice2LoadPowerDN
// Extracts:    UINT2[]
//----------------------------------------------------------------------
int
ExtractSlice2LoadPowerDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractLoadPowerOneSliceDN(l1File, sdsIDs, start, 1,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractSlice2LoadPowerDN

//----------------------------------------------------------------------
// Function:    ExtractSlice2LoadPowerdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractSlice2LoadPowerdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractSlice2LoadPowerDN(l1File, sdsIDs,
                           start, 0, length, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractSlice2LoadPowerdB

//----------------------------------------------------------------------
// Function:    ExtractSlice3LoadPowerDN
// Extracts:    UINT2[]
//----------------------------------------------------------------------
int
ExtractSlice3LoadPowerDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractLoadPowerOneSliceDN(l1File, sdsIDs, start, 2,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractSlice3LoadPowerDN

//----------------------------------------------------------------------
// Function:    ExtractSlice3LoadPowerdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractSlice3LoadPowerdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractSlice3LoadPowerDN(l1File, sdsIDs,
                           start, 0, length, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractSlice3LoadPowerdB

//----------------------------------------------------------------------
// Function:    ExtractSlice4LoadPowerDN
// Extracts:    UINT2[]
//----------------------------------------------------------------------
int
ExtractSlice4LoadPowerDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractLoadPowerOneSliceDN(l1File, sdsIDs, start, 3,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractSlice4LoadPowerDN

//----------------------------------------------------------------------
// Function:    ExtractSlice4LoadPowerdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractSlice4LoadPowerdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractSlice4LoadPowerDN(l1File, sdsIDs,
                           start, 0, length, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractSlice4LoadPowerdB

//----------------------------------------------------------------------
// Function:    ExtractSlice5LoadPowerDN
// Extracts:    UINT2[]
//----------------------------------------------------------------------
int
ExtractSlice5LoadPowerDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractLoadPowerOneSliceDN(l1File, sdsIDs, start, 4,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractSlice5LoadPowerDN

//----------------------------------------------------------------------
// Function:    ExtractSlice5LoadPowerdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractSlice5LoadPowerdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractSlice5LoadPowerDN(l1File, sdsIDs,
                           start, 0, length, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractSlice5LoadPowerdB

//----------------------------------------------------------------------
// Function:    ExtractSlice6LoadPowerDN
// Extracts:    UINT2[]
//----------------------------------------------------------------------
int
ExtractSlice6LoadPowerDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractLoadPowerOneSliceDN(l1File, sdsIDs, start, 5,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractSlice6LoadPowerDN

//----------------------------------------------------------------------
// Function:    ExtractSlice6LoadPowerdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractSlice6LoadPowerdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractSlice6LoadPowerDN(l1File, sdsIDs,
                           start, 0, length, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractSlice6LoadPowerdB

//----------------------------------------------------------------------
// Function:    ExtractSlice7LoadPowerDN
// Extracts:    UINT2[]
//----------------------------------------------------------------------
int
ExtractSlice7LoadPowerDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractLoadPowerOneSliceDN(l1File, sdsIDs, start, 6,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractSlice7LoadPowerDN

//----------------------------------------------------------------------
// Function:    ExtractSlice7LoadPowerdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractSlice7LoadPowerdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractSlice7LoadPowerDN(l1File, sdsIDs,
                           start, 0, length, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractSlice7LoadPowerdB

//----------------------------------------------------------------------
// Function:    ExtractSlice8LoadPowerDN
// Extracts:    UINT2[]
//----------------------------------------------------------------------
int
ExtractSlice8LoadPowerDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractLoadPowerOneSliceDN(l1File, sdsIDs, start, 7,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractSlice8LoadPowerDN

//----------------------------------------------------------------------
// Function:    ExtractSlice8LoadPowerdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractSlice8LoadPowerdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractSlice8LoadPowerDN(l1File, sdsIDs,
                           start, 0, length, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractSlice8LoadPowerdB

//----------------------------------------------------------------------
// Function:    ExtractSlice9LoadPowerDN
// Extracts:    UINT2[]
//----------------------------------------------------------------------
int
ExtractSlice9LoadPowerDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractLoadPowerOneSliceDN(l1File, sdsIDs, start, 8,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractSlice9LoadPowerDN

//----------------------------------------------------------------------
// Function:    ExtractSlice9LoadPowerdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractSlice9LoadPowerdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractSlice9LoadPowerDN(l1File, sdsIDs,
                           start, 0, length, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractSlice9LoadPowerdB

//----------------------------------------------------------------------
// Function:    ExtractSlice10LoadPowerDN
// Extracts:    UINT2[]
//----------------------------------------------------------------------
int
ExtractSlice10LoadPowerDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractLoadPowerOneSliceDN(l1File, sdsIDs, start, 9,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractSlice10LoadPowerDN

//----------------------------------------------------------------------
// Function:    ExtractSlice10LoadPowerdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractSlice10LoadPowerdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractSlice10LoadPowerDN(l1File, sdsIDs,
                           start, 0, length, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractSlice10LoadPowerdB

//----------------------------------------------------------------------
// Function:    ExtractSlice11LoadPowerDN
// Extracts:    UINT2[]
//----------------------------------------------------------------------
int
ExtractSlice11LoadPowerDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractLoadPowerOneSliceDN(l1File, sdsIDs, start, 10,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractSlice11LoadPowerDN

//----------------------------------------------------------------------
// Function:    ExtractSlice11LoadPowerdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractSlice11LoadPowerdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractSlice11LoadPowerDN(l1File, sdsIDs,
                           start, 0, length, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractSlice11LoadPowerdB

//----------------------------------------------------------------------
// Function:    ExtractSlice12LoadPowerDN
// Extracts:    UINT2[]
//----------------------------------------------------------------------
int
ExtractSlice12LoadPowerDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    return(_extractLoadPowerOneSliceDN(l1File, sdsIDs, start, 11,
                                  (unsigned int*)extractResults->dataBuf,
                                  extractResults->validDataMap));
} //ExtractSlice12LoadPowerDN

//----------------------------------------------------------------------
// Function:    ExtractSlice12LoadPowerdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractSlice12LoadPowerdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // extract one at a time
    if (length != 1) return -1;

    int rc = ExtractSlice12LoadPowerDN(l1File, sdsIDs,
                           start, 0, length, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractSlice12LoadPowerdB

//----------------------------------------------------------------------
// Function:    ExtractTotalLoadPowerDN
// Extracts:    UINT4[]
//----------------------------------------------------------------------
int
ExtractTotalLoadPowerDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    unsigned int uintBuffer[100];

    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;

    // get the first slice to find out number of values is returned
    int rc = _extractLoadPowerOneSliceDN(l1File, sdsIDs, start, 0,
                                  uintBuffer, extractResults->validDataMap);
    switch(rc)
    {
        case 0:
        case -1:
            return rc;
        case 1:
        {
            // every slice should only contain 1 value
            unsigned int * uint4P = (unsigned int*)extractResults->dataBuf;
            *uint4P = uintBuffer[0];
            int nextrc;
            for (int sliceIndex=1; sliceIndex < 12; sliceIndex++)
            {
                nextrc = _extractLoadPowerOneSliceDN(l1File, sdsIDs, start,
                                         sliceIndex, uintBuffer,
                                         extractResults->validDataMap);
                if (nextrc == 1)
                    *uint4P += uintBuffer[0];
            }
            return rc;
        }
        default:
        {
            // every slice contains more than 1 value
            unsigned int * uint4P = (unsigned int*)extractResults->dataBuf;
            unsigned int* tempUint4P = 0;
            for (int j=0; j < MAX_NUM_DERIVED_VALUES; j++)
            {
                if (extractResults->validDataMap[j])
                {
                    tempUint4P = uint4P + j;
                    *tempUint4P = uintBuffer[j];
                }
            }
            for (int sliceIndex=1; sliceIndex < 12; sliceIndex++)
            {
                int nextrc = _extractLoadPowerOneSliceDN(l1File, sdsIDs, start,
                                         sliceIndex, uintBuffer,
                                         extractResults->validDataMap);
                if (nextrc > 1)
                {
                    tempUint4P = 0;
                    for (int i=0; i < MAX_NUM_DERIVED_VALUES; i++)
                    {
                        if (extractResults->validDataMap[i])
                        {
                            tempUint4P = uint4P + i;
                            *tempUint4P += uintBuffer[i];
                        }
                    }
                }
                else
                {
                    (void)memset(extractResults->validDataMap, 0,
                                             MAX_NUM_DERIVED_VALUES);
                    return -1;
                }
            }
            return rc;
        }
    }
    return -1;

} //ExtractTotalLoadPowerDN

//----------------------------------------------------------------------
// Function:    ExtractTotalLoadPowerdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractTotalLoadPowerdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    int rc = ExtractTotalLoadPowerDN(l1File, sdsIDs,
                           start, stride, length, p_extractResults);
    switch(rc)
    {
        case 1:
            return(_oneUint4TodB((DerivedExtractResult*)p_extractResults));
        case 0:
        case -1:
            return (rc);
        default:
            return(_allUint4TodB((DerivedExtractResult*)p_extractResults));
    }
    return -1;

} // ExtractTotalLoadPowerdB

//----------------------------------------------------------------------
// Function:    _extractAverageNoiseLoadDN
// Extracts:    FLOAT4
//----------------------------------------------------------------------
int
_extractAverageNoiseLoadDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // these values are accumulative
    static int frameNo = 0;
    static float prevAvgNoiseDN = 0.0;      // <P>(n-1)
    // running total for frameNo < EA_MIN_BANDWIDTH_FRAMES
    static unsigned int runningTotalNoiseDN = 0;

    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // get noise load first, return if fails
    int rc = ExtractNoiseLoadDN(l1File, sdsIDs, start,
                                       stride, length, p_extractResults);
    if (rc <= 0)
        return(rc);

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    int numValidValues = 0;
    unsigned int* uintP = (unsigned int*) extractResults->dataBuf;
    unsigned int noiseSumThisFrame = 0;
    for (int i=0; i < MAX_NUM_DERIVED_VALUES; i++)
    {
        if (extractResults->validDataMap[i])
        {
            // get the sum for all the noise values in this frame
            frameNo++;
            if (frameNo > EA_MIN_BANDWIDTH_FRAMES)
            {
                noiseSumThisFrame += *(uintP + i);
                numValidValues++;
            }
            // just accumulate the noise when frameNo is < min
            else if (frameNo < EA_MIN_BANDWIDTH_FRAMES)
            {
                unsigned int* thisNum = uintP;
                thisNum += i;
printf("%d\n", *thisNum); fflush(stdout);
                runningTotalNoiseDN += (*thisNum);
                //runningTotalNoiseDN += *(uintP + i);
            }
            // get the first running average when frameNo hits the min
            else // frameNo == EA_MIN_BANDWIDTH_FRAMES
                prevAvgNoiseDN = (runningTotalNoiseDN + *(uintP + i)) /
                                             EA_MIN_BANDWIDTH_FRAMES;
        }
    }

    if (frameNo >= EA_MIN_BANDWIDTH_FRAMES)
    {
        // got more than minimum number of frames, do the average algo
        // return one value only
        double newAvgNoiseDN = (double)noiseSumThisFrame /
                                   EA_MIN_BANDWIDTH_FRAMES +
                    ( 1 - (double) numValidValues / EA_MIN_BANDWIDTH_FRAMES) *
                                   prevAvgNoiseDN;

        prevAvgNoiseDN = (float)newAvgNoiseDN;
        memcpy(extractResults->dataBuf, &prevAvgNoiseDN, sizeof(float));
        return 1;
    }
    else
    {
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        return 0;
    }

} // _extractAverageNoiseLoadDN

//----------------------------------------------------------------------
// Function:    _extractAverageEchoLoadDN
// Extracts:    FLOAT4
//----------------------------------------------------------------------
int
_extractAverageEchoLoadDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    // these values are accumulative
    static int frameNo = 0;
    static float prevAvgEchoDN = 0.0;      // <P>(n-1)
    // running total for frameNo < EA_MIN_BANDWIDTH_FRAMES
    static unsigned int runningTotalEchoDN = 0;

    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // get noise load first, return if fails
    int rc = ExtractTotalLoadPowerDN(l1File, sdsIDs, start,
                                       stride, length, p_extractResults);
    if (rc <= 0)
        return(rc);

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;
    int numValidValues = 0;
    unsigned int* uintP = (unsigned int*) extractResults->dataBuf;
    unsigned int echoSumThisFrame = 0;
    for (int i=0; i < MAX_NUM_DERIVED_VALUES; i++)
    {
        if (extractResults->validDataMap[i])
        {
            // get the sum for all the echo values in this frame
            frameNo++;
            if (frameNo > EA_MIN_BANDWIDTH_FRAMES)
            {
                echoSumThisFrame += *(uintP + i);
                numValidValues++;
            }
            // just accumulate the echo when frameNo is < min
            else if (frameNo < EA_MIN_BANDWIDTH_FRAMES)
                runningTotalEchoDN += *(uintP + i);
            // get the first running average when frameNo hits the min
            else // frameNo == EA_MIN_BANDWIDTH_FRAMES
                prevAvgEchoDN = (runningTotalEchoDN + *(uintP + i)) /
                                             EA_MIN_BANDWIDTH_FRAMES;
        }
    }

    if (frameNo >= EA_MIN_BANDWIDTH_FRAMES)
    {
        // got more than minimum number of frames, do the average algo
        // return one value only
        double newAvgEchoDN = (double)echoSumThisFrame /
                                   EA_MIN_BANDWIDTH_FRAMES +
                    ( 1 - (double) numValidValues / EA_MIN_BANDWIDTH_FRAMES) *
                                   prevAvgEchoDN;

        prevAvgEchoDN = (float)newAvgEchoDN;
        memcpy(extractResults->dataBuf, &prevAvgEchoDN, sizeof(float));
        return 1;
    }
    else
    {
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        return 0;
    }

} // _extractAverageEchoLoadDN

//----------------------------------------------------------------------
// Function:    ExtractBandwidthRatioDN
// Extracts:    FLOAT4
//----------------------------------------------------------------------
int
ExtractBandwidthRatioDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    assert(l1File != 0);

    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;

    // get noise load first, return if fails
    int32 tempsdsIDs[5];
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[3];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[4];  // load_cal_noise
    int rc = _extractAverageNoiseLoadDN(l1File, tempsdsIDs, start,
                                       stride, length, extractResults, 0);
    if (rc <= 0)
    {
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        return(rc);
    }
    float avgNoiseDN=0.0;
    memcpy(&avgNoiseDN, extractResults->dataBuf, sizeof(float));

    // now get echo load, return if fails
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[2];  // power_dn
    tempsdsIDs[2] = sdsIDs[3];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[5];  // load_cal_A_power
    tempsdsIDs[4] = sdsIDs[6];  // load_cal_B_power
    rc = _extractAverageEchoLoadDN(l1File, tempsdsIDs, start,
                                       stride, length, extractResults, 0);
    if (rc <= 0)
    {
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        return(rc);
    }
    float avgEchoDN = 0.0;
    memcpy(&avgEchoDN, extractResults->dataBuf, sizeof(float));
    if (avgEchoDN == 0.0)
    {
        fprintf(stderr, "Band Width Ratio: average Echo Filter = 0\n");
        return -1;
    }

    double doubleTemp = 1.0 / 2.9034 * ((double)avgNoiseDN / (double)avgEchoDN);
    float bandWidthRatio = (float) doubleTemp;
    memcpy(extractResults->dataBuf, &bandWidthRatio, sizeof(float));

    (void)memcpy(extractResults->validDataMap, _onlyOneMap,
                                             MAX_NUM_DERIVED_VALUES);
    return 1;

} //ExtractBandwidthRatioDN

//----------------------------------------------------------------------
// Function:    ExtractBandwidthRatiodB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractBandwidthRatiodB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    int rc = ExtractBandwidthRatioDN(l1File, sdsIDs,
                           start, stride, length, p_extractResults);
    DerivedExtractResult* extractResults =
                             (DerivedExtractResult*) p_extractResults;
    switch(rc)
    {
        case 1:
            return(_oneFloat4TodB(extractResults));
        case 0:
            (void)memset(extractResults->validDataMap,
                                0, MAX_NUM_DERIVED_VALUES);
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBandwidthRatiodB

//----------------------------------------------------------------------
// Function:    ExtractGainRatioBeamADN
// Extracts:    FLOAT4
//----------------------------------------------------------------------
int
ExtractGainRatioBeamADN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;

    // get noise load first, return if fails
    DerivedExtractResult noiseExRes;
    noiseExRes.dataBuf = (char*)malloc(100 * sizeof(unsigned int));
    assert(noiseExRes.dataBuf != 0);
    int32 tempsdsIDs[4];
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[3];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[4];  // loop_back_cal_noise
    int rc = ExtractBeamANoiseDN(l1File, tempsdsIDs, start,
                                       stride, length, &noiseExRes);
    if (rc <= 0)
    {
        free(noiseExRes.dataBuf);
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        return(rc);
    }

    // now get echo load, return if fails
    DerivedExtractResult echoExRes;
    echoExRes.dataBuf = (char*)malloc(100 * sizeof(unsigned int));
    assert(echoExRes.dataBuf != 0);
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[2];  // power_dn
    tempsdsIDs[2] = sdsIDs[3];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[5];  // loop_back_cal_A_power
    rc = ExtractBeamAPowerDN(l1File, tempsdsIDs, start,
                                       stride, length, &echoExRes);
    if (rc <= 0)
    {
        (void)free(echoExRes.dataBuf);
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        return(rc);
    }

    unsigned int* noiseDN = (unsigned int*) (noiseExRes.dataBuf);
    unsigned int* echoDN = (unsigned int*) (echoExRes.dataBuf);
    float        gainRatio;
    for (int i=0; i < MAX_NUM_DERIVED_VALUES; i++)
    {
        if (echoExRes.validDataMap[i])
        {
            if (*echoDN == 0)
            {
                fprintf(stderr, "Gain Ratio A: Echo Filter = 0\n");
                return -1;
            }
            gainRatio = (float)*noiseDN / (float)*echoDN;
            (void)memcpy(extractResults->dataBuf + i * sizeof(float),
                               &gainRatio, sizeof(float));
        }
        noiseDN++;
        echoDN++;
    }
    (void)memcpy(extractResults->validDataMap, echoExRes.validDataMap,
                            MAX_NUM_DERIVED_VALUES);
    free(noiseExRes.dataBuf);
    free(echoExRes.dataBuf);
    return rc;

} //ExtractGainRatioBeamADN

//----------------------------------------------------------------------
// Function:    ExtractGainRatioBeamAdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractGainRatioBeamAdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    int rc = ExtractGainRatioBeamADN(l1File, sdsIDs,
                           start, stride, length, p_extractResults);
    switch(rc)
    {
        case 0:
        case -1:
            return rc;
        case 1:
            return(_oneFloat4TodB((DerivedExtractResult*) p_extractResults));
        default:
            return(_allFloat4TodB((DerivedExtractResult*) p_extractResults));
    }
    return -1;

} // ExtractGainRatioBeamAdB

//----------------------------------------------------------------------
// Function:    ExtractGainRatioBeamBDN
// Extracts:    FLOAT4
//----------------------------------------------------------------------
int
ExtractGainRatioBeamBDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    assert(l1File != 0);
    // extract one at a time
    if (length != 1) return -1;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;

    // get noise load first, return if fails
    DerivedExtractResult noiseExRes;
    noiseExRes.dataBuf =
          (char*)malloc(MAX_NUM_DERIVED_VALUES * sizeof(unsigned int));
    assert(noiseExRes.dataBuf != 0);
    int32 tempsdsIDs[4];
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[3];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[4];  // loop_back_cal_noise
    int rc = ExtractBeamBNoiseDN(l1File, tempsdsIDs, start,
                                       stride, length, &noiseExRes);
    if (rc <= 0)
    {
        free(noiseExRes.dataBuf);
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        return(rc);
    }

    // now get echo load, return if fails
    DerivedExtractResult echoExRes;
    echoExRes.dataBuf =
          (char*)malloc(MAX_NUM_DERIVED_VALUES * sizeof(unsigned int));
    assert(echoExRes.dataBuf != 0);
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[2];  // power_dn
    tempsdsIDs[2] = sdsIDs[3];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[5];  // loop_back_cal_B_power
    rc = ExtractBeamBPowerDN(l1File, tempsdsIDs, start,
                                       stride, length, &echoExRes);
    if (rc <= 0)
    {
        free(echoExRes.dataBuf);
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        return(rc);
    }

    unsigned int* noiseDN = (unsigned int*) (noiseExRes.dataBuf);
    unsigned int* echoDN = (unsigned int*) (echoExRes.dataBuf);
    float        gainRatio;
    for (int i=0; i < MAX_NUM_DERIVED_VALUES; i++)
    {
        if (echoExRes.validDataMap[i])
        {
            if (*echoDN == 0)
            {
                fprintf(stderr, "Gain Ratio B: Echo Filter = 0\n");
                return -1;
            }
            gainRatio = (float)*noiseDN / (float)*echoDN;
            (void)memcpy(extractResults->dataBuf + i * sizeof(float),
                               &gainRatio, sizeof(float));
        }
        noiseDN++;
        echoDN++;
    }
    (void)memcpy(extractResults->validDataMap, echoExRes.validDataMap,
                            MAX_NUM_DERIVED_VALUES);
    free(noiseExRes.dataBuf);
    free(echoExRes.dataBuf);
    return rc;

} //ExtractGainRatioBeamBDN

//----------------------------------------------------------------------
// Function:    ExtractGainRatioBeamBdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractGainRatioBeamBdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*)     // unused
{
    int rc = ExtractGainRatioBeamBDN(l1File, sdsIDs,
                           start, stride, length, p_extractResults);
    switch(rc)
    {
        case 0:
        case -1:
            return rc;
        case 1:
            return(_oneFloat4TodB((DerivedExtractResult*) p_extractResults));
        default:
            return(_allFloat4TodB((DerivedExtractResult*) p_extractResults));
    }
    return -1;
} // ExtractGainRatioBeamBdB

//----------------------------------------------------------------------
// Function:    _extractOneReceiverGainADN
// Extracts:    FLOAT4
//----------------------------------------------------------------------
int
_extractOneReceiverGainADN(
TlmHdfFile*      l1File,
int32*           sdsIDs,
int32            start,
int32            stride,
int32            length,
float&           gainDN,
PolynomialTable* polyTable)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    if (polyTable == 0)
    {
        fprintf(stderr, "Receiver Gain A: No polynomial table\n");
        return -1;
    }

    DerivedExtractResult extractResults;
    extractResults.dataBuf = (char*)malloc(100 * sizeof(unsigned int));
    assert(extractResults.dataBuf != 0);
    // get noise load first, return if fails
    int32 tempsdsIDs[4];
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[2];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[3];  // loop_back_cal_noise
    int numNoises = ExtractBeamANoiseDN(l1File, tempsdsIDs, start,
                                       stride, length, &extractResults);
    if (numNoises <= 0)
    {
        free((void*)extractResults.dataBuf);
        return(numNoises);
    }

    unsigned int dnSum=0;
    if (numNoises >= 1)
    {
        unsigned int* uintP;
        for (int i=0; i < MAX_NUM_DERIVED_VALUES; i++)
        {
            if (extractResults.validDataMap[i])
            {
                uintP = (unsigned int*) extractResults.dataBuf + i;
                dnSum += *uintP;
            }
        }
    }
    // now get transmit power a, return if fails
    // reuse the space allocated above
    tempsdsIDs[0] = sdsIDs[4];  // transmit power a
    int rc = ExtractXmitPowerAmWatts(l1File, tempsdsIDs, start,
                             stride, length, &extractResults, polyTable);
    if (rc <= 0)
    {
        free((void*)extractResults.dataBuf);
        return(rc);
    }

    // only have one value
    float xmitPowerMWatts = *((float*)extractResults.dataBuf);
    if (xmitPowerMWatts == 0.0)
    {
        fprintf(stderr, "Tranmit power (denominator) == 0.0\n");
        return -1;
    }

    gainDN =  ((float)dnSum) * pow(10.0, 14.79) / xmitPowerMWatts;

    // CBM: average over all 23 pulses
    if (numNoises > 1) gainDN /= (float)numNoises;

    free((void*)extractResults.dataBuf);

    return 1;

} // _extractOneReceiverGainADN


//----------------------------------------------------------------------
// Function:    ExtractReceiverGainADN
// Extracts:    FLOAT4
//----------------------------------------------------------------------
int
ExtractReceiverGainADN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable* polyTable)
{
    // extract one at a time
    if (length != 1) return -1;

    //-----------------------------------------------------------
    // this holds the index[0-9] extracted previously
    // if _rgACurrentIndex is not set then it is not called from one
    // of the EU extraction function
    //-----------------------------------------------------------
    if (_rgACurrentIndex == 0)
        _rgACurrentIndex = &_rgADNCurrentIndex;
    if (_rgALastDataIndex == 0)
        _rgALastDataIndex = &_rgADNLastDataIndex;
    if (_rgABuf == 0)
        _rgABuf = _rgADNBuf;

    assert(l1File != 0);

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;

    float rcvGain=0.0;
    int rc = _extractOneReceiverGainADN(l1File, sdsIDs, start,
                                  stride, length, rcvGain, polyTable);
    if (rc <= 0)
    {
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        _rgACurrentIndex=0;
        _rgALastDataIndex=0;
        _rgABuf=0;
        return rc;
    }

    // WOM: need to get running average of 10 values
    int i=0;
    // for the first 4 measurements, save and return
    if (*_rgACurrentIndex < 4)
    {
        _rgABuf[*_rgACurrentIndex] = rcvGain;
        (*_rgACurrentIndex)++;
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        _rgACurrentIndex=0;
        _rgALastDataIndex=0;
        _rgABuf=0;
        return 0;
    }
    else if (*_rgACurrentIndex == 4)
    {
        _rgABuf[4] = rcvGain;
        // extract 5 more measurements: [5],[6],[7],[8],[9]
        i=1;
        // j continue to loop, but i increments only when there is valid RG
        for (int j=1; i < 6; j++)
        {
            rc = _extractOneReceiverGainADN(l1File, sdsIDs, start+j,
                                        stride, length, rcvGain, polyTable);
            if (rc == 1)
            {
                _rgABuf[4+i] = rcvGain;
                i++;
                *_rgALastDataIndex = start+j;
                continue;
            }
            else if (rc == 0)
                // can't get receiver gain in this frame, go to next
                continue;
            else
            {
                (void)memset(extractResults->validDataMap, 0,
                                              MAX_NUM_DERIVED_VALUES);
                _rgACurrentIndex=0;
                _rgALastDataIndex=0;
                _rgABuf=0;
                return rc;
            }
        }
    }
    else
    {
        // extract the [9]
        for(int k = *_rgALastDataIndex; ; k++)
        {
            rc = _extractOneReceiverGainADN(l1File, sdsIDs, k,
                                       stride, length, rcvGain, polyTable);
            if (rc == 1)
            {
                _rgABuf[9] = rcvGain;
                *_rgALastDataIndex = k;
                break;
            }
            else if (rc == 0)
                continue;
            else
            {
                (void)memset(extractResults->validDataMap, 0,
                                              MAX_NUM_DERIVED_VALUES);
                _rgACurrentIndex=0;
                _rgALastDataIndex=0;
                _rgABuf=0;
                return rc;
            }
        }
    }

    // now add them up and average it over 10
    float totalRcvGain = 0.0;
    for (i=0; i < 9; i++)
        totalRcvGain += _rgABuf[i];
    float avgRcvGain = totalRcvGain / 10.0;
    (void)memcpy(extractResults->dataBuf, &avgRcvGain, sizeof(float));
    (void)memcpy(extractResults->validDataMap,
                           _onlyOneMap, MAX_NUM_DERIVED_VALUES);

    // save the first 9 rcv gains in the buffer for the next run
    for (i=0; i < 8; i++)
        _rgABuf[i] = _rgABuf[i+1];
    (*_rgACurrentIndex)++;

    _rgACurrentIndex=0;
    _rgALastDataIndex=0;
    _rgABuf=0;

    return 1;

} //ExtractReceiverGainADN

//----------------------------------------------------------------------
// Function:    _extractOneReceiverGainBDN
// Extracts:    FLOAT4
//----------------------------------------------------------------------
int
_extractOneReceiverGainBDN(
TlmHdfFile*      l1File,
int32*           sdsIDs,
int32            start,
int32            stride,
int32            length,
float&           gainDN,
PolynomialTable* polyTable)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    if (polyTable == 0)
    {
        fprintf(stderr, "Receiver Gain B: No polynomial table\n");
        return -1;
    }

    DerivedExtractResult extractResults;
    extractResults.dataBuf = (char*)malloc(100 * sizeof(unsigned int));
    assert(extractResults.dataBuf != 0);
    // get noise load first, return if fails
    int32 tempsdsIDs[4];
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[2];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[3];  // loop_back_cal_noise
    int numNoises = ExtractBeamBNoiseDN(l1File, tempsdsIDs, start,
                                       stride, length, &extractResults);
    if (numNoises <= 0)
    {
        free((void*)extractResults.dataBuf);
        return(numNoises);
    }

    unsigned int dnSum=0;
    if (numNoises >= 1)
    {
        unsigned int* uintP;
        for (int i=0; i < MAX_NUM_DERIVED_VALUES; i++)
        {
            if (extractResults.validDataMap[i])
            {
                uintP = (unsigned int*) extractResults.dataBuf + i;
                dnSum += *uintP;
            }
        }
    }

    // now get transmit power b, return if fails
    // reuse the space allocated above
    tempsdsIDs[0] = sdsIDs[4];  // transmit power a
    int rc = ExtractXmitPowerBmWatts(l1File, tempsdsIDs, start,
                             stride, length, &extractResults, polyTable);
    if (rc <= 0)
    {
        free((void*)extractResults.dataBuf);
        return(rc);
    }

    // only have one value
    float xmitPowerMWatts = *((float*)extractResults.dataBuf);
    if (xmitPowerMWatts == 0.0)
    {
        fprintf(stderr, "Tranmit power (denominator) == 0.0\n");
        return -1;
    }

    gainDN =  ((float)dnSum) * pow(10.0, 14.825) / xmitPowerMWatts;

    // CBM: average over all 23 pulses
    if (numNoises > 1) gainDN /= (float)numNoises;

    free(extractResults.dataBuf);
    return 1;

} // _extractOneReceiverGainBDN

//----------------------------------------------------------------------
// Function:    ExtractReceiverGainBDN
// Extracts:    FLOAT4
//----------------------------------------------------------------------
int
ExtractReceiverGainBDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable* polyTable)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);
    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;

    //-----------------------------------------------------------
    // this holds the index[0-9] extracted previously
    // if _rgBCurrentIndex is not set then it is not called from one
    // of the EU extraction function
    //-----------------------------------------------------------
    if (_rgBCurrentIndex == 0)
        _rgBCurrentIndex = &_rgBDNCurrentIndex;
    if (_rgBLastDataIndex == 0)
        _rgBLastDataIndex = &_rgBDNLastDataIndex;
    if (_rgBBuf == 0)
        _rgBBuf = _rgBDNBuf;

    float rcvGain=0.0;
    int rc = _extractOneReceiverGainBDN(l1File, sdsIDs, start,
                                stride, length, rcvGain, polyTable);
    if (rc <= 0)
    {
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        _rgBCurrentIndex=0;
        _rgBLastDataIndex=0;
        _rgBBuf=0;
        return rc;
    }

    // for the first 4 measurements, save and return
    int i=0;
    if (*_rgBCurrentIndex < 4)
    {
        _rgBBuf[*_rgBCurrentIndex] = rcvGain;
        (*_rgBCurrentIndex)++;
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        _rgBCurrentIndex=0;
        _rgBLastDataIndex=0;
        _rgBBuf=0;
        return 0;
    }
    else if (*_rgBCurrentIndex == 4)
    {
        _rgBBuf[4] = rcvGain;
        // extract 5 more measurements: [5],[6],[7],[8],[9]
        i=1;
        // j continue to loop, but i increments only when there is valid RG
        for (int j=1; i < 6; j++)
        {
            rc = _extractOneReceiverGainBDN(l1File, sdsIDs, start+j,
                                        stride, length, rcvGain, polyTable);
            if (rc == 1)
            {
                _rgBBuf[4+i] = rcvGain;
                i++;
                *_rgBLastDataIndex = start+j;
                continue;
            }
            else if (rc == 0)
                // can't get receiver gain in this frame, go to next
                continue;
            else
            {
                (void)memset(extractResults->validDataMap, 0,
                                              MAX_NUM_DERIVED_VALUES);
                _rgBCurrentIndex=0;
                _rgBLastDataIndex=0;
                _rgBBuf=0;
                return rc;
            }
        }
    }
    else
    {
        // extract the [9]
        for(int k = *_rgBLastDataIndex; ; k++)
        {
            rc = _extractOneReceiverGainADN(l1File, sdsIDs, k,
                                       stride, length, rcvGain, polyTable);
            if (rc == 1)
            {
                _rgBBuf[9] = rcvGain;
                *_rgBLastDataIndex = k;
                break;
            }
            else if (rc == 0)
                continue;
            else
            {
                (void)memset(extractResults->validDataMap, 0,
                                              MAX_NUM_DERIVED_VALUES);
                _rgBCurrentIndex=0;
                _rgBLastDataIndex=0;
                _rgBBuf=0;
                return rc;
            }
        }
    }

    // now add them up and average it over 10
    float totalRcvGain = 0.0;
    for (i=0; i < 9; i++)
        totalRcvGain += _rgBBuf[i];
    float avgRcvGain = totalRcvGain / 10;
    (void)memcpy(extractResults->dataBuf, &avgRcvGain, sizeof(float));
    (void)memcpy(extractResults->validDataMap,
                           _onlyOneMap, MAX_NUM_DERIVED_VALUES);

    // save the first 9 rcv gains in the buffer for the next run
    for (i=0; i < 8; i++)
        _rgBBuf[i] = _rgBBuf[i+1];
    (*_rgBCurrentIndex)++;

    _rgBCurrentIndex=0;
    _rgBLastDataIndex=0;
    _rgBBuf=0;

    return 1;

} //ExtractReceiverGainBDN

//----------------------------------------------------------------------
// Function:    ExtractReceiverGainAdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractReceiverGainAdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable* polyTable)
{
    if (_rgACurrentIndex == 0)
        _rgACurrentIndex = &_rgADBCurrentIndex;
    if (_rgALastDataIndex == 0)
        _rgALastDataIndex = &_rgADBLastDataIndex;
    if (_rgABuf == 0)
        _rgABuf = _rgADBBuf;
    int rc = ExtractReceiverGainADN(l1File, sdsIDs,
                           start, stride, length, p_extractResults, polyTable);
    DerivedExtractResult* extractResults =
                           (DerivedExtractResult*) p_extractResults;
    _rgACurrentIndex = 0;
    _rgALastDataIndex = 0;
    _rgABuf = 0;
    switch(rc)
    {
        case 1:
            (void)_oneFloat4TodB(extractResults);
            return 1;
        case 0:
            (void)memset(extractResults->validDataMap,
                                0, MAX_NUM_DERIVED_VALUES);
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractReceiverGainAdB

//----------------------------------------------------------------------
// Function:    ExtractReceiverGainBdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractReceiverGainBdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable* polyTable)
{
    if (_rgBCurrentIndex == 0)
        _rgBCurrentIndex = &_rgBDBCurrentIndex;
    if (_rgBLastDataIndex == 0)
        _rgBLastDataIndex = &_rgBDBLastDataIndex;
    if (_rgBBuf == 0)
        _rgBBuf = _rgBDBBuf;
    int rc = ExtractReceiverGainBDN(l1File, sdsIDs,
                           start, stride, length, p_extractResults, polyTable);
    _rgBCurrentIndex = 0;
    _rgBLastDataIndex = 0;
    _rgBBuf = 0;
    DerivedExtractResult* extractResults =
                           (DerivedExtractResult*) p_extractResults;
    switch(rc)
    {
        case 1:
            return(_oneFloat4TodB(extractResults));
        case 0:
            (void)memset(extractResults->validDataMap,
                                0, MAX_NUM_DERIVED_VALUES);
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractReceiverGainBdB


#ifndef NOISE_FIGURE_CONST
#define NOISE_FIGURE_CONST  ((1.38 * pow(10.0, -20.0)) * pow(10.0, 6.0) * 290)
#endif

//----------------------------------------------------------------------
// Function:    ExtractNoiseFigureADN
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractNoiseFigureADN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable* polyTable)
{
    DerivedExtractResult* extractResults =
                           (DerivedExtractResult*) p_extractResults;
    DerivedExtractResult privExtRes;
    privExtRes.dataBuf =
          (char*)malloc(MAX_NUM_DERIVED_VALUES * sizeof(unsigned int));
    assert(privExtRes.dataBuf != 0);

    int rc=0;
    int32 tempsdsIDs[5];
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[2];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[3];  // load_cal_noise
    if ((rc = _extractAverageNoiseLoadDN(l1File, sdsIDs,
                           start, stride, length, &privExtRes, 0)) != 1)
    {
        free(privExtRes.dataBuf);
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        return(rc);
    }
    float avgNoiseLoadDN = *((float*)privExtRes.dataBuf);

    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[2];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[4];  // loop_back_cal_noise
    tempsdsIDs[4] = sdsIDs[5];  // transmit_power_a
    if ((rc = ExtractReceiverGainADN(l1File, sdsIDs,
                 start, stride, length, &privExtRes, polyTable)) != 1)
    {
        free(privExtRes.dataBuf);
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        return(rc);
    }
    float rcvGainDN = *((float*)privExtRes.dataBuf);

    float noiseFigDN = avgNoiseLoadDN / (rcvGainDN * NOISE_FIGURE_CONST);
    (void)memcpy(extractResults->dataBuf, &noiseFigDN, sizeof(float));
    (void)memcpy(extractResults->validDataMap,
                           _onlyOneMap, MAX_NUM_DERIVED_VALUES);

    free(privExtRes.dataBuf);
    return 1;

} // ExtractNoiseFigureADN

//----------------------------------------------------------------------
// Function:    ExtractNoiseFigureAdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractNoiseFigureAdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable* polyTable)
{
    int rc = ExtractNoiseFigureADN(l1File, sdsIDs,
                           start, stride, length, p_extractResults, polyTable);
    DerivedExtractResult* extractResults =
                           (DerivedExtractResult*) p_extractResults;
    switch(rc)
    {
        case 1:
            return(_oneFloat4TodB(extractResults));
        case 0:
            (void)memset(extractResults->validDataMap,
                                0, MAX_NUM_DERIVED_VALUES);
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractNoiseFigureAdB

//----------------------------------------------------------------------
// Function:    ExtractNoiseFigureBDN
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractNoiseFigureBDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable* polyTable)
{
    DerivedExtractResult* extractResults =
                           (DerivedExtractResult*) p_extractResults;
    DerivedExtractResult privExtRes;
    privExtRes.dataBuf =
           (char*)malloc(MAX_NUM_DERIVED_VALUES * sizeof(unsigned int));
    assert(privExtRes.dataBuf != 0);

    int rc=0;
    int32 tempsdsIDs[5];
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[2];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[3];  // load_cal_noise
    if ((rc = _extractAverageNoiseLoadDN(l1File, sdsIDs,
                           start, stride, length, &privExtRes, 0)) != 1)
    {
        free(privExtRes.dataBuf);
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        return(rc);
    }
    float avgNoiseLoadDN = *((float*)privExtRes.dataBuf);

    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[2];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[4];  // loop_back_cal_noise
    tempsdsIDs[4] = sdsIDs[5];  // transmit_power_a
    if ((rc = ExtractReceiverGainBDN(l1File, sdsIDs,
                 start, stride, length, &privExtRes, polyTable)) != 1)
    {
        free(privExtRes.dataBuf);
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        return(rc);
    }
    float rcvGainDN = *((float*)privExtRes.dataBuf);

    float noiseFigDN = avgNoiseLoadDN / (rcvGainDN * NOISE_FIGURE_CONST);
    (void)memcpy(extractResults->dataBuf, &noiseFigDN, sizeof(float));
    (void)memcpy(extractResults->validDataMap,
                           _onlyOneMap, MAX_NUM_DERIVED_VALUES);
    free(privExtRes.dataBuf);
    return 1;

} // ExtractNoiseFigureBDN

//----------------------------------------------------------------------
// Function:    ExtractNoiseFigureBdB
// Extracts:    FLOAT4[]
//----------------------------------------------------------------------
int
ExtractNoiseFigureBdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       p_extractResults,
PolynomialTable*  polyTable)
{
    int rc = ExtractNoiseFigureBDN(l1File, sdsIDs,
                           start, stride, length, p_extractResults, polyTable);
    DerivedExtractResult* extractResults =
                           (DerivedExtractResult*) p_extractResults;
    switch(rc)
    {
        case 1:
            return(_oneFloat4TodB(extractResults));
        case 0:
            (void)memset(extractResults->validDataMap,
                                0, MAX_NUM_DERIVED_VALUES);
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractNoiseFigureBdB

#if 0
//----------------------------------------------------------------------
// Function:    ExtractOrbitPeriod
// Extracts:    UINT4[]
//----------------------------------------------------------------------
int
ExtractOrbitPeriod(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // this holds the orbit time extracted previously
    static unsigned long prevOrbitTime=0;

    // extract one at a time
    if (length != 1) return -1;
 
    assert(l1File != 0);
 
    // find out the current orbit time
    unsigned long orbitTime;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &orbitTime)
                         != HDF_SUCCEED)
        return(-1);

    // orbit time is reset, this is the beginning of a new orbit
    // we want the previous ticks
    if (orbitTime <= prevOrbitTime)
    {
        memcpy(buffer, &prevOrbitTime, sizeof(unsigned long));
        prevOrbitTime = orbitTime;
        return 1;
    }
    else
    {
        prevOrbitTime = orbitTime;
        return 0;
    }

} // ExtractOrbitPeriod

//----------------------------------------------------------------------
// Function:    ExtractAntSpinRateDN
// Extracts:    UINT2[][100]
//----------------------------------------------------------------------
int
ExtractAntSpinRateDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // this holds the antenna position extracted previously
    // if _prevAntPos is not set then it is not called from one 
    // of the EU extraction function
    if (_prevAntPos == 0)
        _prevAntPos = &_prevAntPos_dn;

    // extract one at a time
    if (length != 1)
    {
        _prevAntPos = 0;
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        return -1;
    }
 
    assert(l1File != 0);
 
    // find out the raw antenna position
    unsigned short antPos[100];;
    int32 antposSdsIds[1];
    antposSdsIds[0] = sdsIDs[0];
    int rc = ExtractData2D_100(l1File, antposSdsIds, start, 0, 1, antPos);
    if (rc <= 0)
    {
        _prevAntPos = 0;
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        return rc;
    }

    // spin rate is the delta antenna position
    unsigned short * tempBuf = (unsigned short *) buffer;
    for (int i=0; i < 100; i++)
    {
        *tempBuf = antPos[i] - *_prevAntPos;
        *_prevAntPos = antPos[i];
        tempBuf++;
    }
    _prevAntPos = 0;
    return 1;

} // ExtractAntSpinRateDN

//----------------------------------------------------------------------
// Function:    ExtractAntSpinRateDegree
// Extracts:    FLOAT4[][100]
//----------------------------------------------------------------------
int
ExtractAntSpinRateDegree(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable* polyTable)
{
    if (length != 1) return -1;

    if (polyTable == 0)
    {
        fprintf(stderr, "Antenna Spin Rate degrees: No polynomial table\n");
        return -1;
    }

    // get polynomial for antenna position/degrees
    static const Polynomial* antPosPoly=0;
    if (antPosPoly == 0)
    {
        antPosPoly = polyTable->SelectPolynomial(
                                 "antenna_position", "degrees");
    }
    if (antPosPoly == 0)
    {
        fprintf(stderr, "Antenna Spin Rate deg/sec: "
                        "No polynomial for antenna position\n");
        return -1;
    }
    unsigned short spinRateDN[100];
    if (_prevAntPos == 0) _prevAntPos = &_prevAntPos_deg;
    int rc = ExtractAntSpinRateDN(l1File, sdsIDs, start, stride,
                                  1, spinRateDN);
    if (rc <= 0) return rc;

    // apply polynomial to the float array

    float * tempBuf = (float *) buffer;
    for (int i=0; i < 100; i++)
    {
        *tempBuf++ = antPosPoly->Apply((float)spinRateDN[i]);
    }
    return 1;

} // ExtractAntSpinRateDegree

//----------------------------------------------------------------------
// Function:    ExtractAntSpinRateDegSec
// Extracts:    FLOAT4[][100]
//---------------------------------------------------------------------- 
int
ExtractAntSpinRateDegSec(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       ,
VOIDP       buffer,
PolynomialTable* polyTable)
{
    // get the antenna position dn in floats
    // polynomial has been applied
    float spinRateFloat[100];
    if (_prevAntPos == 0) _prevAntPos = &_prevAntPos_deg_sec;
    int rc = ExtractAntSpinRateDegree(l1File, sdsIDs, start, stride,
                                  1, spinRateFloat, polyTable);
    if (rc <= 0) return rc;

    // now, get prf cycle time
    unsigned char prfCycleTime=0;
    int32 prfSdsIDs[1];
    prfSdsIDs[0] = sdsIDs[1];
    if (l1File->GetDatasetData1D(prfSdsIDs[0], start, 1, 1, &prfCycleTime)
                         != HDF_SUCCEED)
        return(-1);

    float * tempBuf = (float *) buffer;
    for (int i=0; i < 100; i++)
    {
        *tempBuf = spinRateFloat[i] * 1000.0 / (float) prfCycleTime;
        tempBuf++;
    }
    return 1;

} // ExtractAntSpinRateDegSec

//----------------------------------------------------------------------
// Function:    ExtractAntSpinRateRotMin
// Extracts:    FLOAT4[][100]
//---------------------------------------------------------------------- 
int
ExtractAntSpinRateRotMin(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable* polyTable)
{
    if (_prevAntPos == 0) _prevAntPos = &_prevAntPos_rot_min;
    int rc = ExtractAntSpinRateDegSec(l1File, sdsIDs, start, stride,
                                  length, buffer, polyTable);
    if (rc <= 0) return rc;

    float * tempBuf = (float *) buffer;
    for (int i=0; i < 100; i++)
    {
        *tempBuf = *tempBuf / 6.0;
        tempBuf++;
    }
    return 1;

} // ExtractAntSpinRateRotMin
#endif

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
VOIDP            p_extractResults,
PolynomialTable* polyTable)
{
    return(Extract_uint1_eu_mWatts(l1File, sdsIDs, start, stride, length,
                          p_extractResults, "transmit_power_a", "dBm",
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
VOIDP            p_extractResults,
PolynomialTable* polyTable)
{
    return(Extract_uint1_eu_mWatts(l1File, sdsIDs, start, stride, length,
                          p_extractResults, "transmit_power_b", "dBm",
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
VOIDP            p_extractResults,
const char*      sdsName,
const char*      euUnitName,
PolynomialTable* polyTable)
{
    assert(l1File != 0);

    if (length != 1) return -1;
    if (polyTable == 0) return -1;

    // alloc space to hold floats
    float dnValue;

    DerivedExtractResult* extractResults =
                 (DerivedExtractResult*) p_extractResults;

    int rc = ExtractData1D_uint1_float(l1File, sdsIDs, start, stride,
                          1, &dnValue);
    if (rc <= 0)
    {
        (void)memset(extractResults->validDataMap, 0, MAX_NUM_DERIVED_VALUES);
        return rc;
    }

    float mWattsValue;
    if (dnValue == 0.0)
        mWattsValue = 0.0;
    else
    {
        const Polynomial* polynomial = polyTable->SelectPolynomial(
                                       sdsName, euUnitName);
        if (polynomial == 0)
        {
            fprintf(stderr,"Tranmit Power (mWatts): need polynomial for dBm\n");
            (void)memset(extractResults->validDataMap,
                                       0, MAX_NUM_DERIVED_VALUES);
            return -1;
        }
        float dBmValue = polynomial->Apply(dnValue);
        mWattsValue = (float) pow( (double) 10.0, (double) dBmValue / 10.0 );
    }

    (void)memcpy(extractResults->dataBuf, &mWattsValue, sizeof(float));
    (void)memcpy(extractResults->validDataMap,
                                _onlyOneMap, MAX_NUM_DERIVED_VALUES);
    return 1;

} // Extract_uint1_eu_mWatts

//----------------------------------------------------------------------
// Function:    ExtractTwt1PowermWatts
// Extracts:    float
//----------------------------------------------------------------------
int
ExtractTwt1PowermWatts(
TlmHdfFile* l1File,
int32*           sdsIDs,
int32            start,
int32            stride,
int32            length,
VOIDP            p_extractResults,
PolynomialTable* polyTable)
{
    return(Extract_uint1_eu_mWatts(l1File, sdsIDs, start, stride, length,
                          p_extractResults, "twt1_drive_power", "dBm",
                          polyTable));
} // ExtractTwt1PowermWatts

//----------------------------------------------------------------------
// Function:    ExtractTwt2PowermWatts
// Extracts:    float
//----------------------------------------------------------------------
int
ExtractTwt2PowermWatts(
TlmHdfFile* l1File,
int32*           sdsIDs,
int32            start,
int32            stride,
int32            length,
VOIDP            p_extractResults,
PolynomialTable* polyTable)
{
    return(Extract_uint1_eu_mWatts(l1File, sdsIDs, start, stride, length,
                          p_extractResults, "twt2_drive_power", "dBm",
                          polyTable));
} // ExtractTwt2PowermWatts

//----------------------------------------------------------------------
// Function:    ExtractPowerCnvtCurrmWatts
// Extracts:    float
//----------------------------------------------------------------------
int
ExtractPowerCnvtCurrmWatts(
TlmHdfFile* l1File,
int32*           sdsIDs,
int32            start,
int32            stride,
int32            length,
VOIDP            p_extractResults,
PolynomialTable* polyTable)
{
    return(Extract_uint1_eu_mWatts(l1File, sdsIDs, start, stride, length,
                          p_extractResults, "power_convert_current", "dBm",
                          polyTable));
} // ExtractPowerCnvtCurrmWatts

//----------------------------------------------------------------------
// Function:    ExtractXmitPwrInnermWatts
// Extracts:    float
//----------------------------------------------------------------------
int
ExtractXmitPwrInnermWatts(
TlmHdfFile* l1File,
int32*           sdsIDs,
int32            start,
int32            stride,
int32            length,
VOIDP            p_extractResults,
PolynomialTable* polyTable)
{
    return(Extract_uint1_eu_mWatts(l1File, sdsIDs, start, stride, length,
                          p_extractResults, "transmit_power_inner", "dBm",
                          polyTable));
} // ExtractXmitPwrInnermWatts

//----------------------------------------------------------------------
// Function:    ExtractXmitPwrOutermWatts
// Extracts:    float
//----------------------------------------------------------------------
int
ExtractXmitPwrOutermWatts(
TlmHdfFile* l1File,
int32*           sdsIDs,
int32            start,
int32            stride,
int32            length,
VOIDP            p_extractResults,
PolynomialTable* polyTable)
{
    return(Extract_uint1_eu_mWatts(l1File, sdsIDs, start, stride, length,
                          p_extractResults, "transmit_power_outer", "dBm",
                          polyTable));
} // ExtractXmitPwrOutermWatts
