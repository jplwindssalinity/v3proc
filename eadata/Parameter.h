//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.11   20 Apr 1998 10:22:50   sally
// change for WindSwatch
// 
//    Rev 1.10   17 Apr 1998 16:51:14   sally
// add L2A and L2B file formats
// 
//    Rev 1.9   06 Apr 1998 16:29:00   sally
// merged with SVT
// 
//    Rev 1.8   30 Mar 1998 15:14:06   sally
// added L2A parameter table
// 
//    Rev 1.7   27 Mar 1998 10:00:02   sally
// added L1A Derived data
// 
//    Rev 1.6   23 Mar 1998 15:39:48   sally
// adapt to derived science data
// 
//    Rev 1.5   19 Mar 1998 13:37:18   sally
//  added "days", "hours", "minutes" and "seconds" units
// 
//    Rev 1.4   04 Mar 1998 14:31:26   sally
// change range dialog
// 
//    Rev 1.3   23 Feb 1998 10:28:20   sally
// add limit checker
// 
//    Rev 1.2   20 Feb 1998 10:59:16   sally
// L1 to L1A
// 
//    Rev 1.1   17 Feb 1998 14:47:14   sally
//  NOPM
// 
//    Rev 1.0   04 Feb 1998 14:16:40   daffer
// Initial checking
// Revision 1.3  1998/02/03 00:12:33  sally
// change NRT to L1AP
//
// Revision 1.2  1998/01/30 22:28:35  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef PARAMETER_H
#define PARAMETER_H

static const char rcs_id_parameter_h[] =
    "@(#) $Header$";

//-********************************************************************
//     defines all types concerning HK2 and L1 parameters.
//-********************************************************************

#include <stdio.h>

#include <mfhdf.h>

#include "CommonDefs.h"

class TlmHdfFile;

typedef int (*ExtractFunc) (TlmHdfFile*, int32*, int32, int32, int32, VOIDP);
typedef void (*PrintFunc) (FILE* fp, char* data);

#define PARAM_NAME_LEN  128
#define MEASURABLE_LEN  64
#define UNIT_NAME_LEN   128
#define SDS_NAME_LEN    128

enum
{
    BEAM_1V = 1,
    BEAM_2H = 3,
    BEAM_2V = 7,
    BEAM_3V = 0,
    BEAM_4V = 5,
    BEAM_5V = 2,
    BEAM_5H = 6,
    BEAM_6V = 4
};

//-----------------------
// telemetry definitions 
//-----------------------

enum { TLM_HVPS_ON = 0, TLM_HVPS_OFF, HVPS_STATE_COUNT };
enum { TLM_MODE_WOM = 0, TLM_MODE_CBM, TLM_MODE_SBM,
        TLM_MODE_ROM, NSCAT_MODE_COUNT, EXT_MODE_RHM = NSCAT_MODE_COUNT,
        EXT_MODE_COUNT };
enum { TLM_FRAME_SCI = 0, TLM_FRAME_CAL, FRAME_TYPE_COUNT };
enum { TLM_DSS_A = 0, TLM_DSS_B, DSS_COUNT };
enum { TLM_TWTA_1 = 0, TLM_TWTA_2, TWTA_COUNT };
enum { TLM_TWTA_TRIP_OVERRIDE_DIS = 0, TLM_TWTA_TRIP_OVERRIDE_EN,
    TLM_TWTA_TRIP_OVERRIDE_COUNT };
enum { TLM_SC_MODE_NORMAL = 0, TLM_SC_MODE_DWELL, TLM_SC_MODE_DWELL_2 };
enum { TLM_WTS_1_TWTA_1 = 0, TLM_WTS_1_NOT_TWTA_1 };
enum { TLM_WTS_2_TWTA_2 = 0, TLM_WTS_2_NOT_TWTA_2 };

enum
{
    L1_MODE_WOM = 14,
    L1_MODE_CBM = 7,
    L1_MODE_SBM = 112,
    L1_MODE_ROM = 224,

    L1_HVPS_ON  = 0,
    L1_HVPS_OFF = 1,

    L1_TWTA_1 = 0,
    L1_TWTA_2 = 1,

    L1_ANT_A = 1,
    L1_ANT_B = 0

};

//------------------------
// conceptual definitions 
//------------------------

