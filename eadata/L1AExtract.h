//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.17   16 Oct 1998 09:04:46   sally
// added ExtractL1Time
// 
//    Rev 1.16   13 Oct 1998 15:33:02   sally
// added L1B file
// 
//    Rev 1.15   24 Sep 1998 15:54:58   sally
// add one extract function for full_frame
// 
//    Rev 1.14   08 Sep 1998 16:25:10   sally
// added HK2 FSW subcoms
// 
//    Rev 1.13   13 Aug 1998 16:26:22   sally
// 
//    Rev 1.12   04 Aug 1998 15:59:44   sally
// fixe L1AParTab so that dBm comes from polynomial table and
// mWatts will be calculated
// 
//    Rev 1.11   27 Jul 1998 14:00:14   sally
// passing polynomial table to extraction function
// 
//    Rev 1.10   23 Jul 1998 16:14:04   sally
// pass polynomial table to extractFunc()
// 
//    Rev 1.9   22 Jun 1998 15:26:00   sally
// change to incorporate Barry's update
// 
//    Rev 1.8   19 Jun 1998 14:02:34   sally
// add some bit extraction functions needed by HK2
// 
//    Rev 1.7   03 Jun 1998 10:09:56   sally
// change parameter names and types due to LP's changes
// 
//    Rev 1.6   20 Apr 1998 10:21:28   sally
// change for WindSwatch
// 
//    Rev 1.5   17 Apr 1998 16:48:46   sally
// add L2A and L2B parameter tables
// 
//    Rev 1.4   06 Apr 1998 16:28:04   sally
// merged with SVT
// 
//    Rev 1.3   30 Mar 1998 15:13:54   sally
// added L2A parameter table
// 
//    Rev 1.2   23 Mar 1998 15:35:08   sally
// adapt to derived science data
// 
//    Rev 1.1   20 Feb 1998 10:57:32   sally
// L1 to L1A
// 
//    Rev 1.0   04 Feb 1998 14:15:50   daffer
// Initial checking
// Revision 1.4  1998/02/01 20:21:11  sally
// fixed 2D conversion
//
// Revision 1.3  1998/01/31 00:36:39  sally
// add scale factor
//
// Revision 1.2  1998/01/30 22:28:22  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef L1AEXTRACT_H
#define L1AEXTRACT_H

static const char rcs_id_l1_extract_h[] =
    "@(#) $Header$";

#include <mfhdf.h>

#include "CommonDefs.h"
#include "Itime.h"
#include "TlmHdfFile.h"

#define ORBIT_TIME_TO_MINUTES   (1.0/61440.0)       // 1 / (1024 * 60)
#define ORBIT_TIME_TO_SECONDS   (1.0/1024.0)
#define INST_TIME_TO_DAYS       (1.0/2764800.0)     // 1 / (32 * 60 * 60 * 24)
#define INST_TIME_TO_MS         (1000.0/32.0)

//=====> DEFINES FOR RFS TRIP EXTRACTOR <====
#define TRIP1MSB 0X8C18
#define TRIP1LSB 0X8C19
#define TRIP2MSB 0X8CDC
#define TRIP2LSB 0X8CDD
#define TRIP3MSB 0X8DA0
#define TRIP3LSB 0X8DA1

#define MEMORY_DATA_SIZE        28
#define BINNING_CONSTANTS_SIZE  1504
#define BC_START_ADDR_U1        0xe002  // uplink 1
#define BC_END_ADDR_U1          0xe5e2
#define BC_START_ADDR_U2        0xe5e4  // uplink 2
#define BC_END_ADDR_U2          0xebc4
#define BC_START_ADDR_DF        0x79ba  // default
#define BC_END_ADDR_DF          0x7f9a
#define BC_START_ADDR_DB        0x73d8  // debug
#define BC_END_ADDR_DB          0x79b6

#define L1A_DATA_TYPE    "L1A"

//----------------------------------
// L1A DATA HEADER Special Functions 
//----------------------------------

int     L1ADH_IsL1(const char* header);
Itime   L1ADH_First_Rev_Eq_Crossing_Time(const char* header);

//--------------------------
// L1A DATA HEADER Functions 
//--------------------------

char L1ADH_Get_Value(const char* header, const char* keyword, char* data);
char L1ADH_Complete(const char* header, char* data);
char L1ADH_First_Data_Time(const char* header, char* data);
char L1ADH_Last_Data_Time(const char* header, char* data);
char L1ADH_Data_Type(const char* header, char* data);
char L1ADH_First_Rev_Eq_Crossing_Lon(const char* header, char* data);
char L1ADH_First_Rev_Eq_Crossing_Time(const char* header, char* data);

//----------------------
// L1A Special Functions 
//----------------------

Itime   L1A_Itime(const char* frame);
int     L1A_HVPS_On(const char* frame);
char    L1A_BC_Accumulate(char* binning_constants, char* flag,
            unsigned short memory_address, char* memory_data,
            unsigned short bc_start_address, unsigned short bc_end_address);
 
