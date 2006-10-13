//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// Revision 1.9  1999/09/30 23:01:39  sally
// update 9/30/99
//
// 
//    Rev 1.11   07 Sep 1999 13:33:02   sally
//  add interface for Global Attributes
// 
//    Rev 1.10   20 Aug 1999 14:11:48   sally
// ignore bad HDF files and continue processing
// 
//    Rev 1.9   04 Aug 1999 11:07:18   sally
// need to get around HDF's maximum of 32 files
// 
//    Rev 1.8   15 Mar 1999 14:19:00   sally
// add some methods for getting user's start and end indexes
// 
//    Rev 1.7   03 Nov 1998 15:59:08   sally
// adapt to Vdata
// 
//    Rev 1.6   10 Jun 1998 16:24:22   sally
// delete the dataset id after RemoveCurrent()
// 
//    Rev 1.5   01 Jun 1998 10:59:20   sally
// add polynomial table to limit checking
// 
//    Rev 1.4   28 May 1998 13:18:36   daffer
// worked on HdfGlobalAttr and ParseGlobalAttr
// 
//    Rev 1.3   22 May 1998 16:51:28   daffer
// Added ParseGlobalAttr
// 
//    Rev 1.2   08 May 1998 15:18:48   sally
// improved testHdf
// 
//    Rev 1.1   08 May 1998 10:47:24   sally
// added global attribute methods
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

HdfFile::StatusE
HdfFile::OpenFile(void)
{
    if (_SDfileID == FAIL)
    {
        // open the HDF file
        if ((_SDfileID = SDstart(_filename, DFACC_RDONLY)) == FAIL)
            return(_status = ERROR_OPENING_FILE);
    }

    if (_hFileID == FAIL)
    {
        if ((_hFileID = Hopen(_filename, DFACC_READ, 0)) == HDF_FAIL)
            return(_status = ERROR_OPENING_FILE);

        Vstart(_hFileID);
    }

    return(HdfFile::OK);

} // HdfFile::OpenFile

