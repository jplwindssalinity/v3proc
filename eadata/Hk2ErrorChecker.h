//=========================================================
// Copyright  (C)1995, California Institute of Technology.
// U.S. Government sponsorship under
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.1   22 May 1998 16:22:30   sally
// took out error message handling
// 
//    Rev 1.0   20 May 1998 11:20:26   sally
// Initial revision.

// $Date$
// $Revision$
// $Author$
//
//=========================================================

#ifndef HK2ERRORCHECKER_H
#define HK2ERRORCHECKER_H

static const char rcs_id_hk2_error_checker_h[]=
    "@(#) $Header$";

#include "Itime.h"
#include "ErrorChecker.h"

//=================//
// HK2ErrorChecker //
//=================//

class HK2ErrorChecker : public ErrorChecker
{
public:
    HK2ErrorChecker();
    virtual ~HK2ErrorChecker();

    int     OpenParamDataSets(TlmHdfFile* tlmFile);  // return 0 if failed
    void    CloseParamDataSets(TlmHdfFile* tlmFile);

    void    ReportBasicInfo(FILE* ofp);
    int     Check(TlmHdfFile* tlmFile, int32 startIndex, FILE* ofp);
    void    Summarize(FILE* ofp);
    int     AnyErrors();

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

    Parameter      *_timeParamP;
    Parameter      *_sasK19ParamP, *_sasK20ParamP;
    Parameter      *_sesK15ParamP, *_sesK16ParamP;
    Parameter      *_twtaK11ParamP, *_twtaK12ParamP;
    Parameter      *_modeParamP;

    StateElement   _timeStateElement;
    StateElement   _sasK19StateElement, _sasK20StateElement;
    StateElement   _sesK15StateElement, _sesK16StateElement;
    StateElement   _twtaK11StateElement, _twtaK12StateElement;
    StateElement   _modeStateElement;

};

#endif // HK2ERRORCHECKER_H
