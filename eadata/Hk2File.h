//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   04 Aug 1999 11:07:24   sally
// need to get around HDF's maximum of 32 files
// 
//    Rev 1.0   01 May 1998 14:45:52   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef HK2FILE_H
#define HK2FILE_H

static const char rcs_id_Hk2File_h[] = "@(#) $Id$";

#include "TimeTlmFile.h"

//=========//
// HK2File //
//=========//

class HK2File : public TimeTlmFile
{
public:

    HK2File(const char*  filename,                 // IN
            StatusE&     returnStatus,             // OUT
            const Itime  startTime = INVALID_TIME, // IN: BOF if invalid
            const Itime  endTime = INVALID_TIME);  // IN: EOF if invalid

    virtual ~HK2File();

protected:

    virtual StatusE     _selectTimeDataset(void);
    virtual void        _closeTimeDataset(void);
    virtual StatusE     _getTime(int32 index, Itime* recTime);

    virtual int32       _binarySearchStart(
                             Itime  userTime,    // user's start time
                             int32  startIndex,  // start index of search
                             int32  endIndex);   // end index of search
    virtual int32       _binarySearchEnd(
                             Itime  userTime,    // user's end time
                             int32  startIndex,  // start index of search
                             int32  endIndex);   // end index of search
};

#endif
