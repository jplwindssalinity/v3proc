//=========================================================
// Copyright  (C)1995, California Institute of Technology.
// U.S. Government sponsorship under
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.16   07 Oct 1999 13:59:14   sally
// added L2Ahr file type
// 
//    Rev 1.15   15 Sep 1999 15:18:52   sally
// change some default values, fix logic
// 
//    Rev 1.14   26 Jul 1999 16:49:38   sally
// Group flag for L1A is 1, Hk2 is 3
// 
//    Rev 1.13   26 Jul 1999 15:43:32   sally
// add "Source Sequence Count" and "group flag" checking
// 
//    Rev 1.12   13 Jul 1999 12:38:44   sally
// comment out the first_header_packet parameter, LP is not ready
// 
//    Rev 1.11   09 Jul 1999 16:07:16   sally
// add error checkers for "Source Sequence Count" and "Group Flag" in header
// 
//    Rev 1.10   25 Jun 1999 10:30:44   sally
// add some error conditions from Lee
// 
//    Rev 1.9   08 Jun 1999 13:55:30   sally
// took out "Unexpected Cycle Count" - wrong parameter
// should be "packet sequence count"
// 
//    Rev 1.8   23 Jul 1998 16:13:58   sally
// pass polynomial table to extractFunc()
// 
//    Rev 1.7   22 May 1998 16:23:12   sally
// added error message handling
// 
//    Rev 1.6   20 May 1998 11:18:42   sally
// checks everything but error messages
// 
//    Rev 1.5   19 May 1998 15:49:54   sally
// move some function from L1AErrorChecker to ErrorChecker
// took out comparison function
// 
//    Rev 1.4   19 May 1998 14:44:50   sally
// 
//    Rev 1.3   18 May 1998 14:48:28   sally
// added error checker for L1A
// 
//    Rev 1.2   13 May 1998 16:28:12   sally
 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

static const char rcs_id[] =
    "@(#) $Header$";

#include <assert.h>
#include <stdio.h>
#include "L1AErrorChecker.h"
#include "L1AExtract.h"
#include "Parameter.h"
#include "ParTab.h"
#include "State.h"

