//=========================================================
// Copyright  (C)1995, California Institute of Technology.
// U.S. Government sponsorship under
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.9   26 Jul 1999 16:49:32   sally
// Group flag for L1A is 1, Hk2 is 3
// 
//    Rev 1.8   26 Jul 1999 15:43:24   sally
// add "Source Sequence Count" and "group flag" checking
// 
//    Rev 1.7   09 Jul 1999 16:06:32   sally
// add error checkers for "Source Sequence Count" and "Group Flag" in header
// 
//    Rev 1.6   25 Jun 1999 10:30:36   sally
// add some error conditions from Lee
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
    ERROR_MSSN_SERL_TLM,
    ERROR_RELAY_CHANGE,

    ERROR_CDS_RESET,
    ERROR_A2D_TIMEOUT,
    ERROR_MISS_SC_TIME,
    ERROR_FAULT_PROTECT,
    ERROR_WATCHDOG_TIMEOUT,
    ERROR_POWER_ON_RESET,

    ERROR_MSSN_TLM_TABLES_CHANGED,
    ERROR_SERL_TLM_TABLES_CHANGED,
    ERROR_DOPPLER_TABLES_CHANGED,
    ERROR_RANGE_GATE_TABLES_CHANGED,
    ERROR_SES_PARAM_TABLES_CHANGED,
    ERROR_TWTA_MON_EN_DIS,
    ERROR_TWTA_PWR_MON_FP_EN_DIS,
    ERROR_SUPP_HTR_MODE_CHANGE,
    ERROR_CDS_HARD_RESET,
    ERROR_SAS_MULTI_DATA_LOSS,
    ERROR_SES_MULTI_DATA_LOSS,
    ERROR_SOFT_RESET,
    ERROR_EQ_XING_MISSED,
    ERROR_MODE_CHANGE,

    ERROR_MODULATION,
    ERROR_TWT_TRIP_OVERRIDE,

    ERROR_PBI_UNEXPECTED_STATE,
    ERROR_MEMORY_WRITE_FAIL,
    ERROR_R2_MODIFIED,
    ERROR_BCRTM_FAILURE,
    ERROR_MEM_WRITE_PROT_VIOLATE,
    ERROR_WATCH_DOG_TIMER_EXPIRED,

    ERROR_TWT_1_CNVRT_OC_TRIP,
    ERROR_TWT_1_UV_TRIP,
    ERROR_TWT_1_BODY_OC_TRIP,
    ERROR_TWT_2_CNVRT_OC_TRIP,
    ERROR_TWT_2_UV_TRIP,
    ERROR_TWT_2_BODY_OC_TRIP,
    ERROR_PLL_OUT_OF_LOCK,
    ERROR_TRS_CMD_SUCCESS,
    ERROR_BODY_OC_TRIP_CNTL,
    ERROR_SES_WATCH_DOG_TIMEOUT,
    ERROR_RAM_START_ERROR,
    ERROR_SES_ROM_START_ERROR,
    ERROR_SES_RESET,
    ERROR_SES_PORT_PARITY_ERROR,

    // relay status
    ERROR_K15_SES_A_B,
    ERROR_K12_TWTA_1_2,
    ERROR_K11_TWTA_1_2,
    ERROR_K10_TWTA_ON_OFF,
    ERROR_K9_TWTA_ON_OFF,
    ERROR_K20_SAS_A_B,
    ERROR_K19_SAS_A_B,
    ERROR_K16_SES_A_B,
    ERROR_SAS_B_SPIN_RATE,
    ERROR_K22_SES_SUPP_HTR_ON_OFF,
    ERROR_K21_SES_SUPP_HTR_ON_OFF,
    ERROR_SAS_A_SPIN_RATE,

    ERROR_MODE,
    ERROR_RX_PROTECT,
    ERROR_GRID,
    ERROR_RUNNING_ERROR_COUNT,
    ERROR_TWT_SHUTDOWN_DISABLE,
    ERROR_VALID_COMMAND_COUNT,
    ERROR_INVALID_COMMAND_COUNT,
    ERROR_MODE_CHANGE_MISMATCHED,
    ERROR_SAS_ANT_RELEASE_INTERLOCK,

    ERROR_SRC_SEQ_COUNT,
    ERROR_HEADER_GROUP_FLAG
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

// report error if value is set to 1
int ErrorValueSet(ErrorChecker*, char*, char*, FILE*, ERROR_CHECK_PARAM_E);

int ErrorCdsReset(ErrorChecker*, char*, char*, FILE*);
int ErrorA2DTimeout(ErrorChecker*, char*, char*, FILE*);
int ErrorMissSCTime(ErrorChecker*, char*, char*, FILE*);
int ErrorMssnSerlTlm(ErrorChecker*, char*, char*, FILE*);
int ErrorFaultProtect(ErrorChecker*, char*, char*, FILE*);
int ErrorWatchdogTimeout(ErrorChecker*, char*, char*, FILE*);
int ErrorPowerOnReset(ErrorChecker*, char*, char*, FILE*);

