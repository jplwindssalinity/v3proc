//=========================================================//
// Copyright  (C)1995, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#include <assert.h>

#include "LimitState.h"
#include "L1AExtract.h"
#include "ParTab.h"
#include "TlmHdfFile.h"

static const char rcs_id_limit_state_c[] = "@(#) $Id$";

const char* limitStatusStrings[] =
{
    "Within Limits (OK)",
    "Caution Low",
    "Caution High",
    "Action Low",
    "Action High",
    "Limit Checking OK",
    "Invalid Parameter",
    "Extraction Error",
    "Wrong Data Type",
    "Missing SDS Name",
    "Applying Polynomial to Non Float",
    "Polynomial Required Not in Table",
    "Missing Polynomial Table"
};

const char*
GetLimitStatusString(
LimitStatusE   limitStatus)
{
    if (limitStatus < 0 || limitStatus > (int)ElementNumber(limitStatusStrings))
        return("Unknown");
    else
        return(limitStatusStrings[(int)limitStatus]);

} // GetLimitStatusString

//*******************************************************//
//      L1ALimitStatePair
//*******************************************************//
L1ALimitStatePair::L1ALimitStatePair(
int    mode,
int    twt,
int    twta,
int    frame)
: LimitStatePair(),
  stateA(mode, twt, twta, frame), stateB(mode, twt, twta, frame),
  oldState(&stateA), newState(&stateB)
{
    modeParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                        OPERATIONAL_MODE, UNIT_HEX_BYTES);
    k9ParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                      K9_TWTA_POWER, UNIT_MAP);
    k10ParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                      K10_TWTA_POWER, UNIT_MAP);
    k11ParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                      K11_TWTA_SELECT, UNIT_MAP);
    k12ParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                      K12_TWTA_SELECT, UNIT_MAP);
    frameParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                        TRUE_CAL_PULSE_POS, UNIT_DN);
    assert(modeParamP != 0 && k9ParamP != 0 & k10ParamP != 0 &&
           k11ParamP != 0 && k12ParamP != 0 && frameParamP != 0);
} //L1ALimitStatePair::L1ALimitStatePair

// default constructor
L1ALimitStatePair::L1ALimitStatePair()
: LimitStatePair(),
oldState(&stateA), newState(&stateB)
{
    modeParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                        OPERATIONAL_MODE, UNIT_HEX_BYTES);
    k9ParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                      K9_TWTA_POWER, UNIT_MAP);
    k10ParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                      K10_TWTA_POWER, UNIT_MAP);
    k11ParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                      K11_TWTA_SELECT, UNIT_MAP);
    k12ParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                      K12_TWTA_SELECT, UNIT_MAP);
    frameParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                        TRUE_CAL_PULSE_POS, UNIT_DN);
    assert(modeParamP != 0 && k9ParamP != 0 & k10ParamP != 0 &&
           k11ParamP != 0 && k12ParamP != 0 && frameParamP != 0);
}//L1ALimitStatePair::L1ALimitStatePair

// copy constructor
L1ALimitStatePair::L1ALimitStatePair(
const L1ALimitStatePair& old)
: LimitStatePair(old), stateA(old.stateA), stateB(old.stateB),
  oldState(old.oldState), newState(old.newState)
{
    modeParamP = new Parameter(*(old.modeParamP));
    k9ParamP = new Parameter(*(old.k9ParamP));
    k10ParamP = new Parameter(*(old.k10ParamP));
    k11ParamP = new Parameter(*(old.k11ParamP));
    k12ParamP = new Parameter(*(old.k12ParamP));
    frameParamP = new Parameter(*(old.frameParamP));
    assert(modeParamP != 0 && k9ParamP != 0 & k10ParamP != 0 &&
           k11ParamP != 0 && k12ParamP != 0 && frameParamP != 0);

}//1LimitStatePair::L1ALimitStatePair

L1ALimitStatePair::~L1ALimitStatePair()
{
    if (modeParamP) delete modeParamP;
    if (k9ParamP) delete k9ParamP;
    if (k10ParamP) delete k10ParamP;
    if (k11ParamP) delete k11ParamP;
    if (k12ParamP) delete k12ParamP;
    if (frameParamP) delete frameParamP;
    modeParamP = k9ParamP = k10ParamP = k11ParamP = k12ParamP = frameParamP = 0;

}//L1ALimitStatePair::~L1ALimitStatePair

