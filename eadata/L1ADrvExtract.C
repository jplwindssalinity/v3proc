//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
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
#include "ArgsPlus.h"
#include "PolyTable.h"
#include "Polynomial.h"

static const char rcs_id_L1ADrvExtract_C[] = "@(#) $Header$";

//----------------------------------------------------------------------
// Function:    ExtractBeamANoiseDN
// Extracts:    UINT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamANoiseDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 1, 5, 9, ....
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from noise_dn
        unsigned int allBuffer[100];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData2D_100(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [1], [5], [9] ...
        unsigned int* uintP = (unsigned int*) buffer;
        for (int i=1; i < 100; i+= 4)
            *uintP++ = allBuffer[i];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam A)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and odd
        if (status <= 0 || status % 2 == 0)
            return 0;

        // get it from loop_back_cal_noise
        unsigned int tempBuffer=0;
        if (l1File->GetDatasetData1D(sdsIDs[3], start, 1, 1,
                        (VOIDP)&tempBuffer) != HDF_SUCCEED)
            return (-1);
   
        unsigned int* uintP = (unsigned int*) buffer;
        *uintP = tempBuffer;
        return 1;
    }
    return 0;

} //ExtractBeamANoiseDN

//----------------------------------------------------------------------
// Function:    ExtractBeamANoisedB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamANoisedB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned int tempBuff[25];
    int rc = ExtractBeamANoiseDN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*)buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamANoisedB

//----------------------------------------------------------------------
// Function:    ExtractBeamBNoiseDN
// Extracts:    UINT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamBNoiseDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 2, 6, 10 ...
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from noise_dn
        unsigned int allBuffer[100];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData2D_100(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the 2, 6, 10
        unsigned int* uintP = (unsigned int*) buffer;
        for (int i=2; i < 100; i+= 4)
            *uintP++ = allBuffer[i];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be even number (Beam B)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and even
        if (status <= 0 || status % 2 != 0)
            return 0;

        // get it from loop_back_cal_noise
        unsigned int tempBuffer=0;
        if (l1File->GetDatasetData1D(sdsIDs[3], start, 1, 1,
                        (VOIDP)&tempBuffer) != HDF_SUCCEED)
            return (-1);
   
        unsigned int* uintP = (unsigned int*) buffer;
        *uintP = tempBuffer;
        return 1;
    }
    return 0;

} //ExtractBeamBNoiseDN

//----------------------------------------------------------------------
// Function:    ExtractBeamBNoisedB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamBNoisedB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned int tempBuff[25];
    int rc = ExtractBeamBNoiseDN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*)buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamBNoisedB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice1DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice1DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 1, 5, 9, ....
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [1], [5], [9] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=1; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][0];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam A)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and odd
        if (status <= 0 || status % 2 == 0)
            return 0;

        // get it from loop_back_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[0];
        return 1;
    }
    return 0;

} //ExtractBeamASlice1DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice1dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice1dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamASlice1DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamASlice1dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice2DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice2DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 1, 5, 9, ....
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [1], [5], [9] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=1; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][1];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam A)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and odd
        if (status <= 0 || status % 2 == 0)
            return 0;

        // get it from loop_back_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[1];
        return 1;
    }
    return 0;

} //ExtractBeamASlice2DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice2dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice2dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamASlice2DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamASlice2dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice3DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice3DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 1, 5, 9, ....
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [1], [5], [9] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=1; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][2];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam A)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and odd
        if (status <= 0 || status % 2 == 0)
            return 0;

        // get it from loop_back_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[2];
        return 1;
    }
    return 0;

} //ExtractBeamASlice3DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice3dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice3dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamASlice3DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamASlice3dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice4DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice4DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 1, 5, 9, ....
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [1], [5], [9] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=1; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][3];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam A)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and odd
        if (status <= 0 || status % 2 == 0)
            return 0;

        // get it from loop_back_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[3];
        return 1;
    }
    return 0;

} //ExtractBeamASlice4DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice4dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice4dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamASlice4DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamASlice4dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice5DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice5DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 1, 5, 9, ....
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [1], [5], [9] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=1; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][4];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam A)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and odd
        if (status <= 0 || status % 2 == 0)
            return 0;

        // get it from loop_back_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[4];
        return 1;
    }
    return 0;

} //ExtractBeamASlice5DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice5dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice5dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamASlice5DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamASlice5dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice6DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice6DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 1, 5, 9, ....
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [1], [5], [9] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=1; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][5];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam A)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and odd
        if (status <= 0 || status % 2 == 0)
            return 0;

        // get it from loop_back_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[5];
        return 1;
    }
    return 0;

} //ExtractBeamASlice6DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice6dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice6dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamASlice6DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamASlice6dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice7DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice7DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 1, 5, 9, ....
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [1], [5], [9] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=1; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][6];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam A)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and odd
        if (status <= 0 || status % 2 == 0)
            return 0;

        // get it from loop_back_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[6];
        return 1;
    }
    return 0;

} //ExtractBeamASlice7DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice7dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice7dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamASlice7DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamASlice7dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice8DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice8DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 1, 5, 9, ....
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [1], [5], [9] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=1; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][7];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam A)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and odd
        if (status <= 0 || status % 2 == 0)
            return 0;

        // get it from loop_back_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[7];
        return 1;
    }
    return 0;

} //ExtractBeamASlice8DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice8dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice8dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamASlice8DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamASlice8dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice9DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice9DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 1, 5, 9, ....
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [1], [5], [9] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=1; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][8];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam A)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and odd
        if (status <= 0 || status % 2 == 0)
            return 0;

        // get it from loop_back_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[8];
        return 1;
    }
    return 0;

} //ExtractBeamASlice9DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice9dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice9dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamASlice9DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamASlice9dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice10DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice10DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 1, 5, 9, ....
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [1], [5], [9] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=1; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][9];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam A)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and odd
        if (status <= 0 || status % 2 == 0)
            return 0;

        // get it from loop_back_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[9];
        return 1;
    }
    return 0;

} //ExtractBeamASlice10DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice10dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice10dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamASlice10DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamASlice10dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice11DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice11DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 1, 5, 9, ....
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [1], [5], [9] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=1; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][10];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam A)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and odd
        if (status <= 0 || status % 2 == 0)
            return 0;

        // get it from loop_back_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[10];
        return 1;
    }
    return 0;

} //ExtractBeamASlice11DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice11dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice11dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamASlice11DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamASlice11dB

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice12DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice12DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 1, 5, 9, ....
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [1], [5], [9] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=1; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][11];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam A)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and odd
        if (status <= 0 || status % 2 == 0)
            return 0;

        // get it from loop_back_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[11];
        return 1;
    }
    return 0;

} //ExtractBeamASlice12DN

