//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.2   18 Aug 1998 10:57:12   sally
// make L1ADrvExtract return any number of values
// 
//    Rev 1.1   23 Jul 1998 16:13:06   sally
// pass polynomial table to extractFunc()
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

    virtual StatusE    HoldExtract(TlmHdfFile*       tlmFile,
                                   int32             startIndex,
                                   PolynomialTable*  polyTable=0);
protected:
    void    _getThisPlusTime(
                           TlmHdfFile*    tlmFile,
                           Parameter*     param_ptr,
                           char*          newData,
                           char*          thisBuf,
                           int            paramIndex,
                           int            k);
    StatusE _copyAllValues(const DerivedExtractResult* extractResults,
                           int                         numParams,
                           TlmHdfFile*                 tlmFile,
                           int32                       thisIndex);

}; // DerivedParamList

#endif
