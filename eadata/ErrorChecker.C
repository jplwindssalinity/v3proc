//=========================================================
// Copyright  (C)1995, California Institute of Technology.
// U.S. Government sponsorship under
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.11   16 Sep 1999 08:59:16   sally
// check TWT errors only when that TWT is selected
// 
//    Rev 1.10   15 Sep 1999 15:18:26   sally
// change some default values, fix logic
// 
//    Rev 1.9   26 Jul 1999 16:49:16   sally
// Group flag for L1A is 1, Hk2 is 3
// 
//    Rev 1.8   26 Jul 1999 15:43:02   sally
// add "Source Sequence Count" and "group flag" checking
// 
//    Rev 1.7   09 Jul 1999 16:06:00   sally
// add error checkers for "Source Sequence Count" and "Group Flag" in header
// 
//    Rev 1.6   25 Jun 1999 10:30:14   sally
// add some error conditions from Lee
// 
//    Rev 1.5   20 May 1998 11:19:28   sally
// handles dummy error checker (place holders)
// 
//    Rev 1.4   19 May 1998 15:49:18   sally
// move some function from L1AErrorChecker to ErrorChecker
// 
//    Rev 1.3   18 May 1998 14:47:44   sally
// added error checker for L1A
// 
//    Rev 1.2   13 May 1998 16:28:04   sally
 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

static const char rcs_id[] =
    "@(#) $Header$";

#include <assert.h>
#include <stdio.h>

#include "Parameter.h"
#include "ErrorChecker.h"
#include "State.h"

//======================//
// ErrorChecker methods //
//======================//

ErrorChecker::ErrorChecker()
:	prev(0), current(0), _errorStateTableSize(0)
{
    dataRecCount = 0;
    return;
};

ErrorChecker::~ErrorChecker()
{
    if (current) delete current;
    if (prev) delete prev;
    return;
};

int
ErrorChecker::OpenParamDataSets(
TlmHdfFile*    tlmFile)
{
    assert(_errorStateTableSize > 0);

    for (int i=0; i < _errorStateTableSize; i++)
    {
        // skip the NULL error checkers
        if (parametersP[i] == NONE_ERROR_CHECKER)
            continue;

        int32 dataType, dataStartIndex, dataLength, numDimensions;
        char tempString[BIG_SIZE];
        assert(parametersP[i] != 0 && parametersP[i]->sdsNames != 0);
        (void)strncpy(tempString, parametersP[i]->sdsNames, BIG_SIZE);
        char* oneSdsName=0;
        int j=0;
        for (oneSdsName = (char*)strtok(tempString, ",");
                           oneSdsName;
                           oneSdsName = (char*)strtok(0, ","), j++)
        {
            parametersP[i]->sdsIDs[j] = HDF_FAIL;
            parametersP[i]->sdsIDs[j] = tlmFile->SelectDataset(
                                         oneSdsName, dataType, dataStartIndex,
                                         dataLength, numDimensions);
            if (parametersP[i]->sdsIDs[j] == HDF_FAIL)
                return 0;
        }
    }
    return 1;
 
}//ErrorChecker::OpenParamDataSets

void
ErrorChecker::CloseParamDataSets(
TlmHdfFile*    tlmFile)
{
    assert(_errorStateTableSize > 0);
 
    for (int i=0; i < _errorStateTableSize; i++)
    {
        // skip the NULL error checkers
        if (parametersP[i] == NONE_ERROR_CHECKER)
            continue;

        assert(parametersP[i] != 0);
        for (int j=0; j < parametersP[i]->numSDSs; j++)
        {
            (void)tlmFile->CloseDataset(parametersP[i]->sdsIDs[j]);
            parametersP[i]->sdsIDs[j] = HDF_FAIL;
        }
    }
    return;
 
}//ErrorChecker::CloseParamDataSets

