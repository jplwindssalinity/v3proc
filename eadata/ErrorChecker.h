//=========================================================
// Copyright  (C)1995, California Institute of Technology.
// U.S. Government sponsorship under
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.5   20 May 1998 11:20:04   sally
// handles dummy error checker (place holders)
// 
//    Rev 1.4   19 May 1998 15:49:46   sally
// move some function from L1AErrorChecker to ErrorChecker
// 
//    Rev 1.3   18 May 1998 14:47:58   sally
// added error checker for L1A
// 
//    Rev 1.2   13 May 1998 16:28:10   sally
// y

// $Date$
// $Revision$
// $Author$
//
//=========================================================

#ifndef ERRORCHECKER_H
#define ERRORCHECKER_H

static const char rcs_id_error_checker_h[] =
    "@(#) $Header$";

#include "Itime.h"
#include "State.h"

#define INITIALIZE_FORMAT   "\n(INITIAL ERROR): %s\n"
#define ERROR_FORMAT        "\n=== ERROR === %s ===\n"
#define WARNING_FORMAT      "\n-- WARNING -- %s --\n"
#define NOTIFY_FORMAT       "\n++ NOTIFY ++ %s ++\n"
#define CLEAR_FORMAT        "\n... CLEAR ... %s ...\n"

#define NONE_ERROR_CHECKER  ((Parameter*)-1)

// the order must match the elements of ErrorTabEntry
enum ERROR_CHECK_PARAM_E
{
    ERROR_MODE = 0,
    ERROR_RX_PROTECT,
    ERROR_GRID,
    ERROR_TWT_1_BODY_OC_TRIP,
    ERROR_TWT_2_BODY_OC_TRIP,
    ERROR_TWT_1_CNVRT_OC_TRIP,
    ERROR_TWT_2_CNVRT_OC_TRIP,
    ERROR_TWT_1_UV_TRIP,
    ERROR_TWT_2_UV_TRIP,
    ERROR_ROM_START_ERROR,
    ERROR_RAM_START_ERROR,
    ERROR_RUNNING_ERROR_COUNT,
    ERROR_TWT_TRIP_OVERRIDE,
    ERROR_TWTA_MONITOR_DISABLE,
    ERROR_TWT_SHUTDOWN_DISABLE,
    ERROR_DOPPLER_ORBIT_STEP,
    ERROR_MODE_CHANGE,
    ERROR_VALID_COMMAND_COUNT,
    ERROR_INVALID_COMMAND_COUNT,
    ERROR_MODE_CHANGE_MISMATCHED,
    ERROR_TRS_CMD_SUCCESS,
    ERROR_SAS_ANT_RELEASE_INTERLOCK
};

//==============//
// ErrorChecker //
//==============//

class ErrorChecker
{
public:
    ErrorChecker();
    virtual ~ErrorChecker();

    virtual int     OpenParamDataSets(TlmHdfFile* tlmFile);
    virtual void    CloseParamDataSets(TlmHdfFile* tlmFile);

    virtual void    ReportBasicInfo(FILE* ofp)=0;
    virtual int     Check(TlmHdfFile* tlmFile, int32 startIndex, FILE* ofp)=0;
    virtual void    Summarize(FILE* ofp) = 0;
    virtual int     AnyErrors() = 0;

    Parameter**     parametersP;
    State*          prev;
    State*          current;
    unsigned int    dataRecCount;

protected:
    virtual int     SetState(
                          TlmHdfFile*   tlmFile,
                          int32         startIndex,
                          State*        lastState,
                          State*        newState);

    int             _errorStateTableSize;

}; // ErrorChecker

//============//
// ErrorEntry //
//============//

typedef int (*ErrorFunc) (  ErrorChecker*   obj,
                            char*           name,
                            char*           format,
                            FILE*           ofp);

struct ErrorTabEntry
{
    char*             name;
    char*             format;
    ErrorFunc         errorFunc;   // checking "EACH INSTANCE", 1 if no change
    StateTabEntry     stateEntry;
    unsigned int      count;
};


//===========//
// functions //
//===========//

int ErrorCheckMode(ErrorChecker*, char*, char*, FILE*);
int ErrorRxProtect(ErrorChecker*, char*, char*, FILE*);
int ErrorGridDisable(ErrorChecker*, char*, char*, FILE*);
int ErrorTwta1BodyOcTrip(ErrorChecker*, char*, char*, FILE*);
int ErrorTwta2BodyOcTrip(ErrorChecker*, char*, char*, FILE*);
int ErrorTwta1CnvrtOcTrip(ErrorChecker*, char*, char*, FILE*);
int ErrorTwta2CnvrtOcTrip(ErrorChecker*, char*, char*, FILE*);
int ErrorTwta1UvTrip(ErrorChecker*, char*, char*, FILE*);
int ErrorTwta2UvTrip(ErrorChecker*, char*, char*, FILE*);
int ErrorRomStartError(ErrorChecker*, char*, char*, FILE*);
int ErrorRamStartError(ErrorChecker*, char*, char*, FILE*);
int ErrorRunningErrors(ErrorChecker*, char*, char*, FILE*);
int ErrorTwtTripOvrd(ErrorChecker*, char*, char*, FILE*);
int ErrorTwtaMonitor(ErrorChecker*, char*, char*, FILE*);
int ErrorTwtShutdown(ErrorChecker*, char*, char*, FILE*);
int ErrorUnexpectedCycle(ErrorChecker*, char*, char*, FILE*);
int ErrorModeChange(ErrorChecker*, char*, char*, FILE*);
int ErrorValidCmdCnt(ErrorChecker*, char*, char*, FILE*);
int ErrorInvalidCmdCnt(ErrorChecker*, char*, char*, FILE*);
int ErrorTrsCmdSucc(ErrorChecker*, char*, char*, FILE*);

#if 0
int Error_Flag_Transition(
    FILE*               ofp,
    char*               error_name,
    char*               parameter_name,
    char*               label_format,
    State*              current_state,
    StateElement_uc*    current_elem,
    State*              prev_state,
    StateElement_uc*    prev_elem,
    const char*         map[],
    unsigned char       ok_value);

#endif

#endif // ERRORCHECKER_H
