//=========================================================//
// Copyright  (C)1996, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef ERRORCHECKER_H
#define ERRORCHECKER_H

static const char rcs_id_error_checker_h[] =
    "@(#) $Id$";

#include "Itime.h"
#include "State.h"

#define INITIALIZE_FORMAT   "\n(INITIAL ERROR): %s\n"
#define ERROR_FORMAT        "\n=== ERROR === %s ===\n"
#define WARNING_FORMAT      "\n-- WARNING -- %s --\n"
#define CLEAR_FORMAT        "\n... CLEAR ... %s ...\n"

//==============//
// ErrorChecker //
//==============//

class ErrorChecker
{
public:
    ErrorChecker();
    ~ErrorChecker();

    virtual void    Check(FILE* ofp, char* dataRec) = 0;
    virtual void    Summarize(FILE* ofp) = 0;
    virtual void    SetState(State* state, State* last_state,
                        char* dataRec) = 0;
    virtual int     AnyErrors() = 0;

    State*          prev;
    State*          current;

    unsigned long   dataRecCount;
};

//============//
// ErrorEntry //
//============//

typedef int ErrorFunc(  ErrorChecker*   obj,
                        FILE*           ofp,
                        char*           name,
                        char*           format);

struct ErrorEntry
{
    char*           name;
    char*           format;
    ErrorFunc*      function;
    unsigned long   count;
};

//===========//
// functions //
//===========//

void ReportBasicInfo(
    FILE*   ofp,
    State*  state);

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

//------------------//
// both L1 and HK   //
//------------------//

ErrorFunc   Err_HVPS_Backup_Off;
ErrorFunc   Err_TWTA_UV_Trip;
ErrorFunc   Err_TWTA_OC_Trip;
ErrorFunc   Err_TWTA_Body_OC_Trip;
ErrorFunc   Err_Bin_Param;
ErrorFunc   Err_Def_Bin_Const;
ErrorFunc   Err_Lack_Start_Reqs;
ErrorFunc   Err_Error_Queue_Full;
ErrorFunc   Err_Fault_Counter;
ErrorFunc   Err_Mode;
ErrorFunc   Err_ULM_Unlocked;
ErrorFunc   Err_SLM_Unlocked;
ErrorFunc   Err_Trip_Override_En;
ErrorFunc   Err_TWT_Mon_Dis;
ErrorFunc   Err_HVPS_Shutdown_Dis;

//---------//
// L1 only //
//---------//

ErrorFunc   Err_Rx_Pro;
ErrorFunc   Err_CSB;
ErrorFunc   Err_Cycle_Counters;

//-----------//
// HK  only //
//-----------//

ErrorFunc   Err_Ant_2_Undep;
ErrorFunc   Err_Ant_5_Undep;
ErrorFunc   Err_Ant_14_Undep;
ErrorFunc   Err_Ant_36_Undep;
ErrorFunc   Err_Relay_HVPS;
ErrorFunc   Err_WTS_TWTA;
ErrorFunc   Err_Relay_TWTA;
ErrorFunc   Err_Relay_DSS;
ErrorFunc   Err_Rep_Heat_Dis;
ErrorFunc   Err_Spare_Heat_Dis;

#endif // ERRORCHECKER_H