//------------------------------------------
// L1A DATA RECORD Functions: Basic Elements 
// parameters:
//    TlmHdfFile*       l1file    IN
//    int32(long int)   sdsID     IN
//    int32             start     IN
//    int32             stride    IN
//    int32             length    IN
//    VOIDP(void*)      buffer    OUT
//    PolynomialTable*  polyTable IN
//------------------------------------------

int ExtractData1D        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);

int ExtractData1D_uint1_float(TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData1D_int2_float(TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData1D_uint2_float(TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData1D_uint4_float(TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);

int ExtractData1D_int_char3  (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);


int ExtractData2D_4      (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData2D_5      (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData2D_8      (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData2D_12     (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData2D_13     (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData2D_16     (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData2D_76     (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData2D_49     (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData2D_100    (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData2D_810    (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData2D_76_uint2_float(TlmHdfFile*, int32*,int32,int32,int32,VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData2D_76_int2_float(TlmHdfFile*, int32*,int32,int32,int32,VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData2D_100_uint2_float (TlmHdfFile*, int32*, int32, int32, int32,
                                          VOIDP, PolynomialTable* polyTable=0);
int ExtractData2D_100_int2_float (TlmHdfFile*, int32*, int32, int32, int32,
                                          VOIDP, PolynomialTable* polyTable=0);
int ExtractData2D_810_uint2_float(TlmHdfFile*, int32*,int32,int32,int32,VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData2D_810_int2_float(TlmHdfFile*, int32*,int32,int32,int32,VOIDP,
                                          PolynomialTable* polyTable=0);


int ExtractData3D_2_8    (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData3D_76_4_uint2_float(TlmHdfFile*, int32*,int32,int32,int32,VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData3D_76_4_int2_float(TlmHdfFile*, int32*,int32,int32,int32,VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData3D_100_12 (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractData3D_100_8_int2_float (TlmHdfFile*, int32*, int32, int32,
                                  int32, VOIDP, PolynomialTable* polyTable=0);
int ExtractData3D_100_8_uint2_float (TlmHdfFile*, int32*, int32, int32,
                                  int32, VOIDP, PolynomialTable* polyTable=0);

int Extract1of8Bits(TlmHdfFile*,
                    int32* sdsIDs,
                    int32           start,
                    int32           stride,
                    int32           length,
                    unsigned char   whichBit,   // which bit to extract
                    VOIDP           buffer,
                    PolynomialTable*);

int ExtractSomeOf8Bits(
                    TlmHdfFile*,
                    int32*          sdsIDs,
                    int32           start,
                    int32           stride,
                    int32           length,
                    unsigned char   leftMostBit,
                    unsigned char   numBits,
                    VOIDP           buffer,
                    PolynomialTable*);

int Extract8Bit0         (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit1         (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit2         (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit3         (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit4         (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit5         (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit6         (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit7         (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit0_1       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit0_3       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit0_4       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit0_6       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit1_2       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit2_3       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit3_7       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit4_5       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit4_6       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit5_6       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit5_7       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract8Bit6_7       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);

int Extract16Bit0        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract16Bit1        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract16Bit2        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract16Bit3        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract16Bit4        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract16Bit5        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract16Bit6        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract16Bit7        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract16Bit8        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract16Bit9        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract16Bit10       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract16Bit11       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract16Bit12       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract16Bit13       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract16Bit14       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract16Bit15       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract16Bit0_1      (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract16Bit2_3      (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);

int Extract32Bit0        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit1        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit2        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit3        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit4        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit5        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit6        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit7        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit8        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit9        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit10       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit11       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit12       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit13       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit14       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit15       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit16       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit17       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit18       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit19       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit20       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit21       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit22       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit23       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit24       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit25       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit26       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit27       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit28       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);

int Extract32Bit0_1      (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract32Bit4_6      (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);

int ExtractTaiTime       (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);

int ExtractL1Time        (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);

int ExtractXmitPowerAdBm (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractXmitPowerBdBm (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);

int Extract_uint1_eu_dB  (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          const char* sdsName,
                                          const char* unitName,
                                          PolynomialTable* polyTable=0);

int ExtractXmitPowerAmWatts (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int ExtractXmitPowerBmWatts (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);
int Extract_uint1_eu_mWatts  (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          const char* sdsName,
                                          const char* unitName,
                                          PolynomialTable* polyTable=0);

//--------------------------------------------
// some general extract+converson functions
//--------------------------------------------
int ExtractData1D_m_km   (TlmHdfFile*, int32*, int32, int32, int32, VOIDP,
                                          PolynomialTable* polyTable=0);

#endif //L1AEXTRACT_H
