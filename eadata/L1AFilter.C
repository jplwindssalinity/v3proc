//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.4   23 Jul 1998 16:14:42   sally
// pass polynomial table to extractFunc()
// 
//    Rev 1.3   01 May 1998 16:45:20   sally
// add some more filters, and changed some names, per Lee Poulsen
// 
//    Rev 1.2   23 Mar 1998 15:36:02   sally
// adapt to derived science data
// 
//    Rev 1.1   20 Feb 1998 10:58:20   sally
// L1 to L1A
// 
//    Rev 1.0   04 Feb 1998 14:15:58   daffer
// Initial checking
// Revision 1.3  1998/01/30 22:29:11  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

static const char rcs_id_L1AFilter_C[] = "@(#) $Header$";

#include "assert.h"
#include "Filter.h"
#include "L1AExtract.h"
#include "Parameter.h"
#include "ParTab.h"

//============================================================
// L1AF_Standby_Mode
// TRUE when the instrument is in Standby Mode
//============================================================

char
L1AF_Standby_Mode(
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

}//L1AF_Standby_Mode

//============================================================
// L1AF_Rx_Only_Mode
// TRUE when the instrument is in Receive Only Mode
//============================================================


char
L1AF_Rx_Only_Mode(
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

}//L1AF_Rx_Only_Mode

//============================================================
// L1AF_Calibration_Mode
// TRUE when the instrument is in Calibration Mode
//============================================================

char
L1AF_Calibration_Mode(
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

}//L1AF_Calibration_Mode

//============================================================
// L1AF_Wind_Obs_Mode
// TRUE when the instrument is in Wind Observation Mode
//============================================================

char
L1AF_Wind_Obs_Mode(
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

}//L1AF_Wind_Obs_Mode

//============================================================
// L1AF_Sci_Frame
// TRUE when the telemetry frame is a science frame
//============================================================

char
L1AF_NonCal_Frame(
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

}//L1AF_NonCal_Frame

//============================================================
// L1AF_Cal_Frame
// TRUE when the telemetry frame is a calibration frame
//============================================================

char
L1AF_Cal_Frame(
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

}//L1AF_Cal_Frame

//============================================================
// L1AF_TWT_On
// TRUE when the HVPS is on
//============================================================

char
L1AF_TWT_On(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    unsigned char status=0;
    if (param1->extractFunc(tlmFile, param1->sdsIDs, startIndex,
               1, 1, &status, 0) && status == L1_TWT_ON)
        return (1);
    else
        return (0);

}//L1AF_TWT_On

//============================================================
// L1AF_TWT_Off
// TRUE when the TWT is off
//============================================================

char
L1AF_TWT_Off(
Filter*        filterP,
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    assert(filterP != 0);
    Parameter* param1 = filterP->parametersP[0];
    assert(param1 != 0 && param1->sdsIDs != 0 && param1->sdsIDs[0] != HDF_FAIL);

    unsigned char status=0;
    if (param1->extractFunc(tlmFile, param1->sdsIDs, startIndex,
               1, 1, &status, 0) && status == L1_TWT_OFF)
        return (1);
    else
        return (0);

}//L1AF_TWT_Off


//============================================================
// L1AF_Beam_A
// TRUE when the Beam A is selected
//============================================================

char
L1AF_Beam_A(
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

}//L1AF_Beam_A

//============================================================
// L1AF_Beam_B
// TRUE when the Beam B is selected
//============================================================

char
L1AF_Beam_B(
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

}//L1AF_Beam_B

//============================================================
// L1AF_TWTA_1
// TRUE when the TWTA 1 is selected
//============================================================

char
L1AF_TWTA_1(
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

}//L1AF_TWTA_1

//============================================================
// L1AF_TWTA_2
// TRUE when the TWTA 2 is selected
//============================================================

char
L1AF_TWTA_2(
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

}//L1AF_TWTA_2

//============================================================
// L1AF_SAS_A
// TRUE when the SAS A is selected
//============================================================

char
L1AF_SAS_A(
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

}//L1AF_SAS_A

//============================================================
// L1AF_SAS_B
// TRUE when the SAS B is selected
//============================================================

char
L1AF_SAS_B(
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

}//L1AF_SAS_B

//============================================================
// L1AF_SES_A
// TRUE when the SES A is selected
//============================================================

char
L1AF_SES_A(
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

}//L1AF_SES_A

//============================================================
// L1AF_SES_B
// TRUE when the SES B is selected
//============================================================

char
L1AF_SES_B(
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

}//L1AF_SES_B

//============================================================
// L1AF_Modulation_On
// TRUE when the Modulation is on
//============================================================

char
L1AF_Modulation_On(
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

}//L1AF_Modulation_On

//============================================================
// L1AF_Modulation_Off
// TRUE when the Modulation is off
//============================================================

char
L1AF_Modulation_Off(
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

}//L1AF_Modulation_Off

//============================================================
// L1AF_Rx_Protect_On
// TRUE when the Receive Protect is on
//============================================================

char
L1AF_Rx_Protect_On(
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

}//L1AF_Rx_Protect_On

//============================================================
// L1AF_Rx_Protect_Off
// TRUE when the Receive Protect is off
//============================================================

char
L1AF_Rx_Protect_Off(
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

}//L1AF_Rx_Protect_Off

//============================================================
// L1AF_Grid_Normal
// TRUE when the Grid is normal
//============================================================

char
L1AF_Grid_Normal(
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

}//L1AF_Grid_Normal

//============================================================
// L1AF_Grid_Dsbl
// TRUE when the Grid is disabled
//============================================================

char
L1AF_Grid_Dsbl(
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

}//L1AF_Grid_Dsbl

//============================================================
// L1AF_SAS_A_Spin19_8
// TRUE when the SAS A spin rate is 19.8 rpm
//============================================================

char
L1AF_SAS_A_Spin19_8(
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

}//L1AF_SAS_A_Spin19_8

//============================================================
// L1AF_SAS_A_Spin18_0
// TRUE when the SAS A spin rate is 18.0 rpm
//============================================================

char
L1AF_SAS_A_Spin18_0(
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

}//L1AF_SAS_A_Spin18_0

//============================================================
// L1AF_SAS_B_Spin19_8
// TRUE when the SAS B spin rate is 19.8 rpm
//============================================================

char
L1AF_SAS_B_Spin19_8(
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

}//L1AF_SAS_B_Spin19_8

//============================================================
// L1AF_SAS_B_Spin18_0
// TRUE when the SAS B spin rate is 18.0 rpm
//============================================================

char
L1AF_SAS_B_Spin18_0(
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

}//L1AF_SAS_B_Spin18_0
