//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.8   11 Sep 1998 10:29:14   sally
// add mWatts for all dBm units
// 
//    Rev 1.7   27 Jul 1998 13:59:36   sally
// passing polynomial table to extraction function
// 
//    Rev 1.6   23 Jul 1998 16:13:54   sally
// pass polynomial table to extractFunc()
// 
//    Rev 1.5   19 Jun 1998 16:54:12   sally
// added "Orbit Period" in L1A Derived Data  
// 
//    Rev 1.4   06 Apr 1998 16:27:18   sally
// merged with SVT
// 
//    Rev 1.3   01 Apr 1998 13:34:08   sally
// added comments of return values
// 
//    Rev 1.2   30 Mar 1998 15:13:46   sally
// added L2A parameter table
// 
//    Rev 1.1   27 Mar 1998 09:58:48   sally
// added L1A Derived data
// 
//    Rev 1.0   24 Mar 1998 16:02:32   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef L1ADRVEXTRACT_H
#define L1ADRVEXTRACT_H

static const char rcs_id_l1a_drv_extract_h[] =
    "@(#) $Header$";

#include <mfhdf.h>

#include "CommonDefs.h"
#include "Itime.h"
#include "TlmHdfFile.h"
#include "PolyTable.h"

//-------------------------------------------------------------
// return:
//    25: 25 values, each value is to be time spaced
//     1: 1 value 
//     0: no value
//    -1: error occured
//-------------------------------------------------------------

int ExtractBeamANoiseDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamANoisedB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBNoiseDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBNoisedB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);

int ExtractBeamASlice1DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice1dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice2DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice2dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice3DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice3dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice4DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice4dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice5DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice5dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice6DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice6dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice7DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice7dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice8DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice8dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice9DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice9dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice10DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice10dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice11DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice11dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice12DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamASlice12dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);

int ExtractBeamBSlice1DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice1dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice2DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice2dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice3DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice3dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice4DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice4dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice5DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice5dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice6DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice6dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice7DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice7dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice8DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice8dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice9DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice9dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice10DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice10dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice11DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice11dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice12DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBSlice12dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamAPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamAPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBeamBPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractNoiseLoadDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractNoiseLoaddB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice1LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice1LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice2LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice2LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice3LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice3LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice4LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice4LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice5LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice5LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice6LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice6LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice7LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice7LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice8LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice8LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice9LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice9LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice10LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice10LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice11LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice11LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice12LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractSlice12LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractTotalLoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractTotalLoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBandwidthRatioDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractBandwidthRatiodB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractGainRatioBeamADN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractGainRatioBeamAdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractGainRatioBeamBDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractGainRatioBeamBdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractOneReceiverGainADN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable);
int ExtractReceiverGainADN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable);
int ExtractOneReceiverGainBDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable);
int ExtractReceiverGainBDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable);
int ExtractReceiverGainAdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable);
int ExtractReceiverGainBdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable);
int ExtractNoiseFigureADN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable);
int ExtractNoiseFigureAdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable);
int ExtractNoiseFigureBDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable);
int ExtractNoiseFigureBdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable);
int ExtractOrbitPeriod(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractAverageNoiseLoadDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractAntSpinRateDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractAntSpinRateDegree(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable=0);
int ExtractAntSpinRateDegSec(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable);
int ExtractAntSpinRateRotMin(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable);
int ExtractTwt1PowermWatts(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable);
int ExtractTwt2PowermWatts(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable);
int ExtractPowerCnvtCurrmWatts(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable);
int ExtractXmitPwrInnermWatts(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable);
int ExtractXmitPwrOutermWatts(TlmHdfFile*, int32*, int32, int32, int32, VOIDP, 
                                            PolynomialTable* polyTable);

#endif //L1ADRVEXTRACT_H