int
ErrorChecker::SetState(
TlmHdfFile*     tlmFile,
int32           startIndex,
State*          lastState,
State*          newState)
{
    assert(_errorStateTableSize > 0);
    for (int i=0; i < _errorStateTableSize; i++)
    {
        // skip the NULL error checkers
        if (parametersP[i] == NONE_ERROR_CHECKER)
            continue;

        assert(parametersP[i] != 0);
        if (newState->StateExtract(parametersP[i]->extractFunc,
                      tlmFile, parametersP[i]->sdsIDs,
                      startIndex,
                      lastState->stateElements[i],
                      newState->stateElements[i],
                      newState->stateElements[i].value)
                                     == StateElement::ERROR)
            return 0;
    }
    return 1;

} // ErrorChecker::SetState

#if 0
//-----------------------//
// Error_Flag_Transition //
//-----------------------//
// prints an error message
// returns 1 on error in frame (whether or not reported), 0 otherwise

int
Error_Flag_Transition(
    FILE*               ofp,
    char*               error_name,
    char*               parameter_name,
    char*               label_format,
    State*              current_state,
    StateElement*       current_elem,
    State*              prev_state,
    StateElement*       prev_elem,
    const char*         map[],
    unsigned char       ok_value)
{
    if (current_elem->condition == StateElement::UNINITIALIZED)
        return (0);     // current is invalid

    // current is valid
    if (current_elem->value == ok_value)
    {
        // current is ok
        if (prev_elem->condition != StateElement::UNINITIALIZED &&
            prev_elem->value != ok_value)
        {
            // previous is valid but bad - error cleared
            fprintf(ofp, CLEAR_FORMAT, error_name);
            fprintf(ofp, "  %s  %s = %d (%s)\n",
                prev_state->time_string, parameter_name,
                prev_elem->value, map[prev_elem->value]);
            ReportBasicInfo(ofp, prev_state);
            fprintf(ofp, "  %s  %s = %d (%s)\n",
                current_state->time_string, parameter_name,
                current_elem->value, map[current_elem->value]);
            ReportBasicInfo(ofp, current_state);
            fflush(ofp);
        }
        return(0);
    }
    else
    {
        // current is bad
        if (prev_elem->condition == StateElement::UNINITIALIZED)
        {
            // starts out as bad - let user know
            fprintf(ofp, INITIALIZE_FORMAT, error_name);
            fprintf(ofp, "  %s  %s = %d (%s)\n",
                current_state->time_string, parameter_name,
                current_elem->value, map[current_elem->value]);
            ReportBasicInfo(ofp, current_state);
            fflush(ofp);
        }
        else if (prev_elem->value == ok_value)
        {
            // previous is ok - transition to bad
            fprintf(ofp, label_format, error_name);
            fprintf(ofp, "  %s  %s = %d (%s)\n",
                prev_state->time_string, parameter_name,
                prev_elem->value, map[prev_elem->value]);
            ReportBasicInfo(ofp, prev_state);
            fprintf(ofp, "  %s  %s = %d (%s)\n",
                current_state->time_string, parameter_name,
                current_elem->value, map[current_elem->value]);
            ReportBasicInfo(ofp, current_state);
            fflush(ofp);
        }
        else if (current_elem->value != prev_elem->value)
        {
            // still bad, but a different bad
            fprintf(ofp, label_format, error_name);
            fprintf(ofp, "  %s  %s = %d (%s)\n",
                prev_state->time_string, parameter_name,
                prev_elem->value, map[prev_elem->value]);
            ReportBasicInfo(ofp, prev_state);
            fprintf(ofp, "  %s  %s = %d (%s)\n",
                current_state->time_string, parameter_name,
                current_elem->value, map[current_elem->value]);
            ReportBasicInfo(ofp, current_state);
            fflush(ofp);
        }
        return(1);  // frame is bad
    }
    return(0);      // should never get here, but let's be safe
}
#endif