enum { ELECTRONICS_ON, ELECTRONICS_OFF };
enum { REPLACEMENT_HEATER_ENABLED, REPLACEMENT_HEATER_DISABLED };
enum { DSS_A, DSS_B };
enum { TWTA_1, TWTA_2 };
enum { HVPS_ON, HVPS_OFF };
enum { SPARE_HEATER_ENABLED, SPARE_HEATER_DISABLED };

/*
enum INSTRUMENT_ELECTRONICS_E
{
    IE_ON, IE_OFF, IE_UNKNOWN
};

enum REPLACEMENT_HEATER_E
{
    RH_ENABLED, RH_DISABLED
};

enum SPARE_HEATER_E
{
    SH_ENABLED, SH_DISABLED
};

enum NSCAT_MODE_E
{
    SBM, ROM, CCM, DBM, WOM, NSCAT_MODE_COUNT,
    RHM = NSCAT_MODE_COUNT, EXT_MODE_COUNT, MODE_UNKNOWN = EXT_MODE_COUNT
};

enum HVPS_STATE_E
{
    HVPS_ON, HVPS_OFF, HVPS_STATE_COUNT, HVPS_UNKNOWN = HVPS_STATE_COUNT
};

enum DSS_SELECT_E
{
    DSS_A, DSS_B, DSS_COUNT, DSS_UNKNOWN = DSS_COUNT
};

enum TWTA_SELECT_E
{
    TWTA_1, TWTA_2, TWTA_COUNT, TWTA_UNKNOWN = TWTA_COUNT
};

enum FRAME_TYPE_E
{
    SCI, CAL, FRAME_TYPE_COUNT
};

enum WTS_1_E
{
    WTS_TWTA_1, WTS_NOT_TWTA_1, WTS_1_COUNT
};

enum WTS_2_E
{
    WTS_TWTA_2, WTS_NOT_TWTA_2, WTS_2_COUNT
};

enum SC_MODE_E
{
    NORMAL, DWELL, DWELL_2, SC_MODE_COUNT
};

enum RELAY_E
{
    SET, RESET, RELAY_COUNT, RELAY_UNKNOWN = RELAY_COUNT
};

enum TWTA_BOC_TRIP_E
{
    TWTA_BOC_TRIP_ENABLED, TWTA_BOC_TRIP_DISABLED, TWTA_BOC_TRIP_UNKNOWN
};

enum RFS_TRIP_MON_E
{
    RFS_TRIP_MON_DISABLED, RFS_TRIP_MON_ENABLED, RFS_TRIP_MON_UNKNOWN
};

enum HVPS_SHUTDOWN_E
{
    HVPS_SHUTDOWN_DISABLED, HVPS_SHUTDOWN_ENABLED, HVPS_SHUTDOWN_UNKNOWN
};
*/

#define MEAS_TIME           "Time"
#define MEAS_STATUS         "Status"
#define MEAS_UNKNOWN        "Unknown"
#define MEAS_VOLTAGE        "Voltage"
#define MEAS_CURRENT        "Current"
#define MEAS_POWER          "Power"
#define MEAS_TEMPERATURE    "Temperature"
#define MEAS_QUANTITY       "Quantity"
#define MEAS_MAP            "Map"
#define MEAS_ID             "ID"
#define MEAS_LOCATION       "Location"
#define MEAS_DISTANCE       "Distance"
#define MEAS_VELOCITY       "Velocity"
#define MEAS_ORIENTATION    "Orientation"
#define MEAS_RATE           "Rate"
#define MEAS_ADDRESS        "Address"
#define MEAS_DATA           "Data"
#define MEAS_GAIN           "Gain"
#define MEAS_NOISE_FIGURE   "Noise Figure"
#define MEAS_ANGLE          "Angle"

#define SOURCE_UNKNOWN_STRING           "Unknown"
#define SOURCE_L1A_STRING               "L1A"
#define SOURCE_L1AP_STRING              "L1AP"
#define SOURCE_L1A_DERIVED_STRING       "L1ADerived"
#define SOURCE_HK2_STRING               "HK2"
#define SOURCE_L2A_STRING               "L2A"
#define SOURCE_L2B_STRING               "L2B"

