//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.10   07 Sep 1999 13:33:06   sally
//  add interface for Global Attributes
// 
//    Rev 1.9   04 Aug 1999 11:07:20   sally
// need to get around HDF's maximum of 32 files
// 
//    Rev 1.8   15 Mar 1999 14:19:34   sally
// add some methods for getting user's start and end indexes
// 
//    Rev 1.7   03 Nov 1998 15:59:44   sally
// adapt to Vdata
// 
//    Rev 1.6   28 May 1998 13:18:34   daffer
// worked on HdfGlobalAttr and ParseGlobalAttr
// 
//    Rev 1.5   22 May 1998 16:51:28   daffer
// Added ParseGlobalAttr
// 
//    Rev 1.4   08 May 1998 10:47:38   sally
// added global attribute methods
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
    "@(#) $Header$";

#include <mfhdf.h>
#include <string.h>

#include "Parameter.h"
#include "EAList.h"
#include "SafeString.h"


//------------------------------------------
//
//  HdfGlobalAttr
//  Used in parsing Global Attributes from HDF files
//
//------------------------------------------
class HdfGlobalAttr 
{
 public:
    HdfGlobalAttr();
    ~HdfGlobalAttr();

    char       *name;
    DataTypeE  type;
    int        dims[2];
    VOIDP      data;
}; 

inline int operator==(const HdfGlobalAttr& a, const HdfGlobalAttr& b)
{
    return(strcmp(a.name, b.name) == 0 && a.type == b.type ? 1 : 0);
}

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
        ERROR_OUT_OF_MEMORY,
        ERROR_ALLOCATING_FILENAME,
        ERROR_OPENING_FILE,
        ERROR_INVALID_DATASET_NAME,
        ERROR_INVALID_DATASET_ID,
        ERROR_SELECTING_DATASET,
        ERROR_CLOSING_DATASET,
        ERROR_GET_FILE_INFO,
        ERROR_GET_ATTR_INFO,
        ERROR_GLOBAL_ATTR_NOT_FOUND,
        ERROR_INVALID_GLOBAL_ATTR_INDEX,
        ERROR_PARSE_GLOBAL_ATTR,
        ERROR_GET_ATTR,
        ERROR_BAD_DATATYPE,
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
        ERROR_SELECTING_PARAMETER,
        ERROR_DESELECTING_PARAMETER,
        UNKNOWN_CONDITION,
        NO_MORE_DATA 
    };

    HdfFile(const char*           filename,            // IN
            HdfFile::StatusE&     returnStatus);       // OUT

    virtual ~HdfFile();

    //------------------------------------------------------
    // constructor closes the file at the end.
    // so if application uses its member methods,
    // it doesn't have to call OpenFile().
    // Otherwise, it must be called before anything could be done.
    //------------------------------------------------------
    StatusE         OpenFile(void);

    //------------------------------------------------------
    // get the info about the global attributes
    // then get the global attributes themselves
    //------------------------------------------------------
    int32           GetNumGlobalAttrs(void) { return _numGlobAttr; }
    virtual StatusE GetGlobalAttrInfo(
                                const char*   attrName,      // IN
                                int32&        attrIndex,     // OUT
                                DataTypeE&    eaType,        // OUT
                                int32&        numValues);    // OUT

    virtual StatusE GetGlobalAttr(
                                int32         attrIndex,     // IN
                                DataTypeE     eaType,        // IN
                                VOIDP         attrBuf,       // IN/OUT
                                int32         numValues = 1);// IN

    virtual StatusE ParseGlobalAttr(
                                char*           attrName,       // IN
                                VOIDP           attrBuf,        // IN
                                HdfGlobalAttr*  globalAttr);    // IN/OUT

    virtual StatusE ParseGlobalAttr(
                                char*           attrName,       // IN
                                HdfGlobalAttr&  globalAttr);    // IN/OUT

    virtual StatusE ParseGlobalAttr(
                                int32           attrIndex,      // IN
                                HdfGlobalAttr&  globalAttr);    // IN/OUT

    //------------------------------------------------------
    // select dataset, return dataset index ID or HDF_FAIL
    // use GetStatus() to get detailed error status
    //------------------------------------------------------
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

    //------------------------------------------------------
    // open or close all the datasets that are related to this parameter
    //------------------------------------------------------
    StatusE         OpenParamDatasets(Parameter* paramP);
    StatusE         CloseParamDatasets(Parameter* paramP);

    //------------------------------------------------------
    // read one dimensional dataset, return HDF_SUCCEED or HDF_FAIL
    // use GetStatus() to get detailed error status
    //------------------------------------------------------
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
    int32           GetSDfileID(void) { return _SDfileID; }
    StatusE         GetStatus(void) { return(_status); }
    void            ClearStatus(void) { _status = HdfFile::OK; }
    int             GetDataLength(void) { return _dataLength; }
    int32           GetNumDatasets(void) { return _numDatasets; }

                    // return 1 if ok, else 0
    static int      HdfTypeToEaType(int32       hdfType,   // IN
                                    DataTypeE&  eaType);   // OUT
    static int      EaTypeToHdfType(DataTypeE   eaType,    // IN
                                    int32&      hdfType);  // OUT

protected:
    StatusE         _DupFilename(const char* filename);
    void            _CloseFile(void);

    EAList<int32>   _datasetIDs;

    int32           _SDfileID;
    int32           _hFileID;
    int32           _numDatasets;
    int32           _numGlobAttr;
    StatusE         _status;
    char*           _filename;
    int32           _dataLength;

    int32           _startIndex;
    int32           _endIndex;

};
#endif
