//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   01 Apr 1998 13:36:56   sally
// Initial revision.
// $Date$
// $Revision4
// $Author$
//
//
//=========================================================

#ifndef DRVPARAMLIST_H
#define DRVPARAMLIST_H

static const char rcs_id_Drv_parameter_list_h[] =
    "@(#) $Header$";

#include "ParameterList.h"

//===============
// DerivedParamList
//===============

class DerivedParamList : public ParameterList
{
public:

    DerivedParamList();
    virtual ~DerivedParamList();

#if 0
    virtual StatusE    Extract(    TlmHdfFile* tlmFile,
                                   int32       startIndex);
#endif

    virtual StatusE    HoldExtract(TlmHdfFile* tlmFile,
                                   int32       startIndex);
protected:
    struct ExtractResult
    {
        int    numExtracted;
        char*  dataBuf;
    };

    StatusE _copyAllValues(unsigned int         maxNumExtracted,
                           const ExtractResult* extractResults,
                           int                  numParams,
                           TlmHdfFile*          tlmFile,
                           int32                thisIndex);

}; // DerivedParamList

#endif
