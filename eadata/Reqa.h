//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.2   01 Sep 1998 16:39:08   sally
// change operator== to Match in order to pass outFP
// 
//    Rev 1.1   28 Aug 1998 16:30:28   sally
// use REQA's file number to match REQQ file
// 
//    Rev 1.0   28 Aug 1998 11:21:20   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#ifndef REQA_H
#define REQA_H

#include <stdio.h>
#include "ReqaqList.h"
#include "Reqq.h"

static const char rcs_id_reqa_h[] =
    "@(#) $Header$";


//============
// Reqq class 
//============

class Reqa
{
public:

    // read from REQA file
    Reqa(const char*  filename,    // reqq file path + name
         FILE*        outFP);

    virtual ~Reqa();
    ReqqStatusE     GetStatus() { return (_status);};

    const char*     GetFilename() const { return _filename; };

    int             GetFileNumber(void) const
                        { return _reqaList->GetFileNumber(); };
    char            GetFileVersion(void) const
                        { return _reqaList->GetFileVersion(); };

    int             Match(const Reqq& reqq, FILE* outFP);

private:
    char*               _filename;
    ReqqStatusE         _status;
    ReqaRecordList*     _reqaList;
};

#endif
