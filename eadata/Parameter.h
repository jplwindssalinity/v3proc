//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.37   25 May 1999 14:05:56   sally
// add L2Ax for Bryan Stiles
// 
//    Rev 1.36   23 Feb 1999 11:13:26   sally
// L2A array size chaned from 810 to 3240
// 
//    Rev 1.35   07 Dec 1998 15:44:42   sally
// add new parameters for sliced power_dn
// 
//    Rev 1.34   20 Nov 1998 16:03:36   sally
// change some data types and limit check arrays
// 
//    Rev 1.33   10 Nov 1998 08:52:00   sally
// add delta instrument time because the instrument seems to skip cycle
// 
//    Rev 1.32   03 Nov 1998 16:01:48   sally
// add source sequence count
// 
//    Rev 1.31   28 Oct 1998 15:04:48   sally
// add new units for L1B Hdf
// 
//    Rev 1.29   13 Oct 1998 15:34:26   sally
// added L1B file
// 
//    Rev 1.28   08 Oct 1998 16:17:28   sally
// add L1B type
// 
//    Rev 1.27   08 Sep 1998 16:25:14   sally
// added HK2 FSW subcoms
// 
//    Rev 1.26   18 Aug 1998 10:58:42   sally
// make L1ADrvExtract return any number of values
// 
//    Rev 1.25   27 Jul 1998 14:00:40   sally
// passing polynomial table to extraction function
// 
//    Rev 1.24   23 Jul 1998 16:14:50   sally
// pass polynomial table to extractFunc()
// 
//    Rev 1.23   29 Jun 1998 16:52:04   sally
// added embedded commands checking
// 
//    Rev 1.22   22 Jun 1998 15:26:06   sally
// change to incorporate Barry's update
// 
//    Rev 1.21   19 Jun 1998 16:54:16   sally
// added "Orbit Period" in L1A Derived Data  
// 
//    Rev 1.20   03 Jun 1998 10:10:24   sally
// change parameter names and types due to LP's changes
// 
//    Rev 1.19   28 May 1998 15:51:50   sally
// changed some L1A parameter names
// 
//    Rev 1.18   18 May 1998 14:48:02   sally
// added error checker for L1A
// 
//    Rev 1.17   11 May 1998 14:33:28   sally
// fixed some HK2 data
// 
//    Rev 1.16   04 May 1998 10:52:54   sally
// added HK2 filters
// 
//    Rev 1.15   01 May 1998 16:45:28   sally
// add some more filters, and changed some names, per Lee Poulsen
// 
//    Rev 1.14   01 May 1998 14:47:44   sally
// added HK2 file
// 
//    Rev 1.13   28 Apr 1998 15:57:28   sally
// added scatterometer housekeeping (1553) data for HK2
// 
//    Rev 1.12   27 Apr 1998 15:49:54   sally
// update HK2 data
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
class PolynomialTable;

typedef int (*ExtractFunc) (TlmHdfFile*, int32*, int32, int32, int32,
                            VOIDP, PolynomialTable*);
typedef void (*PrintFunc) (FILE* fp, char* data);

#define PARAM_NAME_LEN  128
#define MEASURABLE_LEN  64
#define UNIT_NAME_LEN   128
#define SDS_NAME_LEN    128

#define MAX_NUM_DERIVED_VALUES    100

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

    L1_TWT_ON  = 0,
    L1_TWT_OFF = 1,

    L1_BEAM_A = 1,
    L1_BEAM_B = 0,

    L1_MODULATION_ON  = 1,
    L1_MODULATION_OFF = 0,

    L1_RX_PROTECT_ON  = 1,
    L1_RX_PROTECT_OFF = 0,

    L1_GRID_DISABLE = 1,
    L1_GRID_NORMAL = 0,

    L1_SAS_A_SPIN_18_0 = 1,
    L1_SAS_A_SPIN_19_8 = 0,

    L1_SAS_B_SPIN_18_0 = 1,
    L1_SAS_B_SPIN_19_8 = 0
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
#define MEAS_FREQUENCY      "Frequency"