//------------------------------------------
// Error State Table for L1A
//------------------------------------------
ErrorTabEntry L1aStateTable[] =
{
    // ERROR_MSSN_SERL_TLM
    { "Mission or Serial TLM Error", ERROR_FORMAT, ErrorMssnSerlTlm,
             { ERROR_FLAGS_03, UNIT_MAP } },

    // ERROR_RELAY_CHANGE
    { "Relay Set/Reset Changed", WARNING_FORMAT, ErrorRelayChanged,
             { STATUS_TABLE_CHANGE_FLAGS_03, UNIT_MAP } },


    // ERROR_CDS_RESET
    { "CDS System Reset", ERROR_FORMAT, ErrorCdsReset,
             { ERROR_FLAGS_07, UNIT_MAP } },

    // ERROR_A2D_TIMEOUT
    { "A2D Timeout", ERROR_FORMAT, ErrorA2DTimeout,
             { ERROR_FLAGS_05, UNIT_MAP } },

    // ERROR_MISS_SC_TIME
    { "Missing Spacecraft Time", ERROR_FORMAT, ErrorMissSCTime,
             { ERROR_FLAGS_04, UNIT_MAP } },

    // ERROR_FAULT_PROTECT
    { "Fault Protection Event", ERROR_FORMAT, ErrorFaultProtect,
             { ERROR_FLAGS_02, UNIT_MAP } },

    // ERROR_WATCHDOG_TIMEOUT
    { "Watchdog Timeout Reset Missed", ERROR_FORMAT, ErrorWatchdogTimeout,
             { ERROR_FLAGS_01, UNIT_MAP } },

    // ERROR_POWER_ON_RESET
    { "Power On Reset", ERROR_FORMAT, ErrorPowerOnReset,
             { ERROR_FLAGS_00, UNIT_MAP } },


    // ERROR_MSSN_TLM_TABLES_CHANGED
    { "Mission TLM Tables Changed", WARNING_FORMAT, ErrorMssnTlmTablesChanged,
             { STATUS_TABLE_CHANGE_FLAGS_15, UNIT_MAP } },

    // ERROR_SERL_TLM_TABLES_CHANGED
    { "Serial TLM Tables Changed", WARNING_FORMAT, ErrorSerlTlmTablesChanged,
             { STATUS_TABLE_CHANGE_FLAGS_14, UNIT_MAP } },

    // ERROR_DOPPLER_TABLES_CHANGED
    { "Doppler Tables Changed", WARNING_FORMAT, ErrorDopplerTablesChanged,
             { STATUS_TABLE_CHANGE_FLAGS_13, UNIT_MAP } },

    // ERROR_RANGE_GATE_TABLES_CHANGED
    { "Range Gate Tables Changed", WARNING_FORMAT, ErrorRangeGateTablesChanged,
             { STATUS_TABLE_CHANGE_FLAGS_12, UNIT_MAP } },

    // ERROR_SES_PARAM_TABLES_CHANGED
    { "SES Param Tables Changed", WARNING_FORMAT, ErrorSESParamTablesChanged,
             { STATUS_TABLE_CHANGE_FLAGS_11, UNIT_MAP } },

    // ERROR_TWTA_MON_EN_DIS
    { "TWTA monitor En/Dis", ERROR_FORMAT, ErrorTwtaMonEnDis,
             { STATUS_TABLE_CHANGE_FLAGS_10, UNIT_MAP } },

    // ERROR_TWTA_PWR_MON_FP_EN_DIS
    { "TWTA Power monitor FP En/Dis", ERROR_FORMAT, ErrorPwrMonFPEndis,
             { STATUS_TABLE_CHANGE_FLAGS_09, UNIT_MAP } },

    // ERROR_SUPP_HTR_MODE_CHANGE
    { "SES Suppl Htr Mode Chg En/Dis", ERROR_FORMAT, ErrorSesSuppHtrMode,
             { STATUS_TABLE_CHANGE_FLAGS_08, UNIT_MAP } },

    // ERROR_CDS_HARD_RESET
    { "CDS Hard Reset", ERROR_FORMAT, ErrorCdsHardReset,
             { STATUS_TABLE_CHANGE_FLAGS_07, UNIT_MAP } },

    // ERROR_SAS_MULTI_DATA_LOSS
    { "SAS Multi Data Loss FP En/Dis", ERROR_FORMAT, ErrorSasMultiDataLoss,
             { STATUS_TABLE_CHANGE_FLAGS_06, UNIT_MAP } },

    // ERROR_SES_MULTI_DATA_LOSS
    { "SES Multi Data Loss FP En/Dis", ERROR_FORMAT, ErrorSesMultiDataLoss,
             { STATUS_TABLE_CHANGE_FLAGS_05, UNIT_MAP } },

    // ERROR_SOFT_RESET
    { "Soft Reset", ERROR_FORMAT, ErrorSoftReset,
             { STATUS_TABLE_CHANGE_FLAGS_02, UNIT_MAP } },

    // ERROR_EQ_XING_MISSED
    { "Equator Crossing Missed", ERROR_FORMAT, ErrorEqXingMissed,
             { STATUS_TABLE_CHANGE_FLAGS_01, UNIT_MAP } },

    // ERROR_MODE_CHANGE
    { "Mode Change", NOTIFY_FORMAT, ErrorModeChange,
             { STATUS_TABLE_CHANGE_FLAGS_00, UNIT_MAP } },

    // ERROR_MODULATION
    { "Modulation", WARNING_FORMAT, ErrorModulation,
             { SES_CONFIG_FLAGS_03, UNIT_MAP } },

    // ERROR_TWT_TRIP_OVERRIDE
    { "TWT Trip Override", WARNING_FORMAT, ErrorTwtTripOverride,
             { SES_CONFIG_FLAGS_02, UNIT_MAP } },


    // ERROR_PBI_UNEXPECTED_STATE
    { "Payload in Unexpected State", ERROR_FORMAT, ErrorUnexpectedState,
             { PBI_FLAG_05, UNIT_MAP } },

    // ERROR_MEMORY_WRITE_FAIL
    { "Memory Write Test Failed", ERROR_FORMAT, ErrorWriteTestFailed,
             { PBI_FLAG_04, UNIT_MAP } },

    // ERROR_R2_MODIFIED
    { "R2 Modified", ERROR_FORMAT, ErrorR2Modified,
             { PBI_FLAG_03, UNIT_MAP } },

    // ERROR_BCRTM_FAILURE
    { "BCRTM (Build-in-Test) Failure", ERROR_FORMAT, ErrorBCRTMFailure,
             { PBI_FLAG_02, UNIT_MAP } },

    // ERROR_MEM_WRITE_PROT_VIOLATE
    { "Memory Write Protect Violation", ERROR_FORMAT, ErrorMemWriteProtViolate,
             { PBI_FLAG_01, UNIT_MAP } },

    // ERROR_WATCH_DOG_TIMER_EXPIRED
    { "Watch Dog Timer Expired", ERROR_FORMAT, ErrorWatchDogTimerExpired,
             { PBI_FLAG_00, UNIT_MAP } },


    // ERROR_TWT_1_CNVRT_OC_TRIP
    { "TWT 1 Converter Overcurrent Trip", ERROR_FORMAT, ErrorTwt1CnvrtOcTrip,
             { DISCRETE_STATUS_1_07, UNIT_MAP } },

    // ERROR_TWT_1_UV_TRIP
    { "TWT 1 Undervoltage Trip", ERROR_FORMAT, ErrorTwt1UvTrip,
             { DISCRETE_STATUS_1_06, UNIT_MAP } },

    // ERROR_TWT_1_BODY_OC_TRIP
    { "TWT 1 Body Overcurrent Trip", ERROR_FORMAT, ErrorTwt1BodyOcTrip,
             { DISCRETE_STATUS_1_05, UNIT_MAP } },

    // ERROR_TWT_2_CNVRT_OC_TRIP
    { "TWT 2 Converter Overcurrent Trip", ERROR_FORMAT, ErrorTwt2CnvrtOcTrip,
             { DISCRETE_STATUS_1_04, UNIT_MAP } },

    // ERROR_TWT_2_UV_TRIP
    { "TWT 2 Undervoltage Trip", ERROR_FORMAT, ErrorTwt2UvTrip,
             { DISCRETE_STATUS_1_03, UNIT_MAP } },

    // ERROR_TWT_2_BODY_OC_TRIP
    { "TWT 2 Body Overcurrent Trip", ERROR_FORMAT, ErrorTwt2BodyOcTrip,
             { DISCRETE_STATUS_1_02, UNIT_MAP } },

    // ERROR_PLL_OUT_OF_LOCK
    { "PLL Out of Lock", ERROR_FORMAT, ErrorPllOutOfLock,
             { DISCRETE_STATUS_1_01, UNIT_MAP } },

    // ERROR_TRS_CMD_SUCCESS
    { "TRS Cmd Success", ERROR_FORMAT, ErrorTrsCmdSucc,
             { DISCRETE_STATUS_2_04, UNIT_MAP } },

    // ERROR_BODY_OC_TRIP_CNTL
    { "TWT Body OC Trip Control", ERROR_FORMAT, ErrorBodyOcTripCntl,
             { DISCRETE_STATUS_2_00, UNIT_MAP } },

    // ERROR_SES_WATCH_DOG_TIMEOUT
    { "SES Watch Dog Timer Event", ERROR_FORMAT, ErrorSesWatchDogTimeout,
             { DISCRETE_STATUS_3_04, UNIT_MAP } },

    // ERROR_RAM_START_ERROR
    { "RAM Start Up Error", ERROR_FORMAT, ErrorRamStartError,
             { DISCRETE_STATUS_3_03, UNIT_MAP } },

    // ERROR_SES_ROM_START_ERROR
    { "SES ROM Start Up Error", ERROR_FORMAT, ErrorSesRomStartError,
             { DISCRETE_STATUS_3_02, UNIT_MAP } },

    // ERROR_SES_RESET
    { "SES Reset Event", WARNING_FORMAT, ErrorSesReset,
             { DISCRETE_STATUS_3_01, UNIT_MAP } },

    // ERROR_SES_PORT_PARITY_ERROR
    { "SES Port Parity Error", ERROR_FORMAT, ErrorPortParityError,
             { DISCRETE_STATUS_3_00, UNIT_MAP } },

    // ERROR_K15_SES_A_B
    { "K15 SES A/B", WARNING_FORMAT, ErrorK15SesAB,
             { K15_SES_SELECT, UNIT_MAP } },

    // ERROR_K12_TWTA_1_2
    { "K12 TWTA 1/2", WARNING_FORMAT, ErrorK12Twta12,
             { K12_TWTA_SELECT, UNIT_MAP } },

    // ERROR_K11_TWTA_1_2
    { "K11 TWTA 1/2", WARNING_FORMAT, ErrorK11Twta12,
             { K11_TWTA_SELECT, UNIT_MAP } },

    // ERROR_K10_TWTA_ON_OFF
    { "K10 TWTA On/Off", WARNING_FORMAT, ErrorK10TwtaOnOff,
             { K10_TWTA_POWER, UNIT_MAP } },

    // ERROR_K9_TWTA_ON_OFF
    { "K9 TWTA On/Off", WARNING_FORMAT, ErrorK9TwtaOnOff,
             { K9_TWTA_POWER, UNIT_MAP } },

    // ERROR_K20_SAS_A_B
    { "K20 SAS A/B", WARNING_FORMAT, ErrorK20SasAB,
             { K20_SAS_SELECT, UNIT_MAP } },

    // ERROR_K19_SAS_A_B
    { "K19 SAS A/B", WARNING_FORMAT, ErrorK19SasAB,
             { K19_SAS_SELECT, UNIT_MAP } },

    // ERROR_K16_SES_A_B
    { "K16 SES A/B", WARNING_FORMAT, ErrorK16SesAB,
             { K16_SES_SELECT, UNIT_MAP } },

    // ERROR_SAS_B_SPIN_RATE
    { "SAS-B Spin Rate", WARNING_FORMAT, ErrorSasBSpinRate,
             { SAS_B_SPIN_RATE, UNIT_MAP } },

    // ERROR_K22_SES_SUPP_HTR_ON_OFF
    { "K22 SES Supl Heater On/Off", WARNING_FORMAT, ErrorK22SesSuppHtrOnOff,
             { K22_SES_SUPP_HTR_PWR, UNIT_MAP } },

    // ERROR_K21_SES_SUPP_HTR_ON_OFF
    { "K21 SES Supl Heater On/Off", WARNING_FORMAT, ErrorK21SesSuppHtrOnOff,
             { K21_SES_SUPP_HTR_PWR, UNIT_MAP } },

    // ERROR_SAS_A_SPIN_RATE
    { "SAS-A Spin Rate", WARNING_FORMAT, ErrorSasASpinRate,
             { SAS_A_SPIN_RATE, UNIT_MAP } },


    // ERROR_MODE
    { "Invalid Mode", ERROR_FORMAT, ErrorCheckMode,
             { OPERATIONAL_MODE, UNIT_HEX_BYTES } },

    // ERROR_RX_PROTECT
    { "RX Protect in wrong mode", ERROR_FORMAT, ErrorRxProtect,
             { SES_CONFIG_FLAGS_01, UNIT_MAP } },

    // ERROR_GRID
    { "Grid Disable in wrong mode", ERROR_FORMAT, ErrorGridDisable,
             { SES_CONFIG_FLAGS_00, UNIT_MAP } },

    // ERROR_RUNNING_ERROR_COUNT
    { "Error Count Changed", ERROR_FORMAT, ErrorRunningErrors,
             { RUNNING_ERROR_COUNT, UNIT_COUNTS } },

    // ERROR_TWT_SHUTDOWN_DISABLE
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },


    // ERROR_VALID_COMMAND_COUNT
    { "Valid Command Count", NOTIFY_FORMAT, ErrorValidCmdCnt,
             { VALID_COMMAND_COUNT, UNIT_COUNTS } },

    // ERROR_INVALID_COMMAND_COUNT
    { "Invalid Command Count", ERROR_FORMAT, ErrorInvalidCmdCnt,
             { INVALID_COMMAND_COUNT, UNIT_COUNTS } },

    // ERROR_MODE_CHANGE_MISMATCHED
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },
 
    // ERROR_SAS_ANT_RELEASE_INTERLOCK
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_SRC_SEQ_COUNT
    { "Unexpected Source Sequence Count", ERROR_FORMAT, ErrorL1ABadSrcSeq,
             { DELTA_SRC_SEQ_COUNT, UNIT_COUNTS } },

    // ERROR_HEADER_GROUP_FLAG
    { "Unexpected Group Flag", ERROR_FORMAT, ErrorL1ABadGrpFlg,
             { HEADER_GROUP_FLAG, UNIT_DN } }
};