IotBoolean
L1ALimitStatePair::OpenParamDataSets(
TlmHdfFile*    tlmFile)
{
    if (modeParamP == 0 || k9ParamP == 0 || k10ParamP == 0 || 
            k11ParamP == 0 || k12ParamP == 0 || frameParamP == 0)
        return 0;

    int32 dataType, dataStartIndex, dataLength, numDimensions;
    char tempString[BIG_SIZE];

    (void)strncpy(tempString, modeParamP->sdsNames, BIG_SIZE);
    char *oneSdsName = (char*)strtok(tempString, ",");
    if (oneSdsName)
    {
        modeParamP->sdsIDs[0] = HDF_FAIL;
        modeParamP->sdsIDs[0] = tlmFile->SelectDataset(
                                 oneSdsName, dataType, dataStartIndex,
                                 dataLength, numDimensions);
        if (modeParamP->sdsIDs[0] == HDF_FAIL)
            return(0);
    }
    else
        return 0;

    (void)strncpy(tempString, k9ParamP->sdsNames, BIG_SIZE);
    oneSdsName = (char*)strtok(tempString, ",");
    if (oneSdsName)
    {
        k9ParamP->sdsIDs[0] = HDF_FAIL;
        k9ParamP->sdsIDs[0] = tlmFile->SelectDataset(
                                 oneSdsName, dataType, dataStartIndex,
                                 dataLength, numDimensions);
        if (k9ParamP->sdsIDs[0] == HDF_FAIL)
            return(0);
    }
    else
        return 0;

    (void)strncpy(tempString, k10ParamP->sdsNames, BIG_SIZE);
    oneSdsName = (char*)strtok(tempString, ",");
    if (oneSdsName)
    {
        k10ParamP->sdsIDs[0] = HDF_FAIL;
        k10ParamP->sdsIDs[0] = tlmFile->SelectDataset(
                                 oneSdsName, dataType, dataStartIndex,
                                 dataLength, numDimensions);
        if (k10ParamP->sdsIDs[0] == HDF_FAIL)
            return(0);
    }
    else
        return 0;

    (void)strncpy(tempString, k11ParamP->sdsNames, BIG_SIZE);
    oneSdsName = (char*)strtok(tempString, ",");
    if (oneSdsName)
    {
        k11ParamP->sdsIDs[0] = HDF_FAIL;
        k11ParamP->sdsIDs[0] = tlmFile->SelectDataset(
                                 oneSdsName, dataType, dataStartIndex,
                                 dataLength, numDimensions);
        if (k11ParamP->sdsIDs[0] == HDF_FAIL)
            return(0);
    }
    else
        return 0;

    (void)strncpy(tempString, k12ParamP->sdsNames, BIG_SIZE);
    oneSdsName = (char*)strtok(tempString, ",");
    if (oneSdsName)
    {
        k12ParamP->sdsIDs[0] = HDF_FAIL;
        k12ParamP->sdsIDs[0] = tlmFile->SelectDataset(
                                 oneSdsName, dataType, dataStartIndex,
                                 dataLength, numDimensions);
        if (k12ParamP->sdsIDs[0] == HDF_FAIL)
            return(0);
    }
    else
        return 0;

    (void)strncpy(tempString, frameParamP->sdsNames, BIG_SIZE);
    oneSdsName = (char*)strtok(tempString, ",");
    if (oneSdsName)
    {
        frameParamP->sdsIDs[0] = HDF_FAIL;
        frameParamP->sdsIDs[0] = tlmFile->SelectDataset(
                                 oneSdsName, dataType, dataStartIndex,
                                 dataLength, numDimensions);
        if (frameParamP->sdsIDs[0] == HDF_FAIL)
            return(0);
    }
    else
        return 0;

    return 1;

}//L1ALimitStatePair::OpenParamDataSets

IotBoolean
L1ALimitStatePair::CloseParamDataSets(
TlmHdfFile*    tlmFile)
{
    assert(modeParamP->sdsIDs != 0 && k9ParamP->sdsIDs != 0 &&
           k10ParamP->sdsIDs != 0 && k11ParamP->sdsIDs != 0 &&
           k12ParamP->sdsIDs != 0 && frameParamP->sdsIDs != 0);
    // try to close all datasets, then return status
    IotBoolean closeOK = 1;

    if (tlmFile->CloseDataset(modeParamP->sdsIDs[0]) == HDF_FAIL)
        closeOK = 0;
    if (tlmFile->CloseDataset(k9ParamP->sdsIDs[0]) == HDF_FAIL)
        closeOK = 0;
    if (tlmFile->CloseDataset(k10ParamP->sdsIDs[0]) == HDF_FAIL)
        closeOK = 0;
    if (tlmFile->CloseDataset(k11ParamP->sdsIDs[0]) == HDF_FAIL)
        closeOK = 0;
    if (tlmFile->CloseDataset(k12ParamP->sdsIDs[0]) == HDF_FAIL)
        closeOK = 0;
    if (tlmFile->CloseDataset(frameParamP->sdsIDs[0]) == HDF_FAIL)
        closeOK = 0;

    return(closeOK);

}//L1ALimitStatePair::CloseParamDataSets

