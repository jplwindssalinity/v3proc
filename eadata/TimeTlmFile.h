//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   04 Aug 1999 11:07:38   sally
// need to get around HDF's maximum of 32 files
// 
//    Rev 1.0   01 May 1998 14:45:54   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef TIMETLMFILE_H
#define TIMETLMFILE_H

static const char rcs_id_TimeTlmFile_h[] =
    "@(#) $Header$";

#include <string.h>

#include "CommonDefs.h"
#include "TlmHdfFile.h"
#include "Itime.h"
#include "Parameter.h"

//-------------------------------------------------------------------
// TimeTlmFile:
//  This is an abstract class used as a generic Telemetry File
//  which contains "time" parameter in each frame.
//-------------------------------------------------------------------

class TimeTlmFile : public TlmHdfFile
{
public:

    TimeTlmFile(
           const char*  filename,                 // IN
           SourceIdE    dataType,                 // IN
           StatusE&     returnStatus,             // OUT
           const Itime  startTime = INVALID_TIME, // IN: BOF if invalid
           const Itime  endTime = INVALID_TIME);  // IN: EOF if invalid


    virtual ~TimeTlmFile();

    virtual Itime       GetFirstDataTime(void) { return(_firstDataRecTime); }
    virtual Itime       GetLastDataTime(void) { return(_lastDataRecTime); }

    virtual StatusE     Range(FILE* ofp);

protected:

    virtual StatusE     _setTimeParam(void);

    virtual StatusE     _setFileIndices(void);

    // these need to be called by constructor so they can't be pure virtual
    virtual StatusE     _selectTimeDataset(void)=0;
    virtual void        _closeTimeDataset(void)=0;
    virtual StatusE     _getTime(int32 index, Itime* recTime)=0;

    virtual int32       _binarySearchStart(
                             Itime  userTime,    // user's start time
                             int32  startIndex,  // start index of search
                             int32  endIndex)=0;   // end index of search
    virtual int32       _binarySearchEnd(
                             Itime  userTime,    // user's end time
                             int32  startIndex,  // start index of search
                             int32  endIndex)=0;   // end index of search

    int32       _timeSdsID[1];
    Itime       _userStartTime;    // user's start time
    Itime       _userEndTime;      // user's end time
    Itime       _firstDataRecTime; // time of the first data(subclass fills in)
    Itime       _lastDataRecTime;  // time of the last data(subclass fills in)

};

#endif