const int L1aStateTableSize = ElementNumber(L1aStateTable);

//========================//
// L1AErrorChecker methods //
//========================//

L1AErrorChecker::L1AErrorChecker()
:   _timeParamP(0),
    _sasK19ParamP(0), _sasK20ParamP(0),
    _sesK15ParamP(0), _sesK16ParamP(0),
    _twtaK11ParamP(0), _twtaK12ParamP(0),
    _modeParamP(0), _errorCounterP(0), _errorMsgP(0)
{
    // get parameters for current states
    _timeParamP = ParTabAccess::GetParameter(SOURCE_L1A, UTC_TIME, UNIT_CODE_A);
    assert(_timeParamP != 0);
    _sasK19ParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                      K19_SAS_SELECT, UNIT_MAP);
    assert(_sasK19ParamP != 0);
    _sasK20ParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                      K20_SAS_SELECT, UNIT_MAP);
    assert(_sasK20ParamP != 0);
    _sesK15ParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                      K15_SES_SELECT, UNIT_MAP);
    assert(_sesK15ParamP != 0);
    _sesK16ParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                      K16_SES_SELECT, UNIT_MAP);
    assert(_sesK16ParamP != 0);
    _twtaK11ParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                      K11_TWTA_SELECT, UNIT_MAP);
    assert(_twtaK11ParamP != 0);
    _twtaK12ParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                      K12_TWTA_SELECT, UNIT_MAP);
    assert(_twtaK12ParamP != 0);
    _modeParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                          OPERATIONAL_MODE, UNIT_HEX_BYTES);
    assert(_modeParamP != 0);
    _errorCounterP = ParTabAccess::GetParameter(SOURCE_L1A,
                                      RUNNING_ERROR_COUNT, UNIT_COUNTS);
    assert(_errorCounterP != 0);
    _errorMsgP = ParTabAccess::GetParameter(SOURCE_L1A, ERROR_MSG, UNIT_DN);
    assert(_errorMsgP != 0);

    _timeStateElement.AllocValue(_timeParamP->byteSize);
    _sasK19StateElement.AllocValue(_sasK19ParamP->byteSize);
    _sasK20StateElement.AllocValue(_sasK20ParamP->byteSize);
    _sesK15StateElement.AllocValue(_sesK15ParamP->byteSize);
    _sesK16StateElement.AllocValue(_sesK16ParamP->byteSize);
    _twtaK11StateElement.AllocValue(_twtaK11ParamP->byteSize);
    _twtaK12StateElement.AllocValue(_twtaK12ParamP->byteSize);
    _modeStateElement.AllocValue(_modeParamP->byteSize);
    _errorStateElement.AllocValue(_errorCounterP->byteSize);
    _errorMsgStateElement.AllocValue(_errorMsgP->byteSize);
    assert(_timeStateElement.value != 0 && _sasK19StateElement.value != 0
        && _sasK20StateElement.value != 0 && _sesK15StateElement.value != 0
        && _sesK16StateElement.value != 0 && _twtaK11StateElement.value != 0
        && _twtaK12StateElement.value != 0 && _modeStateElement.value != 0
        && _errorStateElement.value != 0 && _errorMsgStateElement.value != 0 );

    // get parameters for error table
    parametersP = new Parameter*[L1aStateTableSize];
    int i=0;
    for (i=0; i < L1aStateTableSize; i++)
    {
        if (L1aStateTable[i].stateEntry.paramId == PARAM_UNKNOWN)
            parametersP[i] = NONE_ERROR_CHECKER;
        else
            parametersP[i] = ParTabAccess::GetParameter(SOURCE_L1A,
                                L1aStateTable[i].stateEntry.paramId,
                                L1aStateTable[i].stateEntry.unitId);
        assert(parametersP[i] != 0);
    }

    prev = new State(L1aStateTableSize);
    current = new State(L1aStateTableSize);
    for (i=0; i < L1aStateTableSize; i++)
    {
        if (parametersP[i] != NONE_ERROR_CHECKER)
        {
            prev->stateElements[i].AllocValue(parametersP[i]->byteSize);
            current->stateElements[i].AllocValue(parametersP[i]->byteSize);
            assert(prev->stateElements[i].value != 0 &&
                   current->stateElements[i].value != 0);
        }
    }

    _errorStateTableSize = L1aStateTableSize;

    return;
}