//----------------------------------------------------------------------
// Function:    ExtractBeamASlice12dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamASlice12dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamASlice12DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamASlice12dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice1DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice1DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 2, 6, 10...
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [2], [6], [10] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=2; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][0];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam B)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and even
        if (status <= 0 || status % 2 != 0)
            return 0;

        // get it from loop_back_cal_B_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[0];
        return 1;
    }
    return 0;

} //ExtractBeamBSlice1DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice1dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice1dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamBSlice1DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamBSlice1dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice2DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice2DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 2, 6, 10...
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [2], [6], [10] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=2; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][1];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam B)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and even
        if (status <= 0 || status % 2 != 0)
            return 0;

        // get it from loop_back_cal_B_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[1];
        return 1;
    }
    return 0;

} //ExtractBeamBSlice2DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice2dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice2dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamBSlice2DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamBSlice2dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice3DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice3DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 2, 6, 10...
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [2], [6], [10] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=2; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][2];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam B)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and even
        if (status <= 0 || status % 2 != 0)
            return 0;

        // get it from loop_back_cal_B_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[2];
        return 1;
    }
    return 0;

} //ExtractBeamBSlice3DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice3dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice3dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamBSlice3DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamBSlice3dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice4DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice4DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 2, 6, 10...
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [2], [6], [10] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=2; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][3];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam B)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and even
        if (status <= 0 || status % 2 != 0)
            return 0;

        // get it from loop_back_cal_B_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[3];
        return 1;
    }
    return 0;

} //ExtractBeamBSlice4DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice4dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice4dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamBSlice4DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamBSlice4dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice5DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice5DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 2, 6, 10...
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [2], [6], [10] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=2; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][4];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam B)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and even
        if (status <= 0 || status % 2 != 0)
            return 0;

        // get it from loop_back_cal_B_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[4];
        return 1;
    }
    return 0;

} //ExtractBeamBSlice5DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice5dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice5dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamBSlice5DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamBSlice5dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice6DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice6DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 2, 6, 10...
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [2], [6], [10] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=2; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][5];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam B)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and even
        if (status <= 0 || status % 2 != 0)
            return 0;

        // get it from loop_back_cal_B_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[5];
        return 1;
    }
    return 0;

} //ExtractBeamBSlice6DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice6dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice6dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamBSlice6DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamBSlice6dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice7DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice7DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 2, 6, 10...
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [2], [6], [10] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=2; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][6];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam B)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and even
        if (status <= 0 || status % 2 != 0)
            return 0;

        // get it from loop_back_cal_B_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[6];
        return 1;
    }
    return 0;

} //ExtractBeamBSlice7DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice7dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice7dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamBSlice7DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamBSlice7dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice8DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice8DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 2, 6, 10...
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [2], [6], [10] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=2; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][7];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam B)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and even
        if (status <= 0 || status % 2 != 0)
            return 0;

        // get it from loop_back_cal_B_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[7];
        return 1;
    }
    return 0;

} //ExtractBeamBSlice8DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice8dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice8dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamBSlice8DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamBSlice8dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice9DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice9DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 2, 6, 10...
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [2], [6], [10] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=2; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][8];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam B)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and even
        if (status <= 0 || status % 2 != 0)
            return 0;

        // get it from loop_back_cal_B_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[8];
        return 1;
    }
    return 0;

} //ExtractBeamBSlice9DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice9dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice9dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamBSlice9DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamBSlice9dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice10DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice10DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 2, 6, 10...
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [2], [6], [10] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=2; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][9];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam B)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and even
        if (status <= 0 || status % 2 != 0)
            return 0;

        // get it from loop_back_cal_B_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[9];
        return 1;
    }
    return 0;

} //ExtractBeamBSlice10DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice10dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice10dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamBSlice10DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamBSlice10dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice11DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice11DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 2, 6, 10...
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [2], [6], [10] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=2; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][10];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam B)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and even
        if (status <= 0 || status % 2 != 0)
            return 0;

        // get it from loop_back_cal_B_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[10];
        return 1;
    }
    return 0;

} //ExtractBeamBSlice11DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice11dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice11dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamBSlice11DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamBSlice11dB

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice12DN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice12DN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       ,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse 2, 6, 10...
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [2], [6], [10] ...
        unsigned short* ushortP = (unsigned short*) buffer;
        for (int i=2; i < 100; i+= 4)
            *ushortP++ = allBuffer[i][11];
        return (25);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be odd number (Beam B)
        // and > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0 and even
        if (status <= 0 || status % 2 != 0)
            return 0;

        // get it from loop_back_cal_B_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        unsigned short* ushortP = (unsigned short*) buffer;
        *ushortP = tempBuffer[11];
        return 1;
    }
    return 0;

} //ExtractBeamBSlice12DN

