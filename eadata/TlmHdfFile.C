//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.3   17 Apr 1998 16:51:24   sally
// add L2A and L2B file formats
// 
//    Rev 1.2   03 Mar 1998 13:28:00   sally
// support range
// 
//    Rev 1.1   12 Feb 1998 16:47:50   sally
// add start and end time
// Revision 1.2  1998/01/30 22:29:25  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

static const char rcs_id[] =
    "@(#) $Id$";

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "TlmHdfFile.h"

//=========//
// TlmHdfFile //
//=========//

TlmHdfFile::TlmHdfFile(
const char*     filename,
SourceIdE       dataType,
StatusE&        returnStatus,
const Itime     startTime,
const Itime     endTime)
:   HdfFile(filename, returnStatus), _dataType(dataType),
    _userStartTime(startTime), _userEndTime(endTime),
    _firstDataRecTime(INVALID_TIME), _lastDataRecTime(INVALID_TIME),
    _userNextIndex(HDF_FAIL), _userStartIndex(HDF_FAIL),
    _userEndIndex(HDF_FAIL)
{
    if (_status != HdfFile::OK)
    {
        returnStatus = _status;
        return;
    }

    returnStatus = _status;
    return;
}

TlmHdfFile::~TlmHdfFile()
{
    _userNextIndex = HDF_FAIL;
    _userStartIndex = HDF_FAIL;
    _userEndIndex = HDF_FAIL;

    return;

}//TlmHdfFile::~TlmHdfFile

HdfFile::StatusE
TlmHdfFile::GetNextIndex(
int32&    nextIndex)       // IN/OUT
{
    if (_status != OK)
        return (_status);

    if (_userNextIndex > _userEndIndex)
        return (_status = NO_MORE_DATA);
    else
    {
        nextIndex = _userNextIndex++;
        return (_status = OK);
    }
}//TlmHdfFile::GetNextIndex