L1AErrorChecker::~L1AErrorChecker()
{
    if (_timeParamP) delete _timeParamP;
    if (_sasK19ParamP) delete _sasK19ParamP;
    if (_sasK20ParamP) delete _sasK20ParamP;
    if (_sesK15ParamP) delete _sesK15ParamP;
    if (_sesK16ParamP) delete _sesK16ParamP;
    if (_twtaK11ParamP) delete _twtaK11ParamP;
    if (_twtaK12ParamP) delete _twtaK12ParamP;
    if (_modeParamP) delete _modeParamP;
    if (_errorCounterP) delete _errorCounterP;
    if (_errorMsgP) delete _errorMsgP;

    int i=0;
    for (i=0; i < L1aStateTableSize; i++)
    {
        if (parametersP[i] != NONE_ERROR_CHECKER && parametersP[i])
            delete parametersP[i];
        parametersP[i] = 0;
    }
    delete parametersP;
    parametersP = 0;

    if (prev)
    {
        delete prev;
        prev = 0;
    }

    if (current)
    {
        delete current;
        current = 0;
    }

    return;
}

int
L1AErrorChecker::OpenParamDataSets(
TlmHdfFile*    tlmFile)
{
    // open the parameters needs by basic reporting first
    if (OpenInternalStateDataSets(tlmFile) == 0)
        return 0;

    return(ErrorChecker::OpenParamDataSets(tlmFile));

}//L1AErrorChecker::OpenParamDataSets