int
ErrorCheckMode(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    unsigned char mode = 
          *(unsigned char*)obj->current->stateElements[ERROR_MODE].value;
    if (mode == 0x0e || mode == 0xe0 || mode == 0x70 || mode == 0x07)
        // pass
        return 1;
    else
    {
        // value out of range
        const char* modeString;
        int eaModeNo = L1ModeToTlmMode(mode);
        if (eaModeNo < 0)
            modeString = "unknown mode";
        else
            modeString = mode_map[eaModeNo];
        fprintf(ofp, format, name);
        fprintf(ofp, "  %s  Mode = %d (%s)\n",
            obj->current->time_string, mode, modeString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }
    
} //ErrorCheckMode

int
ErrorRxProtect(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    unsigned char mode = 
        *(unsigned char*)obj->current->stateElements[ERROR_MODE].value;
    unsigned char rxProtect = 
        *(unsigned char*)obj->current->stateElements[ERROR_RX_PROTECT].value;

    // if SBM then rx protect must be 1, 0 all other modes
    if ( (mode == L1_MODE_SBM && rxProtect != L1_RX_PROTECT_ON) ||
               (mode != L1_MODE_SBM && rxProtect == L1_RX_PROTECT_ON) )
    {
        // value out of range
        const char* modeString;
        int eaModeNo = L1ModeToTlmMode(mode);
        if (eaModeNo < 0)
            modeString = "unknown mode";
        else
            modeString = mode_map[eaModeNo];
        fprintf(ofp, format, name);
        fprintf(ofp, "  %s   Rx Protect = %d, Mode = %d (%s)\n",
            obj->current->time_string, rxProtect, mode, modeString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorRxProtect

int
ErrorGridDisable(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    unsigned char mode = 
        *(unsigned char*)obj->current->stateElements[ERROR_MODE].value;
    unsigned char gridDisable = 
        *(unsigned char*)obj->current->stateElements[ERROR_GRID].value;

    // if SBM or ROM then grid disable must be 1, 0 all other modes
    if ( ((mode == L1_MODE_SBM || mode == L1_MODE_ROM) &&
                            gridDisable != L1_GRID_DISABLE) ||
               ((mode == L1_MODE_WOM || mode == L1_MODE_CBM) &&
                            gridDisable != L1_GRID_NORMAL) )
    {
        // value out of range
        const char* modeString;
        int eaModeNo = L1ModeToTlmMode(mode);
        if (eaModeNo < 0)
            modeString = "unknown mode";
        else
            modeString = mode_map[eaModeNo];
        fprintf(ofp, format, name);
        fprintf(ofp, "  %s   Grid Disable = %d, Mode = %d (%s)\n",
            obj->current->time_string, gridDisable, mode, modeString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorGridDisable

int
ErrorRunningErrors(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_RUNNING_ERROR_COUNT]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_RUNNING_ERROR_COUNT]);

    if (currentElement->condition != StateElement::CURRENT)
        return 1;

    unsigned short currentCnt = *(unsigned short*) currentElement->value;
    unsigned short prevCnt = *(unsigned short*) prevElement->value;

    // changed
    if (prevElement->condition == StateElement::CURRENT &&
                                   prevCnt != currentCnt) 
    {
        fprintf(ofp, format, name);
        fprintf(ofp, "  %s   Error Count = %d\n",
            obj->current->time_string, currentCnt);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorRunningErrors

#if 0
int
ErrorTwtaMonEnDis(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_TWTA_MON_EN_DIS]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_TWTA_MON_EN_DIS]);

    if (currentElement->condition != StateElement::CURRENT)
        return 1;

    unsigned char currentTrip = *(unsigned char*) currentElement->value;
    unsigned char prevTrip = *(unsigned char*) prevElement->value;

    // changed
    if (prevTrip != currentTrip ||
            (currentTrip == 0 &&
                   prevElement->condition == StateElement::UNINITIALIZED))
    {
        fprintf(ofp, format, name);
        char* tripString;
        if (currentTrip == 1)
            tripString = "enabled";
        else
            tripString = "disabled";
        fprintf(ofp, "  %s   TWT Monitor En/Dis = %d (%s)\n",
            obj->current->time_string, currentTrip, tripString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorTwtaMonEnDis
#endif

int
ErrorTwtShutdown(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_TWT_SHUTDOWN_DISABLE]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_TWT_SHUTDOWN_DISABLE]);

    if (currentElement->condition != StateElement::CURRENT)
        return 1;

    unsigned char currentTrip = *(unsigned char*) currentElement->value;
    unsigned char prevTrip = *(unsigned char*) prevElement->value;

    // changed
    if (prevTrip != currentTrip ||
            (currentTrip == 1 &&
                   prevElement->condition == StateElement::UNINITIALIZED))
    {
        fprintf(ofp, format, name);
        char* tripString;
        if (currentTrip == 1)
            tripString = "tripped";
        else
            tripString = "cleared";
        fprintf(ofp, "  %s   TWT Shutdown Disabled = %d (%s)\n",
            obj->current->time_string, currentTrip, tripString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorTwtShutdown

int
ErrorBadSrcSeq(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp,
int             delta)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_SRC_SEQ_COUNT]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_SRC_SEQ_COUNT]);

    // if this is the first extracted value, ignore this (the value will be bad)
    if (prevElement->condition != StateElement::CURRENT || 
           currentElement->condition != StateElement::CURRENT)
        return 1;

    unsigned short currentSrcSeq = *(unsigned short*) currentElement->value;

    // the value must be == delta
    if (currentSrcSeq != delta)
    {
        fprintf(ofp, format, name);
        fprintf(ofp, "  %s   Delta Source Sequence Count = %d\n",
            obj->current->time_string, currentSrcSeq);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorBadSrcSeq

int
ErrorHk2BadSrcSeq(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    return(ErrorBadSrcSeq(obj, name, format, ofp, 1));

} // ErrorHk2BadSrcSeq

int
ErrorL1ABadSrcSeq(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    return(ErrorBadSrcSeq(obj, name, format, ofp, 3));

} // ErrorL1ABadSrcSeq

int
ErrorBadGrpFlg(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp,
int             delta)
{
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_HEADER_GROUP_FLAG]);

    if (currentElement->condition != StateElement::CURRENT)
        return 1;

    unsigned char currentGrpFlg = *(unsigned char*) currentElement->value;

    // the value must be == delta
    if (currentGrpFlg != delta)
    {
        fprintf(ofp, format, name);
        fprintf(ofp, "  %s   Group Flag = %d\n",
                          obj->current->time_string, currentGrpFlg);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorBadGrpFlg

int
ErrorL1ABadGrpFlg(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    return(ErrorBadGrpFlg(obj, name, format, ofp, 1));

} // ErrorL1ABadGrpFlg

int
ErrorHk2BadGrpFlg(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    return(ErrorBadGrpFlg(obj, name, format, ofp, 3));
 
} // ErrorHk2BadGrpFlg
 

int
ErrorValidCmdCnt(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_VALID_COMMAND_COUNT]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_VALID_COMMAND_COUNT]);

    if (prevElement->condition != StateElement::CURRENT || 
           currentElement->condition != StateElement::CURRENT)
        return 1;

    unsigned char currentCnt = *(unsigned char*) currentElement->value;
    unsigned char prevCnt = *(unsigned char*) prevElement->value;

    // changed
    if (prevCnt != currentCnt)
    {
        fprintf(ofp, format, name);
        fprintf(ofp, "  %s   Valid Command Count = %d (was %d)\n",
            obj->current->time_string, currentCnt, prevCnt);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorValidCmdCnt

int
ErrorInvalidCmdCnt(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_INVALID_COMMAND_COUNT]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_INVALID_COMMAND_COUNT]);

    if (prevElement->condition != StateElement::CURRENT || 
           currentElement->condition != StateElement::CURRENT)
        return 1;

    unsigned char currentCnt = *(unsigned char*) currentElement->value;
    unsigned char prevCnt = *(unsigned char*) prevElement->value;

    // changed
    if (prevCnt != currentCnt)
    {
        fprintf(ofp, format, name);
        fprintf(ofp, "  %s   Invalid Command Count = %d (was %d)\n",
            obj->current->time_string, currentCnt, prevCnt);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorInvalidCmdCnt

int
ErrorModeChangeMismatch(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* changeCurrentElement =
            &(obj->current->stateElements[ERROR_MODE_CHANGE]);
    StateElement* modeCurrentElement =
            &(obj->current->stateElements[ERROR_MODE]);
    if (changeCurrentElement->condition != StateElement::CURRENT || 
               modeCurrentElement->condition != StateElement::CURRENT)
        return 1;

    StateElement* modePrevElement =
            &(obj->prev->stateElements[ERROR_MODE]);
    int modeChanged = DefaultStateCompareFunc(*modePrevElement,
            *modeCurrentElement, modeCurrentElement->byteSize);

    unsigned char changeBit = *(unsigned char*) changeCurrentElement->value;

    // if two condition don't agree, error
    if (modeChanged && ! changeBit)
    {
        fprintf(ofp, format, name);
        fprintf(ofp, "  %s   Mode Changed from %d to %d, "
                     "but Mode Change bit is 0\n",
                     obj->current->time_string,
                     *(unsigned char*) modePrevElement->value,
                     *(unsigned char*) modeCurrentElement->value);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }
    else if ( ! modeChanged && changeBit)
    {
        fprintf(ofp, format, name);
        fprintf(ofp, "  %s  Mode Change bit is 1, but Mode (%d) is unchanged\n",
                     obj->current->time_string,
                     *(unsigned char*) modeCurrentElement->value);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorModeChange

// general function for error if bit value == 1
int
ErrorValueSet(
ErrorChecker*        obj,
char*                name,
char*                format,
FILE*                ofp,
ERROR_CHECK_PARAM_E  errorIndex,
unsigned char        goodValue)
{
    unsigned char flag = 
          *(unsigned char*)obj->current->stateElements[errorIndex].value;
    if (flag == goodValue)
        // pass
        return 1;
    else
    {
        // event occured
        fprintf(ofp, format, name);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }
    
} //ErrorValueSet

int ErrorCdsReset(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format, ofp, ERROR_CDS_RESET, 0)); }

int ErrorA2DTimeout(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format, ofp, ERROR_A2D_TIMEOUT, 0)); }