#define SOURCE_UNKNOWN_STRING           "Unknown"
#define SOURCE_L1A_STRING               "L1A"
#define SOURCE_L1AP_STRING              "L1AP"
#define SOURCE_L1A_DERIVED_STRING       "L1ADerived"
#define SOURCE_L1B_STRING               "L1B"
#define SOURCE_L2A_STRING               "L2A"
#define SOURCE_L2Ax_STRING              "L2Ax"
#define SOURCE_L2B_STRING               "L2B"
#define SOURCE_HK2_STRING               "HK2"

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
    CORRES_INSTR_TIME,
    FSW_MISSION_NO,
    FSW_MISSION_VERSION_NO,
    FSW_BUILD_NO,
    PBI_FLAG,
    FILL_DATA,
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
    RECEIVER_GAIN_A,
    RECEIVER_GAIN_B,
    NOISE_FIGURE_A,
    NOISE_FIGURE_B,
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
    PRF_CYCLE_TIME_EU,
    RANGE_GATE_DELAY_INNER,
    RANGE_GATE_DELAY_OUTER,
    RANGE_GATE_WIDTH_INNER,
    RANGE_GATE_WIDTH_OUTER,
    TRANSMIT_PULSE_WIDTH,
    TRUE_CAL_PULSE_POS,
    TRANSMIT_POWER_INNER,
    TRANSMIT_POWER_OUTER,
    PRECISION_COUPLER_TEMP_EU,
    RCV_PROTECT_SW_TEMP_EU,
    BEAM_SELECT_SW_TEMP_EU,
    RECEIVER_TEMP_EU,
    ANTENNA_POS,
    LOOP_BACK_CAL_A_POWER,
    LOOP_BACK_CAL_B_POWER,
    LOOP_BACK_CAL_NOISE,
    LOAD_CAL_A_POWER,
    LOAD_CAL_B_POWER,
    LOAD_CAL_NOISE,
    POWER_DN,
    NOISE_DN,
    SPARE_SCIENCE_DATA,
    FRAME_INST_STATUS,
    FRAME_INST_STATUS_00_01,
    FRAME_INST_STATUS_02,
    FRAME_INST_STATUS_03,
    FRAME_INST_STATUS_04_06,
    FRAME_INST_STATUS_07,
    FRAME_INST_STATUS_08,
    FRAME_INST_STATUS_09,
    FRAME_INST_STATUS_10,
    FRAME_INST_STATUS_11,
    FRAME_ERR_STATUS,
    FRAME_QUALITY_FLAG,
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
    WVC_SELECTION,
    HK2_FRAME_COUNT,
    TORQUE_ROD_1_STATUS,
    TORQUE_ROD_2_STATUS,
    TORQUE_ROD_3_STATUS,
    CSM_STATUS,
    CBM_STATUS,
    TABLE_UPLOAD_IN_PROG,
    TABLE_DOWNLOAD_IN_PROG,
    EEPROM_PROG_STATUS,
    EEPROM_PROG_TOGGLE,
    FLTSW_CMD_ACC_CNT,
    FLTSW_CMD_REJ_CNT,
    CMD_STATUS,
    CSM_EXEC_CMD_CNT,
    CBM_EXEC_CMD_CNT,
    TABLE_UPLOAD_RX_CNT,
    TABLE_CHECKSUM_ACC_CNT,
    TABLE_CHECKSUM_REJ_CNT,
    AUTO_WAIT_STATUS,
    STAR_TRACKER_ATT,
    SOLAR_ARRAY_1_ENBL,
    SOLAR_ARRAY_2_ENBL,
    GYRO_DATA_QUALITY,
    PROC_RESET_FLAG,
    FLTSW_MINOR_FRAME_TIME,
    SAS_EA_A_PWR_TEMP,
    SAS_DUPLEX_BEARING_TEMP,
    SES_TEMP_1,
    TWT1_BASE_TEMP,
    CDS_A_TEMP,
    HK2_PSU_TEMP,
    RATE_ERROR_DETECTED,
    ATT_ERROR_DETECTED,
    ATT_DETERM_OVERRIDE,
    CBM_ACTIVE_BLOCK_CNT,
    AUTO_SCAT_MSG_STATUS,
    AUTO_SCAT_TEMP_MON,
    CDS_SAFE_SEQ_STATUS,
    SES_SAFE_SEQ_STATUS,
    SAS_SAFE_SEQ_STATUS,
    CDS_PSU_BUS_VOLT,
    CDS_PRIM_BUS_CURRENT,
    SASA_PRIM_BUS_CURRENT,
    SASB_PRIM_BUS_CURRENT,
    SESA_PRIM_BUS_CURRENT,
    SESB_PRIM_BUS_CURRENT,
    TWTA1_PRIM_BUS_CURRENT,
    TWTA2_PRIM_BUS_CURRENT,
    TWTA1_BODY_REG_CURR,
    TWTA2_BODY_REG_CURR,
    TWTA1_ION_PUMP_CURR,
    TWTA2_ION_PUMP_CURR,
    TWTA_DRIVE_PWR,
    SES_TRS_XMIT_PWR,
    SASA_EAA_MOTOR_CURR,
    SASB_EAB_MOTOR_CURR,
    SASA_EAA_SPIN_RATE,
    SASB_EAB_SPIN_RATE,
    SASA_EAA_SAA_SEC_VOLT,
    SASB_EAB_SAA_SEC_VOLT,
    CDS_PSU_3_TEMP,
    CDS_IDP_A_TEMP,
    CDS_IDP_B_TEMP,
    TWTA_1_HVPS_CHAS_TEMP,
    TWTA_2_HVPS_CHAS_TEMP,
    TWTA_1_BASE_TEMP,
    TWTA_2_BASE_TEMP,
    SES_DC_CONV_TEMP,
    SES_PREC_COUPLER_TEMP,
    SES_GAIN_ATTEN_TEMP,
    SES_RX_PROTECT_SW_TEMP,
    SAS_RJ_TEMP,
    CDS_ERR_RESET_FLAG,
    STALE_DATA_TOGGLE,
    A2D_TIMEOUT_FLAG,
    MISS_SC_TIME_HLDC,
    MTLM_STLM_ERR,
    FAULT_PROTECT_EVENT,
    WATCHDOG_TIMEOUT,
    POWER_ON_RESET,
    TWTA_1_CONV_OC,
    TWTA_1_UNDER_VOLT,
    TWTA_1_BODY_OC,
    TWTA_2_CONV_OC,
    TWTA_2_UNDER_VOLT,
    TWTA_2_BODY_OC,
    PLL_OUT_OF_LOCK,
    BEAM_SELECT,
    SES_WATCHDOG_TIMER_EVENT,
    SES_TRS_CMD_SUCC,
    SES_RX_PROTECT,
    SES_SERIAL_PRT_PARITY,
    GRID_INHIBIT,
    TWT_BODY_OC_TRIP,

    GPS_TIME_TAG_MONTH,
    GPS_TIME_TAG_DAY,
    GPS_TIME_TAG_YEAR,
    GPS_TIME_TAG_HOURS,
    GPS_TIME_TAG_MINUTES,
    GPS_TIME_TAG_SECONDS,
    GPS_TIME_TAG_NSECONDS,
    CURRENT_DOP,
    NUM_VISIBLE_SAT,
    NUM_SAT_TRACKED,
    SAT_1_ID,
    SAT_1_TRACK_MODE,
    SAT_2_ID,
    SAT_2_TRACK_MODE,
    SAT_3_ID,
    SAT_3_TRACK_MODE,
    SAT_4_ID,
    SAT_4_TRACK_MODE,
    SAT_5_ID,
    SAT_5_TRACK_MODE,
    SAT_6_ID,
    SAT_6_TRACK_MODE,
    SAT_7_ID,
    SAT_7_TRACK_MODE,
    SAT_8_ID,
    SAT_8_TRACK_MODE,
    SAT_9_ID,
    SAT_9_TRACK_MODE,
    SAT_10_ID,
    SAT_10_TRACK_MODE,
    SAT_11_ID,
    SAT_11_TRACK_MODE,
    SAT_12_ID,
    SAT_12_TRACK_MODE,
    POS_PROP_MODE,
    POOR_GEOMETRY,
    THREE_D_FIX,
    ALTITUDE_HOLD,
    ACQUIRE_SAT,
    STORE_NEW_ALMANAC,
    INSUFF_VISIBLE_SAT,
    BAD_ALMANAC,
    GPS_POS_X_RAW_1,
    GPS_POS_X_RAW_2,
    GPS_POS_Y_RAW_1,
    GPS_POS_Y_RAW_2,
    GPS_POS_Z_RAW_1,
    GPS_POS_Z_RAW_2,
    GPS_VEL_X_RAW_1,
    GPS_VEL_X_RAW_2,
    GPS_VEL_Y_RAW_1,
    GPS_VEL_Y_RAW_2,
    GPS_VEL_Z_RAW_1,
    GPS_VEL_Z_RAW_2,
    GPS_VTCW_MS_SEG,
    GPS_VTCW_MID_SEG,
    GPS_VTCW_LS_SEG,
    GYRO_FRAME_COUNTER,
    GYRO_HIGH_VOLT_STAT,
    GYRO_X_DITH_STAT,
    GYRO_Y_DITH_STAT,
    GYRO_Z_DITH_STAT,
    GYRO_X_INTENSITY,
    GYRO_Y_INTENSITY,
    GYRO_Z_INTENSITY,
    GYRO_PWR_SUPP_P15V_STAT,
    GYRO_PWR_SUPP_P120V_STAT,
    WHEEL_1_SPEED_DIR,
    WHEEL_2_SPEED_DIR,
    WHEEL_3_SPEED_DIR,
    WHEEL_4_SPEED_DIR,
    STATUS_CODE,
    COMMAND_ID,
    TLM_FORMAT,
    INIT_COMPLETE,
    CURRENT_STATE,
    CURRENT_MODE,
    SSR1_SELECTED_PWR,
    SSR2_SELECTED_PWR,
    SSR1_EDAC_ENABLED,
    SSR2_EDAC_ENABLED,
    SSR1_CURR_PWR_STATE,
    SSR2_CURR_PWR_STATE,
    ADDR_SW_EXCEPTION,
    CORR_MEM_ERR_CNT,
    UNCORR_MEM_ERR_CNT,
    SSR1_RX_STATUS,
    SSR1_XMIT_STATUS,
    SSR2_RX_STATUS,
    SSR2_XMIT_STATUS,
    PART_0_SSR1_REC_ADDR,
    PART_0_SSR2_REC_ADDR,
    PART_0_SSR1_UNFIX_ERR,
    PART_0_SSR1_PBK_ADDR,
    PART_0_SSR2_UNFIX_ERR,
    PART_0_SSR2_PBK_ADDR,
    PART_1_SSR1_REC_ADDR,
    PART_1_SSR2_REC_ADDR,
    PART_1_SSR1_UNFIX_ERR,
    PART_1_SSR1_PBK_ADDR,
    PART_1_SSR2_UNFIX_ERR,
    PART_1_SSR2_PBK_ADDR,
    SSR1_CORR_MEM_FLAG,
    SSR1_CORR_MEM_ERR_CNT,
    SSR2_CORR_MEM_FLAG,
    SSR2_CORR_MEM_ERR_CNT,
    SSR1_UNCORR_MEM_FLAG,
    SSR1_UNCORR_MEM_ERR_CNT,
    SSR2_UNCORR_MEM_FLAG,
    SSR2_UNCORR_MEM_ERR_CNT,
    MESSAGE_CHECKSUM,
    ADCS_CYCLE_START_1,
    ADCS_CYCLE_START_2,
    ADCS_CYCLE_START_3,
    STAR_TRKR_ATT_Q1,
    STAR_TRKR_ATT_Q2,
    STAR_TRKR_ATT_Q3,
    STAR_TRKR_ATT_Q4,
    AXIS1_AVG_PRD1,
    AXIS2_AVG_PRD1,
    AXIS3_AVG_PRD1,
    AXIS1_AVG_PRD2,
    AXIS2_AVG_PRD2,
    AXIS3_AVG_PRD2,
    AXIS1_AVG_PRD3,
    AXIS2_AVG_PRD3,
    AXIS3_AVG_PRD3,
    AXIS1_AVG_PRD4,
    AXIS2_AVG_PRD4,
    AXIS3_AVG_PRD4,
    AXIS1_AVG_PRD5,
    AXIS2_AVG_PRD5,
    AXIS3_AVG_PRD5,
    ATT_UPDATE_RATE_X,
    CONTROL_TORQUE_X,
    ATT_UPDATE_RATE_Y,
    CONTROL_TORQUE_Y,
    ATT_UPDATE_RATE_Z,
    CONTROL_TORQUE_Z,
    MAX_RESIDUAL,
    SUN_SENSOR1_INTENSITY,
    WHEEL_1_TORQUE_CMD,
    SUN_SENSOR2_INTENSITY,
    WHEEL_2_TORQUE_CMD,
    SUN_SENSOR3_INTENSITY,
    WHEEL_3_TORQUE_CMD,
    SUN_SENSOR4_INTENSITY,
    WHEEL_4_TORQUE_CMD,
    SUN_SENSOR5_INTENSITY,
    CURRENT_ADCS_STATE,
    ACT_CNTL_ENB_DSB,
    WHELL_1_OVERSPEED,
    WHELL_2_OVERSPEED,
    WHELL_3_OVERSPEED,
    WHELL_4_OVERSPEED,
    SUN_SENSOR6_INTENSITY,
    STAR_TRACK_1_DATA_REQ_ENB,
    STAR_TRACK_2_DATA_REQ_ENB,
    ATT_UPDATE_RATE_STATUS,
    INT_PROPAGATOR_VALID,
    GPS_DATA_VALID_FLAG,
    START_GPS_CARRIER_DATA,
    STAR_TRACK_1_CCD_TEMP,
    SUN_SENSOR7_INTENSITY,
    STAR_TRACK_1_BASE_TEMP,
    SUN_SENSOR8_INTENSITY,
    STAR_TRACK_1_LENS_TEMP,
    SUN_SENSOR9_INTENSITY,
    STAR_1_P2_VOLT,
    SUN_SENSOR10_INTENSITY,
    STAR_1_M8_VOLT,
    STAR_1_P5_VOLT,
    SUN_SENSOR11_INTENSITY,
    STAR_1_M5_VOLT,
    STAR_1_BG_READ,
    SUN_SENSOR12_INTENSITY,
    STAR_1_FF_CNT,
    STAR_1_FALALRM_CNT,
    SUN_SENSOR13_INTENSITY,
    STAR_1_TLM_OFFSET,
    SUN_SENSOR14_INTENSITY,
    STAR_2_CCD_TEMP,
    MEASURED_MAG_FIELD_X,
    STAR_2_BASE_TEMP,
    MEASURED_MAG_FIELD_Y,
    STAR_2_LENS_TEMP,
    MEASURED_MAG_FIELD_Z,
    STAR_2_P2_VOLT,
    STAR_2_M8_VOLT,
    MEASURED_SUN_VECTOR_X,
    STAR_2_P5_VOLT,
    STAR_2_M5_VOLT,
    MEASURED_SUN_VECTOR_Y,
    STAR_2_BG_READ,
    MEASURED_SUN_VECTOR_Z,
    STAR_2_FF_CNT,
    STAR_2_FALARM_CNT,
    ECFF_TARGET_ID,
    STAR_2_TLM_OFFSET,
    FFT_TARGET_ID,
    RATE_SEN_ATT_Q1,
    DESIRED_ATT_Q1,
    RATE_SEN_ATT_Q2,
    DESIRED_ATT_Q2,
    RATE_SEN_ATT_Q3,
    DESIRED_ATT_Q3,
    RATE_SEN_ATT_Q4,
    DESIRED_ATT_Q4,
    CONTROL_FRAME_ATT_Q1,
    NEXT_ORBIT_POS_X,
    CONTROL_FRAME_ATT_Q2,
    NEXT_ORBIT_POS_Y,
    CONTROL_FRAME_ATT_Q3,
    NEXT_ORBIT_POS_Z,
    CONTROL_FRAME_ATT_Q4,
    NEXT_ORBIT_VEL_X,
    CONTROL_FRAME_RATE_X,
    NEXT_ORBIT_VEL_Y,
    CONTROL_FRAME_RATE_Y,
    NEXT_ORBIT_VEL_Z,
    CONTROL_FRAME_RATE_Z,
    MODEL_MAG_FIELD_VX,
    MEASURED_ATT_Q1,
    MODEL_MAG_FIELD_VY,
    MEASURED_ATT_Q2,
    MODEL_MAG_FIELD_VZ,
    MEASURED_ATT_Q3,
    MODEL_SUN_VX,
    MEASURED_ATT_Q4,
    MODEL_SUN_VY,
    TOTAL_MOMENTUM_1,
    MODEL_SUN_VZ,
    TOTAL_MOMENTUM_2,
    CALC_SOLAR_ARRAY1_POS,
    TOTAL_MOMENTUM_3,
    CALC_SOLAR_ARRAY2_POS,
    PRI_SEC_ANG_STATUS,
    PRI_TER_ANG_STATUS,
    PRI_FOUR_ANG_STATUS,
    PRI_ORBIT_ANG_STATUS,
    SEC_TER_ANG_STATUS,
    SEC_FOUR_ANG_STATUS,
    SEC_ORBIT_ANG_STATUS,
    TER_FOUR_ANG_STATUS,
    TER_ORBIT_ANG_STATUS,
    FOUR_ORBIT_ANG_STATUS,
    ORBIT_INTERP_METHOD,
    VECTOR_PAIR_SELECTION,
    SOLAR_ARRAY1_POT_READING,
    ATT_DERTERM_METHOD,
    ATT_UPDATE_METHOD,
    SOLUTION_STATUS,
    SOLAR_ARRAY2_POT_READING,
    ACTIVE_CDU,
    CMD_ORBIT_INTERP_METHOD,
    RADIUS_VIOLATION,
    SPEED_VIOLATION,
    ORBIT_STATE_VALID,
    TARGET_TABLE_SELECT,
    MEAS_MAG_FIELD,
    MEAS_SUN_VECTOR,
    GYRO_TEMP_1,
    GYRO_TEMP_2,
    GYRO_TEMP_3,
    ORBIT_PERIOD,
    ANT_SPIN_RATE,
    FRAME_INST_STATUS_12,
    FRAME_INST_STATUS_13,
    FRAME_INST_STATUS_14,
    FRAME_INST_STATUS_15,
    FRAME_INST_STATUS_16,
    FRAME_INST_STATUS_17,
    FRAME_INST_STATUS_18,
    FRAME_INST_STATUS_19,
    FRAME_INST_STATUS_20,
    FRAME_INST_STATUS_21,
    FRAME_INST_STATUS_22,
    FRAME_INST_STATUS_23,
    FRAME_INST_STATUS_24,
    FRAME_INST_STATUS_25,
    FRAME_INST_STATUS_26,
    FRAME_INST_STATUS_27,
    FRAME_INST_STATUS_28,
    FRAME_ERR_STATUS_00,
    FRAME_ERR_STATUS_01,
    FRAME_ERR_STATUS_02,
    FRAME_ERR_STATUS_03,
    FRAME_ERR_STATUS_04,
    FRAME_ERR_STATUS_05,
    FRAME_ERR_STATUS_06,
    FRAME_ERR_STATUS_07,
    FRAME_ERR_STATUS_08,
    FRAME_ERR_STATUS_09,
    FRAME_ERR_STATUS_10,
    FRAME_ERR_STATUS_11,
    FRAME_ERR_STATUS_12,
    FRAME_ERR_STATUS_13,
    FRAME_ERR_STATUS_14,
    FRAME_ERR_STATUS_15,
    FRAME_ERR_STATUS_16,
    FRAME_ERR_STATUS_17,
    FRAME_ERR_STATUS_18,
    FRAME_ERR_STATUS_19,
    FRAME_ERR_STATUS_20,
    FRAME_ERR_STATUS_21,
    FRAME_ERR_STATUS_22,
    FRAME_ERR_STATUS_23,
    FRAME_ERR_STATUS_24,
    FRAME_ERR_STATUS_25,
    FRAME_QUALITY_FLAG_00_01,
    FRAME_QUALITY_FLAG_02_03,
    FRAME_QUALITY_FLAG_04,
    NUM_PULSES,
    SC_LAT,
    SC_LON,
    SC_ALT,
    BANDWIDTH_RATIO,
    X_CAL_A,
    X_CAL_B,   
    FREQUENCY_SHIFT,
    ANTENNA_AZIMUTH,
    SNR,
    KPC_A,
    SLICE_QUAL_FLAG,
    SLICE_LAT,
    SLICE_LON,
    SLICE_SIGMA0,
    X_FACTOR,
    SLICE_AZIMUTH,
    SLICE_INCIDENCE,
    SLICE_SNR,
    SLICE_KPC_A,
    HK2_SRC_SEQ_COUNT,
    HK2_DELTA_SRC_SEQ_COUNT,
    DELTA_INSTRUMENT_TIME,
    POWER_DN_SLICE_1,
    POWER_DN_SLICE_2,
    POWER_DN_SLICE_3,
    POWER_DN_SLICE_4,
    POWER_DN_SLICE_5,
    POWER_DN_SLICE_6,
    POWER_DN_SLICE_7,
    POWER_DN_SLICE_8,
    POWER_DN_SLICE_9,
    POWER_DN_SLICE_10,
    POWER_DN_SLICE_11,
    POWER_DN_SLICE_12,
    WVC_ROW_TIME
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
    UNIT_L1ATIME,
    UNIT_KHZ,       // kilo herz
    UNIT_RAD_SEC,   // Radians/sec
    UNIT_DEG_SEC,   // Degrees/sec
    UNIT_ROT_MIN,   // Rotation/min
    UNIT_NM,
    UNIT_EU,
    UNIT_TELSA,     // Mag field
    UNIT_KM_SEC,    // Km/sec
    UNIT_HZ,        // Hertz
    UNIT_KMPS,      // kilometers per second
    UNIT_RADIANS,
    UNIT_DB_DN       // DB of DN
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
    SOURCE_L1B,
    SOURCE_HK2,
    SOURCE_L2A,
    SOURCE_L2Ax,
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
    DATA_INT1_3240,
    DATA_UINT2,
    DATA_UINT2_12,
    DATA_UINT2_76,
    DATA_UINT2_100,
    DATA_UINT2_810,
    DATA_UINT2_3240,
    DATA_UINT2_100_12,
    DATA_UINT2_4,
    DATA_UINT2_5,
    DATA_UINT2_25,
    DATA_UINT2_2_8,
    DATA_INT2,
    DATA_INT2_100,
    DATA_UINT3,
    DATA_UINT4,
    DATA_UINT4_4,
    DATA_UINT4_12,
    DATA_UINT4_25,
    DATA_UINT4_100,
    DATA_UINT4_100_12,
    DATA_INT4,
    DATA_FLOAT4,
    DATA_FLOAT8,
    DATA_FLOAT4_12,
    DATA_FLOAT4_25,
    DATA_FLOAT4_76,
    DATA_FLOAT4_76_4,
    DATA_FLOAT4_100,
    DATA_FLOAT4_810,
    DATA_FLOAT4_3240,
    DATA_FLOAT4_100_8,
    DATA_ITIME,
    DATA_CHAR1,
    DATA_CHAR2,
    DATA_CHAR3,
    DATA_CHAR4,
    DATA_CHAR13,
    DATA_CHAR16,
    DATA_CHAR28,
    DATA_CHAR32
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

struct DerivedExtractResult
{
    unsigned char  validDataMap[MAX_NUM_DERIVED_VALUES];
    int            numExtracted;
    char*          dataBuf;
};

inline int operator==(const Parameter& a, const Parameter& b)
{
    return( a.paramId == b.paramId && a.sourceId == b.sourceId &&
            a.unitId == b.unitId ? 1 : 0);
}

inline int          // >= 0 are good modes, < 0 is bad
L1ModeToTlmMode(
unsigned char    l1Mode)
{
    switch(l1Mode)
    {
        case L1_MODE_WOM:
            return(TLM_MODE_WOM);
            break;
        case L1_MODE_CBM:
            return(TLM_MODE_CBM);
            break;
        case L1_MODE_SBM:
            return(TLM_MODE_SBM);
            break;
        case L1_MODE_ROM:
            return(TLM_MODE_ROM);
            break;
        default:
            return -1;
    }
}
#endif //PARAMETER_H