void
L1AErrorChecker::CloseParamDataSets(
TlmHdfFile*    tlmFile)
{
    // close the parameters needs by basic reporting first
    CloseInternalStateDataSets(tlmFile);

    ErrorChecker::CloseParamDataSets(tlmFile);

    return;
 
}//L1AErrorChecker::CloseParamDataSets

int
L1AErrorChecker::OpenInternalStateDataSets(
TlmHdfFile*    tlmFile)
{
    Parameter** internalParameters= new Parameter*[10];
    internalParameters[0] = _timeParamP;
    internalParameters[1] = _sasK19ParamP;
    internalParameters[2] = _sasK20ParamP;
    internalParameters[3] = _sesK15ParamP;
    internalParameters[4] = _sesK16ParamP;
    internalParameters[5] = _twtaK11ParamP;
    internalParameters[6] = _twtaK12ParamP;
    internalParameters[7] = _modeParamP;
    internalParameters[8] = _errorCounterP;
    internalParameters[9] = _errorMsgP;
    
    for (int i=0; i < 10; i++)
    {
        int32 dataType, dataStartIndex, dataLength, numDimensions;
        char tempString[BIG_SIZE];
        (void)strncpy(tempString, internalParameters[i]->sdsNames, BIG_SIZE);
        char* oneSdsName=0;
        int j=0;
        for (oneSdsName = (char*)strtok(tempString, ",");
                           oneSdsName;
                           oneSdsName = (char*)strtok(0, ","), j++)
        {
            internalParameters[i]->sdsIDs[j] = HDF_FAIL;
            internalParameters[i]->sdsIDs[j] = tlmFile->SelectDataset(
                                         oneSdsName, dataType, dataStartIndex,
                                         dataLength, numDimensions);
            if (internalParameters[i]->sdsIDs[j] == HDF_FAIL)
                return 0;
        }
    }
    delete [] internalParameters;

    return 1;

} // OpenInternalStateDataSets

