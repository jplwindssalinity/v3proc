//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.2   01 May 1998 16:45:24   sally
// add some more filters, and changed some names, per Lee Poulsen
// 
//    Rev 1.1   20 Feb 1998 10:58:24   sally
// L1 to L1A
// 
//    Rev 1.0   04 Feb 1998 14:16:00   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:24  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef L1AFILTER_H
#define L1AFILTER_H

static const char rcs_id_l1_filter_h[] = "@(#) $Header$";

//---------------------
// L1A Filter Functions 
//---------------------

char L1AF_Standby_Mode(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_Rx_Only_Mode(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_Calibration_Mode(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_Wind_Obs_Mode(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_NonCal_Frame(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_Cal_Frame(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_TWT_On(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_TWT_Off(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_Beam_A(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_Beam_B(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_TWTA_1(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_TWTA_2(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_SAS_A(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_SAS_B(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_SES_A(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_SES_B(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_Modulation_On(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_Modulation_Off(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_Rx_Protect_On(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_Rx_Protect_Off(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_Grid_Normal(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_Grid_Dsbl(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_SAS_A_Spin19_8(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_SAS_A_Spin18_0(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_SAS_B_Spin19_8(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char L1AF_SAS_B_Spin18_0(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
 
#endif // L1AFILTER_H
