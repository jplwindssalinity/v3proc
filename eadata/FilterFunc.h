//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.0   04 May 1998 10:53:48   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#ifndef FILTERFUNC_H
#define FILTERFUNC_H

static const char rcs_id_filterfunc_h[] = 
    "@(#) $Header$";

//---------------------
// Filter Functions 
//---------------------

char Filter_Standby_Mode(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_Rx_Only_Mode(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_Calibration_Mode(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_Wind_Obs_Mode(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_NonCal_Frame(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_Cal_Frame(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_TWT_On(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_TWT_Off(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_Beam_A(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_Beam_B(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_TWTA_1(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_TWTA_2(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_SAS_A(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_SAS_B(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_SES_A(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_SES_B(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_Modulation_On(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_Modulation_Off(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_Rx_Protect_On(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_Rx_Protect_Off(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_Grid_Normal(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_Grid_Dsbl(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_SAS_A_Spin19_8(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_SAS_A_Spin18_0(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_SAS_B_Spin19_8(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
char Filter_SAS_B_Spin18_0(Filter* filterP, TlmHdfFile* tlmFile, int32 startIndex);
 
#endif // FILTERFUNC_H