void
L1AErrorChecker::CloseInternalStateDataSets(
TlmHdfFile*    tlmFile)
{
    Parameter** internalParameters= new Parameter*[10];
    internalParameters[0] = _timeParamP;
    internalParameters[1] = _sasK19ParamP;
    internalParameters[2] = _sasK20ParamP;
    internalParameters[3] = _sesK15ParamP;
    internalParameters[4] = _sesK16ParamP;
    internalParameters[5] = _twtaK11ParamP;
    internalParameters[6] = _twtaK12ParamP;
    internalParameters[7] = _modeParamP;
    internalParameters[8] = _errorCounterP;
    internalParameters[9] = _errorMsgP;
    for (int i=0; i < 10; i++)
    {
        for (int j=0; j < internalParameters[i]->numSDSs; j++)
        {
            (void)tlmFile->CloseDataset(internalParameters[i]->sdsIDs[j]);
            internalParameters[i]->sdsIDs[j] = HDF_FAIL;
        }
    }
    delete [] internalParameters;
    return;
 
}//L1AErrorChecker::CloseParamDataSets

//----------//
// SetState //
//----------//

int
L1AErrorChecker::SetState(
TlmHdfFile*     tlmFile,
int32           startIndex,
State*          lastState,
State*          newState)
{
    if (SetInternalState(tlmFile, startIndex) == 0)
        return 0;

    if (ErrorChecker::SetState(tlmFile, startIndex, lastState, newState) == 0)
        return 0;

    Itime* iTimeP = (Itime*)_timeStateElement.value;
    (void)iTimeP->ItimeToCodeA(newState->time_string);

    return 1;
}

int
L1AErrorChecker::SetInternalState(
TlmHdfFile*     tlmFile,
int32           startIndex)
{
    Parameter** internalParameters= new Parameter*[10];
    internalParameters[0] = _timeParamP;
    internalParameters[1] = _sasK19ParamP;
    internalParameters[2] = _sasK20ParamP;
    internalParameters[3] = _sesK15ParamP;
    internalParameters[4] = _sesK16ParamP;
    internalParameters[5] = _twtaK11ParamP;
    internalParameters[6] = _twtaK12ParamP;
    internalParameters[7] = _modeParamP;
    internalParameters[8] = _errorCounterP;
    internalParameters[9] = _errorMsgP;

    StateElement** stateElements = new StateElement*[10];
    stateElements[0] = &_timeStateElement;
    stateElements[1] = &_sasK19StateElement;
    stateElements[2] = &_sasK20StateElement;
    stateElements[3] = &_sesK15StateElement;
    stateElements[4] = &_sesK16StateElement;
    stateElements[5] = &_twtaK11StateElement;
    stateElements[6] = &_twtaK12StateElement;
    stateElements[7] = &_modeStateElement;
    stateElements[8] = &_errorStateElement;
    stateElements[9] = &_errorMsgStateElement;

    for (int i=0; i < 10; i++)
    {
        int rc = internalParameters[i]->extractFunc(tlmFile,
                      internalParameters[i]->sdsIDs,
                      startIndex, 1, 1, stateElements[i]->value, 0);
        if (rc > 0)
            stateElements[i]->condition = StateElement::CURRENT;
        else if (rc < 0)
        {
            stateElements[i]->condition = StateElement::ERROR;
            delete [] internalParameters;
            delete [] stateElements;
            return 0;
        }
        else
            stateElements[i]->condition = StateElement::HELD;
    }

    delete [] internalParameters;
    delete [] stateElements;
    return 1;

} // L1AErrorChecker::SetInternalState

