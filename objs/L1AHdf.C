//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l1ahdf_c[] =
	"@(#) $Id$";

#include <assert.h>
#include <memory.h>

#include "L1AHdf.h"
#include "ParTab.h"


//========//
// L1AHdf //
//========//

L1AHdf::L1AHdf(
const char*               filename,                 // IN
HdfFile::StatusE&         returnStatus,             // OUT
const Itime               startTime,
const Itime               endTime)
: L1AFile(filename, returnStatus, startTime, endTime), L1A()
{
    // make sure all parent classes are instanciated ok
    if (HdfFile::_status != HdfFile::OK)
        return;

    // allocate space for 2 beams, 50 cycles per beam, 12 slices per spot
    L1A::frame.Allocate(2, 50, 12);

    for (int i=0; i < l1aParamTableSize; i++)
    {
        Parameter* param = l1aParamTable[i].param;
        param = ParTabAccess::GetParameter(SOURCE_L1A, 
                          l1aParamTable[i].paramId, l1aParamTable[i].unitId);
        if (param == 0)
        {
            returnStatus = HdfFile::_status = ERROR_INVALID_DATASET_NAME;
            return;
        }

        if (OpenParamDatasets(param) != HdfFile::OK)
        {
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
        l1aParamTable[i].param = param;
    }

    if (L1A::AllocateBuffer() == 0)
    {
        returnStatus = HdfFile::_status = ERROR_OUT_OF_MEMORY;
        return;
    }
    returnStatus = HdfFile::_status = HdfFile::OK;
    return;

} // L1AHdf::L1AHdf

L1AHdf::~L1AHdf()
{
    Parameter* param=0;
    for (int i=0; i < l1aParamTableSize; i++)
    {
        param = l1aParamTable[i].param;
        if (param != 0)
        {
            // ignore the closing errors
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

} // L1AHdf::~L1AHdf

Parameter*
L1AHdf::GetParameter(
ParamIdE      paramId,
UnitIdE       unitId)
{
    for (int i=0; i < l1aParamTableSize; i++)
    {
        if (l1aParamTable[i].paramId == paramId &&
                    l1aParamTable[i].unitId == unitId)
            return(l1aParamTable[i].param);
    }
    return 0;

} // L1AHdf::GetParameter

int
L1AHdf::ReadL1AHdfDataRec(void)
{
    // get next index if more data
    int32 index;
    if (GetNextIndex(index) != HdfFile::OK)
        return 0;

    return(ReadL1AHdfDataRec(index));

} // L1AHdf::ReadL1AHdfDataRec

int
L1AHdf::ReadL1AHdfDataRec(
int32   index)   // next HDF data index
{
    Parameter* param = 0;
    for (int i=0; i < l1aParamTableSize; i++)
    {
        param = l1aParamTable[i].param;
        if (param == 0)
        {
            fprintf(stderr, "NULL parameter: id = %d, unit = %d\n",
                           l1aParamTable[i].paramId, l1aParamTable[i].unitId);
            return 0;
        }
        int rc = param->extractFunc(this, param->sdsIDs, index,
                               1, 1, param->data, 0);
        if (rc <= 0)
        {
            fprintf(stderr, "Error occured when extracting L1A: %s\n",
                               param->paramName);
            return 0;
        }
    }

    return(frame.UnpackHdf(this));

} // L1AHdf::ReadL1AHdfDataRec

int
L1AHdf::WriteDataRec(void)
{
    frame.Pack(buffer);
    return(Write(buffer, bufferSize));
}
