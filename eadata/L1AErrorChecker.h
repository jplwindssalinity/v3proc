//=========================================================//
// Copyright  (C)1995, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef L1AERRORCHECKER_H
#define L1AERRORCHECKER_H

static const char rcs_id_l1_error_checker_h[]="@(#) $Id$";

#include "Itime.h"
#include "ErrorChecker.h"

#define ERROR_MSG_NAME  "Error Message"

//=================//
// L1AErrorChecker //
//=================//

class L1AErrorChecker : public ErrorChecker
{
public:
    L1AErrorChecker();
    ~L1AErrorChecker();

    void    Check(FILE* ofp, char* dataRec);
    void    Summarize(FILE* ofp);
    void    SetState(State* state, State* last_state, char* dataRec);
    int     AnyErrors();

    unsigned long       errorMessageCount[32];

protected:
    ErrorEntry*     errorTable1;
    ErrorEntry*     errorTable2;
};

//===================//
// Special Functions //
//===================//

ErrorFunc   Err_Error_Msg;

#endif // L1AERRORCHECKER_H
