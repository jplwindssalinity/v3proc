//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.7   17 Apr 1998 16:48:50   sally
// add L2A and L2B parameter tables
// 
//    Rev 1.6   24 Mar 1998 15:57:06   sally
// de-warn for GNU
// 
//    Rev 1.5   24 Mar 1998 09:29:50   sally
// de-warn for GNU
// 
//    Rev 1.4   23 Mar 1998 15:35:14   sally
// adapt to derived science data
// 
//    Rev 1.3   20 Feb 1998 10:58:12   sally
// L1 to L1A
// 
//    Rev 1.2   17 Feb 1998 14:45:54   sally
// NOPM
// 
//    Rev 1.1   12 Feb 1998 16:47:26   sally
// add start and end time
// Revision 1.2  1998/01/30 22:29:10  daffer
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include "L1AFile.h"
#include "L1AExtract.h"
#include "ParTab.h"

//========//
// L1AFile //
//========//

L1AFile::L1AFile(
const char*     filename,
StatusE&        returnStatus,
const Itime     startTime,
const Itime     endTime)
:   TlmHdfFile(filename, SOURCE_L1A, returnStatus, startTime, endTime)
{
    // check TlmHdfFile construction
    if (_status != HdfFile::OK)
        return;

    _firstDataRecTime = INVALID_TIME;
    _lastDataRecTime = INVALID_TIME;

    if (startTime != INVALID_TIME && endTime != INVALID_TIME &&
                     startTime > endTime)
    {
        _status = returnStatus = ERROR_USER_STARTTIME_AFTER_ENDTIME;
        return;
    }

    const char* sdsNames = ParTabAccess::GetSdsNames(SOURCE_L1A, TAI_TIME);
    if (sdsNames == 0)
    {
        _status = returnStatus = ERROR_SEEKING_TIME_PARAM;
        return;
    }

    char tempString[BIG_SIZE];
    (void)strncpy(tempString, sdsNames, BIG_SIZE);
    int32 dataType, dataStartIndex, dataLength, numDimensions;
    char *oneSdsName = (char*)strtok(tempString, ",");
    if (oneSdsName == 0)
    {
        _status = returnStatus = ERROR_SEEKING_TIME_PARAM;
        return;
    }
    //--------------------------------------------
    // get the first and last data record time
    //--------------------------------------------
    _timeSdsID[0] = SelectDataset(oneSdsName, dataType,
                         dataStartIndex, dataLength, numDimensions);
    if (_timeSdsID[0] == HDF_FAIL)
    {
        _status = returnStatus = ERROR_SELECTING_DATASET;
        return;
    }
    if (_getTime(0, &_firstDataRecTime) != OK)
    {
        _status = returnStatus = ERROR_EXTRACT_DATA;
        return;
    }

    if (_getTime(_dataLength - 1, &_lastDataRecTime) != OK)
    {
        _status = returnStatus = ERROR_EXTRACT_DATA;
        return;
    }

    // check the time parameter in the file
    if (_firstDataRecTime > _lastDataRecTime)
    {
        fprintf(stderr, "%s: first data time is later than last\n", _filename);
        _status = returnStatus = ERROR_FILE_STARTTIME_AFTER_ENDTIME;
        return;
    }

    // if data is out of range, then return "no more data"
    if ((_userStartTime != INVALID_TIME && _userStartTime > _lastDataRecTime)
      || (_userEndTime != INVALID_TIME && _userEndTime < _firstDataRecTime))
    {
        returnStatus = _status = NO_MORE_DATA;
        return;
    }

    // set the start and end indices according to user's start and end time
    if (_setFileIndices() != OK)
    {
        returnStatus = _status;
        return;
    }

    _userNextIndex = _userStartIndex;

    returnStatus = _status;
    return;

}//L1AFile::L1AFile

L1AFile::~L1AFile()
{
    // close "time" data set
    if (_timeSdsID[0] != HDF_FAIL)
    {
        (void)CloseDataset(_timeSdsID[0]);
        _timeSdsID[0] = HDF_FAIL;
    }

    return;

} //L1AFile::~L1AFile

