//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   04 Feb 1998 14:15:28   daffer
// Initial checking
// Revision 1.3  1998/01/31 00:36:39  sally
// add scale factor
//
// Revision 1.2  1998/01/30 22:29:04  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef HDFFILE_C_INCLUDED
#define HDFFILE_C_INCLUDED

static const char HdfFile_c_rcs_id[] =
    "@(#) $Header$";

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "HdfFile.h"

//=========
// HdfFile 
//=========

HdfFile::HdfFile(
const char*     filename,
StatusE&        returnStatus)
:   _SDfileID(FAIL), _status(OK), _filename(0), _dataLength(0)
{
    // save the filename
    if ((returnStatus = _DupFilename(filename)) != OK)
        return;

    // open the HDF file
    if ((_SDfileID = SDstart(filename, DFACC_RDONLY)) == FAIL)
    {
        returnStatus = _status = ERROR_OPENING_FILE;
        return;
    }

    //----------------------------------------------------------
    // since all datasets have the same length, get the length
    // from the 1st dataset
    //----------------------------------------------------------
    int32 datasetID = SDselect(_SDfileID, 0);
    if (datasetID == FAIL)
    {
        returnStatus = _status = ERROR_SELECTING_DATASET;
        return;
    }

    // get the info about the dataset
    char    name[MAX_NC_NAME];
    int32   dimSizes[MAX_VAR_DIMS];
    int32   numDimensions=0;
    int32   dataType=0;
    int32   numAttr=0;
    int hdfStatus = SDgetinfo(datasetID, name, &numDimensions, dimSizes,
                               &dataType, &numAttr);
    if (hdfStatus == FAIL)
    {
        returnStatus = _status = ERROR_GET_DATASET_INFO;
        return;
    }

    // if there is no data, return error
    if ((_dataLength = dimSizes[0]) == 0)
    {
        returnStatus = _status = ERROR_EMPTY_DATASET;
        return;
    }
    _startIndex = 0;
    _endIndex = dimSizes[0] - 1;

    returnStatus = _status;

    return;
}

HdfFile::~HdfFile()
{
    // free memory
    if (_filename)
        free(_filename);

    // close all datasets first (in case user forgets to close them)
    long int* idP=0;
    (void)_datasetIDs.GetHead();
    while ((idP = _datasetIDs.RemoveCurrent()) != 0)
    {
        (void)SDendaccess(*idP);
        delete idP;
    }

    // close the HDF
    if (_SDfileID != FAIL)
        SDend(_SDfileID);

    return;
}

int32                        // dataset id if successful, else HDF_FAIL
HdfFile::SelectDataset(
const char*  datasetName,    // IN
int32&       dataType,       // OUT
int32&       dataStartIndex, // OUT
int32&       dataLength,     // OUT
int32&       numDimensions)  // OUT
{
    assert(datasetName != 0 && _SDfileID != FAIL);

    // map dataset name to dataset index ID
    int32 datasetIndex = SDnametoindex(_SDfileID, (char*)datasetName);
    if (datasetIndex == FAIL)
    {
        _status = ERROR_INVALID_DATASET_NAME;
        return HDF_FAIL;
    }

    // select the dataset
    int32 datasetID = SDselect(_SDfileID, datasetIndex);
    if (datasetID == FAIL)
    {
        _status = ERROR_SELECTING_DATASET;
        return HDF_FAIL;
    }

    // get the info about the dataset
    char    name[MAX_NC_NAME];
    int32   dimSizes[MAX_VAR_DIMS];
    int32   numAttr=0;
    int hdfStatus = SDgetinfo(datasetID, name, &numDimensions, dimSizes,
                               &dataType, &numAttr);
    if (hdfStatus == FAIL)
    {
        _status = ERROR_GET_DATASET_INFO;
        return HDF_FAIL;
    }

    // if there is no data, return error
    if ((dataLength = dimSizes[0]) == 0)
    {
        _status = ERROR_EMPTY_DATASET;
        return HDF_FAIL;
    }
    dataStartIndex = _startIndex;
    if (dataLength > (_endIndex + 1))
    {
        _status = ERROR_UNEVEN_DATASET_LENGTH;
        return HDF_FAIL;
    }

    dataLength = _endIndex - _startIndex + 1;

    // save the dataset ID in the internal list
    long int* idP = new long int(datasetID);
#if 0
printf("new ID = %d\n", *idP);
#endif
    _datasetIDs.Append(idP);

    return datasetID;

} //HdfFile::SelectDataset

