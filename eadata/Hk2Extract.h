//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   07 Jul 1999 13:23:24   sally
// add a few more extraction functions
// 
//    Rev 1.0   08 Sep 1998 16:25:56   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef HK2EXTRACT_H
#define HK2EXTRACT_H

static const char rcs_id_hk2_extract_h[] =
    "@(#) $Header$";

#include <mfhdf.h>

#include "CommonDefs.h"
#include "Itime.h"
#include "TlmHdfFile.h"

 
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

int ExtractData1D_Odd (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int ExtractData1D_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int ExtractData1D_int2_float_Odd (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int ExtractData1D_int2_float_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int ExtractData1D_int1_float_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int ExtractData1D_uint1_float_Odd (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int ExtractData1D_uint1_float_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int ExtractData1D_uint2_float_Odd (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int ExtractData1D_uint2_float_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit0_Odd (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit0_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit1_Odd (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit1_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit2_Odd (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit2_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit3_Odd (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit3_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit4_Odd (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit4_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit5_Odd (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit5_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit6_Odd (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit6_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit7_Odd (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit7_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit5_6_Odd (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit0_1_Odd (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit2_3_Odd (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit6_7_Odd (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit4_5_Odd (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit6_7_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit5_7_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit4_5_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit3_7_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit0_3_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);
int Extract8Bit1_3_Even (TlmHdfFile*, int32*, int32, int32, int32,
                                      VOIDP, PolynomialTable* polyTable=0);


#endif //HK2EXTRACT_H
