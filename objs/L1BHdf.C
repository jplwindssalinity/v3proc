//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l1b_c[] =
	"@(#) $Id$";

#include <assert.h>
#include <memory.h>

#include "L1BHdf.h"
#include "ParTab.h"


//========//
// L1BHdf //
//========//

L1BHdf::L1BHdf(
const char*               filename,                 // IN
HdfFile::StatusE&         returnStatus,             // OUT
const Itime               startTime,
const Itime               endTime)
: L1BHdfFile(filename, returnStatus, startTime, endTime), L1B()
{
    // make sure all parent classes are instanciated ok
    if (HdfFile::_status != HdfFile::OK)
        return;

    for (int i=0; i < l1bMeasTableSize; i++)
    {
        Parameter* param = l1bMeasTable[i].param;
        param = ParTabAccess::GetParameter(SOURCE_L1B, 
                          l1bMeasTable[i].paramId, l1bMeasTable[i].unitId);
        if (param == 0)
        {
            returnStatus = HdfFile::_status = ERROR_INVALID_DATASET_NAME;
            return;
        }

        // get the first data set name string
        if (param->sdsIDs == 0)
        {
            returnStatus = HdfFile::_status = ERROR_INVALID_DATASET_NAME;
            return;
        }
        char tempString[BIG_SIZE];
        (void)strncpy(tempString, param->sdsNames, BIG_SIZE);
        char *oneSdsName= (char*)strtok(tempString, ",");
        if (oneSdsName == 0)
        {
            returnStatus = HdfFile::_status = ERROR_INVALID_DATASET_NAME;
            return;
        }
        int32 dataType=0, startIndex=0, dataLength=0, numDimensions=0;
        param->sdsIDs[0] = SelectDataset( oneSdsName,  dataType,
                                 startIndex, dataLength, numDimensions);
        if (param->sdsIDs[0] == HDF_FAIL)
        {
            fprintf(stderr, "Select dataset %s failed\n", oneSdsName);
            returnStatus = HdfFile::_status = ERROR_SELECTING_DATASET;
            return;
        }
        param->data = (char *)malloc(param->byteSize);
        if (param->data == 0)
        {
            fprintf(stderr, "Out of Memory\n");
            returnStatus = HdfFile::_status = ERROR_OUT_OF_MEMORY;
            return;
        }
        l1bMeasTable[i].param = param;
    }
    returnStatus = HdfFile::_status = HdfFile::OK;
    return;

} // L1BHdf::L1BHdf

L1BHdf::~L1BHdf()
{
    Parameter* param=0;
    for (int i=0; i < l1bMeasTableSize; i++)
    {
        param = l1bMeasTable[i].param;
        if (param != 0)
        {
            (void) CloseParamDatasets(param);
            if (param->data != 0)
            {
                free((void*) param->data);
                param->data = 0;
            }
            delete param;
            param = 0;
        }
    }
	return;

} // L1BHdf::~L1BHdf

Parameter*
L1BHdf::GetParameter(
ParamIdE      paramId,
UnitIdE       unitId)
{
    for (int i=0; i < l1bMeasTableSize; i++)
    {
        if (l1bMeasTable[i].paramId == paramId &&
                    l1bMeasTable[i].unitId == unitId)
            return(l1bMeasTable[i].param);
    }
    return 0;

} // L1BHdf::GetParameter

int
L1BHdf::ReadL1BHdfDataRec(void)
{
    // get next index if more data
    int32 index;
    if (GetNextIndex(index) != HdfFile::OK)
        return 0;

    return(ReadL1BHdfDataRec(index));

} // L1BHdf::ReadL1BHdfDataRec

int
L1BHdf::ReadL1BHdfDataRec(
int32   index)   // next HDF data index
{
    Parameter* param = 0;
    for (int i=0; i < l1bMeasTableSize; i++)
    {
        param = l1bMeasTable[i].param;
        if (param == 0)
        {
            fprintf(stderr, "NULL parameter: id = %d, unit = %d\n",
                           l1bMeasTable[i].paramId, l1bMeasTable[i].unitId);
            return 0;
        }
        int rc = param->extractFunc(this, param->sdsIDs, index,
                               1, 1, param->data, 0);
        if (rc <= 0)
        {
            fprintf(stderr, "Error occured when extracting L1B: %s\n",
                               param->paramName);
            return 0;
        }
    }

    return(frame.spotList.UnpackL1BHdf(this, index));

} // L1BHdf::ReadL1BHdfDataRec