enum ParamIdE
{
    PARAM_UNKNOWN,
    FRAME_TIME,
    UTC_TIME,
    TAI_TIME,
    INSTRUMENT_TIME,
    ORBIT_TIME,
    X_POS,
    Y_POS,
    Z_POS,
    X_VEL,
    Y_VEL,
    Z_VEL,
    ROLL,
    PITCH,
    YAW,
    CURRENT_MTLM_TABLE_ID,
    ERROR_FLAGS,
    ERROR_FLAGS_00,
    ERROR_FLAGS_01,
    ERROR_FLAGS_02,
    ERROR_FLAGS_03,
    ERROR_FLAGS_04,
    ERROR_FLAGS_05,
    ERROR_FLAGS_06,
    ERROR_FLAGS_07,
    TABLE_READOUT_TYPE,
    TABLE_READOUT_OFFSET,
    TABLE_READOUT_DATA,
    OPERATIONAL_MODE,
    PRF_COUNT,
    STATUS_TABLE_CHANGE_FLAGS,
    STATUS_TABLE_CHANGE_FLAGS_00,
    STATUS_TABLE_CHANGE_FLAGS_01,
    STATUS_TABLE_CHANGE_FLAGS_02,
    STATUS_TABLE_CHANGE_FLAGS_03,
    STATUS_TABLE_CHANGE_FLAGS_04,
    STATUS_TABLE_CHANGE_FLAGS_05,
    STATUS_TABLE_CHANGE_FLAGS_06,
    STATUS_TABLE_CHANGE_FLAGS_07,
    STATUS_TABLE_CHANGE_FLAGS_08,
    STATUS_TABLE_CHANGE_FLAGS_09,
    STATUS_TABLE_CHANGE_FLAGS_10,
    STATUS_TABLE_CHANGE_FLAGS_11,
    STATUS_TABLE_CHANGE_FLAGS_12,
    STATUS_TABLE_CHANGE_FLAGS_13,
    STATUS_TABLE_CHANGE_FLAGS_14,
    STATUS_TABLE_CHANGE_FLAGS_15,
    ERROR_MSG,
    ERROR_MSG_HISTORY,
    VALID_COMMAND_COUNT,
    INVALID_COMMAND_COUNT,
    CAL_PULSE_POS,
    PRF_CYCLE_TIME,
    RANGE_GATE_A_DELAY,
    RANGE_GATE_A_WIDTH,
    RANGE_GATE_B_DELAY,
    RANGE_GATE_B_WIDTH,
    DOPPLER_SHIFT_CMD_PRF1,
    DOPPLER_SHIFT_CMD_PRF2,
    PULSE_WIDTH,
    RECEIVER_GAIN,
    SES_CONFIG_FLAGS,
    SES_CONFIG_FLAGS_00,
    SES_CONFIG_FLAGS_01,
    SES_CONFIG_FLAGS_02,
    SES_CONFIG_FLAGS_03,
    SES_CONFIG_FLAGS_04,
    SES_CONFIG_FLAGS_05,
    SES_CONFIG_FLAGS_06,
    SES_CONFIG_FLAGS_07,
    SES_DATA_OVERRUN_COUNT,
    SES_DATA_UNDERRUN_COUNT,
    PREDICT_ANT_POS_COUNT,
    RUNNING_ERROR_COUNT,
    SES_RESET_POSITION,
    DOPPLER_ORBIT_STEP,
    PRF_ORBIT_STEP_CHANGE,
    CMD_HISTORY,
    CALC_ANT_MAX_GRP_CNT,
    ADEOS_TIME,
    FSW_MISSION_NO,
    FSW_MISSION_VERSION_NO,
    FSW_BUILD_NO,
    PSU_ELEC_BUS_VOLT,
    CDS_CURRENT,
    SES_A_CURRENT,
    SES_B_CURRENT,
    TWTA_1_CURRENT,
    TWTA_2_CURRENT,
    SAS_A_CURRENT,
    SAS_B_CURRENT,
    PCU_SEC_VOLT_P12,
    PCU_SEC_VOLT_N12,
    PCU_SEC_VOLT_P30,
    PCU_SEC_VOLT_VME_P3_3,
    PCU_ELEC_BUS_VOLT_P5_ISO,
    IDP_A_TEMP,
    IDP_B_TEMP,
    PSU_TEMP,
    RELAY_STATUS,
    SAS_A_SPIN_RATE,
    K21_SES_SUPP_HTR_PWR,
    K22_SES_SUPP_HTR_PWR,
    K23_SPARE,
    SAS_B_SPIN_RATE,
    K24_SPARE,
    K16_SES_SELECT,
    K19_SAS_SELECT,
    K20_SAS_SELECT,
    K9_TWTA_POWER,
    K10_TWTA_POWER,
    K11_TWTA_SELECT,
    K12_TWTA_SELECT,
    K25_SPARE,
    K26_SPARE,
    K15_SES_SELECT,
    EA_A_MOTOR_CURRENT,
    EA_A_SEC_VOLT_P5,
    EA_A_SEC_VOLT_P14,
    EA_A_SPIN_RATE,
    EA_A_SAA_TORQUE_CMD,
    EA_B_MOTOR_CURRENT,
    EA_B_SEC_VOLT_P5,
    EA_B_SEC_VOLT_P14,
    EA_B_SPIN_RATE,
    EA_B_SAA_TORQUE_CMD,
    DRIVE_MOTOR_TEMP,
    EA_A_PWR_SUPPLY_TEMP,
    EA_B_PWR_SUPPLY_TEMP,
    DUPLEX_BEARING_TEMP,
    SIMPLEX_BEARING_TEMP,
    RJ_TEMP,
    A2D_P12V_XCPL,
    TWT_1_BODY_REG_VOLT,
    TWT_1_ION_PUMP_CURRENT,
    TWT_1_BODY_CURRENT,
    TWT_1_DRIVE_PWR,
    TWT_2_BODY_REG_VOLT,
    TWT_2_ION_PUMP_CURRENT,
    TWT_2_BODY_CURRENT,
    TWT_2_DRIVE_PWR,
    TRANSMIT_PWR_A,
    TRANSMIT_PWR_B,
    PWR_CONVERT_CURRENT,
    PRECISION_COUPLER_TEMP,
    TWT_1_HVPS_CHASSIS_TEMP,
    TWT_1_BASE_TEMP,
    TWT_2_HVPS_CHASSIS_TEMP,
    TWT_2_BASE_TEMP,
    RCV_PROTECT_SW_TEMP,
    POWER_CONVERTER_TEMP,
    GAIN_ATTEN_TEMP,
    BEAM_SELECT_SW_TEMP,
    SCP_TEMP,
    RECEIVER_TEMP,
    EXCITER_A_TEMP,
    EXCITER_B_TEMP,
    DISCRETE_STATUS_1,
    DISCRETE_STATUS_1_00,
    DISCRETE_STATUS_1_01,
    DISCRETE_STATUS_1_02,
    DISCRETE_STATUS_1_03,
    DISCRETE_STATUS_1_04,
    DISCRETE_STATUS_1_05,
    DISCRETE_STATUS_1_06,
    DISCRETE_STATUS_1_07,
    DISCRETE_STATUS_2,
    DISCRETE_STATUS_2_00,
    DISCRETE_STATUS_2_03,
    DISCRETE_STATUS_2_04,
    DISCRETE_STATUS_3,
    DISCRETE_STATUS_3_00,
    DISCRETE_STATUS_3_01,
    DISCRETE_STATUS_3_02,
    DISCRETE_STATUS_3_03,
    DISCRETE_STATUS_3_04,
    TRS_STATUS_COUNT,
    SES_DATA_ERROR_FLAGS,
    CDS_MEMORY_DUMP_ADDR,
    CDS_MEMORY_DUMP_DATA,
    SES_MEMORY_DUMP_ADDR,
    SES_MEMORY_DUMP_DATA,
    PCD_ENTRY,
    PULSE_REP_INTERVAL,
    RANGE_GATE_DELAY_INNER,
    RANGE_GATE_DELAY_OUTER,
    RANGE_GATE_WIDTH_INNER,
    RANGE_GATE_WIDTH_OUTER,
    TRUE_CAL_PULSE_POS,
    PRECISION_COUPLER_TEMP_EU,
    RCV_PROTECT_SW_TEMP_EU,
    GAIN_ATTEN_TEMP_EU,
    BEAM_SELECT_SW_TEMP_EU,
    RECEIVER_TEMP_EU,
    ANTENNA_POS_1_DN,
    ANTENNA_POS_2_50_DN,
    ANTENNA_POS_51_DN,
    ANTENNA_POS_52_100_DN,
    ANTENNA_AZIMUTH,
    LOOP_BACK_CAL_POWER,
    REP_LOOP_BACK_CAL_POWER,
    LOOP_BACK_CAL_NOISE,
    REP_LOOP_BACK_CAL_NOISE,
    LOAD_CAL_POWER,
    REP_LOAD_CAL_POWER,
    LOAD_CAL_NOISE,
    REP_LOAD_CAL_NOISE,
    POWER_DN,
    NOISE_DN,
    SPARE_SCIENCE_DATA,
    L1A_FRAME_INST_STATUS,
    L1A_FRAME_INST_STATUS_00_01,
    L1A_FRAME_INST_STATUS_02,
    L1A_FRAME_INST_STATUS_03,
    L1A_FRAME_INST_STATUS_04_06,
    L1A_FRAME_INST_STATUS_07,
    L1A_FRAME_INST_STATUS_08,
    L1A_FRAME_INST_STATUS_09,
    L1A_FRAME_INST_STATUS_10,
    L1A_FRAME_INST_STATUS_11,
    L1A_FRAME_ERR_STATUS,
    PULSE_QUALITY_FLAG,
    NOISE_POWER_BEAM_A,
    NOISE_POWER_BEAM_B,
    SLICE_1_POWER_BEAM_A,
    SLICE_2_POWER_BEAM_A,
    SLICE_3_POWER_BEAM_A,
    SLICE_4_POWER_BEAM_A,
    SLICE_5_POWER_BEAM_A,
    SLICE_6_POWER_BEAM_A,
    SLICE_7_POWER_BEAM_A,
    SLICE_8_POWER_BEAM_A,
    SLICE_9_POWER_BEAM_A,
    SLICE_10_POWER_BEAM_A,
    SLICE_11_POWER_BEAM_A,
    SLICE_12_POWER_BEAM_A,
    SLICE_1_POWER_BEAM_B,
    SLICE_2_POWER_BEAM_B,
    SLICE_3_POWER_BEAM_B,
    SLICE_4_POWER_BEAM_B,
    SLICE_5_POWER_BEAM_B,
    SLICE_6_POWER_BEAM_B,
    SLICE_7_POWER_BEAM_B,
    SLICE_8_POWER_BEAM_B,
    SLICE_9_POWER_BEAM_B,
    SLICE_10_POWER_BEAM_B,
    SLICE_11_POWER_BEAM_B,
    SLICE_12_POWER_BEAM_B,
    TOTAL_12_SLICES_BEAM_A,
    TOTAL_12_SLICES_BEAM_B,
    NOISE_LOAD,
    SLICE_1_LOAD_POWER,
    SLICE_2_LOAD_POWER,
    SLICE_3_LOAD_POWER,
    SLICE_4_LOAD_POWER,
    SLICE_5_LOAD_POWER,
    SLICE_6_LOAD_POWER,
    SLICE_7_LOAD_POWER,
    SLICE_8_LOAD_POWER,
    SLICE_9_LOAD_POWER,
    SLICE_10_LOAD_POWER,
    SLICE_11_LOAD_POWER,
    SLICE_12_LOAD_POWER,
    TOTAL_LOAD_POWER,
    BANDWIDTH_RATIO_NOISE_ECHO_ALPHA,
    GAIN_RATIO_NOISE_ECHO_BEAM_A_BETA,
    GAIN_RATIO_NOISE_ECHO_BEAM_B_BETA,
    ROW_NUMBER,
    NUM_SIGMA0,
    CELL_LAT,
    CELL_LON,
    CELL_AZIMUTH,
    CELL_INCIDENCE,
    SIGMA0,
    SIGMA0_ATTN_AMSR,
    SIGMA0_ATTN_MAP,
    KP_ALPHA,
    KP_BETA,
    KP_GAMMA,
    SIGMA0_QUAL_FLAG,
    SIGMA0_MODE_FLAG,
    SURFACE_FLAG,
    CELL_INDEX,
    WVC_SIGMA0_SPARE,
    WVC_ROW,
    WVC_LAT,
    WVC_LON,
    WVC_INDEX,
    NUM_IN_FORE,
    NUM_IN_AFT,
    NUM_OUT_FORE,
    NUM_OUT_AFT,
    WVC_QUALITY_FLAG,
    ATTEN_CORR,
    MODEL_SPEED,
    MODEL_DIR,
    NUM_AMBIGS,
    WIND_SPEED,
    WIND_DIR,
    WIND_SPEED_ERR,
    WIND_DIR_ERR,
    MAX_LIKELIHOOD_EST,
    WVC_SELECTION
};