int ErrorMissSCTime(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format, ofp, ERROR_MISS_SC_TIME, 0)); }

int ErrorMssnSerlTlm(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format, ofp, ERROR_MSSN_SERL_TLM, 0)); }

int ErrorFaultProtect(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format, ofp, ERROR_FAULT_PROTECT, 0)); }

int ErrorWatchdogTimeout(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format, ofp, ERROR_WATCHDOG_TIMEOUT,0)); }

int ErrorPowerOnReset(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format, ofp, ERROR_POWER_ON_RESET, 0)); }

int ErrorMssnTlmTablesChanged(ErrorChecker* obj, char* name,
                                     char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format,
                                     ofp, ERROR_MSSN_TLM_TABLES_CHANGED, 0)); }
int ErrorSerlTlmTablesChanged(ErrorChecker* obj, char* name,
                                     char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format,
                                     ofp, ERROR_SERL_TLM_TABLES_CHANGED, 0)); }
int ErrorDopplerTablesChanged(ErrorChecker* obj, char* name,
                                     char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format,
                                     ofp, ERROR_DOPPLER_TABLES_CHANGED, 0)); }
int ErrorRangeGateTablesChanged(ErrorChecker* obj, char* name,
                                     char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format,
                                     ofp, ERROR_RANGE_GATE_TABLES_CHANGED,0)); }