int ErrorMssnTlmTablesChanged(ErrorChecker*, char*, char*, FILE*);
int ErrorSerlTlmTablesChanged(ErrorChecker*, char*, char*, FILE*);
int ErrorDopplerTablesChanged(ErrorChecker*, char*, char*, FILE*);
int ErrorRangeGateTablesChanged(ErrorChecker*, char*, char*, FILE*);
int ErrorSESParamTablesChanged(ErrorChecker*, char*, char*, FILE*);
int ErrorCdsHardReset(ErrorChecker*, char*, char*, FILE*);
int ErrorRelayChanged(ErrorChecker*, char*, char*, FILE*);
int ErrorSoftReset(ErrorChecker*, char*, char*, FILE*);
int ErrorModeChange(ErrorChecker*, char*, char*, FILE*);
int ErrorWriteTestFailed(ErrorChecker*, char*, char*, FILE*);
int ErrorMemWriteProtViolate(ErrorChecker*, char*, char*, FILE*);
int ErrorTrsCmdSucc(ErrorChecker*, char*, char*, FILE*);
int ErrorSesWatchDogTimeout(ErrorChecker*, char*, char*, FILE*);
int ErrorRamStartError(ErrorChecker*, char*, char*, FILE*);
int ErrorSesRomStartError(ErrorChecker*, char*, char*, FILE*);
int ErrorSesReset(ErrorChecker*, char*, char*, FILE*);
int ErrorPortParityError(ErrorChecker*, char*, char*, FILE*);

int ErrorTwtaMonEnDis(ErrorChecker*, char*, char*, FILE*);
int ErrorPwrMonFPEndis(ErrorChecker*, char*, char*, FILE*);
int ErrorSesSuppHtrMode(ErrorChecker*, char*, char*, FILE*);
int ErrorSasMultiDataLoss(ErrorChecker*, char*, char*, FILE*);
int ErrorSesMultiDataLoss(ErrorChecker*, char*, char*, FILE*);
int ErrorEqXingMissed(ErrorChecker*, char*, char*, FILE*);
int ErrorModulation(ErrorChecker*, char*, char*, FILE*);
int ErrorTwtTripOverride(ErrorChecker*, char*, char*, FILE*);
int ErrorUnexpectedState(ErrorChecker*, char*, char*, FILE*);
int ErrorR2Modified(ErrorChecker*, char*, char*, FILE*);
int ErrorBCRTMFailure(ErrorChecker*, char*, char*, FILE*);
int ErrorWatchDogTimerExpired(ErrorChecker*, char*, char*, FILE*);
int ErrorTwt1CnvrtOcTrip(ErrorChecker*, char*, char*, FILE*);
int ErrorTwt1UvTrip(ErrorChecker*, char*, char*, FILE*);
int ErrorTwt1BodyOcTrip(ErrorChecker*, char*, char*, FILE*);
int ErrorTwt2CnvrtOcTrip(ErrorChecker*, char*, char*, FILE*);
int ErrorTwt2UvTrip(ErrorChecker*, char*, char*, FILE*);
int ErrorTwt2BodyOcTrip(ErrorChecker*, char*, char*, FILE*);
int ErrorPllOutOfLock(ErrorChecker*, char*, char*, FILE*);
int ErrorBodyOcTripCntl(ErrorChecker*, char*, char*, FILE*);


int ErrorK15SesAB(ErrorChecker*, char*, char*, FILE*);
int ErrorK12Twta12(ErrorChecker*, char*, char*, FILE*);
int ErrorK11Twta12(ErrorChecker*, char*, char*, FILE*);
int ErrorK10TwtaOnOff(ErrorChecker*, char*, char*, FILE*);
int ErrorK9TwtaOnOff(ErrorChecker*, char*, char*, FILE*);
int ErrorK20SasAB(ErrorChecker*, char*, char*, FILE*);
int ErrorK19SasAB(ErrorChecker*, char*, char*, FILE*);
int ErrorK16SesAB(ErrorChecker*, char*, char*, FILE*);
int ErrorSasBSpinRate(ErrorChecker*, char*, char*, FILE*);
int ErrorK22SesSuppHtrOnOff(ErrorChecker*, char*, char*, FILE*);
int ErrorK21SesSuppHtrOnOff(ErrorChecker*, char*, char*, FILE*);
int ErrorSasASpinRate(ErrorChecker*, char*, char*, FILE*);

int ErrorHk2BadSrcSeq(ErrorChecker*, char*, char*, FILE*);
int ErrorL1ABadSrcSeq(ErrorChecker*, char*, char*, FILE*);
int ErrorHk2BadGrpFlg(ErrorChecker*, char*, char*, FILE*);
int ErrorL1ABadGrpFlg(ErrorChecker*, char*, char*, FILE*);

#endif // ERRORCHECKER_H
