//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   16 Oct 1998 09:06:06   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef L1BHDFFILE_H
#define L1BHDFFILE_H

static const char rcs_id_L1BHdfFile_H[] = "@(#) $Id$";

#include "TimeTlmFile.h"

//===============//
// L1BHDFFILE    //
//===============//

class L1BHdfFile : public TimeTlmFile
{
public:

    L1BHdfFile(const char*  filename,                 // IN
               StatusE&     returnStatus,             // OUT
               const Itime  startTime = INVALID_TIME, // IN: BOF if invalid
               const Itime  endTime = INVALID_TIME);  // IN: EOF if invalid

    virtual ~L1BHdfFile();

protected:

    virtual StatusE     _getTime(int32 index, Itime* recTime);

    virtual int32       _binarySearchStart(
                             Itime  userTime,    // user's start time
                             int32  startIndex,  // start index of search
                             int32  endIndex);   // end index of search
    virtual int32       _binarySearchEnd(
                             Itime  userTime,    // user's end time
                             int32  startIndex,  // start index of search
                             int32  endIndex);   // end index of search

    int32       _hFileID;
    int32       _timeVdID[1];
};

#endif