int ErrorSESParamTablesChanged(ErrorChecker* obj, char* name,
                                     char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format,
                                     ofp, ERROR_SES_PARAM_TABLES_CHANGED, 0)); }

int ErrorCdsHardReset(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format, ofp, ERROR_CDS_HARD_RESET, 0)); }

int ErrorRelayChanged(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format, ofp, ERROR_RELAY_CHANGE, 0)); }

int ErrorSoftReset(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format, ofp, ERROR_SOFT_RESET, 0)); }

int ErrorModeChange(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format, ofp, ERROR_MODE_CHANGE, 0)); }

int ErrorWriteTestFailed(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format, ofp, ERROR_MEMORY_WRITE_FAIL,0));}

int ErrorMemWriteProtViolate(ErrorChecker* obj, char* name,
                                         char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format,
                                         ofp, ERROR_MEM_WRITE_PROT_VIOLATE,0));}

int ErrorTrsCmdSucc(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format, ofp, ERROR_TRS_CMD_SUCCESS,1));}

int ErrorSesWatchDogTimeout(ErrorChecker* obj, char* name,
                                           char* format, FILE* ofp)
{ return(ErrorValueSet(obj, name, format, ofp, ERROR_SES_WATCH_DOG_TIMEOUT,0));}

int ErrorRamStartError(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorValueSet(obj, name, format, ofp, ERROR_RAM_START_ERROR,0));}