//----------------------------------------------------------------------
// Function:    ExtractBeamBSlice12dB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamBSlice12dB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamBSlice12DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*) buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamBSlice12dB

//----------------------------------------------------------------------
// Function:    ExtractBeamAPowerDN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamAPowerDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamASlice1DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            int i=0;
            // every slice should contain 25 values
            unsigned int *uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ = (unsigned int)tempBuff[i];
            (void)ExtractBeamASlice2DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamASlice3DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamASlice4DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamASlice5DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamASlice6DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamASlice7DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamASlice8DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamASlice9DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamASlice10DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamASlice11DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamASlice12DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];

            return 25;
        }
        case 1:
        {
            // every slice should only contain 1 value
            unsigned int *uintP = (unsigned int*)buffer;
            *uintP = (unsigned int)tempBuff[0];
            (void)ExtractBeamASlice2DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamASlice3DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamASlice4DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamASlice5DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamASlice6DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamASlice7DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamASlice8DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamASlice9DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamASlice10DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamASlice11DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamASlice12DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} //ExtractBeamAPowerDN

//----------------------------------------------------------------------
// Function:    ExtractBeamAPowerdB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamAPowerdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned int tempBuff[25];
    int rc = ExtractBeamAPowerDN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*)buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float* floatP = (float*)buffer;
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(floatP, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractBeamAPowerdB

//----------------------------------------------------------------------
// Function:    ExtractBeamBPowerDN
// Extracts:    UINT2[][25]
//----------------------------------------------------------------------
int
ExtractBeamBPowerDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff[25];
    int rc = ExtractBeamBSlice1DN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            int i=0;
            // every slice should contain 25 values
            unsigned int *uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ = (unsigned int)tempBuff[i];
            (void)ExtractBeamBSlice2DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamBSlice3DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamBSlice4DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamBSlice5DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamBSlice6DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamBSlice7DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamBSlice8DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamBSlice9DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamBSlice10DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamBSlice11DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];
            (void)ExtractBeamBSlice12DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            uintP = (unsigned int*)buffer;
            for (i=0; i < 25; i++)
                *uintP++ += (unsigned int)tempBuff[i];

            return 25;
        }
        case 1:
        {
            // every slice should only contain 1 value
            unsigned int *uintP = (unsigned int*)buffer;
            *uintP = (unsigned int)tempBuff[0];
            (void)ExtractBeamBSlice2DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamBSlice3DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamBSlice4DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamBSlice5DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamBSlice6DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamBSlice7DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamBSlice8DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamBSlice9DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamBSlice10DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamBSlice11DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            (void)ExtractBeamBSlice12DN(l1File, sdsIDs,
                                start, stride, length, tempBuff);
            *uintP += (unsigned int)tempBuff[0];
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} //ExtractBeamBPowerDN