int
L1ALimitStatePair::offset(void)
{
    return ((newState->mode * HVPS_STATE_COUNT * TWTA_COUNT * FRAME_TYPE_COUNT +
                newState->twt * TWTA_COUNT * FRAME_TYPE_COUNT +
                newState->twta * FRAME_TYPE_COUNT + newState->frame) * 4);

}//L1ALimitStatePair::offset

int
L1ALimitStatePair::numStates(void)
{
    return (NSCAT_MODE_COUNT * HVPS_STATE_COUNT *
                      TWTA_COUNT * FRAME_TYPE_COUNT * 4);

}//L1ALimitStatePair::numStates

void
L1ALimitStatePair::PrintNewState(
FILE*   fp)
{
    _printState(fp, newState);
    fprintf(fp, "\n");

}//L1ALimitStatePair::PrintNewState

inline void
L1ALimitStatePair::_printState(
FILE*               fp,
L1ALimitState*   state)
{
    fprintf(fp, "Mode=%s, TWT=%s, TWTA=%s, Frame=%s",
                    mode_map[state->mode], twt_map[state->twt],
                    twta_map[state->twta], cmf_map[state->frame]);

}//L1ALimitStatePair::PrintNewState

void
L1ALimitStatePair::PrintChange(
FILE*   fp)
{

    _printState(fp, oldState);
    fprintf(fp, " -> ");
    _printState(fp, newState);
    fprintf(fp, "\n");

}//L1ALimitStatePair::PrintChange

char
L1ALimitStatePair::ApplyNewFrame(
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    // get mode, twt and frame
    unsigned char buffer=0;
    int mode, frame;
    if ( ! modeParamP->extractFunc(tlmFile, modeParamP->sdsIDs,
                     startIndex, 1, 1, &buffer, 0))
        return 0;
    mode = (int) buffer;
   
    switch(mode)
    {
        case L1_MODE_WOM:
            oldState->mode = TLM_MODE_WOM;
            break;
        case L1_MODE_CBM:
            oldState->mode = TLM_MODE_CBM;
            break;
        case L1_MODE_SBM:
            oldState->mode = TLM_MODE_SBM;
            break;
        case L1_MODE_ROM:
            oldState->mode = TLM_MODE_ROM;
            break;
        default:
            return 0;
    }
     
    unsigned char k9, k10;
    if ( ! k9ParamP->extractFunc(tlmFile, k9ParamP->sdsIDs,
                     startIndex, 1, 1, &k9, 0))
        return 0;
    if ( ! k10ParamP->extractFunc(tlmFile, k10ParamP->sdsIDs,
                     startIndex, 1, 1, &k10, 0))
        return 0;
    oldState->twt = (k9 == k10 ? TLM_HVPS_ON : TLM_HVPS_OFF);

    unsigned char k11, k12;
    if ( ! k11ParamP->extractFunc(tlmFile, k11ParamP->sdsIDs,
                     startIndex, 1, 1, &k11, 0))
        return 0;
    if ( ! k12ParamP->extractFunc(tlmFile, k12ParamP->sdsIDs,
                     startIndex, 1, 1, &k12, 0))
        return 0;
    oldState->twta = (k11 == k12 ? TLM_TWTA_1 : TLM_TWTA_2);

    char calPulsePos=0;
    if ( ! frameParamP->extractFunc(tlmFile, frameParamP->sdsIDs,
                     startIndex, 1, 1, &calPulsePos, 0))
        return 0;

    frame = (int) calPulsePos;
    if (frame > 0)
        oldState->frame = TLM_FRAME_CAL;
    else
        oldState->frame = TLM_FRAME_SCI;

    if (*oldState == *newState)
        return (0);
    else
    {
        // limit state changed, swap the pointer and return 1
        L1ALimitState* temp = newState;
        newState = oldState;
        oldState = temp;
        return (1);
    }
}//L1ALimitStatePair::ApplyNewFrame

