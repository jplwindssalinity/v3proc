//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.3   20 Apr 1998 15:19:18   sally
// change List to EAList
// 
//    Rev 1.2   20 Feb 1998 10:57:06   sally
// 
// L1 to L1A
// 
//    Rev 1.1   12 Feb 1998 16:47:06   sally
// add start and end time
// .
// Revision 1.4  1998/01/31 00:36:39  sally
// add scale factor
//
// Revision 1.3  1998/01/30 22:28:19  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef HDFFILE_H
#define HDFFILE_H

static const char rcs_id_HdfFile_h[] =
    "@(#) $Id$";

#include <mfhdf.h>

#include "EAList.h"

//-------------------------------------------------------------------
// HdfFile:
//  This is an base class used as a generic HDF File.
//  The file is assumed to have same length of data in all datasets.
//-------------------------------------------------------------------

#ifndef HDF_FAIL
#define HDF_FAIL FAIL
#endif

#ifndef HDF_SUCCEED
#define HDF_SUCCEED SUCCEED
#endif

class HdfFile
{
public:
    enum { SEARCH_START, SEARCH_END };
    enum StatusE
    {
        //---------//
        // General //
        //---------//

        OK,
        ERROR_ALLOCATING_FILENAME,
        ERROR_OPENING_FILE,
        ERROR_INVALID_DATASET_NAME,
        ERROR_INVALID_DATASET_ID,
        ERROR_SELECTING_DATASET,
        ERROR_CLOSING_DATASET,
        ERROR_GET_DATASET_INFO,
        ERROR_EMPTY_DATASET,
        ERROR_ACCESS_BEFORE_START,
        ERROR_ACCESS_AFTER_END,
        ERROR_READING_1D_DATA,
        ERROR_READING_MD_DATA,
        ERROR_UNEVEN_DATASET_LENGTH,

        ERROR_SEEKING_TIME_PARAM,
        ERROR_EXTRACT_TIME_PARAM,
        ERROR_EXTRACT_DATA,
        ERROR_USER_STARTTIME_AFTER_ENDTIME,
        ERROR_FILE_STARTTIME_AFTER_ENDTIME,

        ERROR_SEEKING_TIMESEARCH,
        ERROR_ALLOCATING_TIMESEARCH,
        ERROR_READING_TIMESEARCH,
        ERROR_EXTRACTING_TIMESEARCH,
        ERROR_EXTRACTING_BEFORE_START_TIME,
        ERROR_EXTRACTING_AFTER_END_TIME,

        UNKNOWN_CONDITION,
        NO_MORE_DATA 
    };

    HdfFile(const char*    filename,            // IN
            StatusE&       returnStatus);       // OUT

    virtual ~HdfFile();

    //----------------------------------------------------------------
    // select dataset, return dataset index ID or HDF_FAIL
    // use GetStatus() to get detailed error status
    //----------------------------------------------------------------
    virtual int32   SelectDataset(
                                const char*   datasetName,    // IN
                                int32&        dataType,       // OUT
                                int32&        dataStartIndex, // OUT
                                int32&        dataLength,     // OUT
                                int32&        numDimensions); // OUT

    virtual int   GetScaleFactor(
                              int32    sdsId,          // IN: sds ID
                              float64& factor);        // OUT: factor

    // close dataset, return HDF_SUCCEED or HDF_FAIL
    virtual int     CloseDataset(int32    datasetID);    // IN

    //----------------------------------------------------------------
    // read one dimensional dataset, return HDF_SUCCEED or HDF_FAIL
    // use GetStatus() to get detailed error status
    //----------------------------------------------------------------
    virtual int     GetDatasetData1D(
                                int32   datasetID,     // IN
                                int32   start,         // IN
                                int32   stride,        // IN, 1=no skipping
                                int32   dataLength,    // IN
                                VOIDP   data);         // OUT

    //----------------------------------------------------------------
    // read multi dimensional dataset, return HDF_SUCCEED or HDF_FAIL
    // use GetStatus() to get detailed error status
    // all dimensoins are assumed to have same length of data
    //----------------------------------------------------------------
    virtual int     GetDatasetDataMD(
                                int32   datasetID,     // IN
                                int32*  start,         // IN
                                int32*  stride,        // IN,
                                int32*  dataLength,    // IN
                                VOIDP   data);         // OUT


    const char*     GetFileName(void) { return _filename; }
    StatusE         GetStatus(void) { return(_status); }
    void            ClearStatus(void) { _status = HdfFile::OK; }
    int             GetDataLength(void) { return _dataLength; }

protected:
    StatusE         _DupFilename(const char* filename);

    EAList<int32>   _datasetIDs;

    int32           _SDfileID;
    StatusE         _status;
    char*           _filename;
    int32           _dataLength;

    int32           _startIndex;
    int32           _endIndex;

};

#endif