int ErrorSesRomStartError(ErrorChecker* obj, char* name, char* format,FILE* ofp)
  { return(ErrorValueSet(obj, name, format, ofp, ERROR_SES_ROM_START_ERROR,0));}

int ErrorSesReset(ErrorChecker* obj, char* name, char* format,FILE* ofp)
{ return(ErrorValueSet(obj, name, format, ofp, ERROR_SES_RESET,0));}

int ErrorPortParityError(ErrorChecker* obj, char* name, char* format,FILE* ofp)
{ return(ErrorValueSet(obj, name, format, ofp, ERROR_SES_PORT_PARITY_ERROR,0));}

// general function for error if value changes from previous
int
ErrorValueChanged(
ErrorChecker*        obj,
char*                name,
char*                format,
FILE*                ofp,
ERROR_CHECK_PARAM_E  errorIndex,
unsigned char        defaultValue,
const char*          defaultString,
const char*          badString)
{
    StateElement* prevElement = &(obj->prev->stateElements[errorIndex]);
    StateElement* currentElement = &(obj->current->stateElements[errorIndex]);

    if (currentElement->condition != StateElement::CURRENT)
        return 1;

    unsigned char currentTrip = *(unsigned char*) currentElement->value;
    unsigned char prevTrip = *(unsigned char*) prevElement->value;

    // changed or initial value is the default
    if ((prevElement->condition == StateElement::CURRENT &&
         prevTrip != currentTrip) ||
            (prevElement->condition == StateElement::UNINITIALIZED &&
                                    currentTrip != defaultValue))
    {
        fprintf(ofp, format, name);
        const char* valueString;
        if (currentTrip == defaultValue)
            valueString = defaultString;
        else
            valueString = badString;
        fprintf(ofp, "  %s   %s = %d (%s)\n",
            obj->current->time_string, name, currentTrip, valueString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorValueChanged

int ErrorTwtaMonEnDis(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorValueChanged(obj, name, format, ofp,
                      ERROR_TWTA_MON_EN_DIS, 1, "Enabled", "Disabled")); }

int ErrorPwrMonFPEndis(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorValueChanged(obj, name, format, ofp,
                      ERROR_TWTA_MON_EN_DIS, 1, "Enabled", "Disabled")); }

int ErrorSesSuppHtrMode(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorValueChanged(obj, name, format, ofp,
                      ERROR_SUPP_HTR_MODE_CHANGE, 1, "Enabled", "Disabled")); }

int ErrorSasMultiDataLoss(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueChanged(obj, name, format, ofp,
                      ERROR_SAS_MULTI_DATA_LOSS, 1, "Enabled", "Disabled")); }

int ErrorSesMultiDataLoss(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueChanged(obj, name, format, ofp,
                      ERROR_SES_MULTI_DATA_LOSS, 1, "Enabled", "Disabled")); }

int ErrorEqXingMissed(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueChanged(obj, name, format, ofp,
                      ERROR_EQ_XING_MISSED, 0, "Received", "Missed")); }

int ErrorModulation(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueChanged(obj, name, format, ofp,
                      ERROR_MODULATION, 1, "On", "Off")); }