HK2LimitStatePair::HK2LimitStatePair(
int     mode, 
int     twt, 
int     twta)
: LimitStatePair(),
stateA(mode, twt, twta), stateB(mode, twt, twta),
oldState(&stateA), newState(&stateB)
{
    modeParamP = ParTabAccess::GetParameter(SOURCE_HK2,
                                      OPERATIONAL_MODE, UNIT_HEX_BYTES);
    k9ParamP = ParTabAccess::GetParameter(SOURCE_HK2,
                                      K9_TWTA_POWER, UNIT_MAP);
    k10ParamP = ParTabAccess::GetParameter(SOURCE_HK2,
                                      K10_TWTA_POWER, UNIT_MAP);
    k11ParamP = ParTabAccess::GetParameter(SOURCE_HK2,
                                      K11_TWTA_SELECT, UNIT_MAP);
    k12ParamP = ParTabAccess::GetParameter(SOURCE_HK2,
                                      K12_TWTA_SELECT, UNIT_MAP);
    assert(modeParamP != 0 && k9ParamP != 0 & k10ParamP != 0 &&
           k11ParamP != 0 && k12ParamP != 0);

} // HK2LimitStatePair::HK2LimitStatePair

// default constructor
HK2LimitStatePair::HK2LimitStatePair()
: LimitStatePair(), oldState(&stateA), newState(&stateB)
{
    modeParamP = ParTabAccess::GetParameter(SOURCE_HK2,
                                      OPERATIONAL_MODE, UNIT_MAP);
    k9ParamP = ParTabAccess::GetParameter(SOURCE_HK2,
                                      K9_TWTA_POWER, UNIT_MAP);
    k10ParamP = ParTabAccess::GetParameter(SOURCE_HK2,
                                      K10_TWTA_POWER, UNIT_MAP);
    k11ParamP = ParTabAccess::GetParameter(SOURCE_HK2,
                                      K11_TWTA_SELECT, UNIT_MAP);
    k12ParamP = ParTabAccess::GetParameter(SOURCE_HK2,
                                      K12_TWTA_SELECT, UNIT_MAP);
    assert(modeParamP != 0 && k9ParamP != 0 & k10ParamP != 0 &&
           k11ParamP != 0 && k12ParamP != 0);

} // HK2LimitStatePair::HK2LimitStatePair

// copy constructor
HK2LimitStatePair::HK2LimitStatePair(
const HK2LimitStatePair&       old)
: LimitStatePair(old), stateA(old.stateA), stateB(old.stateB),
  oldState(old.oldState), newState(old.newState)
{
    modeParamP = new Parameter(*(old.modeParamP));
    k9ParamP = new Parameter(*(old.k9ParamP));
    k10ParamP = new Parameter(*(old.k10ParamP));
    k11ParamP = new Parameter(*(old.k11ParamP));
    k12ParamP = new Parameter(*(old.k12ParamP));
    assert(modeParamP != 0 && k9ParamP != 0 & k10ParamP != 0 &&
           k11ParamP != 0 && k12ParamP != 0);

} // HK2LimitStatePair::HK2LimitStatePair

HK2LimitStatePair::~HK2LimitStatePair()
{
    if (modeParamP) delete modeParamP;
    if (k9ParamP) delete k9ParamP;
    if (k10ParamP) delete k10ParamP;
    if (k11ParamP) delete k11ParamP;
    if (k12ParamP) delete k12ParamP;
    modeParamP = k9ParamP = k10ParamP = k11ParamP = k12ParamP = 0;

} // HK2LimitStatePair::~HK2LimitStatePair

