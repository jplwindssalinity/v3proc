//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.2   20 Aug 1999 14:12:08   sally
// ignore bad HDF files and continue processing
// 
//    Rev 1.1   04 Aug 1999 11:07:22   sally
// need to get around HDF's maximum of 32 files
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
    if (_status != HdfFile::OK)
        return;

    _status = _setTimeParam();
    _CloseFile();

}//HK2File::HK2File

void
HK2File::_closeTimeDataset(void)
{
    // close "time" data set
    if (_timeSdsID[0] != HDF_FAIL)
    {
        (void)CloseDataset(_timeSdsID[0]);
        _timeSdsID[0] = HDF_FAIL;
    }
    return;
} // HK2File::_closeTimeDataset

HK2File::~HK2File()
{
    _closeTimeDataset();
    return;

} //HK2File::~HK2File

HdfFile::StatusE
HK2File::_selectTimeDataset(void)
{
    const char* sdsNames = ParTabAccess::GetSdsNames(SOURCE_HK2, TAI_TIME);
    if (sdsNames == 0)
        return(_status = ERROR_SEEKING_TIME_PARAM);

    char tempString[BIG_SIZE];
    (void)strncpy(tempString, sdsNames, BIG_SIZE);
    int32 dataType, dataStartIndex, dataLength, numDimensions;
    char *oneSdsName = (char*)strtok(tempString, ",");
    if (oneSdsName == 0)
        return(_status = ERROR_SEEKING_TIME_PARAM);

    //--------------------------------------------
    // get the first and last data record time
    //--------------------------------------------
    _timeSdsID[0] = SelectDataset(oneSdsName, dataType,
                         dataStartIndex, dataLength, numDimensions);
    if (_timeSdsID[0] == HDF_FAIL)
        return(_status = ERROR_SELECTING_DATASET);

    return(HdfFile::OK);

} // HK2File::_selectTimeDataset

HK2File::StatusE
HK2File::_getTime(
int32     index,    // dataset index
Itime*    recTime)  // OUT: record time
{
    assert(_timeSdsID[0] != HDF_FAIL);

    //if (_timeSdsID[0] == HDF_FAIL);
    //    _selectTimeDataset();

    if ( ! ExtractTaiTime(this, _timeSdsID, index, 1, 1, recTime))
        return (_status = ERROR_EXTRACT_TIME_PARAM);

    return (_status = HdfFile::OK);

} //HK2File::_getTime

int32
HK2File::_binarySearchStart(
Itime      userTime,
int32      startIndex,
int32      endIndex)
{
    Itime startTime, endTime;
    if (_getTime(startIndex, &startTime) != HdfFile::OK)
        return(HDF_FAIL);
    if (_getTime(endIndex, &endTime) != HdfFile::OK)
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
    if (_getTime(midIndex, &midTime) != HdfFile::OK)
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
    if (_getTime(startIndex, &startTime) != HdfFile::OK)
        return(HDF_FAIL);
    if (_getTime(endIndex, &endTime) != HdfFile::OK)
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
    if (_getTime(midIndex, &midTime) != HdfFile::OK)
        return(HDF_FAIL);

    if (midTime >= userTime)
        // start time is in the first half
        return(_binarySearchEnd(userTime, startIndex, midIndex));
    else
        // start time is in the second half
        return(_binarySearchEnd(userTime, midIndex, endIndex));

}//HK2File::_binarySearchEnd
