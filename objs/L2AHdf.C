//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l2a_c[] =
	"@(#) $Id$";

#include <assert.h>
#include <memory.h>
#include <math.h>

#include "L2AHdf.h"
#include "ParTab.h"
#include "Constants.h"


//========//
// L2AHdf //
//========//

L2AHdf::L2AHdf(
const char*               filename,       // IN
SourceIdE                 sourceId,       // IN
HdfFile::StatusE&         returnStatus)   // OUT
: NoTimeTlmFile(filename, sourceId, returnStatus), L2A(),
  currentRowNo(1), currentCellNo(0)
{
    // make sure all parent classes are instanciated ok
    if (HdfFile::_status != HdfFile::OK)
        return;

    Parameter* param=0;
    for (int i=0; i < l2aMeasTableSize; i++)
    {
        Parameter* param = l2aMeasTable[i].param;
        param = ParTabAccess::GetParameter(_sourceType, 
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

    if (sourceId == SOURCE_L2Ahr){
      header.crossTrackResolution = header.alongTrackResolution = 12.5;
      header.crossTrackBins = 152;
      header.alongTrackBins = 3248;
      header.zeroIndex = 76;
    }
    else{
      header.crossTrackResolution = header.alongTrackResolution = 25;
      header.crossTrackBins = 76;
      header.alongTrackBins = 1624;
      header.zeroIndex = 38;
    }
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

    if (sourceId == SOURCE_L2A)
        numCells = MAX_L2AHDF_NUM_CELLS;
    else if (sourceId == SOURCE_L2Ax)
        numCells = MAX_L2AxHDF_NUM_CELLS;
    else
        numCells = MAX_L2AhrHDF_NUM_CELLS;

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
L2AHdf::ConvertRow(){

  /************** Check for output file   ****/
    if (_outputFp == NULL) return(0);

  /**** write header if necessary ***/
    if (! _headerWritten)
    {
        if (! header.Write(_outputFp))
            return(0);
        _headerWritten = 1;
    }

    int alongTrackBins=header.alongTrackBins;
    int crossTrackBins=header.crossTrackBins;
    /************* Check for completion ********/
    if (currentRowNo > alongTrackBins)
    {
        HdfFile::_status = HdfFile::NO_MORE_DATA;
        return(-1);
    }

    /*************************************************/
    /****** Allocate Space for Measurement Lists *****/
    /*************************************************/

    MeasList meas_list_row[crossTrackBins];
    

    /**************************************************/
    /**** Extract Information from  HDF file          */
    /**************************************************/

    Parameter* param = 0;
    for (int i=0; i < GetDataLength(); i++)
    {
        // extract only if row_number is the target one

        param = ExtractParameter(ROW_NUMBER, UNIT_DN, i);
        assert(param != 0);
        short* row = (short*) (param->data);
        if (*row != (short)currentRowNo)
            continue;

        param = ExtractParameter(NUM_SIGMA0, UNIT_DN, i);
        assert(param != 0);
        short* num_measP = (short*) (param->data);
        int num_meas=(int)*num_measP;

        // extract all the required datasets 

        param = ExtractParameter(CELL_INDEX, UNIT_DN, i);
        assert(param != 0);
        unsigned char*  cell_index = (unsigned char*)param->data;
        
        param = ExtractParameter(SIGMA0_MODE_FLAG, UNIT_DN, i);
        assert(param != 0);
        unsigned short* ModeFlags = (unsigned short*) (param->data);

        param = ExtractParameter(SIGMA0_QUAL_FLAG, UNIT_DN, i);
        assert(param != 0);
        unsigned short* QualFlags = (unsigned short*) (param->data);

	param = ExtractParameter(SIGMA0, UNIT_DB, i);
	assert(param != 0);
	float* sigma0db = (float*)param->data;

	param = ExtractParameter(SURFACE_FLAG, UNIT_DN, i);
	assert(param != 0);
        unsigned short* surface_flags= (unsigned short*)param->data;
   
	param = ExtractParameter(CELL_LON, UNIT_RADIANS, i);
	assert(param != 0);
	float* cell_lon= (float*)param->data;

	param = ExtractParameter(CELL_LAT, UNIT_RADIANS, i);
	assert(param != 0);
	float* cell_lat= (float*)param->data;

	param = ExtractParameter(CELL_AZIMUTH, UNIT_DEGREES, i);
	assert(param != 0);
	float* northazimuth_degrees= (float*)param->data;
 
	param = ExtractParameter(CELL_INCIDENCE, UNIT_RADIANS, i);
	assert(param != 0);
	float* incidence_angle = (float*)param->data;

	param = ExtractParameter(KP_ALPHA, UNIT_DN, i);
	assert(param != 0);
        float* A = (float*)param->data;

	param = ExtractParameter(KP_BETA, UNIT_DN, i);
	assert(param != 0);
	float* B= (float*)param->data;
	param = ExtractParameter(KP_GAMMA, UNIT_DN, i);
	assert(param != 0);
	float* C = (float*)param->data;

        param = ExtractParameter(SIGMA0_ATTN_MAP, UNIT_DB, i);
        assert(param !=0);
        float* attn= (float*)param->data;
        
        /**********************************************************/
        /* Loop through arrays constructing new measurements      */
        /**********************************************************/
	for (int j=0; j < num_meas; j++){
          // skip non wind observation mode stuff
	  if ((ModeFlags[j] & 0x0003) != 0)
                continue;

          // skip bad measurements
	  if (QualFlags[j] & 0x0001)
	      continue;

          // create new meas
          Meas* new_meas= new Meas;

          //------------------------------------------------//
          // construct measurement from L2AHDF arrays       //
          //------------------------------------------------//

          // incidenceAngle
          new_meas->incidenceAngle=incidence_angle[j];

          // value
          new_meas->value=sigma0db[j]+attn[j]/(cos(new_meas->incidenceAngle));
          new_meas->value=(float) pow( 10.0, (double)new_meas->value/10.0);
	  if ( QualFlags[j] & 0x0004 )
	    new_meas->value = -(new_meas->value); 
    
	  // ignore bandwidth and tx pulse width   

          // landflag
          new_meas->landFlag=(int)(surface_flags[j]  &  0x00000003);

	  // outline: leave it alone
          
          // centroid
          new_meas->centroid.SetAltLonGDLat(0.0, cell_lon[j], cell_lat[j]);
         
          // beamIdx
	  new_meas->beamIdx = (int) (ModeFlags[j] & 0x0004);
          new_meas->beamIdx >>= 2;

          // measType
	  if (new_meas->beamIdx == 0)  // beam A => H, B => V
	    new_meas->measType = Meas::HH_MEAS_TYPE;
	  else
	    new_meas->measType = Meas::VV_MEAS_TYPE;
 
          // eastAzimuth
	  new_meas->eastAzimuth = (450.0 - northazimuth_degrees[j]) * dtr;
	  if (new_meas->eastAzimuth >= two_pi) new_meas->eastAzimuth -= two_pi;


          // numSlices
          new_meas->numSlices=-1;
       
          // A
          new_meas->A=A[j];

	  // B
          new_meas->B=B[j];

	  // C
          new_meas->C=C[j];
          
          // append new measurement to appropriate list
          if (meas_list_row[cell_index[j]-1].Append(new_meas) == 0)
	    return(0);
	}
    }
    /**************************************************/
    /****** Write Out Measurement Lists      **********/
    /**************************************************/
    int ati = currentRowNo - 1;
    int rev=0;
    for(int cti=1;cti<=crossTrackBins;cti++){
      char ctichar=(unsigned char) cti-1;
      if(meas_list_row[cti-1].NodeCount()==0) continue;
      if (fwrite((void *)&rev, sizeof(unsigned int), 1, _outputFp) != 1 ||
        fwrite((void *)&ati, sizeof(int), 1, _outputFp) != 1 ||
        fwrite((void *)&ctichar, sizeof(unsigned char), 1, _outputFp) != 1 ||
        meas_list_row[cti-1].Write(_outputFp) != 1)
	{
	  return(0);
	}            
    }
    currentRowNo++;
    return(1);
}

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