enum UnitIdE
{
    UNIT_UNKNOWN,
    UNIT_AMPS,
    UNIT_AUTOTIME,
    UNIT_CODE_A,    // for time
    UNIT_COUNTS,
    UNIT_DAYS,
    UNIT_DB,        // dB
    UNIT_DBM,       // dBm
    UNIT_DEC_ADDR,  // decimal address
    UNIT_DEGREES,
    UNIT_DEGREES_C, // degrees Celcius
    UNIT_DN,        // data numbers
    UNIT_DPS,       // degrees per second
    UNIT_HEX_ADDR,  // hexadecimal address
    UNIT_HEX_BYTES, // hexadecimal bytes
    UNIT_HOURS,
    UNIT_KILOMETERS,
    UNIT_MA,        // milliamps
    UNIT_MAP,
    UNIT_METERS,
    UNIT_MINUTES,
    UNIT_MPS,       // meters per second
    UNIT_MWATTS,    // milliwatts
    UNIT_PULSE_CYCLES,
    UNIT_SECONDS,
    UNIT_TAI_SECONDS,   // seconds without adjusted by "sinceTime"
    UNIT_TICKS,     // ticks of clocks
    UNIT_TOGGLE,
    UNIT_UA,        // microamps
    UNIT_VOLTS,
    UNIT_WATTS,
    UNIT_MS,        // milliseconds
    UNIT_ID,        // id numbers
    UNIT_PIXELS,
    UNIT_RPM,
    UNIT_OZIN,
    UNIT_L1ATIME
};

