//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.3   20 Apr 1998 15:19:46   sally
// change List to EAList
// 
//    Rev 1.2   01 Apr 1998 13:36:16   sally
// for L1A Derived table
// 
//    Rev 1.1   23 Mar 1998 15:39:58   sally
// adapt to derived science data
// 
//    Rev 1.0   04 Feb 1998 14:16:44   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:37  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef PARAMETERLIST_H
#define PARAMETERLIST_H

static const char rcs_id_parameter_list_h[] =
    "@(#) $Header$";

#include <sys/time.h>

#include "CommonDefs.h"
#include "Parameter.h"
#include "EAList.h"
#include "Itime.h"


// all USE_X_DURATION are in seconds
#define USE_SECONDS_DURATION 60     // use seconds for durations less than...
#define USE_MINUTES_DURATION 3600
#define USE_HOURS_DURATION 86400

class PolynomialTable;

//===============
// ParameterList 
//===============

class ParameterList : public EAList<Parameter>
{
    friend class ParTabAccess;

public:
    enum StatusE
    {
        OK,
        ERROR_ALLOCATING_MEMORY,
        ERROR_SELECTING_PARAMETER,
        ERROR_DESELECTING_PARAMETER,
        ERROR_CONVERTING_TIME,
        ERROR_TOO_FEW_PARAMETERS,
        ERROR_MISSING_SDS_NAME,
        ERROR_APPLY_POLYNOMIAL_TO_NON_FLOAT,
        ERROR_PARAMETER_NEED_POLYNOMIAL,
        ERROR_EXTRACTING_NOTHING,
        ERROR_EXTRACTING_PARAMETER,
        ERROR_TOO_MANY_VALUES,
        ERROR_TOO_FEW_VALUES
    };

    ParameterList();
    virtual ~ParameterList();

    virtual StatusE    OpenParamDataSets(TlmHdfFile*);
    virtual StatusE    CloseParamDataSets(TlmHdfFile*);

    virtual StatusE    Extract(    TlmHdfFile* tlmFile,
                                   int32       startIndex);

    virtual StatusE    HoldExtract(TlmHdfFile* tlmFile,
                                   int32       startIndex);
    virtual StatusE    PrePrint();
    virtual StatusE    Print(FILE* fp);
    virtual StatusE    PrintACEgr(FILE* fp,
                                  Itime start, 
                                  Itime end,
                                  char* comments);

    virtual StatusE    ApplyPolynomialTable(PolynomialTable* polyTable);
    virtual IotBoolean NeedPolynomial(void);

protected:
    unsigned long   _numPairs;          // total number of pairs
    unsigned long   _pairCapacity;      // capacity of allocated pair data
    StatusE         _status;

    StatusE         _SetTimeUnits(Parameter* param);
    Itime           _GetFirstTime(Parameter* param);
    Itime           _GetLastTime(Parameter* param);
    int             _GetDuration(Parameter* param, Itime* duration);
    StatusE         _AdjustTime(Parameter* param);
    char            _ReallocData(void);

};

#endif
