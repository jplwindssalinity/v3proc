//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   23 Jul 1998 16:13:12   sally
// pass polynomial table to extractFunc()
// 
//    Rev 1.0   04 May 1998 10:53:46   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

static const char rcs_id_FilterFunc_C[] = "@(#) $Header$";

#include "assert.h"
#include "Filter.h"
#include "L1AExtract.h"
#include "Parameter.h"
#include "ParTab.h"

//============================================================
// Filter_Standby_Mode
// TRUE when the instrument is in Standby Mode
//============================================================

char
Filter_Standby_Mode(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    unsigned char mode;
    if (param1->extractFunc(tlmFile, param1->sdsIDs,
           startIndex, 1, 1, &mode, 0) && mode == L1_MODE_SBM)
        return (1);
    else
        return (0);

}//Filter_Standby_Mode

//============================================================
// Filter_Rx_Only_Mode
// TRUE when the instrument is in Receive Only Mode
//============================================================


char
Filter_Rx_Only_Mode(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);
 
    unsigned char mode;
    if (param1->extractFunc(tlmFile, param1->sdsIDs,
           startIndex, 1, 1, &mode, 0) && mode == L1_MODE_ROM)
        return (1);
    else
        return (0);

}//Filter_Rx_Only_Mode

//============================================================
// Filter_Calibration_Mode
// TRUE when the instrument is in Calibration Mode
//============================================================

char
Filter_Calibration_Mode(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);
 
    unsigned char mode;
    if (param1->extractFunc(tlmFile, param1->sdsIDs,
           startIndex, 1, 1, &mode, 0) && mode == L1_MODE_CBM)
        return (1);
    else
        return (0);

}//Filter_Calibration_Mode

//============================================================
// Filter_Wind_Obs_Mode
// TRUE when the instrument is in Wind Observation Mode
//============================================================

char
Filter_Wind_Obs_Mode(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);
 
    unsigned char mode;
    if (param1->extractFunc(tlmFile, param1->sdsIDs,
           startIndex, 1, 1, &mode, 0) && mode == L1_MODE_WOM)
        return (1);
    else
        return (0);

}//Filter_Wind_Obs_Mode

//============================================================
// Filter_Sci_Frame
// TRUE when the telemetry frame is a science frame
//============================================================

char
Filter_NonCal_Frame(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    char status=0;
    if (param1->extractFunc(tlmFile, param1->sdsIDs, startIndex,
               1, 1, &status, 0) && status <= 0)
        return (1);
    else
        return (0);

}//Filter_NonCal_Frame

//============================================================
// Filter_Cal_Frame
// TRUE when the telemetry frame is a calibration frame
//============================================================

char
Filter_Cal_Frame(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    char status=0;
    if (param1->extractFunc(tlmFile, param1->sdsIDs, startIndex,
               1, 1, &status, 0) && status > 0)
        return (1);
    else
        return (0);

}//Filter_Cal_Frame

//============================================================
// Filter_TWT_On
// TRUE when the TWT(HVPS) is on
//============================================================

char
Filter_TWT_On(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);
 
    Parameter* param2 = filterP->parametersP[1];
    assert(param2 != 0 && param2->sdsIDs != 0 && param2->sdsIDs[0] != HDF_FAIL);

    unsigned char k9, k10;
    if ( ! param1->extractFunc(tlmFile, param1->sdsIDs, startIndex,
               1, 1, &k9, 0))
        return 0;

    if ( ! param2->extractFunc(tlmFile, param2->sdsIDs, startIndex,
               1, 1, &k10, 0))
        return 0;

    return(k9 == k10 ? 1 : 0);

}//Filter_TWT_On

//============================================================
// Filter_TWT_Off
// TRUE when the TWT is off
//============================================================

char
Filter_TWT_Off(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);
 
    Parameter* param2 = filterP->parametersP[1];
    assert(param2 != 0 && param2->sdsIDs != 0 && param2->sdsIDs[0] != HDF_FAIL);

    unsigned char k9, k10;
    if ( ! param1->extractFunc(tlmFile, param1->sdsIDs, startIndex,
               1, 1, &k9, 0))
        return 0;

    if ( ! param2->extractFunc(tlmFile, param2->sdsIDs, startIndex,
               1, 1, &k10, 0))
        return 0;

    return(k9 != k10 ? 1 : 0);

}//Filter_TWT_Off


//============================================================
// Filter_Beam_A
// TRUE when the Beam A is selected
//============================================================

char
Filter_Beam_A(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    unsigned char status=0;
    if (param1->extractFunc(tlmFile, param1->sdsIDs, startIndex,
               1, 1, &status, 0) && status == L1_BEAM_A)
        return (1);
    else
        return (0);

}//Filter_Beam_A

