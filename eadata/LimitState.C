//=========================================================//
// Copyright  (C)1995, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#include <assert.h>

#include "LimitState.h"
#if 0
#include "HkdtExtract.h"
#endif
#include "L1AExtract.h"
#include "ParTab.h"
#include "TlmHdfFile.h"

static const char rcs_id_limit_state_c[] = "@(#) $Id$";

//*******************************************************//
//      L1ALimitStatePair
//*******************************************************//
L1ALimitStatePair::L1ALimitStatePair(
int    mode,
int    hvps,
int    frame)
: LimitStatePair(),
  stateA(mode, hvps, frame), stateB(mode, hvps, frame),
  oldState(&stateA), newState(&stateB)
{
    modeParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                        OPERATIONAL_MODE, UNIT_HEX_BYTES);
    hvpsParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                        L1A_FRAME_INST_STATUS_08, UNIT_MAP);
    frameParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                        TRUE_CAL_PULSE_POS, UNIT_DN);
} //L1ALimitStatePair::L1ALimitStatePair

// default constructor
L1ALimitStatePair::L1ALimitStatePair()
: LimitStatePair(),
oldState(&stateA), newState(&stateB)
{
    modeParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                        OPERATIONAL_MODE, UNIT_HEX_BYTES);
    hvpsParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                        L1A_FRAME_INST_STATUS_08, UNIT_MAP);
    frameParamP = ParTabAccess::GetParameter(SOURCE_L1A,
                                        TRUE_CAL_PULSE_POS, UNIT_DN);
}//L1ALimitStatePair::L1ALimitStatePair

// copy constructor
L1ALimitStatePair::L1ALimitStatePair(
const L1ALimitStatePair& old)
: LimitStatePair(old), stateA(old.stateA), stateB(old.stateB),
  oldState(old.oldState), newState(old.newState)
{
    modeParamP = new Parameter(*(old.modeParamP));
    hvpsParamP = new Parameter(*(old.hvpsParamP));
    frameParamP = new Parameter(*(old.frameParamP));

}//1LimitStatePair::L1ALimitStatePair

L1ALimitStatePair::~L1ALimitStatePair()
{
    if (modeParamP) delete modeParamP;
    if (hvpsParamP) delete hvpsParamP;
    if (frameParamP) delete frameParamP;
    modeParamP = hvpsParamP = frameParamP = 0;

}//L1ALimitStatePair::~L1ALimitStatePair