void
L1AErrorChecker::ReportBasicInfo(
FILE*           ofp)
{
    if (_timeStateElement.condition == StateElement::UNINITIALIZED)
        return;

    fprintf(ofp, "    Mode = ");
    if (_modeStateElement.condition == StateElement::UNINITIALIZED)
        fprintf(ofp, "?");
    else
    {
        const char* modeString;
        unsigned char tlmMode = *(unsigned char*)_modeStateElement.value;
        int eaModeNo = L1ModeToTlmMode(tlmMode);
        if (eaModeNo < 0)
            modeString = "unknown mode";
        else
            modeString = mode_map[eaModeNo];
        fprintf(ofp, "%d (%s)", tlmMode, modeString);
    }

    fprintf(ofp, ", SAS = ");
    if (_sasK19StateElement.condition == StateElement::UNINITIALIZED ||
              _sasK20StateElement.condition == StateElement::UNINITIALIZED)
        fprintf(ofp, "?");
    else
    {
        char aOrb;
        if (DefaultStateCompareFunc(_sasK19StateElement, _sasK20StateElement,
                                       _sasK19ParamP->byteSize))
            aOrb = 'A';
        else
            aOrb = 'B';
        fprintf(ofp, "%c", aOrb);
    }

    fprintf(ofp, ", SES = ");
    if (_sesK15StateElement.condition == StateElement::UNINITIALIZED ||
              _sesK16StateElement.condition == StateElement::UNINITIALIZED)
        fprintf(ofp, "?");
    else
    {
        char aOrb;
        if (DefaultStateCompareFunc(_sesK15StateElement, _sesK16StateElement,
                                       _sesK15ParamP->byteSize))
            aOrb = 'A';
        else
            aOrb = 'B';
        fprintf(ofp, "%c", aOrb);
    }

    fprintf(ofp, ", TWTA = ");
    if (_twtaK11StateElement.condition == StateElement::UNINITIALIZED ||
              _twtaK12StateElement.condition == StateElement::UNINITIALIZED)
        fprintf(ofp, "?");
    else
    {
        char aOrb;
        if (DefaultStateCompareFunc(_twtaK11StateElement, _twtaK12StateElement,
                                       _twtaK11ParamP->byteSize))
            aOrb = '1';
        else
            aOrb = '2';
        fprintf(ofp, "%c", aOrb);
    }

    fprintf(ofp, ", Error Count = ");
    if (_errorStateElement.condition == StateElement::UNINITIALIZED)
        fprintf(ofp, "?");
    else
    {
        assert(_errorCounterP->printFunc != 0);
        _errorCounterP->printFunc(ofp, (char*)_errorStateElement.value);
    }

    fprintf(ofp, "\n");

    return;

} // L1AErrorChecker::ReportBasicInfo

//-------//
// Check //
//-------//

int
L1AErrorChecker::Check(
TlmHdfFile*     tlmFile,
int32           startIndex,
FILE*           ofp)
{
    //-----------------------//
    // set the current state //
    //-----------------------//
    if (SetState(tlmFile, startIndex, prev, current) == 0)
        return 0;
 
    for (int i=0; i < L1aStateTableSize; i++)
    {
        ErrorTabEntry* errorTabEntry = &(L1aStateTable[i]);

        //-------------------------------------------------------
        // if the result of error checking is good (no change)
        // then no need to do anything.
        //-------------------------------------------------------
        if (errorTabEntry->errorFunc)
        {
            if ((*errorTabEntry->errorFunc) (this,
                        errorTabEntry->name,
                        errorTabEntry->format,
                        ofp))
                continue;
        }

        //-------------------------------------------------------
        // at this point, there is an error
        //-------------------------------------------------------
        errorTabEntry->count++;
    }
 
    //-------------------------//
    // check the error message //
    //-------------------------//
    (void) ErrorMessageCode(this, ERROR_MSG_NAME, ERROR_FORMAT, ofp);

    //-------------------------//
    // save the previous state //
    //-------------------------//
    *prev = *current;
 
    //------------------------//
    // count the data records //
    //------------------------//
    dataRecCount++;
    return 1;
}

//-----------//
// Summarize //
//-----------//

#define PERCENT(A) ((float)(A)*100.0/(float)dataRecCount)

void
L1AErrorChecker::Summarize(
FILE*   ofp)
{
    //------------//
    // basic info //
    //------------//

    fprintf(ofp, "Number of Data Records: %d\n", dataRecCount);

    //----------------//
    // error messages //
    //----------------//

    fprintf(ofp, "\nERROR MESSAGES\n");
    fprintf(ofp, "    Frames   Percentage   Error Message\n");
    char msgString[BIG_SIZE];
    ErrorMsgT* errMsgP;
    for (errMsgP = _errorMsgCodes.GetHead(); errMsgP != 0;
                            errMsgP = _errorMsgCodes.GetNext())
    {
        (void)CodeToErrorMsg(errMsgP->errorMsgCode, msgString);
        fprintf(ofp, "%10u   %10.4f   %s\n",
                              errMsgP->num, PERCENT(errMsgP->num),
                              msgString);
    }

    //--------------//
    // other errors //
    //--------------//

    fprintf(ofp, "\nERROR SUMMARY\n");
    fprintf(ofp, "    Frames   Percentage   Error Description\n");

    for (int i=0; i < L1aStateTableSize; i++)
    {
        ErrorTabEntry* errorTabEntry = &(L1aStateTable[i]);
        if (! errorTabEntry->name)
            continue;
        fprintf(ofp, "%10d   %10.4f   %s\n", errorTabEntry->count,
            PERCENT(errorTabEntry->count), errorTabEntry->name);
    }

    if (! AnyErrors())
        fprintf(ofp, "\nNo errors detected.\n");

    return;

}//L1AErrorChecker::Summarize

//-----------//
// AnyErrors //
//-----------//
// returns non-zero if there were any errors

int
L1AErrorChecker::AnyErrors()
{
    for (int i=0; i < L1aStateTableSize; i++)
    {
        ErrorTabEntry* errorTabEntry = &(L1aStateTable[i]);
        if (! errorTabEntry->name)
            continue;
        if (errorTabEntry->count > 0)
            return(1);
    }
    return 0;

}//L1AErrorChecker::AnyErrors