//============================================================
// Filter_Beam_B
// TRUE when the Beam B is selected
//============================================================

char
Filter_Beam_B(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    unsigned char status=0;
    if (param1->extractFunc(tlmFile, param1->sdsIDs, startIndex,
               1, 1, &status, 0) && status == L1_BEAM_B)
        return (1);
    else
        return (0);

}//Filter_Beam_B

//============================================================
// Filter_TWTA_1
// TRUE when the TWTA 1 is selected
//============================================================

char
Filter_TWTA_1(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    Parameter* param2 = filterP->parametersP[1];
    assert(param2 != 0 && param2->sdsIDs != 0 && param2->sdsIDs[0] != HDF_FAIL);

    unsigned char k11, k12;
    if ( ! param1->extractFunc(tlmFile, param1->sdsIDs, startIndex,
               1, 1, &k11, 0))
        return 0;

    if ( ! param2->extractFunc(tlmFile, param2->sdsIDs, startIndex,
               1, 1, &k12, 0))
        return 0;

    return(k11 == k12 ? 1 : 0);

}//Filter_TWTA_1

//============================================================
// Filter_TWTA_2
// TRUE when the TWTA 2 is selected
//============================================================

char
Filter_TWTA_2(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);
 
    Parameter* param2 = filterP->parametersP[1];
    assert(param2 != 0 && param2->sdsIDs != 0 && param2->sdsIDs[0] != HDF_FAIL);

    unsigned char k11, k12;
    if ( ! param1->extractFunc(tlmFile, param1->sdsIDs, startIndex,
               1, 1, &k11, 0))
        return 0;

    if ( ! param2->extractFunc(tlmFile, param2->sdsIDs, startIndex,
               1, 1, &k12, 0))
        return 0;

    return(k11 != k12 ? 1 : 0);

}//Filter_TWTA_2

//============================================================
// Filter_SAS_A
// TRUE when the SAS A is selected
//============================================================

char
Filter_SAS_A(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);
 
    Parameter* param2 = filterP->parametersP[1];
    assert(param2 != 0 && param2->sdsIDs != 0 && param2->sdsIDs[0] != HDF_FAIL);

    unsigned char k19, k20;
    if ( ! param1->extractFunc(tlmFile, param1->sdsIDs, startIndex,
               1, 1, &k19, 0))
        return 0;

    if ( ! param2->extractFunc(tlmFile, param2->sdsIDs, startIndex,
               1, 1, &k20, 0))
        return 0;

    return(k19 == k20 ? 1 : 0);

}//Filter_SAS_A

//============================================================
// Filter_SAS_B
// TRUE when the SAS B is selected
//============================================================

char
Filter_SAS_B(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);
 
    Parameter* param2 = filterP->parametersP[1];
    assert(param2 != 0 && param2->sdsIDs != 0 && param2->sdsIDs[0] != HDF_FAIL);

    unsigned char k19, k20;
    if ( ! param1->extractFunc(tlmFile, param1->sdsIDs, startIndex,
               1, 1, &k19, 0))
        return 0;

    if ( ! param2->extractFunc(tlmFile, param2->sdsIDs, startIndex,
               1, 1, &k20, 0))
        return 0;

    return(k19 != k20 ? 1 : 0);

}//Filter_SAS_B

//============================================================
// Filter_SES_A
// TRUE when the SES A is selected
//============================================================

char
Filter_SES_A(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);
 
    Parameter* param2 = filterP->parametersP[1];
    assert(param2 != 0 && param2->sdsIDs != 0 && param2->sdsIDs[0] != HDF_FAIL);

    unsigned char k15, k16;
    if ( ! param1->extractFunc(tlmFile, param1->sdsIDs, startIndex,
               1, 1, &k15, 0))
        return 0;

    if ( ! param2->extractFunc(tlmFile, param2->sdsIDs, startIndex,
               1, 1, &k16, 0))
        return 0;

    return(k15 == k16 ? 1 : 0);

}//Filter_SES_A

//============================================================
// Filter_SES_B
// TRUE when the SES B is selected
//============================================================

char
Filter_SES_B(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);
 
    Parameter* param2 = filterP->parametersP[1];
    assert(param2 != 0 && param2->sdsIDs != 0 && param2->sdsIDs[0] != HDF_FAIL);

    unsigned char k15, k16;
    if ( ! param1->extractFunc(tlmFile, param1->sdsIDs, startIndex,
               1, 1, &k15, 0))
        return 0;

    if ( ! param2->extractFunc(tlmFile, param2->sdsIDs, startIndex,
               1, 1, &k16, 0))
        return 0;

    return(k15 != k16 ? 1 : 0);

}//Filter_SES_B