TlmHdfFile::StatusE
L1AFile::Range(
    FILE*   ofp)
{
    fprintf(ofp, "%s : %s\n", source_id_map[_dataType], _filename);
    char start_time_string[CODEA_TIME_LEN];
    _firstDataRecTime.ItimeToCodeA(start_time_string);
    char end_time_string[CODEA_TIME_LEN];
    _lastDataRecTime.ItimeToCodeA(end_time_string);
    fprintf(ofp, "  %s - %s\n", start_time_string, end_time_string);
    fprintf(ofp, "  %ld Data Records\n", _dataLength);
    return(_status);

}//L1AFile::Range

L1AFile::StatusE
L1AFile::_getTime(
int32     index,    // dataset index
Itime*    recTime)  // OUT: record time
{
    assert(_timeSdsID[0] != HDF_FAIL);

    if ( ! ExtractTaiTime(this, _timeSdsID, index, 1, 1, recTime))
        return (_status = ERROR_EXTRACT_TIME_PARAM);

    return (_status = OK);

} //L1AFile::_getTime

//-----------------//
// _setFileIndices //
//-----------------//
// set the first and last data records offsets.
// if either offset cannot be set, both offsets are set to -1.

L1AFile::StatusE
L1AFile::_setFileIndices(void)
{
    _status = OK;

    // find the effective start index
    _userStartIndex = 0;
    if (_userStartTime != INVALID_TIME)
        _userStartIndex = _binarySearchStart(_userStartTime,
                                               0, _dataLength - 1);
    if (_userStartIndex == HDF_FAIL)
        return(_status = ERROR_SEEKING_TIME_PARAM);

    // find the effective end index
    _userEndIndex = _dataLength - 1;
    if (_userEndTime != INVALID_TIME)
        _userEndIndex = _binarySearchEnd(_userEndTime,
                                               0, _dataLength - 1);
    if (_userEndIndex == HDF_FAIL)
        return(_status = ERROR_SEEKING_TIME_PARAM);

    return(_status = OK);

}//L1AFile::_setFileIndices

int32
L1AFile::_binarySearchStart(
Itime      userTime,
int32      startIndex,
int32      endIndex)
{
    Itime startTime, endTime;
    if (_getTime(startIndex, &startTime) != OK)
        return(HDF_FAIL);
    if (_getTime(endIndex, &endTime) != OK)
        return(HDF_FAIL);

    // Is userTime after the endTime
    if (userTime > endTime)
        return(HDF_FAIL);

    // if startTime is after userTime, startTime is it
    if (startTime >= userTime)
        return(startIndex);
    // if startIndex and endIndex are just 1 number apart, endIndex is it
    else if ((endIndex - startIndex) == 1)
        return(endIndex);

    // divide the data indices into two parts, and search
    int32 midIndex = startIndex + (endIndex - startIndex) / 2;
    Itime midTime;
    if (_getTime(midIndex, &midTime) != OK)
        return(HDF_FAIL);

    if (midTime >= userTime)
        // start time is in the first half
        return(_binarySearchStart(userTime, startIndex, midIndex));
    else
        // start time is in the second half
        return(_binarySearchStart(userTime, midIndex, endIndex));

}//L1AFile::_binarySearchStart

int32
L1AFile::_binarySearchEnd(
Itime      userTime,
int32      startIndex,
int32      endIndex)
{
    Itime startTime, endTime;
    if (_getTime(startIndex, &startTime) != OK)
        return(HDF_FAIL);
    if (_getTime(endIndex, &endTime) != OK)
        return(HDF_FAIL);

    // Is userTime after the endTime
    if (userTime < startTime)
        return(HDF_FAIL);

    // if endTime is before userTime, endTime is it
    if (userTime >= endTime)
        return(endIndex);
    // if startIndex and endIndex are just 1 number apart, startIndex is it
    else if ((endIndex - startIndex) == 1)
        return(startIndex);

    // divide the data indices into two parts, and search
    int32 midIndex = startIndex + (endIndex - startIndex) / 2;
    Itime midTime;
    if (_getTime(midIndex, &midTime) != OK)
        return(HDF_FAIL);

    if (midTime >= userTime)
        // start time is in the first half
        return(_binarySearchEnd(userTime, startIndex, midIndex));
    else
        // start time is in the second half
        return(_binarySearchEnd(userTime, midIndex, endIndex));

}//L1AFile::_binarySearchEnd

