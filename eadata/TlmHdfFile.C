//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.5   15 Mar 1999 14:19:42   sally
// add some methods for getting user's start and end indexes
// 
//    Rev 1.4   01 May 1998 14:47:54   sally
// added HK2 file
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
    "@(#) $Header$";

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
const char*              filename,
SourceIdE                sourceType,
HdfFile::StatusE&        returnStatus)
:   HdfFile(filename, returnStatus), _sourceType(sourceType),
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