//============================================================
// Filter_Modulation_On
// TRUE when the Modulation is on
//============================================================

char
Filter_Modulation_On(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    unsigned char modulation;
    if (param1->extractFunc(tlmFile, param1->sdsIDs,
           startIndex, 1, 1, &modulation, 0) && modulation == L1_MODULATION_ON)
        return (1);
    else
        return (0);

}//Filter_Modulation_On

//============================================================
// Filter_Modulation_Off
// TRUE when the Modulation is off
//============================================================

char
Filter_Modulation_Off(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    unsigned char modulation;
    if (param1->extractFunc(tlmFile, param1->sdsIDs,
           startIndex, 1, 1, &modulation, 0) && modulation == L1_MODULATION_OFF)
        return (1);
    else
        return (0);

}//Filter_Modulation_Off

//============================================================
// Filter_Rx_Protect_On
// TRUE when the Receive Protect is on
//============================================================

char
Filter_Rx_Protect_On(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    unsigned char flag;
    if (param1->extractFunc(tlmFile, param1->sdsIDs,
           startIndex, 1, 1, &flag, 0) && flag == L1_RX_PROTECT_ON)
        return (1);
    else
        return (0);

}//Filter_Rx_Protect_On

//============================================================
// Filter_Rx_Protect_Off
// TRUE when the Receive Protect is off
//============================================================

char
Filter_Rx_Protect_Off(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    unsigned char flag;
    if (param1->extractFunc(tlmFile, param1->sdsIDs,
           startIndex, 1, 1, &flag, 0) && flag == L1_RX_PROTECT_OFF)
        return (1);
    else
        return (0);

}//Filter_Rx_Protect_Off

//============================================================
// Filter_Grid_Normal
// TRUE when the Grid is normal
//============================================================

char
Filter_Grid_Normal(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    unsigned char flag;
    if (param1->extractFunc(tlmFile, param1->sdsIDs,
           startIndex, 1, 1, &flag, 0) && flag == L1_GRID_NORMAL)
        return (1);
    else
        return (0);

}//Filter_Grid_Normal

//============================================================
// Filter_Grid_Dsbl
// TRUE when the Grid is disabled
//============================================================

char
Filter_Grid_Dsbl(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    unsigned char flag;
    if (param1->extractFunc(tlmFile, param1->sdsIDs,
           startIndex, 1, 1, &flag, 0) && flag == L1_GRID_DISABLE)
        return (1);
    else
        return (0);

}//Filter_Grid_Dsbl

//============================================================
// Filter_SAS_A_Spin19_8
// TRUE when the SAS A spin rate is 19.8 rpm
//============================================================

char
Filter_SAS_A_Spin19_8(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    unsigned char flag;
    if (param1->extractFunc(tlmFile, param1->sdsIDs,
           startIndex, 1, 1, &flag, 0) && flag == L1_SAS_A_SPIN_19_8)
        return (1);
    else
        return (0);

}//Filter_SAS_A_Spin19_8

//============================================================
// Filter_SAS_A_Spin18_0
// TRUE when the SAS A spin rate is 18.0 rpm
//============================================================

char
Filter_SAS_A_Spin18_0(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    unsigned char flag;
    if (param1->extractFunc(tlmFile, param1->sdsIDs,
           startIndex, 1, 1, &flag, 0) && flag == L1_SAS_A_SPIN_18_0)
        return (1);
    else
        return (0);

}//Filter_SAS_A_Spin18_0

//============================================================
// Filter_SAS_B_Spin19_8
// TRUE when the SAS B spin rate is 19.8 rpm
//============================================================

char
Filter_SAS_B_Spin19_8(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    unsigned char flag;
    if (param1->extractFunc(tlmFile, param1->sdsIDs,
           startIndex, 1, 1, &flag, 0) && flag == L1_SAS_B_SPIN_19_8)
        return (1);
    else
        return (0);

}//Filter_SAS_B_Spin19_8

//============================================================
// Filter_SAS_B_Spin18_0
// TRUE when the SAS B spin rate is 18.0 rpm
//============================================================

char
Filter_SAS_B_Spin18_0(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    unsigned char flag;
    if (param1->extractFunc(tlmFile, param1->sdsIDs,
           startIndex, 1, 1, &flag, 0) && flag == L1_SAS_B_SPIN_18_0)
        return (1);
    else
        return (0);

}//Filter_SAS_B_Spin18_0
