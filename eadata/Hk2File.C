//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   01 May 1998 14:45:46   sally
// Initial revision.
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

#include "Hk2File.h"
#include "L1AExtract.h"
#include "ParTab.h"

//========//
// HK2File //
//========//

HK2File::HK2File(
const char*     filename,
StatusE&        returnStatus,
const Itime     startTime,
const Itime     endTime)
:   TimeTlmFile(filename, SOURCE_HK2, returnStatus, startTime, endTime)
{
    // check TimeTlmFile construction
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

    const char* sdsNames = ParTabAccess::GetSdsNames(SOURCE_HK2, TAI_TIME);
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

}//HK2File::HK2File

HK2File::~HK2File()
{
    // close "time" data set
    if (_timeSdsID[0] != HDF_FAIL)
    {
        (void)CloseDataset(_timeSdsID[0]);
        _timeSdsID[0] = HDF_FAIL;
    }

    return;

} //HK2File::~HK2File

HK2File::StatusE
HK2File::_getTime(
int32     index,    // dataset index
Itime*    recTime)  // OUT: record time
{
    assert(_timeSdsID[0] != HDF_FAIL);

    if ( ! ExtractTaiTime(this, _timeSdsID, index, 1, 1, recTime))
        return (_status = ERROR_EXTRACT_TIME_PARAM);

    return (_status = OK);

} //HK2File::_getTime

int32
HK2File::_binarySearchStart(
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

}//HK2File::_binarySearchStart

int32
HK2File::_binarySearchEnd(
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

}//HK2File::_binarySearchEnd
