//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.11   20 Aug 1999 14:12:14   sally
// ignore bad HDF files and continue processing
// 
//    Rev 1.10   04 Aug 1999 11:07:26   sally
// need to get around HDF's maximum of 32 files
// 
//    Rev 1.9   07 May 1999 13:10:52   sally
// add memory check for CDS and SES
// 
//    Rev 1.8   01 May 1998 14:46:34   sally
// add HK2 file
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
:   TimeTlmFile(filename, SOURCE_L1A, returnStatus, startTime, endTime)
{
    if (_status != HdfFile::OK)
        return;

    _status = _setTimeParam();
    _CloseFile();

}//L1AFile::L1AFile

void
L1AFile::_closeTimeDataset(void)
{
    // close "time" data set
    if (_timeSdsID[0] != HDF_FAIL)
    {
        (void)CloseDataset(_timeSdsID[0]);
        _timeSdsID[0] = HDF_FAIL;
    }
    return;
} //_closeTimeDataset

L1AFile::~L1AFile()
{
    _closeTimeDataset();
    return;

} //L1AFile::~L1AFile

HdfFile::StatusE
L1AFile::_selectTimeDataset(void)
{
    const char* sdsNames = ParTabAccess::GetSdsNames(SOURCE_L1A, TAI_TIME);
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

} //L1AFile::_selectTimeDataset

L1AFile::StatusE
L1AFile::_getTime(
int32     index,    // dataset index
Itime*    recTime)  // OUT: record time
{
    assert(_timeSdsID[0] != HDF_FAIL);

    if ( ! ExtractTaiTime(this, _timeSdsID, index, 1, 1, recTime))
        return (_status = ERROR_EXTRACT_TIME_PARAM);

    return (_status = HdfFile::OK);

} //L1AFile::_getTime

int32
L1AFile::_binarySearchStart(
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

}//L1AFile::_binarySearchStart

int32
L1AFile::_binarySearchEnd(
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

}//L1AFile::_binarySearchEnd