//----------------------------------------------------------------------
// Function:    ExtractBeamBPowerdB
// Extracts:    FLOAT4[][25]
//----------------------------------------------------------------------
int
ExtractBeamBPowerdB(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned int tempBuff[25];
    int rc = ExtractBeamBPowerDN(l1File, sdsIDs,
                           start, stride, length, tempBuff);
    switch(rc)
    {
        case 25:
        {
            float* floatP = (float*)buffer;
            for (int i=0; i < 25; i++)
            {
                float dBValue = (float) 10 * log10((double)tempBuff[i]);
                (void)memcpy(floatP, &dBValue, sizeof(float));
                floatP++;
            }
            return 25;
        }
        case 1:
        {
            float* floatP = (float*)buffer;
            float dBValue = (float) 10 * log10((double)tempBuff[0]);
            (void)memcpy(floatP, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
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
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse [90]
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from noise_dn
        unsigned int allBuffer[100];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData2D_100(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // [90] is what we want
        memcpy(buffer, &(allBuffer[90]), sizeof(unsigned int));
        return (1);
    }
    else if (mode == L1_MODE_WOM)
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
            return 0;

        // get it from load_cal_noise
        unsigned int tempBuffer=0;
        if (l1File->GetDatasetData1D(sdsIDs[3], start, 1, 1,
                        (VOIDP)&tempBuffer) != HDF_SUCCEED)
            return (-1);
   
        memcpy(buffer, &tempBuffer, sizeof(unsigned int));
        return 1;
    }
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
VOIDP       buffer)
{
    unsigned int tempBuff = 0;
    int rc = ExtractNoiseLoadDN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractNoiseLoaddB

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
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse [90]
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [90]
        memcpy(buffer, &(allBuffer[90][0]), sizeof(unsigned short));
        return (1);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0
        if (status <= 0)
            return 0;

        // get it from load_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        memcpy(buffer, &(tempBuffer[0]), sizeof(unsigned short));
        return 1;
    }
    return 0;

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
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff;
    int rc = ExtractSlice1LoadPowerDN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
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
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse [90]
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [90]
        memcpy(buffer, &(allBuffer[90][1]), sizeof(unsigned short));
        return (1);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0
        if (status <= 0)
            return 0;

        // get it from load_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        memcpy(buffer, &(tempBuffer[0]), sizeof(unsigned short));
        return 1;
    }
    return 0;

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
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff;
    int rc = ExtractSlice2LoadPowerDN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
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
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse [90]
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [90]
        memcpy(buffer, &(allBuffer[90][2]), sizeof(unsigned short));
        return (1);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0
        if (status <= 0)
            return 0;

        // get it from load_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        memcpy(buffer, &(tempBuffer[0]), sizeof(unsigned short));
        return 1;
    }
    return 0;

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
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff;
    int rc = ExtractSlice3LoadPowerDN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
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
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse [90]
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [90]
        memcpy(buffer, &(allBuffer[90][3]), sizeof(unsigned short));
        return (1);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0
        if (status <= 0)
            return 0;

        // get it from load_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        memcpy(buffer, &(tempBuffer[0]), sizeof(unsigned short));
        return 1;
    }
    return 0;

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
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff;
    int rc = ExtractSlice4LoadPowerDN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
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
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse [90]
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [90]
        memcpy(buffer, &(allBuffer[90][4]), sizeof(unsigned short));
        return (1);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0
        if (status <= 0)
            return 0;

        // get it from load_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        memcpy(buffer, &(tempBuffer[0]), sizeof(unsigned short));
        return 1;
    }
    return 0;

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
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff;
    int rc = ExtractSlice5LoadPowerDN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
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
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse [90]
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [90]
        memcpy(buffer, &(allBuffer[90][5]), sizeof(unsigned short));
        return (1);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0
        if (status <= 0)
            return 0;

        // get it from load_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        memcpy(buffer, &(tempBuffer[0]), sizeof(unsigned short));
        return 1;
    }
    return 0;

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
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff;
    int rc = ExtractSlice6LoadPowerDN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
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
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse [90]
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [90]
        memcpy(buffer, &(allBuffer[90][6]), sizeof(unsigned short));
        return (1);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0
        if (status <= 0)
            return 0;

        // get it from load_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        memcpy(buffer, &(tempBuffer[0]), sizeof(unsigned short));
        return 1;
    }
    return 0;

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
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff;
    int rc = ExtractSlice7LoadPowerDN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
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
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse [90]
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [90]
        memcpy(buffer, &(allBuffer[90][7]), sizeof(unsigned short));
        return (1);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0
        if (status <= 0)
            return 0;

        // get it from load_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        memcpy(buffer, &(tempBuffer[0]), sizeof(unsigned short));
        return 1;
    }
    return 0;

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
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff;
    int rc = ExtractSlice8LoadPowerDN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
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
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse [90]
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [90]
        memcpy(buffer, &(allBuffer[90][8]), sizeof(unsigned short));
        return (1);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0
        if (status <= 0)
            return 0;

        // get it from load_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        memcpy(buffer, &(tempBuffer[0]), sizeof(unsigned short));
        return 1;
    }
    return 0;

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
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff;
    int rc = ExtractSlice9LoadPowerDN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
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
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse [90]
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [90]
        memcpy(buffer, &(allBuffer[90][9]), sizeof(unsigned short));
        return (1);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0
        if (status <= 0)
            return 0;

        // get it from load_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        memcpy(buffer, &(tempBuffer[0]), sizeof(unsigned short));
        return 1;
    }
    return 0;

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
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff;
    int rc = ExtractSlice10LoadPowerDN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
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
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse [90]
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [90]
        memcpy(buffer, &(allBuffer[90][10]), sizeof(unsigned short));
        return (1);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0
        if (status <= 0)
            return 0;

        // get it from load_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        memcpy(buffer, &(tempBuffer[0]), sizeof(unsigned short));
        return 1;
    }
    return 0;

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
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff;
    int rc = ExtractSlice11LoadPowerDN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
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
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // find out the mode first
    unsigned char mode;
    if (l1File->GetDatasetData1D(sdsIDs[0], start, 1, 1, &mode)
                         != HDF_SUCCEED)
        return(-1);

    // CBM: extract pulse [90]
    if (mode == L1_MODE_CBM)
    {
        // CBM: get all 100 pulses from power_dn
        unsigned short allBuffer[100][12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[1];
        if ( ! ExtractData3D_100_12(l1File, tempsdsIDs, start,
                                   1, 1, (VOIDP)allBuffer))
            return (-1);

        // now just pick the [90]
        memcpy(buffer, &(allBuffer[90][11]), sizeof(unsigned short));
        return (1);
    }
    else if (mode == L1_MODE_WOM)
    {
        //---------------------------------------------------------
        // WOM: true cal pulse pos must be > 0 (Cal Frame)
        //---------------------------------------------------------
        char status=0;
        if (l1File->GetDatasetData1D(sdsIDs[2], start, 1, 1, &status)
                        != HDF_SUCCEED)
            return(-1);
        // true cal pulse pos has to be >0
        if (status <= 0)
            return 0;

        // get it from load_cal_power
        unsigned short tempBuffer[12];
        int32 tempsdsIDs[1];
        tempsdsIDs[0] = sdsIDs[3];
        if (ExtractData2D_12(l1File, tempsdsIDs, start, 1, 1,
                        (VOIDP)tempBuffer) != TRUE)
            return (-1);
   
        memcpy(buffer, &(tempBuffer[0]), sizeof(unsigned short));
        return 1;
    }
    return 0;

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
int32       stride,
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff;
    int rc = ExtractSlice12LoadPowerDN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
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
int32       stride,           // stride is ignored, getting one at a time only
int32       length,
VOIDP       buffer)
{
    unsigned short tempBuff=0;
    int rc = ExtractSlice1LoadPowerDN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            // every slice should only contain 1 value
            unsigned int *uintP = (unsigned int*)buffer;
            *uintP = (unsigned int)tempBuff;
            (void)ExtractSlice2LoadPowerDN(l1File, sdsIDs,
                                start, stride, length, &tempBuff);
            *uintP += (unsigned int)tempBuff;
            (void)ExtractSlice3LoadPowerDN(l1File, sdsIDs,
                                start, stride, length, &tempBuff);
            *uintP += (unsigned int)tempBuff;
            (void)ExtractSlice4LoadPowerDN(l1File, sdsIDs,
                                start, stride, length, &tempBuff);
            *uintP += (unsigned int)tempBuff;
            (void)ExtractSlice5LoadPowerDN(l1File, sdsIDs,
                                start, stride, length, &tempBuff);
            *uintP += (unsigned int)tempBuff;
            (void)ExtractSlice6LoadPowerDN(l1File, sdsIDs,
                                start, stride, length, &tempBuff);
            *uintP += (unsigned int)tempBuff;
            (void)ExtractSlice7LoadPowerDN(l1File, sdsIDs,
                                start, stride, length, &tempBuff);
            *uintP += (unsigned int)tempBuff;
            (void)ExtractSlice8LoadPowerDN(l1File, sdsIDs,
                                start, stride, length, &tempBuff);
            *uintP += (unsigned int)tempBuff;
            (void)ExtractSlice9LoadPowerDN(l1File, sdsIDs,
                                start, stride, length, &tempBuff);
            *uintP += (unsigned int)tempBuff;
            (void)ExtractSlice10LoadPowerDN(l1File, sdsIDs,
                                start, stride, length, &tempBuff);
            *uintP += (unsigned int)tempBuff;
            (void)ExtractSlice11LoadPowerDN(l1File, sdsIDs,
                                start, stride, length, &tempBuff);
            *uintP += (unsigned int)tempBuff;
            (void)ExtractSlice12LoadPowerDN(l1File, sdsIDs,
                                start, stride, length, &tempBuff);
            *uintP += (unsigned int)tempBuff;
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
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
VOIDP       buffer)
{
    unsigned int tempBuff;
    int rc = ExtractBeamAPowerDN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float* floatP = (float*)buffer;
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(floatP, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;

} // ExtractTotalLoadPowerdB

//----------------------------------------------------------------------
// Function:    ExtractAverageNoiseLoadDN
// Extracts:    FLOAT4
//----------------------------------------------------------------------
int
ExtractAverageNoiseLoadDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    // these two values are accumulative
    static int frameNo = 0;
    static float prevNoiseDN = 0.0;

    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // get noise load first, return if fails
    unsigned int noiseDN;
    int rc = ExtractNoiseLoadDN(l1File, sdsIDs, start,
                                       stride, length, &noiseDN);
    if (rc <= 0)
        return(rc);

    float newNoiseDN;
    if (frameNo > 0)
        newNoiseDN = 0.9975 * prevNoiseDN + 0.0025 * noiseDN;
    else
        newNoiseDN = 0.0025 * noiseDN;

    frameNo++;
    prevNoiseDN = newNoiseDN;

    memcpy(buffer, &newNoiseDN, sizeof(float));

    return 1;

} // ExtractAverageNoiseLoadDN

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
VOIDP       buffer)
{
    // these two values are accumulative
    static int frameNo = 0;
    static float prevEchoDN = 0.0;

    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // get noise load first, return if fails
    int32 tempsdsIDs[4];
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[3];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[4];  // load_cal_noise
    float newNoiseDN=0.0;
    int rc = ExtractAverageNoiseLoadDN(l1File, tempsdsIDs, start,
                                       stride, length, &newNoiseDN);
    if (rc <= 0)
        return(rc);

    // now get echo load, return if fails
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[2];  // power_dn
    tempsdsIDs[2] = sdsIDs[3];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[5];  // load_cal_power
    unsigned int echoDN;
    rc = ExtractTotalLoadPowerDN(l1File, tempsdsIDs, start,
                                       stride, length, &echoDN);
    if (rc <= 0)
        return(rc);

    float newEchoDN;
    if (frameNo > 0)
    {
        newEchoDN = 0.9975 * prevEchoDN + 0.0025 * echoDN;
    }
    else
    {
        newEchoDN = 0.0025 * echoDN;
    }
    float newValue = 1 / 2.9034 * (newNoiseDN / newEchoDN);
    memcpy(buffer, &newValue, sizeof(float));

    frameNo++;
    prevEchoDN = newEchoDN;

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
VOIDP       buffer)
{
    float tempBuff;
    int rc = ExtractBandwidthRatioDN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
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
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // get noise load first, return if fails
    int32 tempsdsIDs[4];
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[3];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[4];  // loop_back_cal_noise
    unsigned int noiseDN;
    int rc = ExtractBeamANoiseDN(l1File, tempsdsIDs, start,
                                       stride, length, &noiseDN);
    if (rc <= 0)
        return(rc);

    // now get echo load, return if fails
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[2];  // power_dn
    tempsdsIDs[2] = sdsIDs[3];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[5];  // loop_back_cal_A_power
    unsigned int echoDN;
    rc = ExtractBeamAPowerDN(l1File, tempsdsIDs, start,
                                       stride, length, &echoDN);
    if (rc <= 0)
        return(rc);

    float gainRatio = (float)noiseDN / (float)echoDN;
    (void)memcpy(buffer, &gainRatio, sizeof(float));

    return 1;

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
VOIDP       buffer)
{
    float tempBuff;
    int rc = ExtractGainRatioBeamADN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
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
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // get noise load first, return if fails
    int32 tempsdsIDs[4];
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[3];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[4];  // loop_back_cal_noise
    unsigned int noiseDN;
    int rc = ExtractBeamBNoiseDN(l1File, tempsdsIDs, start,
                                       stride, length, &noiseDN);
    if (rc <= 0)
        return(rc);

    // now get echo load, return if fails
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[2];  // power_dn
    tempsdsIDs[2] = sdsIDs[3];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[5];  // loop_back_cal_B_power
    unsigned int echoDN;
    rc = ExtractBeamBPowerDN(l1File, tempsdsIDs, start,
                                       stride, length, &echoDN);
    if (rc <= 0)
        return(rc);

    float gainRatio = (float)noiseDN / (float)echoDN;
    (void)memcpy(buffer, &gainRatio, sizeof(float));

    return 1;

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
VOIDP       buffer)
{
    float tempBuff;
    int rc = ExtractGainRatioBeamBDN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;
} // ExtractGainRatioBeamBdB