HdfFile::HdfFile(
const char*         filename,
HdfFile::StatusE&   returnStatus)
:   _SDfileID(FAIL), _hFileID(FAIL), _numDatasets(-1),
    _numGlobAttr(-1), _status(HdfFile::OK), _filename(0), _dataLength(0)
{
    // save the filename
    if ((returnStatus = _DupFilename(filename)) != HdfFile::OK)
        return;

    // open the HDF file
    if ((returnStatus = OpenFile()) != HdfFile::OK)
    {
        fprintf(stderr, "Cannot open %s as HDF file\n", filename);
        return;
    }

    // find out info about global attributes and datasets
    if (SDfileinfo(_SDfileID, &_numDatasets, &_numGlobAttr) == FAIL)
    {
        fprintf(stderr, "%s: failed to get file info\n", filename);
        returnStatus = _status = ERROR_GET_FILE_INFO;
        return;
    }

    //----------------------------------------------------------
    // since all datasets have the same length, get the length
    // from the 1st dataset
    //----------------------------------------------------------
    int32 datasetID = SDselect(_SDfileID, 0);
    if (datasetID == FAIL)
    {
        fprintf(stderr, "%s: failed to get dataset info\n", filename);
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
        fprintf(stderr, "%s: failed to get dataset info\n", filename);
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

    _CloseFile();

    returnStatus = _status;

    return;
}

void
HdfFile::_CloseFile(void)
{
    // close the HDF
    if (_SDfileID != FAIL)
    {
        SDend(_SDfileID);
        _SDfileID = FAIL;
    }

    // close the VD HDF
    if (_hFileID != FAIL)
    {
        (void)Vend(_hFileID);
        (void)Hclose(_hFileID);
        _hFileID = FAIL;
    }

}//HdfFile::_CloseFile

HdfFile::~HdfFile()
{
    // free memory
    if (_filename)
        free(_filename);

    // close all datasets first (in case user forgets to close them)
    //long int* idP=0;
    int32* idP=0;
    (void)_datasetIDs.GetHead();
    while ((idP = _datasetIDs.RemoveCurrent()) != 0)
    {
        (void)SDendaccess(*idP);
        delete idP;
    }

    _CloseFile();

    return;
}

HdfFile::StatusE
HdfFile::GetGlobalAttrInfo(
const char*   attrName,      // IN
int32&        attrIndex,     // OUT
DataTypeE&    eaType,        // OUT
int32&        numValues)     // OUT
{
    // open the HDF file if it has not been opened yet
    if (_SDfileID == FAIL)
    {
        if ((_status = OpenFile()) != HdfFile::OK)
            return(_status);
    }
    char globAttrName[MAX_NC_NAME];
    int found=0;
    int32 hdfType=0;
    for (int i=0; i < _numGlobAttr; i++)
    {
        // read each global attribute from the input HDF
        if (SDattrinfo(_SDfileID, i, globAttrName, &hdfType, &numValues)
                        == FAIL)
        {
            return(ERROR_GET_ATTR_INFO);
        }

        // is this the target global attribute name?
        if (strcmp(globAttrName, attrName) != 0)
            continue;

        if ( ! HdfTypeToEaType(hdfType, eaType))
            return(ERROR_BAD_DATATYPE);
        attrIndex = i;
        found = 1;
#ifdef TESTHDF_C
printf("eaType = %d, numValues = %d\n", eaType, numValues);
#endif //TESTHDF_C
        break;
    }

    if (found)
        return(HdfFile::OK);
    else
        return(ERROR_GLOBAL_ATTR_NOT_FOUND);

} //HdfFile::GetGlobalAttrInfo

HdfFile::StatusE
HdfFile::GetGlobalAttr(
int32         attrIndex,     // IN
DataTypeE     eaType,        // IN
VOIDP         attrBuf,       // IN/OUT
int32         numValues)     // IN
{
    // open the HDF file if it has not been opened yet
    if (_SDfileID == FAIL)
    {
        if ((_status = OpenFile()) != HdfFile::OK)
            return(_status);
    }

    if (attrIndex < 0 || attrIndex >= _numGlobAttr)
        return(ERROR_INVALID_GLOBAL_ATTR_INDEX);
    int32 hdfType=0;
    if ( ! EaTypeToHdfType(eaType, hdfType))
        return(ERROR_BAD_DATATYPE);

    int hdfAttrBufSiz = DFKNTsize(hdfType) * numValues;
    VOIDP hdfAttrBuf = (VOIDP) HDmalloc(hdfAttrBufSiz);
    if (hdfAttrBuf == 0)
        return(_status = ERROR_OUT_OF_MEMORY);

    if (SDreadattr(_SDfileID, attrIndex, hdfAttrBuf) == FAIL)
        return(ERROR_GET_ATTR);

    memcpy(attrBuf, hdfAttrBuf, hdfAttrBufSiz);

    HDfree(hdfAttrBuf);

    return(HdfFile::OK);

} //HdfFile::GetGlobalAttr

int32                        // dataset id if successful, else HDF_FAIL
HdfFile::SelectDataset(
const char*  datasetName,    // IN
int32&       dataType,       // OUT
int32&       dataStartIndex, // OUT
int32&       dataLength,     // OUT
int32&       numDimensions)  // OUT
{
    assert(datasetName != 0);
    // open the HDF file if it has not been opened yet
    if (_SDfileID == FAIL)
    {
        if ((_status = OpenFile()) != HdfFile::OK)
            return(_status);
    }

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
    //long int* idP = new long int(datasetID);
    int32* idP = new int32(datasetID);
#if 0
printf("new ID = %d\n", *idP);
#endif
    _datasetIDs.Append(idP);

    return datasetID;

} //HdfFile::SelectDataset

HdfFile::StatusE
HdfFile::OpenParamDatasets(
Parameter*     paramP)
{
    assert(paramP != 0);

    // open the HDF file if it has not been opened yet
    if (_SDfileID == FAIL)
    {
        if ((_status = OpenFile()) != HdfFile::OK)
            return(_status);
    }

    int32 dataType, dataStartIndex, dataLength, numDimensions;
    char tempString[BIG_SIZE];
    (void)strncpy(tempString, paramP->sdsNames, BIG_SIZE);
    char* oneSdsName=0;
    int i=0;
    char* lasts = 0;
    for (oneSdsName = (char*)safe_strtok(tempString, ",", &lasts);
                       oneSdsName;
                       oneSdsName = (char*)safe_strtok(0, ",", &lasts), i++)
    {
        // hold SDS or VD id
        paramP->sdsIDs[i] = HDF_FAIL;

        char vDataName[SHORT_STRING_LEN];
        // look for "v:", open H file, attach VD
        if (sscanf(oneSdsName, "v:%s", vDataName) == 1)
        {
            int32 vDataRefNo = VSfind(_hFileID, vDataName);
            if (vDataRefNo == 0)
                return(_status = ERROR_SELECTING_DATASET);
            paramP->sdsIDs[i] = VSattach(_hFileID, vDataRefNo, "r");
        }
        else
        {
            paramP->sdsIDs[i] = SelectDataset(
                                     oneSdsName, dataType, dataStartIndex,
                                     dataLength, numDimensions);
        }
        if (paramP->sdsIDs[i] == HDF_FAIL)
            return(_status = ERROR_SELECTING_PARAMETER);
    }

    return(_status = HdfFile::OK);

} // HdfFile::OpenParamDatasets

HdfFile::StatusE
HdfFile::CloseParamDatasets(
Parameter*     paramP)
{
    assert(paramP != 0);
    for (int i=0; i < paramP->numSDSs; i++)
    {
        if (paramP->sdsIDs[i] != HDF_FAIL)
        {
            // close it as SDS first, if fails, close it as Vdata
            if (CloseDataset(paramP->sdsIDs[i]) == HDF_FAIL)
            {
                if (VSdetach(paramP->sdsIDs[i]) == HDF_FAIL)
                    return(_status = ERROR_DESELECTING_PARAMETER);
            }
            paramP->sdsIDs[i] = HDF_FAIL;
        }
    }
    return(_status = HdfFile::OK);

} // HdfFile::CloseParamDatasets

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
    //for (long int* idP=_datasetIDs.GetHead(); idP != 0;
    //                        idP=_datasetIDs.GetNext())
    for (int32* idP=_datasetIDs.GetHead(); idP != 0;
                            idP=_datasetIDs.GetNext())
    {
        if (*idP == datasetID)
        {
            //long int* matchidP = _datasetIDs.RemoveCurrent();
            int32* matchidP = _datasetIDs.RemoveCurrent();
            // close the dataset
            if (SDendaccess(datasetID) == FAIL)
            {
                delete matchidP;
                _status = ERROR_CLOSING_DATASET;
                return HDF_FAIL;
            }
            else
            {
                delete matchidP;
                _status = HdfFile::OK;
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
        // close it as SDS first, if fails, close it as Vdata
        hdfStatus = SDreaddata(datasetID, sdStart, NULL, sdEdge, data);
    else
        hdfStatus = SDreaddata(datasetID, sdStart, sdStride, sdEdge, data);

    if (hdfStatus == SUCCEED)
    {
        _status = HdfFile::OK;
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
        _status = HdfFile::OK;
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

// static
int
HdfFile::HdfTypeToEaType(
int32         hdfType,  // IN
DataTypeE&    eaType)   // OUT
{
    switch(hdfType)
    {
        case DFNT_FLOAT32:
            eaType = DATA_FLOAT4;
            break;
        case DFNT_FLOAT64:
            eaType = DATA_FLOAT8;
            break;
        case DFNT_INT8:
        case DFNT_CHAR8:
            eaType = DATA_INT1;
            break;
        case DFNT_UINT8:
        case DFNT_UCHAR8:
            eaType = DATA_UINT1;
            break;
        case DFNT_INT16:
            eaType = DATA_INT2;
            break;
        case DFNT_UINT16:
            eaType = DATA_UINT2;
            break;
        case DFNT_INT32:
            eaType = DATA_INT4;
            break;
        case DFNT_UINT32:
            eaType = DATA_UINT4;
            break;
        default:
            eaType = DATA_UNKNOWN;
            return 0;
            break;
    }
    return 1;

}//HdfFile::HdfTypeToEaType

// static
int
HdfFile::EaTypeToHdfType(
DataTypeE    eaType,   // IN
int32&       hdfType)  // OUT
{
    switch(eaType)
    {
        case DATA_FLOAT4:
            hdfType = DFNT_FLOAT32;
            break;
        case DATA_FLOAT8:
            hdfType = DFNT_FLOAT64;
            break;
        case DATA_INT1:
            hdfType = DFNT_INT8;
            break;
        case DATA_UINT1:
            hdfType = DFNT_UINT8;
            break;
        case DATA_INT2:
            hdfType = DFNT_INT16;
            break;
        case DATA_UINT2:
            hdfType = DFNT_UINT16;
            break;
        case DATA_INT4:
            hdfType = DFNT_INT32;
            break;
        case DATA_UINT4:
            hdfType = DFNT_UINT32;
            break;
        default:
            eaType = DATA_UNKNOWN;
            return 0;
            break;
    }
    return 1;

}//HdfFile::EaTypeToHdfType

HdfFile::StatusE 
HdfFile::ParseGlobalAttr(
char*          attrName,      // IN
VOIDP          attrBuf,       // IN
HdfGlobalAttr* globalAttr)    // IN/OUT
{

    int l;
    int *d;
    char *buf_ptr;
    char ** char_data_ptr=0;
    double *double_data_ptr=0;

    globalAttr->dims[0]=0;
    globalAttr->dims[1]=1;
    d= &globalAttr->dims[0];

    // Get the attribute name 
    globalAttr->name = strdup(attrName);
    
    
    // Now parse the attribute buffer.

    char *lasts=0;
    l=strlen( (char *) attrBuf );
    char * buf= new char[l+1];
    buf_ptr=buf;
    strncpy( buf, (char *) attrBuf, l );
    char *retstring = safe_strtok( buf, "\n", &lasts );
    if (!retstring) 
        return (ERROR_PARSE_GLOBAL_ATTR);
    char *type_string = new char[20];
    sscanf( retstring, "%s", type_string );
    if (strcmp( type_string,"char") == 0) 
        globalAttr->type = DATA_INT1; // 8 bit characters, phah
    else 
        globalAttr->type = DATA_FLOAT8;

    retstring = safe_strtok( 0, "\n", &lasts );
    if (!retstring) 
        return (ERROR_PARSE_GLOBAL_ATTR);

    int ndims=-1, dim1, dim2=1;
    ndims = sscanf( retstring, "%d,%d", &dim1, &dim2 );

    globalAttr->dims[0]=dim1;
    globalAttr->dims[1]=dim2;
    if ( globalAttr->type == DATA_INT1) {
        char_data_ptr = new char *[ dim1*dim2 ];
    } else {
        double_data_ptr = new double [ dim1*dim2 ];
    }
    retstring=safe_strtok( 0, "\n", &lasts );    
    if (!retstring) 
        return (ERROR_PARSE_GLOBAL_ATTR);
    int i=0;
    do {    
        if (globalAttr->type == DATA_FLOAT8 ) {
            *(double_data_ptr+i) = atof(retstring);
        } else {
            l = strlen(retstring);
            // char *p =  new char [ l+1 ];
            char *p = strdup( retstring );
            //            (void) strncpy( p, retstring, l );
            *(char_data_ptr+i) = p;
                
            
        }
        retstring=safe_strtok( 0, "\n", &lasts );    
        i++;
    }  while (retstring);
    
    if (globalAttr->type == DATA_FLOAT8 ) 
        globalAttr->data = (VOIDP) double_data_ptr;
    else
        globalAttr->data = (VOIDP) char_data_ptr;


    delete type_string;
    delete buf_ptr;
    return (HdfFile::OK);

} // ParseGlobalAttr

HdfFile::StatusE 
HdfFile::ParseGlobalAttr(
char*          attrName,      // IN
HdfGlobalAttr& globalAttr)    // IN/OUT
{
    // open the HDF file if it has not been opened yet
    if (_SDfileID == FAIL)
    {
        if ((_status = OpenFile()) != HdfFile::OK)
            return(_status);
    }

    int32 attrIndex=0;
    DataTypeE eaType=DATA_UNKNOWN;
    int32 numValues=0;
    HdfFile::StatusE rc;
    if ((rc = GetGlobalAttrInfo(attrName, attrIndex, eaType, numValues)) !=
                                       HdfFile::OK)
        return rc;

    return(ParseGlobalAttr(attrIndex, globalAttr));

} // HdfFile::ParseGlobalAttr

HdfFile::StatusE 
HdfFile::ParseGlobalAttr(
int32          attrIndex,     // IN
HdfGlobalAttr& globalAttr)    // IN/OUT
{
    // open the HDF file if it has not been opened yet
    if (_SDfileID == FAIL)
    {
        if ((_status = OpenFile()) != HdfFile::OK)
            return(_status);
    }
    char globAttrName[MAX_NC_NAME];
    int32 hdfType=0; 
    int32 numValues=0;
    // read each global attribute from the input HDF
    if (SDattrinfo(_SDfileID, attrIndex, globAttrName,
                                    &hdfType, &numValues) == FAIL)
        return(ERROR_GET_ATTR_INFO);

    if ((globalAttr.name = strdup(globAttrName)) == NULL)
        return(ERROR_OUT_OF_MEMORY);

    if ( ! HdfTypeToEaType(hdfType, globalAttr.type))
        return(ERROR_BAD_DATATYPE);
    
    int attrBufSiz = DFKNTsize(hdfType) * numValues;
    VOIDP attrBuf = (VOIDP) HDmalloc(attrBufSiz);
    if (attrBuf == 0)
        return(ERROR_OUT_OF_MEMORY);

    if (SDreadattr(_SDfileID, attrIndex, attrBuf) == FAIL)
        return(ERROR_GET_ATTR);

    globalAttr.data = attrBuf;

    return(HdfFile::OK);

} // HdfFile::ParseGlobalAttr

// -----------------------
// HdfGlobalAttr Methods
//-------------------------

HdfGlobalAttr::HdfGlobalAttr()
: name(0), data(0)
{
    return;
}

HdfGlobalAttr::~HdfGlobalAttr()
{
    if (name)
    {
        free(name);
        name = 0;
    }

    if (data)
    {
#if 0
        if (type == DATA_FLOAT8)
            HDfree(data);
        else {
            char **p= (char**) data;
            for (int i=0;i<dims[0]*dims[1];i++)
                delete *(p+i);
            HDfree(data);
        }
#endif
        HDfree(data);
        data = 0;
    }
    return;
}


#ifdef TESTHDF_C

main(
int     argc,
char**  argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s HdfFile GlobalAttribute\n", argv[0]);
        exit(0);
    }
    HdfFile::StatusE status = HdfFile::OK;
    HdfFile hdfFile(argv[1], status);
    if (status != HdfFile::OK)
    {
        fprintf(stderr, "HdfFile creation failed\n");
        exit(1);
    }

    int32 attrIndex=0, numValues=0;
    DataTypeE eaType=DATA_UNKNOWN;
    if ((status = hdfFile.GetGlobalAttrInfo(argv[2], attrIndex,
                                eaType, numValues)) != HdfFile::OK)
    {
        fprintf(stderr, "GetGlobalAttrInfo(%s) failed (%d)\n",
                                      argv[2], status);
        exit(1);
    }
    VOIDP attrBuf = (VOIDP) new char[100];
    if ((status = hdfFile.GetGlobalAttr(attrIndex, eaType, attrBuf, numValues))
                                != HdfFile::OK)
    {
        fprintf(stderr, "GetGlobalAttr(%s) failed (%d)\n",
                                      argv[2], status);
        exit(1);
    }

    fprintf(stderr, "%s attribute = [%s]\n", argv[2], (char*)attrBuf);
             
    exit(0);

}//main

#endif // TESTHDF_C
