//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   04 Aug 1999 11:07:34   sally
// need to get around HDF's maximum of 32 files
// 
//    Rev 1.0   01 May 1998 14:45:50   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

static const char rcs_id[] =
    "@(#) $Id$";

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "TimeTlmFile.h"

//=========//
// TimeTlmFile //
//=========//

TimeTlmFile::TimeTlmFile(
const char*     filename,
SourceIdE       sourceType,
StatusE&        returnStatus,
const Itime     startTime,
const Itime     endTime)
:   TlmHdfFile(filename, sourceType, returnStatus),
    _userStartTime(startTime), _userEndTime(endTime),
    _firstDataRecTime(INVALID_TIME), _lastDataRecTime(INVALID_TIME)
{

    // check TimeTlmFile construction
    if (_status != HdfFile::OK)
        return;

    if (startTime != INVALID_TIME && endTime != INVALID_TIME &&
                     startTime > endTime)
    {
        _status = returnStatus = ERROR_USER_STARTTIME_AFTER_ENDTIME;
        return;
    }

}//TimeTlmFile::TimeTlmFile

HdfFile::StatusE
TimeTlmFile::_setTimeParam(void)
{
    _firstDataRecTime = INVALID_TIME;
    _lastDataRecTime = INVALID_TIME;

    if ((_status = _selectTimeDataset()) != HdfFile::OK)
        return(_status);

    if (_getTime(0, &_firstDataRecTime) != HdfFile::OK)
        return(_status = ERROR_EXTRACT_DATA);

    if (_getTime(_dataLength - 1, &_lastDataRecTime) != HdfFile::OK)
        return(_status = ERROR_EXTRACT_DATA);

    // check the time parameter in the file
    if (_firstDataRecTime > _lastDataRecTime)
    {
        fprintf(stderr, "%s: first data time is later than last\n", _filename);

        //--------------------------------------
        // Lee said: report but continue
        // _status = returnStatus = ERROR_FILE_STARTTIME_AFTER_ENDTIME;
        // return;
        //--------------------------------------
    }

    // if data is out of range, then return "no more data"
    if ((_userStartTime != INVALID_TIME && _userStartTime > _lastDataRecTime)
      || (_userEndTime != INVALID_TIME && _userEndTime < _firstDataRecTime))
        return(_status = NO_MORE_DATA);

    // set the start and end indices according to user's start and end time
    if (_setFileIndices() != HdfFile::OK)
        return(_status);

    _userNextIndex = _userStartIndex;

    return(_status = HdfFile::OK);

}//TimeTlmFile::TimeTlmFile

TimeTlmFile::~TimeTlmFile()
{
    _userStartTime = INVALID_TIME;
    _userEndTime = INVALID_TIME;
    _firstDataRecTime = INVALID_TIME;
    _lastDataRecTime = INVALID_TIME;
    return;

}//TimeTlmFile::~TimeTlmFile

TimeTlmFile::StatusE
TimeTlmFile::Range(
    FILE*   ofp)
{
    fprintf(ofp, "%s : %s\n", source_id_map[_sourceType], _filename);
    char start_time_string[CODEA_TIME_LEN];
    _firstDataRecTime.ItimeToCodeA(start_time_string);
    char end_time_string[CODEA_TIME_LEN];
    _lastDataRecTime.ItimeToCodeA(end_time_string);
    fprintf(ofp, "  %s - %s\n", start_time_string, end_time_string);
    fprintf(ofp, "  %ld Data Records\n", _dataLength);
    return(_status);

}//TimeTlmFile::Range

//-----------------//
// _setFileIndices //
//-----------------//
// set the first and last data records offsets.
// if either offset cannot be set, both offsets are set to -1.

HdfFile::StatusE
TimeTlmFile::_setFileIndices(void)
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

}//TimeTlmFile::_setFileIndices
