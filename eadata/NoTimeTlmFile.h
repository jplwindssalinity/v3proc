//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.1   20 Apr 1998 10:22:18   sally
// change for WindSwatch
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#ifndef NoTimeTlmFile_H
#define NoTimeTlmFile_H

static const char rcs_id_NoTimeTlmFile_H[] = "@(#) $Header$";

#include "TlmHdfFile.h"


//---------------------------------------------------
// TLM file which contains no time parameter
//---------------------------------------------------
class NoTimeTlmFile : public TlmHdfFile
{
public:

    NoTimeTlmFile(const char*  filename,                 // IN
                  StatusE&     returnStatus);            // OUT

    virtual ~NoTimeTlmFile();

    virtual StatusE     Range(FILE* ofp);

protected:

    virtual StatusE     _setFileIndices(void);
};

#endif