//---------------------------------
// do not change the order!!
// source_id_map depends on this
//---------------------------------
enum SourceIdE
{
    SOURCE_UNKNOWN,
    SOURCE_L1A,
    SOURCE_L1AP,
    SOURCE_L1A_DERIVED,
    SOURCE_HK2,
    SOURCE_L2A,
    SOURCE_L2B
};

enum DataTypeE
{
    DATA_UNKNOWN,
    DATA_UINT1,
    DATA_UINT1_49,
    DATA_INT1,
    DATA_INT1_76,
    DATA_INT1_810,
    DATA_UINT2,
    DATA_UINT2_76,
    DATA_UINT2_100,
    DATA_UINT2_810,
    DATA_UINT2_100_12,
    DATA_INT2,
    DATA_UINT2_4,
    DATA_UINT2_5,
    DATA_UINT2_25,
    DATA_UINT2_2_8,
    DATA_UINT3,
    DATA_UINT4,
    DATA_UINT4_25,
    DATA_UINT4_100,
    DATA_INT4,
    DATA_FLOAT4,
    DATA_FLOAT8,
    DATA_FLOAT4_25,
    DATA_FLOAT4_76,
    DATA_FLOAT4_76_4,
    DATA_FLOAT4_100,
    DATA_FLOAT4_810,
    DATA_ITIME,
    DATA_UINT4_4,
    DATA_CHAR1,
    DATA_CHAR2,
    DATA_CHAR3,
    DATA_CHAR4,
    DATA_CHAR13,
    DATA_CHAR16,
    DATA_CHAR28
};

