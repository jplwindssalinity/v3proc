//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.3   04 Aug 1999 11:07:30   sally
// need to get around HDF's maximum of 32 files
// 
//    Rev 1.2   03 Nov 1998 16:01:00   sally
// adapt to Vdata
// 
//    Rev 1.1   28 Oct 1998 15:03:52   sally
// Revision 1.1  1998/10/20 21:26:17  sally
// Initial revision
//
// 
//    Rev 1.0   16 Oct 1998 09:06:04   sally
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

#include "L1BHdfFile.h"
#include "L1AExtract.h"
#include "ParTab.h"

//========//
// L1BHdfFile //
//========//

L1BHdfFile::L1BHdfFile(
const char*     filename,
StatusE&        returnStatus,
const Itime     startTime,
const Itime     endTime)
:   TimeTlmFile(filename, SOURCE_L1B, returnStatus, startTime, endTime)
{
    _status = _setTimeParam();
    _CloseFile();

}//L1BHdfFile::L1BHdfFile

void
L1BHdfFile::_closeTimeDataset(void)
{
    // close "time" V data
    if (_timeVdID[0] != HDF_FAIL)
    {
        (void)VSdetach(_timeVdID[0]);
        _timeVdID[0] = HDF_FAIL;
    }

    return;

} //L1BHdfFile::_closeTimeDataset

L1BHdfFile::~L1BHdfFile()
{
    _closeTimeDataset();
    return;

} //L1BHdfFile::~L1BHdfFile

HdfFile::StatusE
L1BHdfFile::_selectTimeDataset(void)
{
    if (_timeVdID[0] != HDF_FAIL)
        return(HdfFile::OK);

    //-------------------------------------------
    // get the start and end time from V data
    //-------------------------------------------
    const char* sdsNames = ParTabAccess::GetSdsNames(SOURCE_L1B, FRAME_TIME);
    if (sdsNames == 0)
        return(_status = ERROR_SEEKING_TIME_PARAM);

    char tempString[BIG_SIZE];
    (void)strncpy(tempString, sdsNames, BIG_SIZE);
    char *oneSdsName = (char*)strtok(tempString, ",");
    if (oneSdsName == 0)
        return(_status = ERROR_SEEKING_TIME_PARAM);

    char vDataName[SHORT_STRING_LEN];
    if (sscanf(oneSdsName, "%*c:%s", vDataName) != 1)
        return(_status = ERROR_SEEKING_TIME_PARAM);

    //--------------------------------------------
    // get the first and last data record time
    //--------------------------------------------
    int32 vDataRefNo = VSfind(_hFileID, vDataName);
    if (vDataRefNo == 0)
        return(_status = ERROR_SELECTING_DATASET);

    if ((_timeVdID[0] = VSattach(_hFileID, vDataRefNo, "r"))== HDF_FAIL)
        return(_status = ERROR_SELECTING_DATASET);

    return(HdfFile::OK);

} // L1BHdfFile::_selectTimeDataset

L1BHdfFile::StatusE
L1BHdfFile::_getTime(
int32     index,    // dataset index
Itime*    recTime)  // OUT: record time
{
    if (_timeVdID[0] == HDF_FAIL);
        _selectTimeDataset();

    if ( ! ExtractL1Time(this, _timeVdID, index, 1, 1, recTime))
        return (_status = ERROR_EXTRACT_TIME_PARAM);

    return (_status = HdfFile::OK);

} //L1BHdfFile::_getTime

int32
L1BHdfFile::_binarySearchStart(
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

}//L1BHdfFile::_binarySearchStart

int32
L1BHdfFile::_binarySearchEnd(
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

}//L1BHdfFile::_binarySearchEnd

#ifdef TESTL1BHDF_C

main(
int     argc,
char**  argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s L1BHdfFile\n", argv[0]);
        exit(1);
    }
    HdfFile::StatusE status = HdfFile::OK;
    L1BHdfFile l1BHdfFile(argv[1], status);
    if (status != HdfFile::OK)
    {
        fprintf(stderr, "L1BHdfFile creation failed\n");
        exit(1);
    }
    return 0;
}

#endif