IotBoolean
L1ALimitStatePair::OpenParamDataSets(
TlmHdfFile*    tlmFile)
{
    if (modeParamP == 0 || hvpsParamP == 0 || frameParamP == 0)
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

    (void)strncpy(tempString, hvpsParamP->sdsNames, BIG_SIZE);
    oneSdsName = (char*)strtok(tempString, ",");
    if (oneSdsName)
    {
        hvpsParamP->sdsIDs[0] = HDF_FAIL;
        hvpsParamP->sdsIDs[0] = tlmFile->SelectDataset(
                                 oneSdsName, dataType, dataStartIndex,
                                 dataLength, numDimensions);
        if (hvpsParamP->sdsIDs[0] == HDF_FAIL)
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
    assert(modeParamP->sdsIDs != 0 && hvpsParamP->sdsIDs != 0 &&
                    frameParamP->sdsIDs != 0);
    // try to close all datasets, then return status
    IotBoolean closeOK = 1;

    if (tlmFile->CloseDataset(modeParamP->sdsIDs[0]) == HDF_FAIL)
        closeOK = 0;
    if (tlmFile->CloseDataset(hvpsParamP->sdsIDs[0]) == HDF_FAIL)
        closeOK = 0;
    if (tlmFile->CloseDataset(frameParamP->sdsIDs[0]) == HDF_FAIL)
        closeOK = 0;

    return(closeOK);

}//L1ALimitStatePair::CloseParamDataSets

int
L1ALimitStatePair::offset(void)
{
    return ((newState->mode * HVPS_STATE_COUNT * FRAME_TYPE_COUNT +
                newState->hvps * FRAME_TYPE_COUNT + newState->frame) * 4);

}//L1ALimitStatePair::offset

int
L1ALimitStatePair::numStates(void)
{
    return (NSCAT_MODE_COUNT * HVPS_STATE_COUNT * FRAME_TYPE_COUNT * 4);

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
    fprintf(fp, "Mode=%s, HVPS=%s, Frame=%s", mode_map[state->mode],
                    hvps_map[state->hvps], cmf_map[state->frame]);

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
    // get mode, hvps and frame
    unsigned char buffer=0;
    int mode, hvps, frame;
    if ( ! modeParamP->extractFunc(tlmFile, modeParamP->sdsIDs,
                     startIndex, 1, 1, &buffer))
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
     

    if ( ! hvpsParamP->extractFunc(tlmFile, hvpsParamP->sdsIDs,
                     startIndex, 1, 1, &buffer))
        return 0;
    hvps = (int) buffer;
    switch(hvps)
    {
        case L1_HVPS_ON:
            oldState->hvps = TLM_HVPS_ON;
            break;
        case L1_HVPS_OFF:
            oldState->hvps = TLM_HVPS_OFF;
            break;
        default:
            return 0;
    }

    char calPulsePos=0;
    if ( ! frameParamP->extractFunc(tlmFile, frameParamP->sdsIDs,
                     startIndex, 1, 1, &calPulsePos))
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

#if 0
int
HkdtLimitStatePair::offset(void)
{
    return ((newState->mode * HVPS_STATE_COUNT * DSS_COUNT * TWTA_COUNT +
                newState->hvps * DSS_COUNT * TWTA_COUNT +
                newState->dss * TWTA_COUNT + newState->twta) * 4);

}//HkdtLimitStatePair::offset

int
HkdtLimitStatePair::numStates(void)
{
    return (EXT_MODE_COUNT * HVPS_STATE_COUNT * DSS_COUNT * TWTA_COUNT * 4);

}//HkdtLimitStatePair::numStates

void
HkdtLimitStatePair::PrintNewState(
FILE*   fp)
{
    _printState(fp, newState);
    fprintf(fp, "\n");

}//HkdtLimitStatePair::PrintNewState

inline void
HkdtLimitStatePair::_printState(
FILE*           fp,
HkdtLimitState* state)
{
    fprintf(fp, "Mode=%s, HVPS=%s, DSS=%s, TWTA=%s",
                    ext_mode_map[state->mode], hvps_map[state->hvps],
                    dss_map[state->dss], twta_map[state->twta]);

}//HkdtLimitStatePair::_printState

void
HkdtLimitStatePair::PrintChange(
FILE*   fp)
{

    _printState(fp, oldState);
    fprintf(fp, " -> ");
    _printState(fp, newState);
    fprintf(fp, "\n");

}//HkdtLimitStatePair::PrintChange

char
HkdtLimitStatePair::ApplyNewFrame(
char*       tlmFrame)
{
    // get mode, hvps and frame
    char buffer;
    if (HK_Ext_Mode(tlmFrame, &buffer) == TRUE)
        oldState->mode = (int) buffer;
    if (HK_HVPS(tlmFrame, &buffer) == TRUE)
        oldState->hvps = (int) buffer;
    if (HK_DSS(tlmFrame, &buffer) == TRUE)
        oldState->dss = (int) buffer;
    if (HK_TWTA(tlmFrame, &buffer) == TRUE)
        oldState->twta = (int) buffer;

    if (*oldState == *newState)
        return (0);
    else
    {
        // limit state changed, swap the pointer and return 1
        HkdtLimitState* temp = newState;
        newState = oldState;
        oldState = temp;
        return (1);
    }
}//HkdtLimitStatePair::ApplyNewFrame
#endif

//*******************************************************//
//   operators for L1ALimitState and HkdtLimitState
//*******************************************************//
int
operator==(
const L1ALimitState& a,
const L1ALimitState& b)
{
    return (a.mode == b.mode && a.hvps == b.hvps && a.frame == b.frame);

}//operator== (const L1ALimitState& a, const L1ALimitState& b)

int
operator!=(
const L1ALimitState& a,
const L1ALimitState& b)
{
    return (a == b ? FALSE : TRUE);

}//operator!= (const L1ALimitState& a, const L1ALimitState& b)

#if 0
int
operator==(
const HkdtLimitState& a,
const HkdtLimitState& b)
{
    return (a.mode == b.mode && a.hvps == b.hvps &&
            a.dss == b.dss && a.twta == b.twta);

}//operator== (const HkdtLimitState& a, const HkdtLimitState& b)

int
operator!=(
const HkdtLimitState& a,
const HkdtLimitState& b)
{
    return (a == b ? FALSE : TRUE);

}//operator!= (const HkdtLimitState& a, const HkdtLimitState& b)
#endif

//************************************************************//
//   operators for L1ALimitStatePair & HkdtLimitStatePair
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

#if 0
int operator==(
const HkdtLimitStatePair& a,
const HkdtLimitStatePair& b)
{
    return (*(a.oldState) == *(b.oldState) &&
                *(a.newState) == *(b.newState));
}//operator==(const HkdtLimitStatePair&, const HkdtLimitStatePair&)
int
operator!=(
const HkdtLimitStatePair& a,
const HkdtLimitStatePair& b)
{
    return (a == b ? FALSE : TRUE);

}//operator!= (const HkdtLimitStatePair& a, const HkdtLimitStatePair& b)
#endif
