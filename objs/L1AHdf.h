//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef L1A_HDF_H
#define L1A_HDF_H

static const char rcs_id_HDF_l1a_h[] =
	"@(#) $Id$";

#include "L1A.h"          // SVT's native L1A file
#include "L1AFile.h"      // EA's L1A Hdf TLM file


class Parameter;

struct L1AParamEntry
{
    ParamIdE    paramId;
    UnitIdE     unitId;
    Parameter*  param;
};

static L1AParamEntry l1aParamTable[] =
{
    { FRAME_TIME, UNIT_L1ATIME },
    { TAI_TIME, UNIT_TAI_SECONDS },
    { INSTRUMENT_TIME, UNIT_COUNTS },
    { ORBIT_TIME, UNIT_COUNTS },
    { X_POS, UNIT_KILOMETERS },
    { Y_POS, UNIT_KILOMETERS },
    { Z_POS, UNIT_KILOMETERS },
    { X_VEL, UNIT_KMPS },
    { Y_VEL, UNIT_KMPS },
    { Z_VEL, UNIT_KMPS },
    { ROLL, UNIT_RADIANS },
    { PITCH, UNIT_RADIANS },
    { YAW, UNIT_RADIANS },
    { TRUE_CAL_PULSE_POS, UNIT_DN },
    { FRAME_INST_STATUS, UNIT_DN },
    { FRAME_ERR_STATUS, UNIT_DN },
    { FRAME_QUALITY_FLAG, UNIT_DN },
    { PULSE_QUALITY_FLAG, UNIT_HEX_BYTES },
    { LOOP_BACK_CAL_A_POWER, UNIT_DN },
    { LOOP_BACK_CAL_NOISE, UNIT_DN },
    { LOAD_CAL_NOISE, UNIT_DN },
    { POWER_DN, UNIT_DN },
    { NOISE_DN, UNIT_DN },
    { ANTENNA_POS, UNIT_DN },
    { PRF_COUNT, UNIT_COUNTS },
    { PRF_CYCLE_TIME, UNIT_DN },
    { RANGE_GATE_A_DELAY, UNIT_DN },
    { RANGE_GATE_A_WIDTH, UNIT_DN },
    { RANGE_GATE_B_DELAY, UNIT_DN },
    { RANGE_GATE_B_WIDTH, UNIT_DN },
    { PULSE_WIDTH, UNIT_DN },
    { PREDICT_ANT_POS_COUNT, UNIT_COUNTS },
    { DOPPLER_ORBIT_STEP, UNIT_COUNTS },
    { PRF_ORBIT_STEP_CHANGE, UNIT_COUNTS },
    { ADEOS_TIME, UNIT_TICKS },
    { CORRES_INSTR_TIME, UNIT_TICKS },
    { PRECISION_COUPLER_TEMP, UNIT_DN },
    { RCV_PROTECT_SW_TEMP, UNIT_DN },
    { BEAM_SELECT_SW_TEMP, UNIT_DN },
    { RECEIVER_TEMP, UNIT_DN },
    { PRF_CYCLE_TIME_EU, UNIT_SECONDS },
    { RANGE_GATE_DELAY_INNER, UNIT_SECONDS },
    { RANGE_GATE_DELAY_OUTER, UNIT_SECONDS },
    { RANGE_GATE_WIDTH_INNER, UNIT_SECONDS },
    { RANGE_GATE_WIDTH_OUTER, UNIT_SECONDS },
    { TRANSMIT_PULSE_WIDTH, UNIT_SECONDS },
    { TRANSMIT_POWER_INNER, UNIT_DBM },
    { TRANSMIT_POWER_OUTER, UNIT_DBM },
    { PRECISION_COUPLER_TEMP_EU, UNIT_DEGREES_C },
    { RCV_PROTECT_SW_TEMP_EU, UNIT_DEGREES_C },
    { BEAM_SELECT_SW_TEMP_EU, UNIT_DEGREES_C },
    { RECEIVER_TEMP_EU, UNIT_DEGREES_C },
};

const int l1aParamTableSize = ElementNumber(l1aParamTable);

//======================================================================
// CLASS
//		L1AHdf
//
// DESCRIPTION
//		The L1AHdf object allows for the easy writing, reading, and
//		manipulating of Level 1A HDF data.
//
//      input file:  HDF L1A file
//      output file: SVT L1A file
//======================================================================

class L1AHdf : public L1AFile, public L1A
{
public:

	//--------------//
	// construction //
	//--------------//

    L1AHdf(const char*         filename,                 // IN
           HdfFile::StatusE&   returnStatus,             // OUT
           const Itime         startTime = INVALID_TIME, // IN: BOF if invalid
           const Itime         endTime = INVALID_TIME);  // IN: EOF if invalid

	virtual ~L1AHdf();

    Parameter*  GetParameter(ParamIdE, UnitIdE);

	//--------------//
	// input/output //
	//--------------//

	int		ReadL1AHdfDataRec(void);        // get data at next index
	int		ReadL1AHdfDataRec(int32 index); // get data at specific index

    int     WriteDataRec(void);

protected:
};

#endif
