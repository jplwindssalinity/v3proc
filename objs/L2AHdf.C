//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l2a_c[] =
	"@(#) $Id$";

#include <assert.h>
#include <memory.h>

#include "L2AHdf.h"
#include "ParTab.h"


//========//
// L2AHdf //
//========//

L2AHdf::L2AHdf(
const char*               filename,                 // IN
HdfFile::StatusE&         returnStatus)             // OUT
: NoTimeTlmFile(filename, returnStatus), L2A(),
  currentRowNo(1), currentCellNo(0)
{
    // make sure all parent classes are instanciated ok
    if (HdfFile::_status != HdfFile::OK)
        return;

    Parameter* param=0;
    for (int i=0; i < l2aMeasTableSize; i++)
    {
        Parameter* param = l2aMeasTable[i].param;
        param = ParTabAccess::GetParameter(SOURCE_L2A, 
                          l2aMeasTable[i].paramId, l2aMeasTable[i].unitId);
        if (param == 0)
        {
            returnStatus = HdfFile::_status = ERROR_INVALID_DATASET_NAME;
            return;
        }

        if (OpenParamDatasets(param) != HdfFile::OK)
        {
            fprintf(stderr, "Select dataset %s failed\n", param->sdsNames);
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
        l2aMeasTable[i].param = param;
    }

    header.crossTrackResolution = header.alongTrackResolution = 25;
    header.crossTrackBins = 76;
    header.alongTrackBins = 1624;
    header.zeroIndex = 38;

    // use wvc_row_time for startTime
    int numRowOne = 0;
    Itime totalRowOneTime(0, 0);
    for (int i=0; i < _dataLength; i++)
    {
        // get row 1 only
        param = ExtractParameter(ROW_NUMBER, UNIT_DN, i);
        assert(param != 0);
        short* shortP = (short*) (param->data);
        if (*shortP != 1)
            continue;

        param = ExtractParameter(WVC_ROW_TIME, UNIT_SECONDS, i);
        assert(param != 0);
        Itime eaItime;
        (void) memcpy(&eaItime, param->data, 6);
        totalRowOneTime += eaItime;
        numRowOne++;
    }
    if (numRowOne > 0)
    {
        Itime rowOneTime = totalRowOneTime / numRowOne;
        header.startTime = (double) (rowOneTime.sec);
    }

    returnStatus = HdfFile::_status = HdfFile::OK;
    return;

} // L2AHdf::L2AHdf

L2AHdf::~L2AHdf()
{
    Parameter* param=0;
    for (int i=0; i < l2aMeasTableSize; i++)
    {
        param = l2aMeasTable[i].param;
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

} // L2AHdf::~L2AHdf

Parameter*
L2AHdf::ExtractParameter(
ParamIdE      paramId,
UnitIdE       unitId,
int32         index)
{
    Parameter* param=0;
    for (int i=0; i < l2aMeasTableSize; i++)
    {
        if (l2aMeasTable[i].paramId == paramId &&
                    l2aMeasTable[i].unitId == unitId)
        {
            param = l2aMeasTable[i].param;
            if (param == 0)
            {
                fprintf(stderr, "NULL parameter: id = %d, unit = %d\n",
                           l2aMeasTable[i].paramId, l2aMeasTable[i].unitId);
                return 0;
            }
            int rc = param->extractFunc(this, param->sdsIDs, index,
                               1, 1, param->data, 0);
            if (rc <= 0)
            {
                fprintf(stderr, "Error occured when extracting L2A: %s\n",
                                           param->paramName);
                return 0;
            }
        }
    }
    return param;

} // L2AHdf::ExtractParameter

int
L2AHdf::ReadL2AHdfCell(void)
{
    if (currentRowNo > MAX_L2AHDF_ROW_NO)
    {
        HdfFile::_status = HdfFile::NO_MORE_DATA;
        return(-1);
    }

    // roll to next cell (or next row)
    if (++currentCellNo > MAX_L2AHDF_CELL_NO)
    {
        currentCellNo = MIN_L2AHDF_ROW_NO;
        if (++currentRowNo > MAX_L2AHDF_ROW_NO)
        {
            HdfFile::_status = HdfFile::NO_MORE_DATA;
            return(-1);
        }
    }

    if (frame.measList.ReadL2AHdfCell(this, currentRowNo, currentCellNo) == 0)
    {
        HdfFile::_status = HdfFile::ERROR_EXTRACT_DATA;
        return(-1);
    }

    // if the list is empty, then no data for this row and this cell index
    if (frame.measList.IsEmpty())
    {
        HdfFile::_status = HdfFile::OK;
        return(0);
    }
    frame.ati = currentRowNo - 1;
    frame.cti = currentCellNo;
    HdfFile::_status = HdfFile::OK;
    return 1;

} // L2AHdf::ReadL2AHdfDataRec