IotBoolean
HK2LimitStatePair::OpenParamDataSets(
TlmHdfFile*    tlmFile)
{
    if (modeParamP == 0 || k9ParamP == 0 || k10ParamP == 0 ||
                k11ParamP == 0 || k12ParamP == 0)
        return 0;

    int32 dataType, dataStartIndex, dataLength, numDimensions;
    char tempString[BIG_SIZE];

    (void)strncpy(tempString, modeParamP->sdsNames, BIG_SIZE);
    char *oneSdsName = (char*)strtok(tempString, ",");
    if (oneSdsName)
    {
        modeParamP->sdsIDs[0] = HDF_FAIL;
        modeParamP->sdsIDs[0] = tlmFile->SelectDataset(
                                 oneSdsName, dataType, dataStartIndex,
                                 dataLength, numDimensions);
        if (modeParamP->sdsIDs[0] == HDF_FAIL)
            return(0);
    }
    else
        return 0;

    (void)strncpy(tempString, k9ParamP->sdsNames, BIG_SIZE);
    oneSdsName = (char*)strtok(tempString, ",");
    if (oneSdsName)
    {
        k9ParamP->sdsIDs[0] = HDF_FAIL;
        k9ParamP->sdsIDs[0] = tlmFile->SelectDataset(
                                 oneSdsName, dataType, dataStartIndex,
                                 dataLength, numDimensions);
        if (k9ParamP->sdsIDs[0] == HDF_FAIL)
            return(0);
    }
    else
        return 0;

    (void)strncpy(tempString, k10ParamP->sdsNames, BIG_SIZE);
    oneSdsName = (char*)strtok(tempString, ",");
    if (oneSdsName)
    {
        k10ParamP->sdsIDs[0] = HDF_FAIL;
        k10ParamP->sdsIDs[0] = tlmFile->SelectDataset(
                                 oneSdsName, dataType, dataStartIndex,
                                 dataLength, numDimensions);
        if (k10ParamP->sdsIDs[0] == HDF_FAIL)
            return(0);
    }
    else
        return 0;

    (void)strncpy(tempString, k11ParamP->sdsNames, BIG_SIZE);
    oneSdsName = (char*)strtok(tempString, ",");
    if (oneSdsName)
    {
        k11ParamP->sdsIDs[0] = HDF_FAIL;
        k11ParamP->sdsIDs[0] = tlmFile->SelectDataset(
                                 oneSdsName, dataType, dataStartIndex,
                                 dataLength, numDimensions);
        if (k11ParamP->sdsIDs[0] == HDF_FAIL)
            return(0);
    }
    else
        return 0;

    (void)strncpy(tempString, k12ParamP->sdsNames, BIG_SIZE);
    oneSdsName = (char*)strtok(tempString, ",");
    if (oneSdsName)
    {
        k12ParamP->sdsIDs[0] = HDF_FAIL;
        k12ParamP->sdsIDs[0] = tlmFile->SelectDataset(
                                 oneSdsName, dataType, dataStartIndex,
                                 dataLength, numDimensions);
        if (k12ParamP->sdsIDs[0] == HDF_FAIL)
            return(0);
    }
    else
        return 0;


    return 1;

}//HK2LimitStatePair::OpenParamDataSets

IotBoolean
HK2LimitStatePair::CloseParamDataSets(
TlmHdfFile*    tlmFile)
{
    assert(modeParamP->sdsIDs != 0 && k9ParamP->sdsIDs != 0 &&
                    k10ParamP->sdsIDs != 0 && k11ParamP->sdsIDs != 0 &&
                    k12ParamP->sdsIDs != 0);
    // try to close all datasets, then return status
    IotBoolean closeOK = 1;

    if (tlmFile->CloseDataset(modeParamP->sdsIDs[0]) == HDF_FAIL)
        closeOK = 0;
    if (tlmFile->CloseDataset(k9ParamP->sdsIDs[0]) == HDF_FAIL)
        closeOK = 0;
    if (tlmFile->CloseDataset(k10ParamP->sdsIDs[0]) == HDF_FAIL)
        closeOK = 0;
    if (tlmFile->CloseDataset(k11ParamP->sdsIDs[0]) == HDF_FAIL)
        closeOK = 0;
    if (tlmFile->CloseDataset(k12ParamP->sdsIDs[0]) == HDF_FAIL)
        closeOK = 0;

    return(closeOK);

}//HK2LimitStatePair::CloseParamDataSets

int
HK2LimitStatePair::offset(void)
{
    return ((newState->mode * HVPS_STATE_COUNT * TWTA_COUNT +
                newState->twt * TWTA_COUNT + newState->twta) * 4);

}//HK2LimitStatePair::offset

int
HK2LimitStatePair::numStates(void)
{
    return (EXT_MODE_COUNT * HVPS_STATE_COUNT * TWTA_COUNT * 4);

}//HK2LimitStatePair::numStates

void
HK2LimitStatePair::PrintNewState(
FILE*   fp)
{
    _printState(fp, newState);
    fprintf(fp, "\n");

}//HK2LimitStatePair::PrintNewState

inline void
HK2LimitStatePair::_printState(
FILE*           fp,
HK2LimitState* state)
{
    fprintf(fp, "Mode=%s, TWT=%s, TWTA=%s",
                    ext_mode_map[state->mode], twt_map[state->twt],
                    twta_map[state->twta]);

}//HK2LimitStatePair::_printState

