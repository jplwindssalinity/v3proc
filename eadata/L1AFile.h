//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.6   01 May 1998 14:46:42   sally
// add HK2 file
// 
//    Rev 1.5   17 Apr 1998 16:48:52   sally
// add L2A and L2B parameter tables
// 
//    Rev 1.4   23 Mar 1998 15:35:22   sally
// adapt to derived science data
// 
//    Rev 1.3   20 Feb 1998 10:58:16   sally
// L1 to L1A
// 
//    Rev 1.2   17 Feb 1998 14:47:08   sally
// NOPM
// 
//    Rev 1.1   12 Feb 1998 16:47:38   sally
// add start and end time
// Revision 1.2  1998/01/30 22:28:23  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef L1AFILE_H
#define L1AFILE_H

static const char rcs_id_L1AFile_h[] = "@(#) $Id$";

#include "TimeTlmFile.h"

//=========//
// L1AFile //
//=========//

class L1AFile : public TimeTlmFile
{
public:

    L1AFile(const char*  filename,                 // IN
            StatusE&     returnStatus,             // OUT
            const Itime  startTime = INVALID_TIME, // IN: BOF if invalid
            const Itime  endTime = INVALID_TIME);  // IN: EOF if invalid

    virtual ~L1AFile();

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
};

#endif
