//=========================================================
// Copyright  (C)1995, California Institute of Technology.
// U.S. Government sponsorship under
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.5   26 May 1998 10:54:58   sally
// added QPF
// 
//    Rev 1.4   22 May 1998 16:23:00   sally
// added error message handling
// 
//    Rev 1.3   18 May 1998 14:48:32   sally
// added error checker for L1A
// 
//    Rev 1.2   13 May 1998 16:28:16   sally

// $Date$
// $Revision$
// $Author$
//
//=========================================================

#ifndef L1AERRORCHECKER_H
#define L1AERRORCHECKER_H

static const char rcs_id_l1_error_checker_h[]=
    "@(#) $Header$";

#include "Itime.h"
#include "EAList.h"
#include "ErrorChecker.h"

#define ERROR_MSG_NAME  "Error Message"

//=================//
// L1AErrorChecker //
//=================//

struct ErrorMsgT
{
    ErrorMsgT(unsigned short errorCode) : errorMsgCode(errorCode), num(1) {};
    unsigned short    errorMsgCode;
    int               num;
};
inline int operator==(const ErrorMsgT& a, const ErrorMsgT& b)
{
    return(a.errorMsgCode == b.errorMsgCode ? 1 : 0);
}


class L1AErrorChecker : public ErrorChecker
{
public:
    L1AErrorChecker();
    virtual ~L1AErrorChecker();

    int     OpenParamDataSets(TlmHdfFile* tlmFile);  // return 0 if failed
    void    CloseParamDataSets(TlmHdfFile* tlmFile);

    void    ReportBasicInfo(FILE* ofp);
    int     Check(TlmHdfFile* tlmFile, int32 startIndex, FILE* ofp);
    void    Summarize(FILE* ofp);
    int     AnyErrors();

    static int   CodeToErrorMsg(unsigned short errorMsgCode,
                                char*          msgString);// user provide space

protected:
    int            OpenInternalStateDataSets(TlmHdfFile* tlmFile);
    void           CloseInternalStateDataSets(TlmHdfFile* tlmFile);

    int            SetState(
                          TlmHdfFile*   tlmFile,
                          int32         startIndex,
                          State*        lastState,
                          State*        newState);

    int            SetInternalState(
                          TlmHdfFile*   tlmFile,
                          int32         startIndex);

    int            ErrorMessageCode(
                          ErrorChecker* obj,
                          char*         name,
                          char*         format,
                          FILE*         ofp);

    void           AddErrorMessageCode(unsigned short  newErrorMsgCode);

    Parameter      *_timeParamP;
    Parameter      *_sasK19ParamP, *_sasK20ParamP;
    Parameter      *_sesK15ParamP, *_sesK16ParamP;
    Parameter      *_twtaK11ParamP, *_twtaK12ParamP;
    Parameter      *_modeParamP, *_errorCounterP;
    Parameter      *_errorMsgP;

    StateElement   _timeStateElement;
    StateElement   _sasK19StateElement, _sasK20StateElement;
    StateElement   _sesK15StateElement, _sesK16StateElement;
    StateElement   _twtaK11StateElement, _twtaK12StateElement;
    StateElement   _modeStateElement, _errorStateElement;
    StateElement   _errorMsgStateElement;

    EAList<ErrorMsgT>   _errorMsgCodes;

};

#endif // L1AERRORCHECKER_H
