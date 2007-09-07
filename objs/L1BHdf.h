//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef HDF_L1B_H
#define HDF_L1B_H

static const char rcs_id_HDF_l1b_h[] =
	"@(#) $Id$";

#include "L1B.h"
#include "L1BHdfFile.h"
#include "LandMap.h"

#ifndef MAX_L1BHDF_NUM_PULSES
#define MAX_L1BHDF_NUM_PULSES   100
#endif

#ifndef MAX_L1BHDF_NUM_SLICES
#define MAX_L1BHDF_NUM_SLICES   8
#endif

class Parameter;

struct L1BParamEntry
{
    ParamIdE    paramId;
    UnitIdE     unitId;
    Parameter*  param;
};

static L1BParamEntry l1bMeasTable[] =
{
    { ORBIT_TIME, UNIT_COUNTS },
    { NUM_PULSES, UNIT_DN },
    { SC_LAT, UNIT_RADIANS },
    { SC_LON, UNIT_RADIANS },
    { SC_ALT, UNIT_KILOMETERS },
    { X_POS, UNIT_KILOMETERS },
    { Y_POS, UNIT_KILOMETERS },
    { Z_POS, UNIT_KILOMETERS },
    { X_VEL, UNIT_KMPS },
    { Y_VEL, UNIT_KMPS },
    { Z_VEL, UNIT_KMPS },
    { ROLL, UNIT_RADIANS },
    { PITCH, UNIT_RADIANS },
    { YAW, UNIT_RADIANS },
    { BANDWIDTH_RATIO, UNIT_DB },
    { X_CAL_A, UNIT_DB },
    { X_CAL_B, UNIT_DB },
    { CELL_LAT, UNIT_RADIANS },
    { CELL_LON, UNIT_RADIANS },
    { SIGMA0, UNIT_DB },
    { FREQUENCY_SHIFT, UNIT_HZ },
    { CELL_AZIMUTH, UNIT_RADIANS },
    { CELL_INCIDENCE, UNIT_RADIANS },
    { ANTENNA_AZIMUTH, UNIT_RADIANS },
    { SNR, UNIT_DB },
    { KPC_A, UNIT_DN },
    { SLICE_LAT, UNIT_RADIANS },
    { SLICE_LON, UNIT_RADIANS },
    { SLICE_SIGMA0, UNIT_DB },
    { X_FACTOR, UNIT_DB },
    { SLICE_AZIMUTH, UNIT_RADIANS },
    { SLICE_INCIDENCE, UNIT_RADIANS },
    { SLICE_SNR, UNIT_DB },
    { SLICE_KPC_A, UNIT_DN },
    { FRAME_INST_STATUS_02, UNIT_MAP },
    { FRAME_INST_STATUS_02, UNIT_MAP },
    { SLICE_QUAL_FLAG, UNIT_DN },
    { SIGMA0_MODE_FLAG, UNIT_DN },
};

const int l1bMeasTableSize = ElementNumber(l1bMeasTable);

//======================================================================
// CLASS
//		L1BHdf
//
// DESCRIPTION
//		The L1BHdf object allows for the easy writing, reading, and
//		manipulating of Level 1B HDF data.
//
//      input file:  HDF L1B file
//      output file: SVT L1B file
//======================================================================

class L1BHdf : public L1BHdfFile, public L1B
{
public:

	//--------------//
	// construction //
	//--------------//

    L1BHdf(const char*         filename,                 // IN
           HdfFile::StatusE&   returnStatus,             // OUT
           const Itime         startTime = INVALID_TIME, // IN: BOF if invalid
           const Itime         endTime = INVALID_TIME);  // IN: EOF if invalid

	virtual ~L1BHdf();

    Parameter*  GetParameter(ParamIdE, UnitIdE);

	//--------------//
	// input/output //
	//--------------//

	int		ReadL1BHdfDataRec(void);        // get data at next index
	int		ReadL1BHdfDataRec(int32 index); // get data at specific index
	int		ReadL1BHdfDataRecCoastal(CoastalMaps* lmap, Antenna* ant);        // get data at next index read data necessary for accurate coastal processing
	int		ReadL1BHdfDataRecCoastal(int32 index, CoastalMaps* lmap, Antenna* ant); // get data at specific index compute quantities needed for coastal processing

    float   configBandwidth;                // band width from config file
    float   configTxPulseWidth;             // tx pulse width from config file

protected:
};

#endif