int
L1AErrorChecker::ErrorMessageCode(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    if (_errorMsgStateElement.condition != StateElement::CURRENT)
        return 1;

    unsigned short errorMsgCode =
                  *(unsigned short*) _errorMsgStateElement.value;

    // not 0
    AddErrorMessageCode(errorMsgCode);
    if (errorMsgCode != 0)
    {
        fprintf(ofp, format, name);
        char msgString[BIG_SIZE];
        (void)CodeToErrorMsg(errorMsgCode, msgString);
        fprintf(ofp, "  %s   Error Message = 0x%x (%s)\n",
                obj->current->time_string, errorMsgCode, msgString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }
    else
        return 1;

} // L1AErrorChecker::ErrorMessageCode

int
L1AErrorChecker::CodeToErrorMsg(
unsigned short        errorCode,      // IN
char*                 msgString)      // IN/OUT
{
    //----------------------------------------------------
    // if the MSB is clear, then this is type 1 error
    //----------------------------------------------------
    if ((errorCode & 0x8000) == 0)
    {
        //------------------------------------------
        // the error code is in the most sig byte
        //------------------------------------------
        unsigned char type1ErrorCode=0;
        (void)memcpy(&type1ErrorCode, &errorCode, 1);
        if (type1ErrorCode >= Type1ErrMsgMapSize)
        {
            (void)sprintf(msgString,
                         "Unknown Type 1 Error (%d)", errorCode);
            return 0;
        }
        else
        {
            (void)strcpy(msgString, type1_error_msg_map[(int)type1ErrorCode]);
            return 1;
        }
    }
    //----------------------------------------------------
    // the MSB is set, this is type 2 error
    //----------------------------------------------------
    else
    {
        //------------------------------------------------------------
        // get the most sig byte, this contains the type 2 error code
        //------------------------------------------------------------
        unsigned char* ucharP = (unsigned char*) &errorCode;
        unsigned char type2ErrorCode=0;
        (void)memcpy(&type2ErrorCode, ucharP, 1);

        //-----------------------------------------------
        // strip the leftmost bit for type 2 error code
        //-----------------------------------------------
        type2ErrorCode = (unsigned char) GetBits(type2ErrorCode, 6, 7);
        if (type2ErrorCode >= Type2ErrMsgMapSize)
        {
            (void)sprintf(msgString,
                         "Unknown Type 2 Error (%d)", type2ErrorCode);
            return 0;
        }

        //------------------------------------------------------------
        // get the least sig byte, this contains the further info
        //------------------------------------------------------------
        ucharP++;
        unsigned char idOrCount=0;
        (void)memcpy(&idOrCount, ucharP, 1);

        //------------------------------------------------
        // error code 0-3, the least sig byte contains
        // CDS FSW critical variable object ID
        //------------------------------------------------
        if (type2ErrorCode <= 3)
        {
            if (idOrCount >= CdsFswCritVarObjIdSize)
            {
                (void)sprintf(msgString,
                         "Unknown FSW Critical Var Object ID(%d)", idOrCount);
                return 0;
            }
            else
            {
                (void)sprintf(msgString, "%s: %s",
                         type2_error_msg_map[(int)type2ErrorCode],
                         cds_fsw_crit_var_obj_id[(int)idOrCount]);
                return 1;
            }
        }
        //------------------------------------------------
        // error code =4, the least sig byte contains
        // CDS FSW Tables object ID
        //------------------------------------------------
        else if (type2ErrorCode == 4)
        {
            if (idOrCount >= CdsFswObjIdSize)
            {
                (void)sprintf(msgString,
                         "Unknown FSW Tables Object ID(%d)", idOrCount);
                return 0;
            }
            else
            {
                (void)sprintf(msgString, "%s: %s",
                         type2_error_msg_map[(int)type2ErrorCode],
                         cds_fsw_obj_id[(int)idOrCount]);
                return 1;
            }
        }
        //------------------------------------------------
        // error code > 4, the least sig byte contains
        // PRF number in which the error occurred
        //------------------------------------------------
        else
        {
            (void)sprintf(msgString, "%s: PRF %d",
                         type2_error_msg_map[(int)type2ErrorCode], idOrCount);
            return 1;
        }
    }
} // L1AErrorChecker::CodeToErrorMsg

void
L1AErrorChecker::AddErrorMessageCode(
unsigned short        errorCode)
{
    //--------------------------------------------------
    // if the error msg code is already in the list,
    // just increment the number, else append the new one
    // in the list
    //--------------------------------------------------
    ErrorMsgT* errMsgP = new ErrorMsgT(errorCode);
    if (_errorMsgCodes.Find(errMsgP))
    {
        ErrorMsgT* currentMsgP = _errorMsgCodes.GetCurrent();
        currentMsgP->num++;
        delete errMsgP;
    }
    else
    {
        _errorMsgCodes.Append(errMsgP);
    }
} // L1AErrorChecker::AddErrorMessageCode