//----------------------------------------------------------------------
// Function:    ExtractOneReceiverGainADN
// Extracts:    FLOAT4
//----------------------------------------------------------------------
int
ExtractOneReceiverGainADN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{


    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // get the polynomial table from the ugly global var
    if (ArgsPlus::PolyTable == 0)
    {
        fprintf(stderr, "Receiver Gain A: No polynomial table\n");
        exit(1);
    }
    static const Polynomial* xmitPowerPoly=0;
    if (xmitPowerPoly == 0)
    {
        xmitPowerPoly = ArgsPlus::PolyTable->SelectPolynomial(
                                 "transmit_power_a", "mWatts");
    }
    if (xmitPowerPoly == 0)
    {
        fprintf(stderr, "Receiver Gain A: No polynomial for trasmit power\n");
        exit(1);
    }

    // get noise load first, return if fails
    int32 tempsdsIDs[4];
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[2];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[3];  // loop_back_cal_noise
    unsigned int noiseDN;
    int rc = ExtractBeamANoiseDN(l1File, tempsdsIDs, start,
                                       stride, length, &noiseDN);
    if (rc <= 0)
        return(rc);

    // now get transmit power a, return if fails
    tempsdsIDs[0] = sdsIDs[4];  // transmit power a
    float xmitPowerDN;
    rc = ExtractData1D_uint1_float(l1File, tempsdsIDs, start,
                                       stride, length, &xmitPowerDN);
    if (rc <= 0)
        return(rc);

    float xmitPowerMWatts = xmitPowerPoly->Apply(xmitPowerDN);

    float rcvGain =  ((float)noiseDN) * pow(10.0, 14.79) / xmitPowerMWatts;

    (void)memcpy(buffer, &rcvGain, sizeof(float));
    return 1;

} //ExtractOneReceiverGainADN


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
VOIDP       buffer)
{
    static int32 currentIndex=0;
    static float rcvGainBuf[10];

    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    float rcvGain;
    int rc = ExtractOneReceiverGainADN(l1File, sdsIDs, start,
                                             stride, length, &rcvGain);
    if (rc <= 0) return rc;

    int i=0;
    // for the first 4 measurements, save and return
    if (currentIndex < 4)
    {
        rcvGainBuf[currentIndex] = rcvGain;
        currentIndex++;
        return 0;
    }
    else if (currentIndex == 4)
    {
        rcvGainBuf[4] = rcvGain;
        // extract 5 more measurements: [5],[6],[7],[8],[9]
        for (i=1; i < 6; i++)
        {
            rc = ExtractOneReceiverGainADN(l1File, sdsIDs, start+i,
                                                 stride, length, &rcvGain);
            if (rc != 1) return rc;
            rcvGainBuf[4+i] = rcvGain;
        }
    }
    else
    {
        // extract the [9]
        rc = ExtractOneReceiverGainADN(l1File, sdsIDs, start+5,
                                                 stride, length, &rcvGain);
        if (rc != 1) return rc;
        rcvGainBuf[9] = rcvGain;
    }

    // now add them up and average it over 10
    float totalRcvGain = 0.0;
    for (i=0; i < 9; i++)
        totalRcvGain += rcvGainBuf[i];
    float avgRcvGain = totalRcvGain / 10;
    (void)memcpy(buffer, &avgRcvGain, sizeof(float));

    // save the first 9 rcv gains in the buffer for the next run
    for (i=0; i < 8; i++)
        rcvGainBuf[i] = rcvGainBuf[i+1];
    currentIndex++;

    return 1;

} //ExtractReceiverGainADN