int ErrorTwtTripOverride(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueChanged(obj, name, format, ofp,
                      ERROR_TWT_TRIP_OVERRIDE, 0, "Off", "On")); }

int ErrorUnexpectedState(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueChanged(obj, name, format, ofp,
                      ERROR_PBI_UNEXPECTED_STATE, 0, "Expected","Unexpected"));}

int ErrorR2Modified(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueChanged(obj, name, format, ofp,
                      ERROR_R2_MODIFIED, 0, "Not Modified","Modified"));}

int ErrorBCRTMFailure(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueChanged(obj, name, format, ofp,
                      ERROR_BCRTM_FAILURE, 0, "No Failure","Failure"));}

int ErrorWatchDogTimerExpired(ErrorChecker* obj, char* name,
                                        char* format,FILE* ofp)
    { return(ErrorValueChanged(obj, name, format, ofp,
                  ERROR_WATCH_DOG_TIMER_EXPIRED, 0, "Not Expired","Expired"));}

ErrorTwt1ValueChanged(
ErrorChecker*        obj,
char*                name,
char*                format,
FILE*                ofp,
ERROR_CHECK_PARAM_E  errorIndex,
unsigned char        defaultValue,
const char*          defaultString,
const char*          badString)
{
    StateElement* k11Element =
                &(obj->current->stateElements[ERROR_K11_TWTA_1_2]);
    unsigned char k11 = *(unsigned char*) k11Element->value;
    StateElement* k12Element =
                &(obj->current->stateElements[ERROR_K12_TWTA_1_2]);
    unsigned char k12 = *(unsigned char*) k12Element->value;

    // if k11 != k12, then TWT 1 is not selected
    if (k11 != k12) return 1;

    return(ErrorValueChanged(obj, name, format, ofp,
                      errorIndex, defaultValue, defaultString, badString));

} // ErrorTwt1ValueChanged

int ErrorTwt1CnvrtOcTrip(ErrorChecker* obj, char* name, char* format, FILE* ofp)
    { return(ErrorTwt1ValueChanged(obj, name, format, ofp,
                      ERROR_TWT_1_CNVRT_OC_TRIP, 0, "Not Tripped","Tripped"));}

int ErrorTwt1UvTrip(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorTwt1ValueChanged(obj, name, format, ofp,
                      ERROR_TWT_1_UV_TRIP, 0, "Not Tripped","Tripped"));}

int ErrorTwt1BodyOcTrip(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorTwt1ValueChanged(obj, name, format, ofp,
                      ERROR_TWT_1_BODY_OC_TRIP, 0, "Not Tripped","Tripped"));}

ErrorTwt2ValueChanged(
ErrorChecker*        obj,
char*                name,
char*                format,
FILE*                ofp,
ERROR_CHECK_PARAM_E  errorIndex,
unsigned char        defaultValue,
const char*          defaultString,
const char*          badString)
{
    StateElement* k11Element =
                &(obj->current->stateElements[ERROR_K11_TWTA_1_2]);
    unsigned char k11 = *(unsigned char*) k11Element->value;
    StateElement* k12Element =
                &(obj->current->stateElements[ERROR_K12_TWTA_1_2]);
    unsigned char k12 = *(unsigned char*) k12Element->value;

    // if k11 == k12, then TWT 2 is not selected
    if (k11 == k12) return 1;

    return(ErrorValueChanged(obj, name, format, ofp,
                      errorIndex, defaultValue, defaultString, badString));

} // ErrorTwt1ValueChanged

int ErrorTwt2CnvrtOcTrip(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorTwt2ValueChanged(obj, name, format, ofp,
                      ERROR_TWT_2_CNVRT_OC_TRIP, 0, "Not Tripped","Tripped"));}

int ErrorTwt2UvTrip(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorTwt2ValueChanged(obj, name, format, ofp,
                      ERROR_TWT_2_UV_TRIP, 0, "Not Tripped","Tripped"));}

int ErrorTwt2BodyOcTrip(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorTwt2ValueChanged(obj, name, format, ofp,
                      ERROR_TWT_2_BODY_OC_TRIP, 0, "Not Tripped","Tripped"));}

