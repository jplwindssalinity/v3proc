//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
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

//-------------------------------------------------------------
// return:
//    25: 25 values, each value is to be time spaced
//     1: 1 value 
//     0: no value
//    -1: error occured
//-------------------------------------------------------------

int ExtractBeamANoiseDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamANoisedB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBNoiseDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBNoisedB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);

int ExtractBeamASlice1DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice1dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice2DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice2dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice3DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice3dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice4DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice4dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice5DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice5dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice6DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice6dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice7DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice7dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice8DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice8dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice9DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice9dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice10DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice10dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice11DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice11dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice12DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamASlice12dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);

int ExtractBeamBSlice1DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice1dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice2DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice2dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice3DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice3dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice4DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice4dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice5DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice5dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice6DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice6dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice7DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice7dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice8DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice8dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice9DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice9dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice10DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice10dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice11DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice11dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice12DN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBSlice12dB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamAPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamAPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBeamBPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractNoiseLoadDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractNoiseLoaddB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice1LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice1LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice2LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice2LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice3LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice3LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice4LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice4LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice5LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice5LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice6LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice6LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice7LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice7LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice8LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice8LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice9LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice9LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice10LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice10LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice11LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice11LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice12LoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractSlice12LoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractTotalLoadPowerDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractTotalLoadPowerdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBandwidthRatioDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractBandwidthRatiodB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractGainRatioBeamADN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractGainRatioBeamAdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractGainRatioBeamBDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractGainRatioBeamBdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractOneReceiverGainADN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractReceiverGainADN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractOneReceiverGainBDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractReceiverGainBDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractReceiverGainAdB(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractNoiseFigureADN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
int ExtractNoiseFigureBDN(TlmHdfFile*, int32*, int32, int32, int32, VOIDP);

#endif //L1ADRVEXTRACT_H