//----------------------------------------------------------------------
// Function:    ExtractOneReceiverGainBDN
// Extracts:    FLOAT4
//----------------------------------------------------------------------
int
ExtractOneReceiverGainBDN(
TlmHdfFile* l1File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer)
{
    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    // get the polynomial table from the ugly global var
    if (ArgsPlus::PolyTable == 0)
    {
        fprintf(stderr, "Receiver Gain B: No polynomial table\n");
        exit(1);
    }
    static const Polynomial* xmitPowerPoly=0;
    if (xmitPowerPoly == 0)
    {
        xmitPowerPoly = ArgsPlus::PolyTable->SelectPolynomial(
                                 "transmit_power_b", "mWatts");
    }
    if (xmitPowerPoly == 0)
    {
        fprintf(stderr, "Receiver Gain B: No polynomial for trasmit power\n");
        exit(1);
    }

    // get noise load first, return if fails
    int32 tempsdsIDs[4];
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[2];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[3];  // loop_back_cal_noise
    unsigned int noiseDN;
    int rc = ExtractBeamBNoiseDN(l1File, tempsdsIDs, start,
                                       stride, length, &noiseDN);
    if (rc <= 0)
        return(rc);

    // now get transmit power a, return if fails
    tempsdsIDs[0] = sdsIDs[4];  // transmit power b
    float xmitPowerDN;
    rc = ExtractData1D_uint1_float(l1File, tempsdsIDs, start,
                                       stride, length, &xmitPowerDN);
    if (rc <= 0)
        return(rc);

    float xmitPowerMWatts = xmitPowerPoly->Apply(xmitPowerDN);

    float rcvGain =  (float)noiseDN  * pow(10.0, 14.825)/ xmitPowerMWatts;

    (void)memcpy(buffer, &rcvGain, sizeof(float));
    return 1;

} //ExtractOneReceiverGainBDN

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
VOIDP       buffer)
{
    static int32 currentIndex=0;
    static float rcvGainBuf[10];

    // extract one at a time
    if (length != 1) return -1;

    assert(l1File != 0);

    float rcvGain;
    int rc = ExtractOneReceiverGainBDN(l1File, sdsIDs, start,
                                             stride, length, &rcvGain);
    if (rc <= 0) return rc;

    // for the first 4 measurements, save and return
    int i=0;
    if (currentIndex < 4)
    {
        rcvGainBuf[currentIndex] = rcvGain;
        currentIndex++;
        return 0;
    }
    else if (currentIndex == 4)
    {
        rcvGainBuf[4] = rcvGain;
        // extract 5 more measurements: [5],[6],[7],[8],[9]
        for (i=1; i < 6; i++)
        {
            rc = ExtractOneReceiverGainBDN(l1File, sdsIDs, start+i,
                                                 stride, length, &rcvGain);
            if (rc != 1) return rc;
            rcvGainBuf[4+i] = rcvGain;
        }
    }
    else
    {
        // extract the [9]
        rc = ExtractOneReceiverGainBDN(l1File, sdsIDs, start+5,
                                                 stride, length, &rcvGain);
        if (rc != 1) return rc;
        rcvGainBuf[9] = rcvGain;
    }

    // now add them up and average it over 10
    float totalRcvGain = 0.0;
    for (i=0; i < 9; i++)
        totalRcvGain += rcvGainBuf[i];
    float avgRcvGain = totalRcvGain / 10;
    (void)memcpy(buffer, &avgRcvGain, sizeof(float));

    // save the first 9 rcv gains in the buffer for the next run
    for (i=0; i < 8; i++)
        rcvGainBuf[i] = rcvGainBuf[i+1];
    currentIndex++;

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
VOIDP       buffer)
{
    float tempBuff;
    int rc = ExtractReceiverGainADN(l1File, sdsIDs,
                           start, stride, length, &tempBuff);
    switch(rc)
    {
        case 1:
        {
            float dBValue = (float) 10 * log10((double)tempBuff);
            (void)memcpy(buffer, &dBValue, sizeof(float));
            return 1;
        }
        case 0:
            return 0;
        default:
            return -1;
    }
    return -1;
} // ExtractReceiverGainAdB


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
VOIDP       buffer)
{
    int rc=0;
    int32 tempsdsIDs[5];
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[2];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[3];  // load_cal_noise
    float avgNoiseLoadDN;
    if ((rc = ExtractAverageNoiseLoadDN(l1File, sdsIDs,
                           start, stride, length, &avgNoiseLoadDN)) != 1)
        return(rc);

    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[2];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[4];  // loop_back_cal_noise
    tempsdsIDs[4] = sdsIDs[5];  // transmit_power_a
    float rcvGainDN;
    if ((rc = ExtractReceiverGainADN(l1File, sdsIDs,
                 start, stride, length, &rcvGainDN)) != 1)
        return(rc);

    float noiseFigDN = avgNoiseLoadDN / (rcvGainDN * NOISE_FIGURE_CONST);
    memcpy(buffer, &noiseFigDN, sizeof(float));
    return 1;

} // ExtractNoiseFigureADN

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
VOIDP       buffer)
{
    int rc=0;
    int32 tempsdsIDs[5];
    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[2];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[3];  // load_cal_noise
    float avgNoiseLoadDN;
    if ((rc = ExtractAverageNoiseLoadDN(l1File, sdsIDs,
                           start, stride, length, &avgNoiseLoadDN)) != 1)
        return(rc);

    tempsdsIDs[0] = sdsIDs[0];  // mode
    tempsdsIDs[1] = sdsIDs[1];  // noise_dn
    tempsdsIDs[2] = sdsIDs[2];  // true_cal_pulse_pos
    tempsdsIDs[3] = sdsIDs[4];  // loop_back_cal_noise
    tempsdsIDs[4] = sdsIDs[5];  // transmit_power_a
    float rcvGainDN;
    if ((rc = ExtractReceiverGainBDN(l1File, sdsIDs,
                 start, stride, length, &rcvGainDN)) != 1)
        return(rc);

    float noiseFigDN = avgNoiseLoadDN / (rcvGainDN * NOISE_FIGURE_CONST);
    memcpy(buffer, &noiseFigDN, sizeof(float));
    return 1;

} // ExtractNoiseFigureBDN