int ErrorPllOutOfLock(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueChanged(obj, name, format, ofp,
                      ERROR_PLL_OUT_OF_LOCK, 0, "Normal","Out of Lock"));}

int ErrorBodyOcTripCntl(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueChanged(obj, name, format, ofp,
                      ERROR_BODY_OC_TRIP_CNTL, 0, "Normal","Disabled"));}

//-----------------------------------------------------------
// general function for error if value toggles, no default
//-----------------------------------------------------------
int
ErrorValueToggled(
ErrorChecker*        obj,
char*                name,
char*                format,
FILE*                ofp,
ERROR_CHECK_PARAM_E  errorIndex,
unsigned char        thisValue,
const char*          thisString,
const char*          otherString)
{
    StateElement* prevElement = &(obj->prev->stateElements[errorIndex]);
    StateElement* currentElement = &(obj->current->stateElements[errorIndex]);

    if (currentElement->condition != StateElement::CURRENT)
        return 1;

    unsigned char currentTrip = *(unsigned char*) currentElement->value;
    unsigned char prevTrip = *(unsigned char*) prevElement->value;

    // changed or initial value is the default
    if (prevTrip != currentTrip &&
            prevElement->condition != StateElement::UNINITIALIZED)
    {
        fprintf(ofp, format, name);
        const char* valueString;
        if (currentTrip == thisValue)
            valueString = thisString;
        else
            valueString = otherString;
        fprintf(ofp, "  %s   %s = %d (%s)\n",
            obj->current->time_string, name, currentTrip, valueString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorValueToggled

int ErrorK15SesAB(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueToggled(obj, name, format, ofp,
                      ERROR_K15_SES_A_B, 0, "Set","Reset"));}

int ErrorK12Twta12(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueToggled(obj, name, format, ofp,
                      ERROR_K12_TWTA_1_2, 0, "Set","Reset"));}

int ErrorK11Twta12(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueToggled(obj, name, format, ofp,
                      ERROR_K11_TWTA_1_2, 0, "Set","Reset"));}

int ErrorK10TwtaOnOff(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueToggled(obj, name, format, ofp,
                      ERROR_K10_TWTA_ON_OFF, 0, "Set","Reset"));}

int ErrorK9TwtaOnOff(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueToggled(obj, name, format, ofp,
                      ERROR_K9_TWTA_ON_OFF, 0, "Set","Reset"));}

int ErrorK20SasAB(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueToggled(obj, name, format, ofp,
                      ERROR_K20_SAS_A_B, 0, "Set","Reset"));}

int ErrorK19SasAB(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueToggled(obj, name, format, ofp,
                      ERROR_K19_SAS_A_B, 0, "Set","Reset"));}

int ErrorK16SesAB(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueToggled(obj, name, format, ofp,
                      ERROR_K16_SES_A_B, 0, "Set","Reset"));}

int ErrorSasBSpinRate(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueToggled(obj, name, format, ofp,
                      ERROR_SAS_B_SPIN_RATE, 0, "19.8 RPM","18.0 RPM"));}

int ErrorK22SesSuppHtrOnOff(ErrorChecker* obj,char* name,char* format,FILE* ofp)
    { return(ErrorValueToggled(obj, name, format, ofp,
                      ERROR_K22_SES_SUPP_HTR_ON_OFF, 0, "Set","Reset"));}

int ErrorK21SesSuppHtrOnOff(ErrorChecker* obj,char* name,char* format,FILE* ofp)
    { return(ErrorValueToggled(obj, name, format, ofp,
                      ERROR_K21_SES_SUPP_HTR_ON_OFF, 0, "Set","Reset"));}

int ErrorSasASpinRate(ErrorChecker* obj, char* name, char* format,FILE* ofp)
    { return(ErrorValueToggled(obj, name, format, ofp,
                      ERROR_SAS_A_SPIN_RATE, 0, "19.8 RPM","18.0 RPM"));}