extern const char *source_id_map[];

//-******************************************************************
//  Parameter:
//      Used by other objects.  No data hiding because other objects
//      such as TableAccess and DataSet need access to members.
//-******************************************************************
struct Parameter
{
    friend int operator==(const Parameter&, const Parameter&);

    Parameter();
    Parameter(      const Parameter&    other);
    virtual ~Parameter();

    Parameter&      operator=(const Parameter& other);

    ParamIdE        paramId;                // PARAM_ANTENNA_CYCLE ...
    char            paramName[PARAM_NAME_LEN];  // variable name
    SourceIdE       sourceId;               // SOURCE_HK2, SOURCE_L1A ...
    char            measurable[MEASURABLE_LEN];
    int32*          sdsIDs;                 // HDF dataset ID array
    int             numSDSs;                // element number in sdsIDs
    char            sdsNames[BIG_SIZE];     // HDF dataset names, sep by ','
    UnitIdE         unitId;                 // UNIT_DAYS, ...
    char            unitName[UNIT_NAME_LEN];    // unit string
    DataTypeE       dataType;               // DATA_FLOAT, DATA_INT ...
    IotBoolean      needPolynomial;         // true if polynomial is needed
    unsigned long   byteSize;               // total number of bytes
    ExtractFunc     extractFunc;
    PrintFunc       printFunc;
    char*           data;                   // array of data

    unsigned char   held;                   // indicates valid held data

};//Parameter

inline int operator==(const Parameter& a, const Parameter& b)
{
    return( a.paramId == b.paramId && a.sourceId == b.sourceId &&
            a.unitId == b.unitId ? 1 : 0);
}

#endif //PARAMETER_H
