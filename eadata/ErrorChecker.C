//=========================================================
// Copyright  (C)1995, California Institute of Technology.
// U.S. Government sponsorship under
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
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
ErrorTwta1BodyOcTrip(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_TWT_1_BODY_OC_TRIP]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_TWT_1_BODY_OC_TRIP]);

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
        fprintf(ofp, "  %s   TWTA 1 Body OC = %d (%s)\n",
            obj->current->time_string, currentTrip, tripString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorTwta1BodyOcTrip

int
ErrorTwta2BodyOcTrip(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_TWT_2_BODY_OC_TRIP]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_TWT_2_BODY_OC_TRIP]);

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
        fprintf(ofp, "  %s   TWTA 2 Body OC = %d (%s)\n",
            obj->current->time_string, currentTrip, tripString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorTwta2BodyOcTrip

int
ErrorTwta1CnvrtOcTrip(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_TWT_1_CNVRT_OC_TRIP]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_TWT_1_CNVRT_OC_TRIP]);

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
        fprintf(ofp, "  %s   TWTA 1 Converter OC = %d (%s)\n",
            obj->current->time_string, currentTrip, tripString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorTwta1CnvrtOcTrip

int
ErrorTwta2CnvrtOcTrip(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_TWT_2_CNVRT_OC_TRIP]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_TWT_2_CNVRT_OC_TRIP]);

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
        fprintf(ofp, "  %s   TWTA 2 Converter OC = %d (%s)\n",
            obj->current->time_string, currentTrip, tripString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorTwta2CnvrtOcTrip

int
ErrorTwta1UvTrip(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_TWT_1_UV_TRIP]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_TWT_1_UV_TRIP]);

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
        fprintf(ofp, "  %s   TWTA 1 Undervoltage = %d (%s)\n",
            obj->current->time_string, currentTrip, tripString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorTwta1UvTrip

int
ErrorTwta2UvTrip(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_TWT_2_UV_TRIP]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_TWT_2_UV_TRIP]);

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
        fprintf(ofp, "  %s   TWTA 2 Undervoltage = %d (%s)\n",
            obj->current->time_string, currentTrip, tripString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorTwta2UvTrip

int
ErrorRomStartError(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_ROM_START_ERROR]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_ROM_START_ERROR]);

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
        fprintf(ofp, "  %s   ROM Start Up Error = %d (%s)\n",
            obj->current->time_string, currentTrip, tripString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorRomStartError

int
ErrorRamStartError(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_RAM_START_ERROR]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_RAM_START_ERROR]);

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
        fprintf(ofp, "  %s   RAM Start Up Error = %d (%s)\n",
            obj->current->time_string, currentTrip, tripString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorRamStartError

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

    unsigned short currentCnt = *(unsigned char*) currentElement->value;
    unsigned short prevCnt = *(unsigned char*) prevElement->value;

    // changed
    if (prevCnt != currentCnt ||
            (currentCnt != 0 &&
                   prevElement->condition == StateElement::UNINITIALIZED))
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

int
ErrorTwtTripOvrd(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_TWT_TRIP_OVERRIDE]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_TWT_TRIP_OVERRIDE]);

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
        fprintf(ofp, "  %s   TWT Trip Override Enabled = %d (%s)\n",
            obj->current->time_string, currentTrip, tripString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorTwtTripOvrd

int
ErrorTwtaMonitor(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_TWTA_MONITOR_DISABLE]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_TWTA_MONITOR_DISABLE]);

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
        fprintf(ofp, "  %s   TWT Monitor Disabled = %d (%s)\n",
            obj->current->time_string, currentTrip, tripString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorTwtaMonitor

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
ErrorUnexpectedCycle(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_DOPPLER_ORBIT_STEP]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_DOPPLER_ORBIT_STEP]);

    if (currentElement->condition != StateElement::CURRENT ||
                 prevElement->condition != StateElement::CURRENT)
        return 1;

    unsigned char currentCycle = *(unsigned char*) currentElement->value;
    unsigned char prevCycle = *(unsigned char*) prevElement->value;

    // changed (the orbit step is btwn 0 and 255)
    if ( ! (prevCycle == 255 && currentCycle == 0 ) &&
                              currentCycle != prevCycle + 1)
    {
        fprintf(ofp, format, name);
        fprintf(ofp, "  %s   Unexpected Cycle = %d, previous was = %d\n",
            obj->current->time_string, currentCycle, prevCycle);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorUnexpectedCycle

int
ErrorModeChange(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_MODE_CHANGE]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_MODE_CHANGE]);

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
            tripString = "changed";
        else
            tripString = "same";
        fprintf(ofp, "  %s   Mode Change = %d (%s)\n",
            obj->current->time_string, currentTrip, tripString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorModeChange

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

int
ErrorTrsCmdSucc(
ErrorChecker*   obj,
char*           name,
char*           format,
FILE*           ofp)
{
    StateElement* prevElement =
            &(obj->prev->stateElements[ERROR_TRS_CMD_SUCCESS]);
    StateElement* currentElement =
            &(obj->current->stateElements[ERROR_TRS_CMD_SUCCESS]);

    if (currentElement->condition != StateElement::CURRENT)
        return 1;

    unsigned char currentValue = *(unsigned char*) currentElement->value;
    unsigned char prevValue = *(unsigned char*) prevElement->value;

    // changed
    if (prevValue != currentValue ||
            (currentValue == 0 &&
                   prevElement->condition == StateElement::UNINITIALIZED))
    {
        fprintf(ofp, format, name);
        char* valueString;
        if (currentValue == 1)
            valueString = "normal";
        else
            valueString = "failure";
        fprintf(ofp, "  %s   TRS Command = %d (%s)\n",
            obj->current->time_string, currentValue, valueString);
        obj->ReportBasicInfo(ofp);
        fflush(ofp);
        return 0;
    }

    return 1;

} // ErrorTrsCmdSucc