int
HdfFile::GetScaleFactor(
int32    sdsId,          // IN: sds ID
float64& factor)         // OUT: factor
{
    float64 calErr, offset, offsetErr;
    int32 dataType;
    intn rc = SDgetcal(sdsId, &factor, &calErr, &offset,
                        &offsetErr, &dataType);
    if (rc == FAIL)
        return HDF_FAIL;
    else
        return HDF_SUCCEED;
 
}//HdfFile::GetScaleFactor
 

int
HdfFile::CloseDataset(
int32      datasetID)
{
    // remove this dataset ID from the internal list
    for (long int* idP=_datasetIDs.GetHead(); idP != 0;
                            idP=_datasetIDs.GetNext())
    {
        if (*idP == datasetID)
        {
            _datasetIDs.RemoveCurrent();
            // close the dataset
            if (SDendaccess(datasetID) == FAIL)
            {
                _status = ERROR_CLOSING_DATASET;
                return HDF_FAIL;
            }
            else
            {
                _status = OK;
                return HDF_SUCCEED;
            }
        }
    }
    // not found
    _status = ERROR_INVALID_DATASET_ID;
    return HDF_FAIL;

} //HdfFile::CloseDataset

int                     // HDF_SUCCEED | HDF_FAIL
HdfFile::GetDatasetData1D(
int32     datasetID,    // IN
int32     start,        // IN
int32     stride,       // IN
int32     dataLength,   // IN
VOIDP     data)         // OUT
{
    assert(data != 0 && start >= 0 && stride >= 1 && dataLength >= 1);

    // make sure data accessing is within the valid range
    if (start < _startIndex)
    {
        _status = ERROR_ACCESS_BEFORE_START;
        return HDF_FAIL;
    }
    if ((start + stride * dataLength - 1) > _endIndex)
    {
        _status = ERROR_ACCESS_AFTER_END;
        return HDF_FAIL;
    }

    int32 sdStart[1], sdStride[1], sdEdge[1];
    sdStart[0] = start,
    sdStride[0] = stride;
    sdEdge[0] = dataLength;

    intn hdfStatus;
    if (stride == 1)
        // more efficient
        hdfStatus = SDreaddata(datasetID, sdStart, NULL, sdEdge, data);
    else
        hdfStatus = SDreaddata(datasetID, sdStart, sdStride, sdEdge, data);

    if (hdfStatus == SUCCEED)
    {
        _status = OK;
        return HDF_SUCCEED;
    }
    else
    {
        _status = ERROR_READING_1D_DATA;
        return HDF_FAIL;
    }
    
} //HdfFile::GetDatasetData1D

int 
HdfFile::GetDatasetDataMD(
int32     datasetID,    // IN
int32*    start,        // IN
int32*    stride,       // IN
int32*    dataLength,   // IN
VOIDP     data)         // OUT
{
    assert(data != 0 && start != 0 && dataLength != 0);

    // make sure data accessing is within the valid range
    if (start[0] < _startIndex)
    {
        _status = ERROR_ACCESS_BEFORE_START;
        return HDF_FAIL;
    }

    int32 tempStride=1;  
    if (stride)
        tempStride = stride[0];

    if ((start[0] + tempStride * dataLength[0] - 1) > _endIndex)
    {
        _status = ERROR_ACCESS_AFTER_END;
        return HDF_FAIL;
    }

    intn hdfStatus = SDreaddata(datasetID, start, stride, dataLength, data);
    
    if (hdfStatus == SUCCEED)
    {
        _status = OK;
        return HDF_SUCCEED;
    }
    else
    {
        _status = ERROR_READING_MD_DATA;
        return HDF_FAIL;
    }
} //HdfFile::GetDatasetData1D


//--------------
// _DupFilename 
//--------------
// duplicate the filename string for a local copy

HdfFile::StatusE
HdfFile::_DupFilename(
const char*     filename)
{
    if ((_filename = strdup(filename)) == NULL)
        _status = ERROR_ALLOCATING_FILENAME;
    return(_status);

}//HdfFile::_DupFilename

#endif
