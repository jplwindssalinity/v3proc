//=========================================================
// Copyright  (C)1995, California Institute of Technology.
// U.S. Government sponsorship under
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.6   26 Jul 1999 16:49:36   sally
// Group flag for L1A is 1, Hk2 is 3
// 
//    Rev 1.5   26 Jul 1999 15:43:30   sally
// add "Source Sequence Count" and "group flag" checking
// 
//    Rev 1.4   09 Jul 1999 16:07:14   sally
// add error checkers for "Source Sequence Count" and "Group Flag" in header
// 
//    Rev 1.3   25 Jun 1999 10:30:40   sally
// add some error conditions from Lee
// 
//    Rev 1.2   23 Jul 1998 16:13:14   sally
// pass polynomial table to extractFunc()
// 
//    Rev 1.1   22 May 1998 16:22:54   sally
// took out error message handling
// 
//    Rev 1.0   20 May 1998 11:20:24   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

static const char rcs_id[] =
    "@(#) $Header$";

#include <assert.h>
#include <stdio.h>
#include "Hk2ErrorChecker.h"
#include "L1AExtract.h"
#include "Parameter.h"
#include "ParTab.h"
#include "State.h"

//------------------------------------------
// Error State Table for HK2
//------------------------------------------
ErrorTabEntry HK2StateTable[] =
{
    // ERROR_MSSN_SERL_TLM
    { "Mission or Serial TLM Error", ERROR_FORMAT, ErrorMssnSerlTlm,
             { MTLM_STLM_ERR, UNIT_MAP } },

    // ERROR_RELAY_CHANGE
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },


    // ERROR_CDS_RESET
    { "CDS System Reset", ERROR_FORMAT, ErrorCdsReset,
             { CDS_ERR_RESET_FLAG, UNIT_MAP } },

    // ERROR_A2D_TIMEOUT
    { "A2D Timeout", ERROR_FORMAT, ErrorA2DTimeout,
             { A2D_TIMEOUT_FLAG, UNIT_MAP } },

    // ERROR_MISS_SC_TIME
    { "Missing Spacecraft Time", ERROR_FORMAT, ErrorMissSCTime,
             { MISS_SC_TIME_HLDC, UNIT_MAP } },

    // ERROR_FAULT_PROTECT
    { "Fault Protection Event", ERROR_FORMAT, ErrorFaultProtect,
             { FAULT_PROTECT_EVENT, UNIT_MAP } },

    // ERROR_WATCHDOG_TIMEOUT
    { "Watchdog Timeout Reset Missed", ERROR_FORMAT, ErrorWatchdogTimeout,
             { WATCHDOG_TIMEOUT, UNIT_MAP } },

    // ERROR_POWER_ON_RESET
    { "Power On Reset", ERROR_FORMAT, ErrorPowerOnReset,
             { POWER_ON_RESET, UNIT_MAP } },


    // ERROR_MSSN_TLM_TABLES_CHANGED
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_SERL_TLM_TABLES_CHANGED
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_DOPPLER_TABLES_CHANGED
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_RANGE_GATE_TABLES_CHANGED
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_SES_PARAM_TABLES_CHANGED
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_TWTA_MON_EN_DIS
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_TWTA_PWR_MON_FP_EN_DIS
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_SUPP_HTR_MODE_CHANGE
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_CDS_HARD_RESET
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_SAS_MULTI_DATA_LOSS
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_SES_MULTI_DATA_LOSS
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_SOFT_RESET
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_EQ_XING_MISSED
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_MODE_CHANGE
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },


    // ERROR_MODULATION
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_TWT_TRIP_OVERRIDE
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },


    // ERROR_PBI_UNEXPECTED_STATE
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_MEMORY_WRITE_FAIL
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_R2_MODIFIED
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_BCRTM_FAILURE
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_MEM_WRITE_PROT_VIOLATE
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_WATCH_DOG_TIMER_EXPIRED
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_TWT_1_CNVRT_OC_TRIP
    { "TWT 1 Converter Overcurrent Trip", ERROR_FORMAT, ErrorTwt1CnvrtOcTrip,
             { TWTA_1_CONV_OC, UNIT_MAP } },

    // ERROR_TWT_1_UV_TRIP
    { "TWT 1 Converter UnderVoltage Trip", ERROR_FORMAT, ErrorTwt1UvTrip,
             { TWTA_1_CONV_OC, UNIT_MAP } },

    // ERROR_TWT_1_BODY_OC_TRIP
    { "TWT 1 Body Overcurrent Trip", ERROR_FORMAT, ErrorTwt1BodyOcTrip,
             { TWTA_1_BODY_OC, UNIT_MAP } },

    // ERROR_TWT_2_CNVRT_OC_TRIP
    { "TWT 2 Converter Overcurrent Trip", ERROR_FORMAT, ErrorTwt2CnvrtOcTrip,
             { TWTA_2_CONV_OC, UNIT_MAP } },

    // ERROR_TWT_2_UV_TRIP
    { "TWT 2 Undervoltage Trip", ERROR_FORMAT, ErrorTwt2UvTrip,
             { TWTA_2_UNDER_VOLT, UNIT_MAP } },

    // ERROR_TWT_2_BODY_OC_TRIP
    { "TWT 2 Body Overcurrent Trip", ERROR_FORMAT, ErrorTwt2BodyOcTrip,
             { TWTA_2_BODY_OC, UNIT_MAP } },

    // ERROR_PLL_OUT_OF_LOCK
    { "PLL Out of Lock", ERROR_FORMAT, ErrorPllOutOfLock,
             { PLL_OUT_OF_LOCK, UNIT_MAP } },

    // ERROR_TRS_CMD_SUCCESS
    { "TRS Cmd Success", ERROR_FORMAT, ErrorTrsCmdSucc,
             { SES_TRS_CMD_SUCC, UNIT_MAP } },

    // ERROR_BODY_OC_TRIP_CNTL
    { "TWT Body OC Trip Control", ERROR_FORMAT, ErrorBodyOcTripCntl,
             { TWT_BODY_OC_TRIP, UNIT_MAP } },

    // ERROR_SES_WATCH_DOG_TIMEOUT
    { "SES Watch Dog Timer Event", ERROR_FORMAT, ErrorSesWatchDogTimeout,
             { SES_WATCHDOG_TIMER_EVENT, UNIT_MAP } },

    // ERROR_RAM_START_ERROR
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_SES_ROM_START_ERROR
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_SES_RESET
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_SES_PORT_PARITY_ERROR
    { "SES Port Parity Error", ERROR_FORMAT, ErrorPortParityError,
             { SES_SERIAL_PRT_PARITY, UNIT_MAP } },


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
             { SES_RX_PROTECT, UNIT_MAP } },

    // ERROR_GRID
    { "Grid Disable in wrong mode", ERROR_FORMAT, ErrorGridDisable,
             { GRID_INHIBIT, UNIT_MAP } },

    // ERROR_RUNNING_ERROR_COUNT
    { 0, 0, 0,
             { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_TWT_SHUTDOWN_DISABLE
    { 0, 0, 0,
             { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_VALID_COMMAND_COUNT
    { "Valid Command Count", NOTIFY_FORMAT, ErrorValidCmdCnt,
             { VALID_COMMAND_COUNT, UNIT_DN } },

    // ERROR_INVALID_COMMAND_COUNT
    { "Invalid Command Count", ERROR_FORMAT, ErrorInvalidCmdCnt,
             { INVALID_COMMAND_COUNT, UNIT_DN } },

    // ERROR_MODE_CHANGE_MISMATCHED
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_SAS_ANT_RELEASE_INTERLOCK
    { 0, 0, 0, { PARAM_UNKNOWN, UNIT_UNKNOWN } },

    // ERROR_SRC_SEQ_COUNT
    { "Unexpected Source Sequence Count", ERROR_FORMAT, ErrorHk2BadSrcSeq,
             { DELTA_SRC_SEQ_COUNT, UNIT_COUNTS } },
 
    // ERROR_HEADER_GROUP_FLAG
    { "Unexpected Group Flag", ERROR_FORMAT, ErrorHk2BadGrpFlg,
             { HEADER_GROUP_FLAG, UNIT_DN } }

};

const int HK2StateTableSize = ElementNumber(HK2StateTable);

//========================//
// HK2ErrorChecker methods //
//========================//

HK2ErrorChecker::HK2ErrorChecker()
:   _timeParamP(0),
    _sasK19ParamP(0), _sasK20ParamP(0),
    _sesK15ParamP(0), _sesK16ParamP(0),
    _twtaK11ParamP(0), _twtaK12ParamP(0),
    _modeParamP(0)
{
    // get parameters for current states
    _timeParamP = ParTabAccess::GetParameter(SOURCE_HK2, UTC_TIME, UNIT_CODE_A);
    assert(_timeParamP != 0);
    _sasK19ParamP = ParTabAccess::GetParameter(SOURCE_HK2,
                                      K19_SAS_SELECT, UNIT_MAP);
    assert(_sasK19ParamP != 0);
    _sasK20ParamP = ParTabAccess::GetParameter(SOURCE_HK2,
                                      K20_SAS_SELECT, UNIT_MAP);
    assert(_sasK20ParamP != 0);
    _sesK15ParamP = ParTabAccess::GetParameter(SOURCE_HK2,
                                      K15_SES_SELECT, UNIT_MAP);
    assert(_sesK15ParamP != 0);
    _sesK16ParamP = ParTabAccess::GetParameter(SOURCE_HK2,
                                      K16_SES_SELECT, UNIT_MAP);
    assert(_sesK16ParamP != 0);
    _twtaK11ParamP = ParTabAccess::GetParameter(SOURCE_HK2,
                                      K11_TWTA_SELECT, UNIT_MAP);
    assert(_twtaK11ParamP != 0);
    _twtaK12ParamP = ParTabAccess::GetParameter(SOURCE_HK2,
                                      K12_TWTA_SELECT, UNIT_MAP);
    assert(_twtaK12ParamP != 0);
    _modeParamP = ParTabAccess::GetParameter(SOURCE_HK2,
                          OPERATIONAL_MODE, UNIT_HEX_BYTES);
    assert(_modeParamP != 0);

    _timeStateElement.AllocValue(_timeParamP->byteSize);
    _sasK19StateElement.AllocValue(_sasK19ParamP->byteSize);
    _sasK20StateElement.AllocValue(_sasK20ParamP->byteSize);
    _sesK15StateElement.AllocValue(_sesK15ParamP->byteSize);
    _sesK16StateElement.AllocValue(_sesK16ParamP->byteSize);
    _twtaK11StateElement.AllocValue(_twtaK11ParamP->byteSize);
    _twtaK12StateElement.AllocValue(_twtaK12ParamP->byteSize);
    _modeStateElement.AllocValue(_modeParamP->byteSize);
    assert(_timeStateElement.value != 0 && _sasK19StateElement.value != 0
        && _sasK20StateElement.value != 0 && _sesK15StateElement.value != 0
        && _sesK16StateElement.value != 0 && _twtaK11StateElement.value != 0
        && _twtaK12StateElement.value != 0 && _modeStateElement.value != 0);

    // get parameters for error table
    parametersP = new Parameter*[HK2StateTableSize];
    int i=0;
    for (i=0; i < HK2StateTableSize; i++)
    {
        if (HK2StateTable[i].stateEntry.paramId == PARAM_UNKNOWN)
            parametersP[i] = NONE_ERROR_CHECKER;
        else
            parametersP[i] = ParTabAccess::GetParameter(SOURCE_HK2,
                                HK2StateTable[i].stateEntry.paramId,
                                HK2StateTable[i].stateEntry.unitId);
        assert(parametersP[i] != 0);
    }

    prev = new State(HK2StateTableSize);
    current = new State(HK2StateTableSize);
    for (i=0; i < HK2StateTableSize; i++)
    {
        if (parametersP[i] != NONE_ERROR_CHECKER)
        {
            prev->stateElements[i].AllocValue(parametersP[i]->byteSize);
            current->stateElements[i].AllocValue(parametersP[i]->byteSize);
            assert(prev->stateElements[i].value != 0 &&
                   current->stateElements[i].value != 0);
        }
    }

    _errorStateTableSize = HK2StateTableSize;

    return;
}

HK2ErrorChecker::~HK2ErrorChecker()
{
    if (_timeParamP) delete _timeParamP;
    if (_sasK19ParamP) delete _sasK19ParamP;
    if (_sasK20ParamP) delete _sasK20ParamP;
    if (_sesK15ParamP) delete _sesK15ParamP;
    if (_sesK16ParamP) delete _sesK16ParamP;
    if (_twtaK11ParamP) delete _twtaK11ParamP;
    if (_twtaK12ParamP) delete _twtaK12ParamP;
    if (_modeParamP) delete _modeParamP;

    int i=0;
    for (i=0; i < HK2StateTableSize; i++)
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
HK2ErrorChecker::OpenParamDataSets(
TlmHdfFile*    tlmFile)
{
    // open the parameters needs by basic reporting first
    if (OpenInternalStateDataSets(tlmFile) == 0)
        return 0;

    return(ErrorChecker::OpenParamDataSets(tlmFile));

}//HK2ErrorChecker::OpenParamDataSets

void
HK2ErrorChecker::CloseParamDataSets(
TlmHdfFile*    tlmFile)
{
    // close the parameters needs by basic reporting first
    CloseInternalStateDataSets(tlmFile);

    ErrorChecker::CloseParamDataSets(tlmFile);

    return;
 
}//HK2ErrorChecker::CloseParamDataSets

int
HK2ErrorChecker::OpenInternalStateDataSets(
TlmHdfFile*    tlmFile)
{
    Parameter** internalParameters= new Parameter*[8];
    internalParameters[0] = _timeParamP;
    internalParameters[1] = _sasK19ParamP;
    internalParameters[2] = _sasK20ParamP;
    internalParameters[3] = _sesK15ParamP;
    internalParameters[4] = _sesK16ParamP;
    internalParameters[5] = _twtaK11ParamP;
    internalParameters[6] = _twtaK12ParamP;
    internalParameters[7] = _modeParamP;
    
    for (int i=0; i < 8; i++)
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
HK2ErrorChecker::CloseInternalStateDataSets(
TlmHdfFile*    tlmFile)
{
    Parameter** internalParameters= new Parameter*[8];
    internalParameters[0] = _timeParamP;
    internalParameters[1] = _sasK19ParamP;
    internalParameters[2] = _sasK20ParamP;
    internalParameters[3] = _sesK15ParamP;
    internalParameters[4] = _sesK16ParamP;
    internalParameters[5] = _twtaK11ParamP;
    internalParameters[6] = _twtaK12ParamP;
    internalParameters[7] = _modeParamP;
    for (int i=0; i < 8; i++)
    {
        for (int j=0; j < internalParameters[i]->numSDSs; j++)
        {
            (void)tlmFile->CloseDataset(internalParameters[i]->sdsIDs[j]);
            internalParameters[i]->sdsIDs[j] = HDF_FAIL;
        }
    }
    delete [] internalParameters;
    return;
 
}//HK2ErrorChecker::CloseParamDataSets

//----------//
// SetState //
//----------//

int
HK2ErrorChecker::SetState(
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
HK2ErrorChecker::SetInternalState(
TlmHdfFile*     tlmFile,
int32           startIndex)
{
    Parameter** internalParameters= new Parameter*[8];
    internalParameters[0] = _timeParamP;
    internalParameters[1] = _sasK19ParamP;
    internalParameters[2] = _sasK20ParamP;
    internalParameters[3] = _sesK15ParamP;
    internalParameters[4] = _sesK16ParamP;
    internalParameters[5] = _twtaK11ParamP;
    internalParameters[6] = _twtaK12ParamP;
    internalParameters[7] = _modeParamP;

    StateElement** stateElements = new StateElement*[8];
    stateElements[0] = &_timeStateElement;
    stateElements[1] = &_sasK19StateElement;
    stateElements[2] = &_sasK20StateElement;
    stateElements[3] = &_sesK15StateElement;
    stateElements[4] = &_sesK16StateElement;
    stateElements[5] = &_twtaK11StateElement;
    stateElements[6] = &_twtaK12StateElement;
    stateElements[7] = &_modeStateElement;

    for (int i=0; i < 8; i++)
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

} // HK2ErrorChecker::SetInternalState

void
HK2ErrorChecker::ReportBasicInfo(
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

    fprintf(ofp, "\n");

    return;

} // HK2ErrorChecker::ReportBasicInfo

//-------//
// Check //
//-------//

int
HK2ErrorChecker::Check(
TlmHdfFile*     tlmFile,
int32           startIndex,
FILE*           ofp)
{
    //-----------------------//
    // set the current state //
    //-----------------------//
    if (SetState(tlmFile, startIndex, prev, current) == 0)
        return 0;
 
    for (int i=0; i < HK2StateTableSize; i++)
    {
        ErrorTabEntry* errorTabEntry = &(HK2StateTable[i]);

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
HK2ErrorChecker::Summarize(
FILE*   ofp)
{
    //------------//
    // basic info //
    //------------//

    fprintf(ofp, "Number of Data Records: %d\n", dataRecCount);

    //--------------//
    // other errors //
    //--------------//

    fprintf(ofp, "\nERROR SUMMARY\n");
    fprintf(ofp, "    Frames   Percentage   Error Description\n");

    for (int i=0; i < HK2StateTableSize; i++)
    {
        ErrorTabEntry* errorTabEntry = &(HK2StateTable[i]);
        if (! errorTabEntry->name)
            continue;
        fprintf(ofp, "%10d   %10.4f   %s\n", errorTabEntry->count,
            PERCENT(errorTabEntry->count), errorTabEntry->name);
    }

    if (! AnyErrors())
        fprintf(ofp, "\nNo errors detected.\n");

    return;

}//HK2ErrorChecker::Summarize

//-----------//
// AnyErrors //
//-----------//
// returns non-zero if there were any errors

int
HK2ErrorChecker::AnyErrors()
{
    for (int i=0; i < HK2StateTableSize; i++)
    {
        ErrorTabEntry* errorTabEntry = &(HK2StateTable[i]);
        if (! errorTabEntry->name)
            continue;
        if (errorTabEntry->count > 0)
            return(1);
    }
    return 0;

}//HK2ErrorChecker::AnyErrors
