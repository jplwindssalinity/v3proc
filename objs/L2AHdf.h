//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef HDF_L2A_H
#define HDF_L2A_H

static const char rcs_id_HDF_l2a_h[] =
	"@(#) $Id$";

#include "L2A.h"
#include "NoTimeTlmFile.h"

#ifndef MAX_L2AHDF_NUM_CELLS
#define MAX_L2AHDF_NUM_CELLS   3240
#endif

#ifndef MAX_L2AxHDF_NUM_CELLS
#define MAX_L2AxHDF_NUM_CELLS   810
#endif

#ifndef MAX_L2AHDF_CELL_NO
#define MIN_L2AHDF_CELL_NO     1
#define MAX_L2AHDF_CELL_NO     76
#endif

#ifndef MAX_L2AHDF_ROW_NO
#define MIN_L2AHDF_ROW_NO     1
#define MAX_L2AHDF_ROW_NO     1624
#endif


class Parameter;

struct L2AParamEntry
{
    ParamIdE    paramId;
    UnitIdE     unitId;
    Parameter*  param;
};

static L2AParamEntry l2aMeasTable[] =
{
    { WVC_ROW_TIME, UNIT_SECONDS },
    { ROW_NUMBER, UNIT_DN },
    { NUM_SIGMA0, UNIT_DN },
    { CELL_LAT, UNIT_RADIANS },
    { CELL_LON, UNIT_RADIANS },
    { CELL_AZIMUTH, UNIT_DEGREES },
    { CELL_INCIDENCE, UNIT_RADIANS },
    { SIGMA0, UNIT_DB },
    { SIGMA0_ATTN_AMSR, UNIT_DB },
    { SIGMA0_ATTN_MAP, UNIT_DB },
    { KP_ALPHA, UNIT_DN },
    { KP_BETA, UNIT_DN },
    { KP_GAMMA, UNIT_DN },
    { SIGMA0_QUAL_FLAG, UNIT_DN },
    { SIGMA0_MODE_FLAG, UNIT_DN },
    { SURFACE_FLAG, UNIT_DN },
    { CELL_INDEX, UNIT_DN }
};

const int l2aMeasTableSize = ElementNumber(l2aMeasTable);

//======================================================================
// CLASS
//		L2AHdf
//
// DESCRIPTION
//		The L2AHdf object allows for the easy writing, reading, and
//		manipulating of Level 2A HDF data.
//
//      input file:  HDF L2A file
//      output file: SVT L2A file
//======================================================================

class L2AHdf : public NoTimeTlmFile, public L2A
{
public:

	//--------------//
	// construction //
	//--------------//

    L2AHdf(const char*         filename,        // IN
           SourceIdE           sourceId,        // IN
           HdfFile::StatusE&   returnStatus);   // OUT

	virtual ~L2AHdf();

    Parameter*  ExtractParameter(ParamIdE, UnitIdE, int32 index);

	//--------------//
	// input/output //
	//--------------//

           // return 1 if OK, 0 if no data, -1 if error or EOF
    int    ReadL2AHdfCell(void);        // get data at next index
    int    ConvertRow();        // get data for next along track index
                                // and write to file
    int    currentRowNo;
    int    currentCellNo;

    int    numCells;       // 3240 or 810
};

#endif