void
HK2LimitStatePair::PrintChange(
FILE*   fp)
{

    _printState(fp, oldState);
    fprintf(fp, " -> ");
    _printState(fp, newState);
    fprintf(fp, "\n");

}//HK2LimitStatePair::PrintChange

char
HK2LimitStatePair::ApplyNewFrame(
TlmHdfFile*    tlmFile,
int32          startIndex)
{
    // get mode, twt and twta
    unsigned char buffer=0;
    int mode;
    if ( ! modeParamP->extractFunc(tlmFile, modeParamP->sdsIDs,
                     startIndex, 1, 1, &buffer, 0))
        return 0;
    mode = (int) buffer;
   
    switch(mode)
    {
        case L1_MODE_WOM:
            oldState->mode = TLM_MODE_WOM;
            break;
        case L1_MODE_CBM:
            oldState->mode = TLM_MODE_CBM;
            break;
        case L1_MODE_SBM:
            oldState->mode = TLM_MODE_SBM;
            break;
        case L1_MODE_ROM:
            oldState->mode = TLM_MODE_ROM;
            break;
        default:
            return 0;
    }

    unsigned char k9, k10;
    if ( ! k9ParamP->extractFunc(tlmFile, k9ParamP->sdsIDs,
                     startIndex, 1, 1, &k9, 0))
        return 0;
    if ( ! k10ParamP->extractFunc(tlmFile, k10ParamP->sdsIDs,
                     startIndex, 1, 1, &k10, 0))
        return 0;
    oldState->twt = (k9 == k10 ? TLM_HVPS_ON : TLM_HVPS_OFF);

    unsigned char k11, k12;
    if ( ! k11ParamP->extractFunc(tlmFile, k11ParamP->sdsIDs,
                     startIndex, 1, 1, &k11, 0))
        return 0;
    if ( ! k12ParamP->extractFunc(tlmFile, k12ParamP->sdsIDs,
                     startIndex, 1, 1, &k12, 0))
        return 0;
    oldState->twta = (k11 == k12 ? TLM_TWTA_1 : TLM_TWTA_2);


    if (*oldState == *newState)
        return (0);
    else
    {
        // limit state changed, swap the pointer and return 1
        HK2LimitState* temp = newState;
        newState = oldState;
        oldState = temp;
        return (1);
    }
}//HK2LimitStatePair::ApplyNewFrame

//*******************************************************//
//   operators for L1ALimitState and HK2LimitState
//*******************************************************//
int
operator==(
const L1ALimitState& a,
const L1ALimitState& b)
{
    return (a.mode == b.mode && a.twt == b.twt && 
               a.twta == b.twta && a.frame == b.frame);

}//operator== (const L1ALimitState& a, const L1ALimitState& b)

int
operator!=(
const L1ALimitState& a,
const L1ALimitState& b)
{
    return (a == b ? FALSE : TRUE);

}//operator!= (const L1ALimitState& a, const L1ALimitState& b)

int
operator==(
const HK2LimitState& a,
const HK2LimitState& b)
{
    return (a.mode == b.mode && a.twt == b.twt &&
            a.twta == b.twta);

}//operator== (const HK2LimitState& a, const HK2LimitState& b)

int
operator!=(
const HK2LimitState& a,
const HK2LimitState& b)
{
    return (a == b ? FALSE : TRUE);

}//operator!= (const HK2LimitState& a, const HK2LimitState& b)

//************************************************************//
//   operators for L1ALimitStatePair & HK2LimitStatePair
//************************************************************//

int operator==(
const L1ALimitStatePair& a,
const L1ALimitStatePair& b)
{
    return (*(a.oldState) == *(b.oldState) &&
                *(a.newState) == *(b.newState));
}//operator==(const L1ALimitStatePair&, const L1ALimitStatePair&)
int
operator!=(
const L1ALimitStatePair& a,
const L1ALimitStatePair& b)
{
    return (a == b ? FALSE : TRUE);

}//operator!= (const L1ALimitStatePair& a, const L1ALimitStatePair& b)

int operator==(
const HK2LimitStatePair& a,
const HK2LimitStatePair& b)
{
    return (*(a.oldState) == *(b.oldState) &&
                *(a.newState) == *(b.newState));
}//operator==(const HK2LimitStatePair&, const HK2LimitStatePair&)
int
operator!=(
const HK2LimitStatePair& a,
const HK2LimitStatePair& b)
{
    return (a == b ? FALSE : TRUE);

}//operator!= (const HK2LimitStatePair& a, const HK2LimitStatePair& b)
