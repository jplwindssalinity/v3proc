//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.32   12 Apr 1999 11:10:18   sally
// fix typo
// 
//    Rev 1.31   26 Mar 1999 15:40:38   sally
// added "L1 Time" unit
// 
//    Rev 1.30   23 Dec 1998 16:32:40   sally
// move "Orbit Period" and "Antenna Spin Rate" from derived L1A to L1A,
// because it returns one single value only, not 100 pulses of values.
// 
//    Rev 1.29   07 Dec 1998 15:42:32   sally
// add sliced power_dn for extraction
// 
//    Rev 1.28   02 Dec 1998 12:54:52   sally
// 
//    Rev 1.27   20 Nov 1998 16:03:26   sally
// change some data types and limit check arrays
// 
//    Rev 1.26   10 Nov 1998 08:51:56   sally
// add delta instrument time because the instrument seems to skip cycle
// 
//    Rev 1.25   09 Nov 1998 11:24:58   sally
// 
//    Rev 1.24   02 Oct 1998 14:23:04   sally
// correct unit type for exciter temp
// 
//    Rev 1.23   18 Aug 1998 15:06:34   sally
// mv mWatts for transmit power to L1ADrvTab.C
// 
//    Rev 1.22   18 Aug 1998 09:17:58   sally
// corrected spelling TRAMSMIT -> TRANSMIT
// 
//    Rev 1.21   18 Aug 1998 08:05:58   daffer
// Corrected mispelled TRANSMIT_PULSE_CHANGE
// 
//    Rev 1.20   13 Aug 1998 16:26:54   sally
// for transmit power: dBm comes from polynomial table, mWatts is derived
// 
//    Rev 1.19   04 Aug 1998 15:59:54   sally
// fixe L1AParTab so that dBm comes from polynomial table and
// mWatts will be calculated
// 
//    Rev 1.18   03 Aug 1998 14:34:26   deliver
// fix yank and put error
// 
//    Rev 1.17   03 Aug 1998 14:20:58   deliver
// fix some units
// 
//    Rev 1.16   03 Aug 1998 14:07:30   sally
// added some units for Barry
// 
//    Rev 1.15   27 Jul 1998 14:00:16   sally
// passing polynomial table to extraction function
// 
//    Rev 1.14   24 Jun 1998 09:46:46   sally
// took out extract Blank
// 
//    Rev 1.13   22 Jun 1998 15:26:02   sally
// change to incorporate Barry's update
// 
//    Rev 1.12   03 Jun 1998 10:09:58   sally
// change parameter names and types due to LP's changes
// 
//    Rev 1.11   28 May 1998 15:51:28   sally
// changed some L1A parameter names
// 
//    Rev 1.10   26 May 1998 16:35:02   daffer
// Changed cmdArgsTable to reflect changes in L1A SIS
// 
//    Rev 1.9   19 May 1998 14:44:02   sally
// updated some parameters according to the new TLM dictionary
// 
//    Rev 1.8   13 May 1998 16:26:56   sally
// changed "time" to "frame_time_secs"
// 
//    Rev 1.7   27 Apr 1998 15:49:44   sally
// .
// 
//    Rev 1.6   06 Apr 1998 16:28:08   sally
// merged with SVT
// 
//    Rev 1.5   23 Mar 1998 15:36:10   sally
// adapt to derived science data
// 
//    Rev 1.4   19 Mar 1998 13:37:10   sally
//  added "days", "hours", "minutes" and "seconds" units
// 
//    Rev 1.3   20 Feb 1998 10:58:28   sally
// L1 to L1A
// 
//    Rev 1.2   17 Feb 1998 14:47:22   sally
//  NOPM
// 
//    Rev 1.1   12 Feb 1998 16:48:28   sally
// add start and end time
// Revision 1.7  1998/02/01 20:21:11  sally
// fixed 2D conversion
//
// Revision 1.6  1998/01/31 00:36:39  sally
// add scale factor
//
// Revision 1.5  1998/01/30 22:29:12  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#include "ParTab.h"
#include "L1AExtract.h"
#include "Print.h"

//-********************************************************************
//     defines a giant table to hold all L1A parameters.
//     Each parameter has up to 5 unit entries
//     which defines how to extract and interpret the data according
//     the unit type.
//-********************************************************************

static const char rcs_id_L1AParTab_C[] = "@(#) $Header$";

const ParTabEntry L1AParTab[] =
{
  { UTC_TIME, "UTC Time", SOURCE_L1A, MEAS_TIME, "frame_time_secs", 7, {
      { UNIT_AUTOTIME, "(auto)",DATA_ITIME, 0, ExtractTaiTime, 0 },
      { UNIT_CODE_A, "Code A",  DATA_ITIME, 0, ExtractTaiTime, pr_itime_codea },
      { UNIT_L1ATIME,"L1 Time", DATA_ITIME, 0, ExtractTaiTime, pr_itime_L1 },
      { UNIT_DAYS,   "days",    DATA_ITIME, 0, ExtractTaiTime, pr_itime_d },
      { UNIT_HOURS,  "hours",   DATA_ITIME, 0, ExtractTaiTime, pr_itime_h },
      { UNIT_MINUTES,"minutes", DATA_ITIME, 0, ExtractTaiTime, pr_itime_m },
      { UNIT_SECONDS,"seconds", DATA_ITIME, 0, ExtractTaiTime, pr_itime_s }
    }
  },
  { TAI_TIME, "TAI Time", SOURCE_L1A, MEAS_TIME, "frame_time_secs", 1, {
      { UNIT_TAI_SECONDS, "seconds after TAI", DATA_FLOAT8, 0,
                                               ExtractData1D, pr_float8_10 },
    }
  },
  { FRAME_TIME, "Frame Time", SOURCE_L1A, MEAS_TIME, "v:frame_time", 7, {
      { UNIT_AUTOTIME, "(auto)",  DATA_ITIME, 0, ExtractL1Time, NULL },
      { UNIT_CODE_A,   "Code A",  DATA_ITIME, 0, ExtractL1Time, pr_itime_codea},
      { UNIT_L1ATIME,  "L1Time",  DATA_ITIME, 0, ExtractL1Time, pr_itime_L1 },
      { UNIT_DAYS,     "days",    DATA_ITIME, 0, ExtractL1Time, pr_itime_d },
      { UNIT_HOURS,    "hours",   DATA_ITIME, 0, ExtractL1Time, pr_itime_h },
      { UNIT_MINUTES,  "minutes", DATA_ITIME, 0, ExtractL1Time, pr_itime_m },
      { UNIT_SECONDS,  "seconds", DATA_ITIME, 0, ExtractL1Time, pr_itime_s }
    }
  },
  { INSTRUMENT_TIME, "Instrument Time", SOURCE_L1A,
                                  MEAS_TIME, "instrument_time", 1, {
      { UNIT_COUNTS, "counts", DATA_FLOAT8, 0, ExtractData1D, pr_float8_10 }
    }
  },
  { DELTA_INSTRUMENT_TIME, "Delta Instrument Time", SOURCE_L1A,
                                  MEAS_TIME, "instrument_time", 1, {
      { UNIT_COUNTS, "counts", DATA_FLOAT8, 0,
                                  ExtractDeltaInstTime, pr_float8_10 }
    }
  },
  { ORBIT_TIME, "Orbit Time", SOURCE_L1A, MEAS_TIME, "orbit_time", 1, {
      { UNIT_COUNTS, "counts", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { X_POS, "X Component of S/C Position", SOURCE_L1A, MEAS_DISTANCE,
                "x_pos", 2, {
      { UNIT_METERS, "meters", DATA_FLOAT4, 0, ExtractData1D, pr_float4_6 },
      { UNIT_KILOMETERS, "km", DATA_FLOAT4, 0, ExtractData1D_m_km, pr_float4_6 }
    }
  },
  { Y_POS, "Y Component of S/C Position", SOURCE_L1A, MEAS_DISTANCE,
                "y_pos", 2, {
      { UNIT_METERS, "meters", DATA_FLOAT4, 0, ExtractData1D, pr_float4_6 },
      { UNIT_KILOMETERS, "km", DATA_FLOAT4, 0, ExtractData1D_m_km, pr_float4_6 }
    }
  },
  { Z_POS, "Z Component of S/C Position", SOURCE_L1A, MEAS_DISTANCE,
                "z_pos", 2, {
      { UNIT_METERS, "meters", DATA_FLOAT4, 0, ExtractData1D, pr_float4_6 },
      { UNIT_KILOMETERS, "km", DATA_FLOAT4, 0, ExtractData1D_m_km, pr_float4_6 }
    }
  },
  { X_VEL, "X Component of S/C Velocity", SOURCE_L1A, MEAS_VELOCITY,
                "x_vel", 2, {
      { UNIT_MPS, "m/s", DATA_FLOAT4, 0, ExtractData1D, pr_float4_6 },
      { UNIT_KMPS, "km/s", DATA_FLOAT4, 0, ExtractData1D_m_km, pr_float4_6 }
    }
  },
  { Y_VEL, "Y Component of S/C Velocity", SOURCE_L1A, MEAS_VELOCITY,
                "y_vel", 2, {
      { UNIT_MPS, "m/s", DATA_FLOAT4, 0, ExtractData1D, pr_float4_6 },
      { UNIT_KMPS, "km/s", DATA_FLOAT4, 0, ExtractData1D_m_km, pr_float4_6 }
    }
  },
  { Z_VEL, "Z Component of S/C Velocity", SOURCE_L1A, MEAS_VELOCITY,
                "z_vel", 2, {
      { UNIT_MPS, "m/s", DATA_FLOAT4, 0, ExtractData1D, pr_float4_6 },
      { UNIT_KMPS, "km/s", DATA_FLOAT4, 0, ExtractData1D_m_km, pr_float4_6 }
    }
  },
  { ROLL, "S/C Roll", SOURCE_L1A, MEAS_ORIENTATION, "roll", 2, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4,
                           0, ExtractData1D_int2_float, pr_float4_6 },
      { UNIT_RADIANS, "radians", DATA_FLOAT4,
                           0, ExtractData1D_int2_float_dtr, pr_float4_6 }
    }
  },
  { PITCH, "S/C Pitch", SOURCE_L1A, MEAS_ORIENTATION, "pitch", 2, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4,
                           0, ExtractData1D_int2_float, pr_float4_6 },
      { UNIT_RADIANS, "radians", DATA_FLOAT4,
                           0, ExtractData1D_int2_float_dtr, pr_float4_6 }
    }
  },
  { YAW, "S/C Yaw", SOURCE_L1A, MEAS_ORIENTATION,
                "yaw", 2, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4,
                           0, ExtractData1D_int2_float, pr_float4_6 },
      { UNIT_RADIANS, "radians", DATA_FLOAT4,
                           0, ExtractData1D_int2_float_dtr, pr_float4_6 }
    }
  },
  { CURRENT_MTLM_TABLE_ID, "Mission TLM Table ID", SOURCE_L1A, MEAS_ID,
                "telemetry_table_id", 1, {
      { UNIT_ID, "id", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { ERROR_FLAGS, "Error Flags", SOURCE_L1A, MEAS_STATUS,
                "status_error_flags", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_binchar }
    }
  },
  { ERROR_FLAGS_00, "Error Flag - Power On Reset", SOURCE_L1A, MEAS_STATUS,
                "status_error_flags", 1, {
      { UNIT_MAP, "1=Event, 0=normal", DATA_UINT1, 0, Extract8Bit0, pr_bit }
    }
  },
  { ERROR_FLAGS_01, "Error Flag - Watch Dog Timeout Reset", SOURCE_L1A,
                MEAS_STATUS, "status_error_flags", 1, {
      { UNIT_MAP, "1=Event, 0=normal", DATA_UINT1, 0, Extract8Bit1, pr_bit }
    }
  },
  { ERROR_FLAGS_02, "Error Flag - Fault Protection Event", SOURCE_L1A,
                MEAS_STATUS, "status_error_flags", 1, {
      { UNIT_MAP, "1=Event, 0=normal", DATA_UINT1, 0, Extract8Bit2, pr_bit }
    }
  },
  { ERROR_FLAGS_03, "Error Flag - MTLM or STIM Error", SOURCE_L1A,
                MEAS_STATUS, "status_error_flags", 1, {
      { UNIT_MAP, "1=Event, 0=normal", DATA_UINT1, 0, Extract8Bit3, pr_bit }
    }
  },
  { ERROR_FLAGS_04, "Error Flag - Missing S/C Time", SOURCE_L1A,
                MEAS_STATUS, "status_error_flags", 1, {
      { UNIT_MAP, "1=Event, 0=normal", DATA_UINT1, 0, Extract8Bit4, pr_bit }
    }
  },
  { ERROR_FLAGS_05, "Error Flag - Pulse Discrete or A2D Timeout", SOURCE_L1A,
                MEAS_STATUS, "status_error_flags", 1, {
      { UNIT_MAP, "1=Event, 0=normal", DATA_UINT1, 0, Extract8Bit5, pr_bit }
    }
  },
  { ERROR_FLAGS_06, "Error Flag - Stale Data Toggle (STLM only)", SOURCE_L1A,
                MEAS_STATUS, "status_error_flags", 1, {
       { UNIT_MAP, "1=Event, 0=normal", DATA_UINT1, 0, Extract8Bit6, pr_bit }
    }
  },
  { ERROR_FLAGS_07, "Error Flag - CDS System Reset", SOURCE_L1A,
				MEAS_STATUS, "status_error_flags", 1, {
      { UNIT_MAP, "1=Event, 0=normal", DATA_UINT1, 0, Extract8Bit7, pr_bit }
    }
  },
  { TABLE_READOUT_TYPE, "Table Readout Type", SOURCE_L1A,
                MEAS_ID, "table_readout_type", 2, {
       { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
       { UNIT_HEX_BYTES, "hex", DATA_UINT1, 0, ExtractData1D, pr_char1x }
    }
  },
  { TABLE_READOUT_OFFSET, "Table Readout Offset", SOURCE_L1A,
            MEAS_QUANTITY, "table_readout_offset", 2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 },
      { UNIT_HEX_BYTES, "hex", DATA_UINT2, 0, ExtractData1D, pr_char2x }
    }
  },
  { TABLE_READOUT_DATA, "Table Readout Data", SOURCE_L1A,
            MEAS_DATA, "table_readout_data", 2, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 },
      { UNIT_HEX_BYTES, "hex", DATA_UINT4, 0, ExtractData1D, pr_char4x }
    }
  },
  { OPERATIONAL_MODE, "Operational Mode", SOURCE_L1A, MEAS_MAP,
                "operational_mode", 2, {
      { UNIT_MAP, "14=WOM, 7=CBM, 112=SBM, 224=ROM", DATA_UINT1,
                0, ExtractData1D, pr_uint1 },
      { UNIT_HEX_BYTES, "0E=WOM, 07=CBM, 70=SBM, E0=ROM", DATA_UINT1,
                0, ExtractData1D, pr_char1x }
    }
  },
  { PRF_COUNT, "PRF Count", SOURCE_L1A, MEAS_QUANTITY, "prf_count", 1, {
      { UNIT_COUNTS, "counts", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { STATUS_TABLE_CHANGE_FLAGS, "Status Table Change Flags",
            SOURCE_L1A, MEAS_STATUS, "status_change_flags", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_binchar2 }
    }
  },
  { STATUS_TABLE_CHANGE_FLAGS_00, "Change Flags - Mode Change",
            SOURCE_L1A, MEAS_STATUS, "status_change_flags", 1, {
      { UNIT_MAP, "1=Changed, 0=Same", DATA_UINT1, 0, Extract16Bit0, pr_bit }
    }
  },
  { STATUS_TABLE_CHANGE_FLAGS_01, "Change Flags - Equator Crossing Missed",
            SOURCE_L1A, MEAS_STATUS, "status_change_flags", 1, {
      { UNIT_MAP, "1=Changed, 0=Same", DATA_UINT1, 0, Extract16Bit1, pr_bit }
    }
  },
  { STATUS_TABLE_CHANGE_FLAGS_02, "Change Flags - Soft Reset",
            SOURCE_L1A, MEAS_STATUS, "status_change_flags", 1, {
      { UNIT_MAP, "1=Changed, 0=Same", DATA_UINT1, 0, Extract16Bit2, pr_bit }
    }
  },
  { STATUS_TABLE_CHANGE_FLAGS_03, "Change Flags - Relay Set/Reset Started",
            SOURCE_L1A, MEAS_STATUS, "status_change_flags", 1, {
      { UNIT_MAP, "1=Changed, 0=Same", DATA_UINT1, 0, Extract16Bit3, pr_bit }
    }
  },
  { STATUS_TABLE_CHANGE_FLAGS_04, "Change Flags - Internal PRF Clock",
            SOURCE_L1A, MEAS_STATUS, "status_change_flags", 1, {
      { UNIT_MAP, "1=from CDS, 0=from SES", DATA_UINT1, 0,
                                     Extract16Bit4, pr_bit }
    }
  },
  { STATUS_TABLE_CHANGE_FLAGS_05,
            "Change Flags - Multi SES Data Loss Fault Detection En/Dis",
            SOURCE_L1A, MEAS_STATUS, "status_change_flags", 1, {
      { UNIT_MAP, "1=Enabled, 0=Disabled", DATA_UINT1, 0, Extract16Bit5, pr_bit }
    }
  },
  { STATUS_TABLE_CHANGE_FLAGS_06,
            "Change Flags - Multi SAS Data Loss Fault Detection En/Dis",
            SOURCE_L1A, MEAS_STATUS, "status_change_flags", 1, {
      { UNIT_MAP, "1=Enabled, 0=Disabled", DATA_UINT1, 0, Extract16Bit6, pr_bit }
    }
  },
  { STATUS_TABLE_CHANGE_FLAGS_07, "Change Flags - Hard Reset",
            SOURCE_L1A, MEAS_STATUS, "status_change_flags", 1, {
      { UNIT_MAP, "1=Changed, 0=Same", DATA_UINT1, 0, Extract16Bit7, pr_bit }
    }
  },
  { STATUS_TABLE_CHANGE_FLAGS_08,
                "Change Flags - SES Suppl Htr Mode Chg Ctrl En/Dis",
            SOURCE_L1A, MEAS_STATUS, "status_change_flags", 1, {
      { UNIT_MAP, "1=Enabled, 0=Disabled", DATA_UINT1, 0, Extract16Bit8, pr_bit }
    }
  },
  { STATUS_TABLE_CHANGE_FLAGS_09,
          "Change Flags - TWTA Low Drive Power Fault Protection En/Dis",
      SOURCE_L1A, MEAS_STATUS, "status_change_flags", 1, {
      { UNIT_MAP, "1=Enabled, 0=Disabled", DATA_UINT1, 0, Extract16Bit9, pr_bit }
    }
  },
  { STATUS_TABLE_CHANGE_FLAGS_10, "Change Flags - TWTA monitor En/Dis",
            SOURCE_L1A, MEAS_STATUS, "status_change_flags", 1, {
      { UNIT_MAP, "1=Enabled, 0=Disabled", DATA_UINT1, 0, Extract16Bit10, pr_bit }
    }
  },
  { STATUS_TABLE_CHANGE_FLAGS_11, "Change Flags - SES Param tables",
            SOURCE_L1A, MEAS_STATUS, "status_change_flags", 1, {
      { UNIT_MAP, "1=Changed, 0=Same", DATA_UINT1, 0, Extract16Bit11, pr_bit }
    }
  },
  { STATUS_TABLE_CHANGE_FLAGS_12, "Change Flags - Range Gate tables",
            SOURCE_L1A, MEAS_STATUS, "status_change_flags", 1, {
      { UNIT_MAP, "1=Changed, 0=Same", DATA_UINT1, 0, Extract16Bit12, pr_bit }
    }
  },
  { STATUS_TABLE_CHANGE_FLAGS_13, "Change Flags - Doppler tables",
            SOURCE_L1A, MEAS_STATUS, "status_change_flags", 1, {
      { UNIT_MAP, "1=Changed, 0=Same", DATA_UINT1, 0, Extract16Bit13, pr_bit }
    }
  },
  { STATUS_TABLE_CHANGE_FLAGS_14, "Change Flags - Serial Tlm tables",
            SOURCE_L1A, MEAS_STATUS, "status_change_flags", 1, {
      { UNIT_MAP, "1=Changed, 0=Same", DATA_UINT1, 0, Extract16Bit14, pr_bit }
    }
  },
  { STATUS_TABLE_CHANGE_FLAGS_15, "Change Flags - Mission Tlm tables",
            SOURCE_L1A, MEAS_STATUS, "status_change_flags", 1, {
      { UNIT_MAP, "1=Changed, 0=Same", DATA_UINT1, 0, Extract16Bit15, pr_bit }
    }
  },
  { ERROR_MSG, "Error Message", SOURCE_L1A, MEAS_STATUS,
                "error_message", 2, {
      { UNIT_HEX_BYTES, "hex", DATA_CHAR2, 0, ExtractData1D, pr_char2x },
      { UNIT_DN, "decimal", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { ERROR_MSG_HISTORY, "Error Message History", SOURCE_L1A,
                MEAS_STATUS, "error_message_history", 2, {
      { UNIT_HEX_BYTES, "hex", DATA_UINT2_5, 0, ExtractData2D_5, pr_char2x_5 },
      { UNIT_DN, "decimal", DATA_UINT2_5, 0, ExtractData2D_5, pr_uint2_5 }
    }
  },
  { VALID_COMMAND_COUNT, "Valid Command Count", SOURCE_L1A,
            MEAS_QUANTITY, "valid_command_count", 1, {
      { UNIT_COUNTS, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { INVALID_COMMAND_COUNT, "Invalid Command Count", SOURCE_L1A,
            MEAS_QUANTITY, "invalid_command_count", 1, {
      { UNIT_COUNTS, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { CAL_PULSE_POS, "Cal Pulse Pos", SOURCE_L1A, MEAS_QUANTITY,
            "specified_cal_pulse_pos", 2, {
      { UNIT_DN, "dn, -1=no cal pulse", DATA_INT1, 0, ExtractData1D,
                                              pr_int1 },
      { UNIT_HEX_BYTES, "hex, FF=no cal pulse", DATA_INT1, 0, ExtractData1D,
                                              pr_char1x }
    }
  },
  { PRF_CYCLE_TIME, "PRF Cycle Time", SOURCE_L1A, MEAS_QUANTITY,
            "prf_cycle_time", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_MS, "ms", DATA_FLOAT4, 1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { RANGE_GATE_A_DELAY, "Range Gate A Delay", SOURCE_L1A, MEAS_QUANTITY,
            "range_gate_a_delay", 3, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_MS, "ms", DATA_FLOAT4, 1, ExtractData1D_uint1_float, pr_float4_6 },
      { UNIT_SECONDS, "seconds", DATA_FLOAT4, 1,
                       ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { RANGE_GATE_A_WIDTH, "Range Gate A Width", SOURCE_L1A, MEAS_QUANTITY,
            "range_gate_a_width", 3, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_MS, "ms", DATA_FLOAT4, 1, ExtractData1D_uint1_float, pr_float4_6 },
      { UNIT_SECONDS, "seconds", DATA_FLOAT4, 1,
                       ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { RANGE_GATE_B_DELAY, "Range Gate B Delay", SOURCE_L1A, MEAS_QUANTITY,
            "range_gate_b_delay", 3, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_MS, "ms", DATA_FLOAT4, 1, ExtractData1D_uint1_float, pr_float4_6 },
      { UNIT_SECONDS, "seconds", DATA_FLOAT4, 1,
                       ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { RANGE_GATE_B_WIDTH, "Range Gate B Width", SOURCE_L1A, MEAS_QUANTITY,
            "range_gate_b_width", 3, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_MS, "ms", DATA_FLOAT4, 1, ExtractData1D_uint1_float, pr_float4_6 },
      { UNIT_SECONDS, "seconds", DATA_FLOAT4, 1,
                       ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { DOPPLER_SHIFT_CMD_PRF1, "Doppler Shift Cmd PRF 1", SOURCE_L1A,
            MEAS_FREQUENCY, "doppler_shift_command_1", 2, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 },
      { UNIT_KHZ, "khz", DATA_FLOAT4, 1, ExtractData1D_uint4_float,pr_float4_6 }
    }
  },
  { DOPPLER_SHIFT_CMD_PRF2, "Doppler Shift Cmd PRF 2", SOURCE_L1A,
            MEAS_FREQUENCY, "doppler_shift_command_2", 2, {
      { UNIT_DN, "decimal", DATA_UINT4, 0, ExtractData1D, pr_uint4 },
      { UNIT_KHZ, "khz", DATA_FLOAT4, 1, ExtractData1D_uint4_float,pr_float4_6 }
    }
  },
  { PULSE_WIDTH, "Pulse Width", SOURCE_L1A, MEAS_QUANTITY, "pulse_width", 3, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_MS, "ms", DATA_FLOAT4, 1, ExtractData1D_uint1_float, pr_float4_6 },
      { UNIT_SECONDS, "seconds", DATA_FLOAT4, 1,
                       ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { RECEIVER_GAIN, "Receiver Gain", SOURCE_L1A, MEAS_QUANTITY,
                 "receiver_gain", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SES_CONFIG_FLAGS, "SES Config Flags", SOURCE_L1A, MEAS_STATUS,
            "ses_configuration_flags", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_binchar }
    }
  },
  { SES_CONFIG_FLAGS_00, "SES Config Flags - Grid", SOURCE_L1A,
            MEAS_STATUS, "ses_configuration_flags", 1, {
      { UNIT_MAP, "1=disable, 0=normal", DATA_UINT1, 0, Extract8Bit0, pr_bit }
    }
  },
  { SES_CONFIG_FLAGS_01, "SES Config Flags - Receive Protect", SOURCE_L1A,
            MEAS_STATUS, "ses_configuration_flags", 1, {
      { UNIT_MAP, "1=on, 0=normal", DATA_UINT1, 0, Extract8Bit1, pr_bit }
    }
  },
  { SES_CONFIG_FLAGS_02, "SES Config Flags - TWT Trip Override", SOURCE_L1A,
            MEAS_STATUS, "ses_configuration_flags", 1, {
      { UNIT_MAP, "1=on, 0=off", DATA_UINT1, 0, Extract8Bit2, pr_bit }
    }
  },
  { SES_CONFIG_FLAGS_03, "SES Config Flags - Modulation", SOURCE_L1A,
            MEAS_STATUS, "ses_configuration_flags", 1, {
      { UNIT_MAP, "1=on, 0=off", DATA_UINT1, 0, Extract8Bit3, pr_bit }
    }
  },
  { SES_CONFIG_FLAGS_04, "SES Config Flags - spare", SOURCE_L1A,
            MEAS_STATUS, "ses_configuration_flags", 1, {
      { UNIT_MAP, "1=on, 0=off", DATA_UINT1, 0, Extract8Bit4, pr_bit }
    }
  },
  { SES_CONFIG_FLAGS_05, "SES Config Flags - spare", SOURCE_L1A,
            MEAS_STATUS, "ses_configuration_flags", 1, {
      { UNIT_MAP, "1=on, 0=off", DATA_UINT1, 0, Extract8Bit5, pr_bit }
    }
  },
  { SES_CONFIG_FLAGS_06, "SES Config Flags - spare", SOURCE_L1A,
            MEAS_STATUS, "ses_configuration_flags", 1, {
      { UNIT_MAP, "1=on, 0=off", DATA_UINT1, 0, Extract8Bit6, pr_bit }
    }
  },
  { SES_CONFIG_FLAGS_07, "SES Config Flags - spare", SOURCE_L1A,
            MEAS_STATUS, "ses_configuration_flags", 1, {
      { UNIT_MAP, "1=on, 0=off", DATA_UINT1, 0, Extract8Bit7, pr_bit }
    }
  },
  { SES_DATA_OVERRUN_COUNT, "SES Data Overrun Count", SOURCE_L1A,
            MEAS_QUANTITY, "ses_data_overrun_count", 1, {
      { UNIT_COUNTS, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SES_DATA_UNDERRUN_COUNT, "SES Data Underrun Count", SOURCE_L1A,
            MEAS_QUANTITY, "ses_data_underrun_count", 1, {
      { UNIT_COUNTS, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { PREDICT_ANT_POS_COUNT, "Predicted Ant Pos Count", SOURCE_L1A,
            MEAS_QUANTITY, "pred_antenna_pos_count", 1, {
      { UNIT_COUNTS, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { RUNNING_ERROR_COUNT, "Running Error Count", SOURCE_L1A,
            MEAS_QUANTITY, "running_error_count", 1, {
      { UNIT_COUNTS, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { SES_RESET_POSITION, "SES Reset Position", SOURCE_L1A,
            MEAS_QUANTITY, "ses_reset_position", 1, {
      { UNIT_COUNTS, "dn", DATA_INT1, 0, ExtractData1D, pr_int1 }
    }
  },
  { DOPPLER_ORBIT_STEP, "Doppler Orbit Step", SOURCE_L1A,
            MEAS_QUANTITY, "doppler_orbit_step", 1, {
      { UNIT_COUNTS, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { PRF_ORBIT_STEP_CHANGE, "PRF Orbit Step Change", SOURCE_L1A,
            MEAS_QUANTITY, "prf_orbit_step_change", 1, {
      { UNIT_COUNTS, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { CMD_HISTORY, "CMD History Queue", SOURCE_L1A,
            MEAS_DATA, "cmd_history_queue", 2, {
      { UNIT_DN, "decimal", DATA_UINT2_4, 0, ExtractData2D_4, pr_uint2_4 },
      { UNIT_HEX_BYTES, "hex bytes", DATA_UINT2_4, 0, ExtractData2D_4,
                                                           pr_char2x_4 }
    }
  },
  { CALC_ANT_MAX_GRP_CNT, "Longest Continuous Sequence of PRFs", SOURCE_L1A,
            MEAS_QUANTITY, "calc_ant_max_grp_count", 1, {
      { UNIT_COUNTS, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { ADEOS_TIME, "VTCW", SOURCE_L1A, MEAS_QUANTITY, "vtcw", 1, {
      { UNIT_TICKS, "ticks", DATA_FLOAT8, 0, ExtractData1D, pr_float8_10 }
    }
  },
  { CORRES_INSTR_TIME, "Corres Instr Time", SOURCE_L1A, MEAS_TIME,
                     "corres_instr_time", 1, {
      { UNIT_TICKS, "ticks", DATA_FLOAT8, 0, ExtractData1D, pr_float8_10 }
    }
  },
  { FSW_MISSION_NO, "FSW Mission No", SOURCE_L1A,
            MEAS_QUANTITY, "fsw_mission_version_num", 1, {
      { UNIT_MAP, "0=QSCAT, 1=SeaWinds", DATA_UINT1, 0, Extract8Bit5_7,
                                                pr_uint1 }
    }
  },
  { FSW_MISSION_VERSION_NO, "FSW Version No", SOURCE_L1A,
            MEAS_QUANTITY, "fsw_mission_version_num", 1, {
      { UNIT_DN, "version number", DATA_UINT1, 0, Extract8Bit0_4,
                                                pr_uint1 }
    }
  },
  { FSW_BUILD_NO, "FSW Build No", SOURCE_L1A,
            MEAS_QUANTITY, "fsw_build_number", 1, {
      { UNIT_COUNTS, "0=initial, 1=A, 2=B,...", DATA_UINT1,
                                    0, ExtractData1D, pr_uint1 }
    }
  },
  { PBI_FLAG, "PBI Flag", SOURCE_L1A, MEAS_STATUS, "pbi_flag", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { FILL_DATA, "Fill Data", SOURCE_L1A, MEAS_QUANTITY, "fill", 1, {
      { UNIT_DN, "dn", DATA_CHAR32, 0, ExtractData2D_8, pr_char32x }
    }
  },
  { PSU_ELEC_BUS_VOLT, "PSU Elec Bus Volt", SOURCE_L1A, MEAS_VOLTAGE,
            "psu_elec_bus_volt", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                            pr_float4_6 }
    }
  },
  { CDS_CURRENT, "CDS Current", SOURCE_L1A, MEAS_CURRENT,
            "cds_current", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                            pr_float4_6 }
    }
  },
  { SES_A_CURRENT, "SES A Current", SOURCE_L1A, MEAS_CURRENT,
            "ses_a_current", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                            pr_float4_6 }
    }
  },
  { SES_B_CURRENT, "SES B Current", SOURCE_L1A, MEAS_CURRENT,
            "ses_b_current", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                            pr_float4_6 }
    }
  },
  { TWTA_1_CURRENT, "TWTA 1 Current", SOURCE_L1A, MEAS_CURRENT,
            "twta_1_current", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                            pr_float4_6 }
    }
  },
  { TWTA_2_CURRENT, "TWTA 2 Current", SOURCE_L1A, MEAS_CURRENT,
            "twta_2_current", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                            pr_float4_6 }
    }
  },
  { SAS_A_CURRENT, "SAS A Current", SOURCE_L1A, MEAS_CURRENT,
            "sas_a_current", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                            pr_float4_6 }
    }
  },
  { SAS_B_CURRENT, "SAS B Current", SOURCE_L1A, MEAS_CURRENT,
            "sas_b_current", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                            pr_float4_6 }
    }
  },
  { PCU_SEC_VOLT_P12, "PCU Sec Volt +12", SOURCE_L1A, MEAS_VOLTAGE,
            "pcu_sec_volt_p12", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                           pr_float4_6 }
    }
  },
  { PCU_SEC_VOLT_N12, "PCU Sec Volt -12", SOURCE_L1A, MEAS_VOLTAGE,
            "pcu_sec_volt_n12", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                           pr_float4_6 }
    }
  },
  { PCU_SEC_VOLT_P30, "PCU Sec Volt +30", SOURCE_L1A, MEAS_VOLTAGE,
            "pcu_sec_volt_p30", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                           pr_float4_6 }
    }
  },
  { PCU_SEC_VOLT_VME_P3_3, "PCU Sec Volt VME +3.3", SOURCE_L1A,
            MEAS_VOLTAGE, "pcu_sec_volt_vme_p3", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                           pr_float4_6 }
    }
  },
  { PCU_ELEC_BUS_VOLT_P5_ISO, "PCU Elec Bus Volt +5 ISO", SOURCE_L1A,
            MEAS_VOLTAGE, "pcu_elec_bus_volt", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                           pr_float4_6 }
    }
  },
  { IDP_A_TEMP, "IDP-A Temperature", SOURCE_L1A, MEAS_TEMPERATURE,
            "idp_a_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { IDP_B_TEMP, "IDP-B Temperature", SOURCE_L1A, MEAS_TEMPERATURE,
            "idp_b_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { PSU_TEMP, "PSU Temperature", SOURCE_L1A, MEAS_TEMPERATURE,
            "psu_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { RELAY_STATUS, "Relay Status", SOURCE_L1A, MEAS_STATUS, "relay_status", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_binchar2 }
    }
  },
  { SAS_A_SPIN_RATE, "SAS-A Spin Rate", SOURCE_L1A, MEAS_STATUS,
            "relay_status", 1, {
      { UNIT_MAP, "0 = 19.8 RPM, 1 = 18.0 RPM", DATA_UINT1, 0,
                                            Extract16Bit0, pr_bit }
    }
  },
  { K21_SES_SUPP_HTR_PWR, "K21 SES Supp Htr Pwr", SOURCE_L1A,
            MEAS_STATUS, "relay_status", 1, {
      { UNIT_MAP, "0=set, 1=reset", DATA_UINT1, 0, Extract16Bit1, pr_bit }
    }
  },
  { K22_SES_SUPP_HTR_PWR, "K22 SES Supp Htr Pwr", SOURCE_L1A,
            MEAS_STATUS, "relay_status", 1, {
      { UNIT_MAP, "0=set, 1=reset", DATA_UINT1, 0, Extract16Bit2, pr_bit }
    }
  },
  { K23_SPARE, "K23 Spare", SOURCE_L1A, MEAS_STATUS, "relay_status", 1, {
      { UNIT_MAP, "0=set, 1=reset", DATA_UINT1, 0, Extract16Bit3, pr_bit }
    }
  },
  { SAS_B_SPIN_RATE, "SAS-B Spin Rate", SOURCE_L1A,
            MEAS_STATUS, "relay_status", 1, {
      { UNIT_MAP, "0 = 19.8 RPM, 1 = 18.0 RPM", DATA_UINT1, 0,
                                             Extract16Bit4, pr_bit }
    }
  },
  { K24_SPARE, "K24 Spare", SOURCE_L1A, MEAS_STATUS, "relay_status", 1, {
      { UNIT_MAP, "0=set, 1=reset", DATA_UINT1, 0, Extract16Bit5, pr_bit }
    }
  },
  { K16_SES_SELECT, "K16 SES Select", SOURCE_L1A,
            MEAS_STATUS, "relay_status", 1, {
      { UNIT_MAP, "0=set, 1=reset", DATA_UINT1, 0, Extract16Bit6, pr_bit }
    }
  },
  { K19_SAS_SELECT, "K19 SAS Select", SOURCE_L1A,
            MEAS_STATUS, "relay_status", 1, {
      { UNIT_MAP, "0=set, 1=reset", DATA_UINT1, 0, Extract16Bit7, pr_bit }
    }
  },
  { K20_SAS_SELECT, "K20 SAS Select", SOURCE_L1A,
            MEAS_STATUS, "relay_status", 1, {
      { UNIT_MAP, "0=set, 1=reset", DATA_UINT1, 0, Extract16Bit8, pr_bit }
    }
  },
  { K9_TWTA_POWER, "K9 TWTA Power", SOURCE_L1A,
            MEAS_STATUS, "relay_status", 1, {
      { UNIT_MAP, "0=set, 1=reset", DATA_UINT1, 0, Extract16Bit9, pr_bit }
    }
  },
  { K10_TWTA_POWER, "K10 TWTA Power", SOURCE_L1A,
            MEAS_STATUS, "relay_status", 1, {
      { UNIT_MAP, "0=set, 1=reset", DATA_UINT1, 0, Extract16Bit10, pr_bit }
    }
  },
  { K11_TWTA_SELECT, "K11 TWTA Select", SOURCE_L1A,
            MEAS_STATUS, "relay_status", 1, {
      { UNIT_MAP, "0=set, 1=reset", DATA_UINT1, 0, Extract16Bit11, pr_bit }
    }
  },
  { K12_TWTA_SELECT, "K12 TWTA Select", SOURCE_L1A,
            MEAS_STATUS, "relay_status", 1, {
      { UNIT_MAP, "0=set, 1=reset", DATA_UINT1, 0, Extract16Bit12, pr_bit }
    }
  },
  { K25_SPARE, "K25 Spare", SOURCE_L1A,
            MEAS_STATUS, "relay_status", 1, {
      { UNIT_MAP, "0=set, 1=reset", DATA_UINT1, 0, Extract16Bit13, pr_bit }
    }
  },
  { K26_SPARE, "K26 Spare", SOURCE_L1A,
            MEAS_STATUS, "relay_status", 1, {
      { UNIT_MAP, "0=set, 1=reset", DATA_UINT1, 0, Extract16Bit14, pr_bit }
    }
  },
  { K15_SES_SELECT, "K15 SES Select", SOURCE_L1A,
            MEAS_STATUS, "relay_status", 1, {
      { UNIT_MAP, "0=set, 1=reset", DATA_UINT1, 0, Extract16Bit15, pr_bit }
    }
  },
  { EA_A_MOTOR_CURRENT, "EA-A Motor Current", SOURCE_L1A, MEAS_CURRENT,
            "ea_a_motor_current", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                          pr_float4_6 }
    }
  },
  { EA_A_SEC_VOLT_P5, "EA-A Sec Volt +5 VDC", SOURCE_L1A, MEAS_VOLTAGE,
            "ea_a_sec_volt_p5", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                          pr_float4_6 }
    }
  },
  { EA_A_SEC_VOLT_P14, "EA-A SAA Sec Volt +14 VDC", SOURCE_L1A, MEAS_VOLTAGE,
            "ea_a_sec_volt_p14", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                          pr_float4_6 }
    }
  },
  { EA_A_SPIN_RATE, "EA-A Spin Rate", SOURCE_L1A, MEAS_RATE,
            "ea_a_spin_rate", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_RPM, "RPM", DATA_FLOAT4, 1, ExtractData1D_uint1_float, pr_float4_6}
    }
  },
  { EA_A_SAA_TORQUE_CMD, "EA-A SAA Torque CMD", SOURCE_L1A, MEAS_ADDRESS,
            "ea_a_saa_torque_cmd", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_OZIN, "oz-in", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                          pr_float4_6 }
    }
  },
  { EA_B_MOTOR_CURRENT, "EA-B Motor Current", SOURCE_L1A, MEAS_CURRENT,
            "ea_b_motor_current", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                          pr_float4_6 }
    }
  },
  { EA_B_SEC_VOLT_P5, "EA-B Sec Volt +5 VDC", SOURCE_L1A, MEAS_VOLTAGE,
            "ea_b_sec_volt_p5", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                          pr_float4_6 }
    }
  },
  { EA_B_SEC_VOLT_P14, "EA-B SAA Sec Volt +14 VDC", SOURCE_L1A, MEAS_VOLTAGE,
            "ea_b_sec_volt_p14", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                          pr_float4_6 }
    }
  },
  { EA_B_SPIN_RATE, "EA-B Spin Rate", SOURCE_L1A, MEAS_RATE,
            "ea_b_spin_rate", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_RPM, "RPM", DATA_FLOAT4, 1, ExtractData1D_uint1_float, pr_float4_6}
    }
  },
  { EA_B_SAA_TORQUE_CMD, "EA-B SAA Torque CMD", SOURCE_L1A, MEAS_ADDRESS,
            "ea_b_saa_torque_cmd", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_OZIN, "oz-in", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                          pr_float4_6 }
    }
  },
  { DRIVE_MOTOR_TEMP, "Drive Motor Temp", SOURCE_L1A, MEAS_TEMPERATURE,
            "drive_motor_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                          pr_float4_6 }
    }
  },
  { EA_A_PWR_SUPPLY_TEMP, "EA-A Pwr Supply Temp", SOURCE_L1A,
            MEAS_TEMPERATURE, "ea_a_power_supply_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                          pr_float4_6 }
    }
  },
  { EA_B_PWR_SUPPLY_TEMP, "EA-B Pwr Supply Temp", SOURCE_L1A,
            MEAS_TEMPERATURE, "ea_b_power_supply_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                          pr_float4_6 }
    }
  },
  { DUPLEX_BEARING_TEMP, "Duplex Bearing Temp", SOURCE_L1A,
            MEAS_TEMPERATURE, "duplex_bearing_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                          pr_float4_6 }
    }
  },
  { SIMPLEX_BEARING_TEMP, "Simplex Bearing Temp", SOURCE_L1A,
            MEAS_TEMPERATURE, "simplex_bearing_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                          pr_float4_6 }
    }
  },
  { RJ_TEMP, "RJ Temp", SOURCE_L1A, MEAS_TEMPERATURE, "rj_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                          pr_float4_6 }
    }
  },
  { A2D_P12V_XCPL, "A2D P12V xcpl", SOURCE_L1A, MEAS_VOLTAGE,
            "a2d_p12v_xcpl", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                          pr_float4_6 }
    }
  },
  { TWT_1_BODY_REG_VOLT, "TWT 1 Body Reg Volt", SOURCE_L1A,
        MEAS_VOLTAGE, "twt1_body_reg_volt", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                          pr_float4_6 }
    }
  },
  { TWT_1_ION_PUMP_CURRENT, "TWT 1 Ion Pump Current", SOURCE_L1A,
            MEAS_CURRENT, "twt1_ion_pump_current", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_UA, "uA", DATA_FLOAT4, 1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { TWT_1_BODY_CURRENT, "TWT 1 Body Current", SOURCE_L1A,
            MEAS_CURRENT, "twt1_body_current", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_MA, "mA", DATA_FLOAT4, 1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { TWT_1_DRIVE_PWR, "TWT 1 Drive Power", SOURCE_L1A,
            MEAS_POWER, "twt1_drive_power", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DBM, "dBm", DATA_FLOAT4, 1, ExtractData1D_uint1_float, pr_float4_6}
    }
  },
  { TWT_2_BODY_REG_VOLT, "TWT 2 Body Reg Volt", SOURCE_L1A,
        MEAS_VOLTAGE, "twt2_body_reg_volt", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4, 1, ExtractData1D_uint1_float,
                                          pr_float4_6 }
    }
  },
  { TWT_2_ION_PUMP_CURRENT, "TWT 2 Ion Pump Current", SOURCE_L1A,
            MEAS_CURRENT, "twt2_ion_pump_current", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_UA, "uA", DATA_FLOAT4, 1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { TWT_2_BODY_CURRENT, "TWT 2 Body Current", SOURCE_L1A,
            MEAS_CURRENT, "twt2_body_current", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_MA, "mA", DATA_FLOAT4, 1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { TWT_2_DRIVE_PWR, "TWT 2 Drive Power", SOURCE_L1A, MEAS_POWER,
            "twt2_drive_power", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DBM, "dBm", DATA_FLOAT4, 1, ExtractData1D_uint1_float, pr_float4_6}
    }
  },
  { TRANSMIT_PWR_A, "Transmit Power A", SOURCE_L1A, MEAS_POWER,
            "transmit_power_a", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DBM, "dBm", DATA_FLOAT4, 1,
                                ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { TRANSMIT_PWR_B, "Transmit Power B", SOURCE_L1A, MEAS_POWER,
            "transmit_power_b", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DBM, "dBm", DATA_FLOAT4, 1,
                                ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { PWR_CONVERT_CURRENT, "PWR Convert Current", SOURCE_L1A,
            MEAS_CURRENT, "power_convert_current", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DBM, "dBm", DATA_FLOAT4, 1,
                          ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { PRECISION_COUPLER_TEMP, "Precision Coupler Temp", SOURCE_L1A,
            MEAS_TEMPERATURE, "precision_coupler_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                               1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { TWT_1_HVPS_CHASSIS_TEMP, "TWT 1 HVPS Chassis Temp", SOURCE_L1A,
            MEAS_TEMPERATURE, "twt1_hvps_chassis_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                               1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { TWT_1_BASE_TEMP, "TWT 1 Base Temp", SOURCE_L1A, MEAS_TEMPERATURE,
            "twt1_base_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                               1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { TWT_2_HVPS_CHASSIS_TEMP, "TWT 2 HVPS Chassis Temp", SOURCE_L1A,
            MEAS_TEMPERATURE, "twt2_hvps_chassis_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                               1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { TWT_2_BASE_TEMP, "TWT 2 Base Temp", SOURCE_L1A, MEAS_TEMPERATURE,
            "twt2_base_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                               1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { RCV_PROTECT_SW_TEMP, "RCV Protect SW Temp", SOURCE_L1A,
            MEAS_TEMPERATURE, "rcv_protect_sw_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                               1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { POWER_CONVERTER_TEMP, "Power Converter Temp", SOURCE_L1A,
            MEAS_TEMPERATURE, "power_converter_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                               1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { GAIN_ATTEN_TEMP, "Gain Atten Temp", SOURCE_L1A, MEAS_TEMPERATURE,
            "gain_atten_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                               1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { BEAM_SELECT_SW_TEMP, "Beam Select SW Temp", SOURCE_L1A,
            MEAS_TEMPERATURE, "beam_select_sw_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                               1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { SCP_TEMP, "SCP Temp", SOURCE_L1A, MEAS_TEMPERATURE, "scp_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                               1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { RECEIVER_TEMP, "Receiver Temp", SOURCE_L1A, MEAS_TEMPERATURE,
            "receiver_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                               1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { EXCITER_A_TEMP, "Exciter A Temp", SOURCE_L1A, MEAS_TEMPERATURE,
            "exciter_a_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                               1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { EXCITER_B_TEMP, "Exciter B Temp", SOURCE_L1A, MEAS_TEMPERATURE,
            "exciter_b_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                               1, ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { DISCRETE_STATUS_1, "Discrete Status 1", SOURCE_L1A, MEAS_STATUS,
            "eng_status_c1", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_binchar }
    }
  },
  { DISCRETE_STATUS_1_00, "Beam Select", SOURCE_L1A, MEAS_STATUS,
            "eng_status_c1", 1, {
      { UNIT_MAP, "1=xmit A/rx B, 0=xmit B/rx A", DATA_UINT1,
                0, Extract8Bit0, pr_bit }
    }
  },
  { DISCRETE_STATUS_1_01, "PLL Out of Lock", SOURCE_L1A, MEAS_STATUS,
            "eng_status_c1", 1, {
      { UNIT_MAP, "1=out of lock, 0=normal", DATA_UINT1, 0, Extract8Bit1,
                                         pr_bit }
    }
  },
  { DISCRETE_STATUS_1_02, "TWT 2 Body Overcurrent Trip",
            SOURCE_L1A, MEAS_STATUS, "eng_status_c1", 1, {
      { UNIT_MAP, "1=tripped, 0=not tripped", DATA_UINT1, 0, Extract8Bit2,
                                         pr_bit }
    }
  },
  { DISCRETE_STATUS_1_03, "TWT 2 Undervoltage Trip", SOURCE_L1A, MEAS_STATUS,
            "eng_status_c1", 1, {
      { UNIT_MAP, "1=tripped, 0=not tripped", DATA_UINT1,
                                         0, Extract8Bit3, pr_bit }
    }
  },
  { DISCRETE_STATUS_1_04, "TWT 2 Cnvrt Overcurrent Trip", SOURCE_L1A,
            MEAS_STATUS, "eng_status_c1", 1,{
      { UNIT_MAP, "1=tripped, 0=not tripped", DATA_UINT1,
                                         0, Extract8Bit4, pr_bit }
    }
  },
  { DISCRETE_STATUS_1_05, "TWT 1 Body Overcurrent Trip",
            SOURCE_L1A, MEAS_STATUS, "eng_status_c1", 1, {
      { UNIT_MAP, "1=tripped, 0=not tripped", DATA_UINT1,
                                         0, Extract8Bit5, pr_bit }
    }
  },
  { DISCRETE_STATUS_1_06, "TWT 1 Undervoltage Trip", SOURCE_L1A,
            MEAS_STATUS, "eng_status_c1", 1, {
      { UNIT_MAP, "1=tripped, 0=not tripped", DATA_UINT1,
                                         0, Extract8Bit6, pr_bit }
    }
  },
  { DISCRETE_STATUS_1_07, "TWT 1 Cnvrt OC Trip Status", SOURCE_L1A,
            MEAS_STATUS, "eng_status_c1", 1, {
      { UNIT_MAP, "1=tripped, 0=not tripped", DATA_UINT1,
                                         0, Extract8Bit7, pr_bit }
    }
  },
  { DISCRETE_STATUS_2, "Discrete Status 2", SOURCE_L1A, MEAS_STATUS,
            "eng_status_c2", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_binchar }
    }
  },
  { DISCRETE_STATUS_2_00, "TWT Body OC Trip Control", SOURCE_L1A, MEAS_STATUS,
            "eng_status_c2", 1, {
      { UNIT_MAP, "0=OK, 1=tripped???", DATA_UINT1, 0, Extract8Bit0, pr_bit }
    }
  },
  { DISCRETE_STATUS_2_03, "Receive Protect", SOURCE_L1A, MEAS_STATUS,
            "eng_status_c2", 1, {
      { UNIT_MAP, "1=protected, 0=not protected", DATA_UINT1,
                0, Extract8Bit3, pr_bit }
    }
  },
  { DISCRETE_STATUS_2_04, "TRS Cmd Success", SOURCE_L1A, MEAS_STATUS,
            "eng_status_c2", 1, {
            { UNIT_MAP, "1=normal, 0=failure", DATA_UINT1,
                0, Extract8Bit4, pr_bit }
        }
  },
  { DISCRETE_STATUS_3, "Discrete Status 3", SOURCE_L1A, MEAS_STATUS,
            "eng_status_c3", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_binchar }
    }
  },
  { DISCRETE_STATUS_3_00, "Serial Port Parity Error", SOURCE_L1A,
            MEAS_STATUS, "eng_status_c3", 1, {
      { UNIT_MAP, "1=Yes, 0=No", DATA_UINT1, 0, Extract8Bit0, pr_bit }
    }
  },
  { DISCRETE_STATUS_3_01, "Reset Event", SOURCE_L1A, MEAS_STATUS,
            "eng_status_c3", 1, {
      { UNIT_MAP, "1=Yes, 0=No", DATA_UINT1, 0, Extract8Bit1, pr_bit }
    }
  },
  { DISCRETE_STATUS_3_02, "ROM Start Up Error", SOURCE_L1A, MEAS_STATUS,
            "eng_status_c3", 1, {
      { UNIT_MAP, "1=Yes, 0=No", DATA_UINT1, 0, Extract8Bit2, pr_bit }
    }
  },
  { DISCRETE_STATUS_3_03, "RAM Start Up Err", SOURCE_L1A, MEAS_STATUS,
            "eng_status_c3", 1, {
      { UNIT_MAP, "1=Yes, 0=No", DATA_UINT1, 0, Extract8Bit3, pr_bit }
    }
  },
  { DISCRETE_STATUS_3_04, "Watch Dog Timer Event", SOURCE_L1A,
            MEAS_STATUS, "eng_status_c3", 1, {
      { UNIT_MAP, "1=Yes, 0=No", DATA_UINT1, 0, Extract8Bit4, pr_bit }
    }
  },
  { TRS_STATUS_COUNT, "TRS Status Count", SOURCE_L1A, MEAS_STATUS,
            "eng_status_c3", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, Extract8Bit5_7, pr_uint1 }
    }
  },
  { SES_DATA_ERROR_FLAGS, "SES Data Error Flags", SOURCE_L1A,
            MEAS_STATUS, "ses_data_error_flags", 1, {
      { UNIT_HEX_BYTES, "hex bytes", DATA_CHAR13, 0, ExtractData2D_13,
                                     pr_char13x },
    }
  },
  { CDS_MEMORY_DUMP_ADDR, "CDS Memory Dump Addr", SOURCE_L1A,
            MEAS_STATUS, "cds_memory_dump_addr", 2, {
      { UNIT_DEC_ADDR, "decimal", DATA_UINT4, 0, ExtractData1D, pr_uint4 },
      { UNIT_HEX_ADDR, "hex", DATA_UINT4, 0, ExtractData1D, pr_char4x }
    }
  },
  { CDS_MEMORY_DUMP_DATA, "CDS Memory Dump Data", SOURCE_L1A,
            MEAS_DATA, "cds_memory_dump_data", 2, {
      { UNIT_DN, "dn", DATA_UINT4_4, 0, ExtractData2D_4, pr_uint4_4 },
      { UNIT_HEX_BYTES, "hex bytes", DATA_UINT4_4,
                                0, ExtractData2D_4, pr_char4x_4 },
    }
  },
  { SES_MEMORY_DUMP_ADDR, "SES Memory Dump Addr", SOURCE_L1A,
            MEAS_ADDRESS, "ses_memory_dump_addr", 2, {
      { UNIT_DEC_ADDR, "decimal", DATA_UINT2, 0, ExtractData1D, pr_uint2 },
      { UNIT_HEX_ADDR, "hex", DATA_UINT2, 0, ExtractData1D, pr_char2x }
    }
  },
  { SES_MEMORY_DUMP_DATA, "SES Memory Dump Data", SOURCE_L1A,
            MEAS_DATA, "ses_memory_dump_data", 2, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 },
      { UNIT_HEX_BYTES, "hex bytes", DATA_UINT4, 0, ExtractData1D, pr_char4x },
    }
  },
  { PCD_ENTRY, "Payload Correction Data Entry", SOURCE_L1A,
            MEAS_DATA, "pcd_entry", 1, {
      { UNIT_HEX_BYTES, "hex bytes", DATA_UINT2_2_8,
                               0, ExtractData3D_2_8, pr_char2x_2_8 },
    }
  },
  { PRF_CYCLE_TIME_EU, "PRF Cycle Time EU", SOURCE_L1A,
            MEAS_TIME, "prf_cycle_time_eu", 1, {
      { UNIT_SECONDS, "seconds", DATA_FLOAT4,
                               0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { RANGE_GATE_DELAY_INNER, "Range Gate Delay Inner", SOURCE_L1A,
            MEAS_TIME, "range_gate_delay_inner", 1, {
      { UNIT_SECONDS, "seconds", DATA_FLOAT4,
                               0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { RANGE_GATE_DELAY_OUTER, "Range Gate Delay Outer", SOURCE_L1A,
            MEAS_TIME, "range_gate_delay_outer", 1, {
      { UNIT_SECONDS, "seconds", DATA_FLOAT4,
                               0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { RANGE_GATE_WIDTH_INNER, "Range Gate Width Inner", SOURCE_L1A,
            MEAS_TIME, "range_gate_width_inner", 1, {
      { UNIT_SECONDS, "seconds", DATA_FLOAT4,
                               0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { RANGE_GATE_WIDTH_OUTER, "Range Gate Width Outer", SOURCE_L1A,
            MEAS_TIME, "range_gate_width_outer", 1, {
      { UNIT_SECONDS, "seconds", DATA_FLOAT4,
                               0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { TRANSMIT_PULSE_WIDTH, "Transmit Pulse Width", SOURCE_L1A,
            MEAS_TIME, "transmit_pulse_width", 1, {
      { UNIT_SECONDS, "seconds", DATA_FLOAT4,
                               0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { TRUE_CAL_PULSE_POS, "True Cal Pulse Pos", SOURCE_L1A,
            MEAS_STATUS, "true_cal_pulse_pos", 1, {
      { UNIT_DN, "dn", DATA_INT1, 0, ExtractData1D, pr_int1 }
    }
  },
  { TRANSMIT_POWER_INNER, "Transmit Power Inner", SOURCE_L1A,
            MEAS_POWER, "transmit_power_inner", 1, {
      { UNIT_DBM, "dBm", DATA_FLOAT4,
                               0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { TRANSMIT_POWER_OUTER, "Transmit Power Outer", SOURCE_L1A,
            MEAS_POWER, "transmit_power_outer", 1, {
      { UNIT_DBM, "dBm", DATA_FLOAT4,
                               0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { PRECISION_COUPLER_TEMP_EU, "Precision Coupler Temp EU", SOURCE_L1A,
            MEAS_TEMPERATURE, "precision_coupler_temp_eu", 1, {
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                               0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { RCV_PROTECT_SW_TEMP_EU, "RCV Protect SW Temp EU", SOURCE_L1A,
            MEAS_TEMPERATURE, "rcv_protect_sw_temp_eu", 1, {
            { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                               0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { BEAM_SELECT_SW_TEMP_EU, "Beam Select SW Temp EU", SOURCE_L1A,
            MEAS_TEMPERATURE, "beam_select_sw_temp_eu", 1, {
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                               0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { RECEIVER_TEMP_EU, "Receiver Temp EU", SOURCE_L1A,
            MEAS_TEMPERATURE, "receiver_temp_eu", 1, {
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                               0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { ANTENNA_POS, "Antenna Pos 1 DN", SOURCE_L1A, MEAS_DATA,
                "antenna_position", 1, {
      { UNIT_DN, "dn", DATA_UINT2_100, 0, ExtractData2D_100, pr_uint2_100 }
    }
  },
  { LOOP_BACK_CAL_A_POWER, "Loop Cal Pulse A Power", SOURCE_L1A,
            MEAS_QUANTITY, "loop_back_cal_A_power", 2, {
      { UNIT_DN, "dn", DATA_UINT4_12, 0, ExtractData2D_12, pr_uint4_12 },
      { UNIT_DB_DN, "dBdn", DATA_FLOAT4_12,
                           0, ExtractFloat4_12_dB, pr_12float4_6 }
    }
  },
  { LOOP_BACK_CAL_B_POWER, "Loop Cal Pulse B Power", SOURCE_L1A,
            MEAS_QUANTITY, "loop_back_cal_B_power", 2, {
      { UNIT_DN, "dn", DATA_UINT4_12,
                           0, ExtractData2D_12, pr_uint4_12 },
      { UNIT_DB_DN, "dBdn", DATA_FLOAT4_12,
                           0, ExtractFloat4_12_dB, pr_12float4_6 }
    }
  },
  { LOOP_BACK_CAL_NOISE, "Loop Cal Pulse Noise", SOURCE_L1A,
            MEAS_QUANTITY, "loop_back_cal_noise", 2, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 },
      { UNIT_DB_DN, "dBdn", DATA_FLOAT4, 0, ExtractFloat4_dB, pr_float4_6 }
    }
  },
  { LOAD_CAL_A_POWER, "Cold Load Cal Pulse A Power", SOURCE_L1A,
            MEAS_QUANTITY, "load_cal_A_power", 2, {
      { UNIT_DN, "dn", DATA_UINT4_12,
                           0, ExtractData2D_12, pr_uint4_12 },
      { UNIT_DB_DN, "dBdn", DATA_FLOAT4_12,
                           0, ExtractFloat4_12_dB, pr_12float4_6 }
    }
  },
  { LOAD_CAL_B_POWER, "Cold Load Cal Pulse B Power", SOURCE_L1A,
            MEAS_QUANTITY, "load_cal_B_power", 2, {
      { UNIT_DN, "dn", DATA_UINT4_12,
                           0, ExtractData2D_12, pr_uint4_12 },
      { UNIT_DB_DN, "dBdn", DATA_FLOAT4_12,
                           0, ExtractFloat4_12_dB, pr_12float4_6 }
    }
  },
  { LOAD_CAL_NOISE, "Cold Load Cal Pulse Noise", SOURCE_L1A,
            MEAS_QUANTITY, "load_cal_noise", 2, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 },
      { UNIT_DB_DN, "dBdn", DATA_FLOAT4, 0, ExtractFloat4_dB, pr_float4_6 }
    }
  },
  { POWER_DN, "Sliced Power Data", SOURCE_L1A,
            MEAS_QUANTITY, "power_dn", 1, {
      { UNIT_DN, "dn", DATA_UINT4_100_12,
                                0, ExtractData3D_100_12, pr_uint4_100_12 }
    }
  },
  { POWER_DN_SLICE_1, "Power Data - Slice 1", SOURCE_L1A,
            MEAS_QUANTITY, "power_dn", 1, {
      { UNIT_DN, "dn", DATA_UINT4_100, 0, ExtractPowerDnSlice1, pr_uint4_100 }
    }
  },
  { POWER_DN_SLICE_2, "Power Data - Slice 2", SOURCE_L1A,
            MEAS_QUANTITY, "power_dn", 1, {
      { UNIT_DN, "dn", DATA_UINT4_100, 0, ExtractPowerDnSlice2, pr_uint4_100 }
    }
  },
  { POWER_DN_SLICE_3, "Power Data - Slice 3", SOURCE_L1A,
            MEAS_QUANTITY, "power_dn", 1, {
      { UNIT_DN, "dn", DATA_UINT4_100, 0, ExtractPowerDnSlice3, pr_uint4_100 }
    }
  },
  { POWER_DN_SLICE_4, "Power Data - Slice 4", SOURCE_L1A,
            MEAS_QUANTITY, "power_dn", 1, {
      { UNIT_DN, "dn", DATA_UINT4_100, 0, ExtractPowerDnSlice4, pr_uint4_100 }
    }
  },
  { POWER_DN_SLICE_5, "Power Data - Slice 5", SOURCE_L1A,
            MEAS_QUANTITY, "power_dn", 1, {
      { UNIT_DN, "dn", DATA_UINT4_100, 0, ExtractPowerDnSlice5, pr_uint4_100 }
    }
  },
  { POWER_DN_SLICE_6, "Power Data - Slice 6", SOURCE_L1A,
            MEAS_QUANTITY, "power_dn", 1, {
      { UNIT_DN, "dn", DATA_UINT4_100, 0, ExtractPowerDnSlice6, pr_uint4_100 }
    }
  },
  { POWER_DN_SLICE_7, "Power Data - Slice 7", SOURCE_L1A,
            MEAS_QUANTITY, "power_dn", 1, {
      { UNIT_DN, "dn", DATA_UINT4_100, 0, ExtractPowerDnSlice7, pr_uint4_100 }
    }
  },
  { POWER_DN_SLICE_8, "Power Data - Slice 8", SOURCE_L1A,
            MEAS_QUANTITY, "power_dn", 1, {
      { UNIT_DN, "dn", DATA_UINT4_100, 0, ExtractPowerDnSlice8, pr_uint4_100 }
    }
  },
  { POWER_DN_SLICE_9, "Power Data - Slice 9", SOURCE_L1A,
            MEAS_QUANTITY, "power_dn", 1, {
      { UNIT_DN, "dn", DATA_UINT4_100, 0, ExtractPowerDnSlice9, pr_uint4_100 }
    }
  },
  { POWER_DN_SLICE_10, "Power Data - Slice 10", SOURCE_L1A,
            MEAS_QUANTITY, "power_dn", 1, {
      { UNIT_DN, "dn", DATA_UINT4_100, 0, ExtractPowerDnSlice10, pr_uint4_100 }
    }
  },
  { POWER_DN_SLICE_11, "Power Data - Slice 11", SOURCE_L1A,
            MEAS_QUANTITY, "power_dn", 1, {
      { UNIT_DN, "dn", DATA_UINT4_100, 0, ExtractPowerDnSlice11, pr_uint4_100 }
    }
  },
  { POWER_DN_SLICE_12, "Power Data - Slice 12", SOURCE_L1A,
            MEAS_QUANTITY, "power_dn", 1, {
      { UNIT_DN, "dn", DATA_UINT4_100, 0, ExtractPowerDnSlice12, pr_uint4_100 }
    }
  },
  { NOISE_DN, "Noise Measurements", SOURCE_L1A,
            MEAS_QUANTITY, "noise_dn", 1, {
      { UNIT_DN, "dn", DATA_UINT4_100, 0, ExtractData2D_100, pr_uint4_100 }
    }
  },
#if 0
// not in the file yet
  { SPARE_SCIENCE_DATA, "Spare Science Data", SOURCE_L1A,
            MEAS_UNKNOWN, "science_data_spare", 1, {
      { UNIT_DN, "dn", DATA_UINT2_100,
                                0, ExtractData2D_100, pr_uint2_100 }
#if 0
            MEAS_UNKNOWN, "science_data_spare", 2, {
      { UNIT_UNKNOWN, "unknown", DATA_FLOAT4_100,
                          1, ExtractData2D_100_uint2_float, pr_float4_6_100 }
#endif
    }
  },
#endif

  //-----------------------
  // Frame Inst Status
  //-----------------------
  { FRAME_INST_STATUS, "Frame Inst Status Flags", SOURCE_L1A,
            MEAS_QUANTITY, "frame_inst_status", 2, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 },
      { UNIT_MAP, "binary", DATA_UINT4, 0, ExtractData1D, pr_binchar4 }
    }
  },
  { FRAME_INST_STATUS_00_01, "Current Mode (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=WOM, 1=CBM, 2=SBM, 3=ROM", DATA_UINT1,
                                  0, Extract32Bit0_1, pr_bit }
    }
  },
  { FRAME_INST_STATUS_02, "First Pulse (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=Inner Beam first, 1=Outer Beam first", DATA_UINT1,
                                  0, Extract32Bit2, pr_bit }
    }
  },
  { FRAME_INST_STATUS_03, "Antenna Spin Rate (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=Nominal(18 RPM), 1=Alternate(19.8 RPM)", DATA_UINT1,
                                  0, Extract32Bit3, pr_bit }
    }
  },
  { FRAME_INST_STATUS_04_06, "Effective Gate Width (Slice Resolution)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=0.0 msec, 1=0.1 msec, ... 6=0.6 msec, 7=N/A", DATA_UINT1,
                                  0, Extract32Bit4_6, pr_uint1 }
    }
  },
  { FRAME_INST_STATUS_07, "Data Acquisition Mode (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=High Res, chirp Mode on, 1=Low Res, chirp Mod Off",
                                  DATA_UINT1, 0, Extract32Bit7, pr_bit }
    }
  },
  { FRAME_INST_STATUS_08, "Cal Pulse Sequence (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0 = Has Cal Pulse, 1 = No Cal Pulse",
                                  DATA_UINT1, 0, Extract32Bit8, pr_bit }
    }
  },
  { FRAME_INST_STATUS_09, "SES A/B (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=A, 1=B", DATA_UINT1, 0, Extract32Bit9, pr_bit }
    }
  },
  { FRAME_INST_STATUS_10, "SAS A/B (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=A, 1=B", DATA_UINT1, 0, Extract32Bit10, pr_bit }
    }
  },
  { FRAME_INST_STATUS_11, "TWTA 1/2 (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=1, 1=2", DATA_UINT1, 0, Extract32Bit11, pr_bit }
    }
  },
  { FRAME_INST_STATUS_12, "TWTA Power (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=on, 1=off", DATA_UINT1, 0, Extract32Bit12, pr_bit }
    }
  },
  { FRAME_INST_STATUS_13, "Grid Disable (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=Normal, 1=Disabled", DATA_UINT1, 0,Extract32Bit13, pr_bit}
    }
  },
  { FRAME_INST_STATUS_14, "Receive Protect On/Normal (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=Normal, 1=On", DATA_UINT1, 0,Extract32Bit14, pr_bit}
    }
  },
  { FRAME_INST_STATUS_15, "TWT Trip Override (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=Off, 1=On", DATA_UINT1, 0,Extract32Bit15, pr_bit}
    }
  },
  { FRAME_INST_STATUS_16, "TWT Body Oc Trip (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=Enabled, 1=Disabled", DATA_UINT1, 0,Extract32Bit16, pr_bit}
    }
  },
  { FRAME_INST_STATUS_17, "Receiver Protect (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=Not Protected, 1=Protected",
                                DATA_UINT1, 0,Extract32Bit17, pr_bit}
    }
  },
  { FRAME_INST_STATUS_18, "Mode Change (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=No Change, 1=Change", DATA_UINT1, 0,Extract32Bit18, pr_bit}
    }
  },
  { FRAME_INST_STATUS_19, "Soft Reset (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=No Reset, 1=Reset", DATA_UINT1, 0,Extract32Bit19, pr_bit}
    }
  },
  { FRAME_INST_STATUS_20, "Relay Set/Reset (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=No Change, 1=Change", DATA_UINT1, 0,Extract32Bit20, pr_bit}
    }
  },
  { FRAME_INST_STATUS_21, "PRF Clock Reset (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=No Reset, 1=Reset", DATA_UINT1, 0,Extract32Bit21, pr_bit}
    }
  },
  { FRAME_INST_STATUS_22, "Hard Reset (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=No Reset, 1=Reset", DATA_UINT1, 0,Extract32Bit22, pr_bit}
    }
  },
  { FRAME_INST_STATUS_23, "TWTA Mon Ena/Dis (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=No Change, 1=Change", DATA_UINT1, 0,Extract32Bit23, pr_bit}
    }
  },
  { FRAME_INST_STATUS_24, "SES Param Table Change (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=No Change, 1=Change", DATA_UINT1, 0,Extract32Bit24, pr_bit}
    }
  },
  { FRAME_INST_STATUS_25, "Range Gate Table Change (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=No Change, 1=Change", DATA_UINT1, 0,Extract32Bit25, pr_bit}
    }
  },
  { FRAME_INST_STATUS_26, "Doppler Binning Table Change (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=No Change, 1=Change", DATA_UINT1, 0,Extract32Bit26, pr_bit}
    }
  },
  { FRAME_INST_STATUS_27, "Serial TLM Table Change (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=No Change, 1=Change", DATA_UINT1, 0,Extract32Bit27, pr_bit}
    }
  },
  { FRAME_INST_STATUS_28, "Mission TLM Table Change (Frame Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_inst_status", 1, {
      { UNIT_MAP, "0=No Change, 1=Change", DATA_UINT1, 0,Extract32Bit28, pr_bit}
    }
  },

  //-----------------------
  // Frame Error Status
  //-----------------------
  { FRAME_ERR_STATUS, "L1A Software Error Status Flags", SOURCE_L1A,
            MEAS_QUANTITY, "frame_err_status", 2, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 },
      { UNIT_MAP, "binary", DATA_UINT4, 0, ExtractData1D, pr_binchar4 }
    }
  },
  { FRAME_ERR_STATUS_00, "Current Error (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=No Error, 1=Error", DATA_UINT1, 0,Extract32Bit0, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_01, "Equator Crossing Missed (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=No Miss, 1=Miss", DATA_UINT1, 0,Extract32Bit1, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_02, "Misalligned Cal Pulse (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=misalligned", DATA_UINT1, 0,Extract32Bit2, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_03, "Power On Reset (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=No Reset, 1=Reset", DATA_UINT1, 0,Extract32Bit3, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_04, "CDS Watchdog Timout Reset (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Timeout(Reset Missed)",
                              DATA_UINT1, 0,Extract32Bit4, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_05, "SES Watchdog Timer Event (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Event", DATA_UINT1, 0,Extract32Bit5, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_06, "Fault Protection Event (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Event", DATA_UINT1, 0,Extract32Bit6, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_07, "TLM Error (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Error", DATA_UINT1, 0,Extract32Bit7, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_08, "Missing SC Time (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Error", DATA_UINT1, 0,Extract32Bit8, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_09, "Discrete A2D Timeout (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Error", DATA_UINT1, 0,Extract32Bit9, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_10, "CDS Reset (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Reset", DATA_UINT1, 0,Extract32Bit10, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_11, "PLL Out of Lock (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Out of Lock", DATA_UINT1, 0,Extract32Bit11, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_12, "TWT2 OC Trip (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Tripped", DATA_UINT1, 0,Extract32Bit12, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_13, "TWT2 UV Trip (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Tripped", DATA_UINT1, 0,Extract32Bit13, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_14, "TWT2 Converter OC Trip (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Tripped", DATA_UINT1, 0,Extract32Bit14, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_15, "TWT1 OC Trip (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Tripped", DATA_UINT1, 0,Extract32Bit15, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_16, "TWT1 UV Trip (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Tripped", DATA_UINT1, 0,Extract32Bit16, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_17, "TWT1 Converter OC Trip (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Tripped", DATA_UINT1, 0,Extract32Bit17, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_18, "Serial Port Parity Error (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Error", DATA_UINT1, 0,Extract32Bit18, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_19, "Reset Event (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Event", DATA_UINT1, 0,Extract32Bit19, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_20, "ROM Startup Error (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Error", DATA_UINT1, 0,Extract32Bit20, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_21, "RAM Startup Error (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Error", DATA_UINT1, 0,Extract32Bit21, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_22, "TRS Cmd Success (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=Succeed, 1=Failed", DATA_UINT1, 0,Extract32Bit22, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_23, "SES Data Loss Detection (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=detected", DATA_UINT1, 0,Extract32Bit23, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_24, "SAS Data Loss Detection (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=detected", DATA_UINT1, 0,Extract32Bit24, pr_bit}
    }
  },
  { FRAME_ERR_STATUS_25, "Payload Bus Interface (Error Status)",
            SOURCE_L1A, MEAS_STATUS, "frame_err_status", 1, {
      { UNIT_MAP, "0=OK, 1=Error", DATA_UINT1, 0,Extract32Bit25, pr_bit}
    }
  },

  //---------------------------
  // Frame Quality Flag
  //---------------------------
  { FRAME_QUALITY_FLAG, "Frame Quality", SOURCE_L1A,
            MEAS_STATUS, "frame_qual_flag", 2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 },
      { UNIT_MAP, "binary", DATA_UINT2, 0, ExtractData1D, pr_binchar2 }
    }
  },
  { FRAME_QUALITY_FLAG_00_01, "Frame Filler (Frame Quality)", SOURCE_L1A,
            MEAS_STATUS, "frame_qual_flag", 1, {
      { UNIT_MAP, "0=No filler, 1=in Pckt 2, 2=in Pckt 3, 3=in Pckts 2&3",
                       DATA_UINT1, 0, Extract16Bit0_1, pr_uint1 }
    }
  },
  { FRAME_QUALITY_FLAG_02_03, "Frame CRC Error (Frame Quality)", SOURCE_L1A,
            MEAS_STATUS, "frame_qual_flag", 1, {
      { UNIT_MAP, "0=No Error, 1=in Pckt 2, 2=in Pckt 3, 3=in Pckts 2&3",
                       DATA_UINT1, 0, Extract16Bit2_3, pr_uint1 }
    }
  },
  { FRAME_QUALITY_FLAG_04, "Data Quality (Frame Quality)",
            SOURCE_L1A, MEAS_STATUS, "frame_qual_flag", 1, {
      { UNIT_MAP, "0=Good, 1=Bad", DATA_UINT1, 0, Extract16Bit4, pr_bit}
    }
  },

  { PULSE_QUALITY_FLAG, "Pulse Quality for each PRI", SOURCE_L1A,
            MEAS_QUANTITY, "pulse_qual_flag", 1, {
      { UNIT_HEX_BYTES, "hex bytes - 0=OK, 1=unreliable",
            DATA_CHAR13, 0, ExtractData2D_13, pr_char13x }
    }
  },
  { ORBIT_PERIOD, "Orbit Period", SOURCE_L1A, MEAS_QUANTITY,
               "orbit_time", 1, {
      { UNIT_COUNTS, "ticks", DATA_UINT4, 0, ExtractOrbitPeriod, pr_uint4 }
    }
  },
  { ANT_SPIN_RATE, "Antenna Spin Rate", SOURCE_L1A, MEAS_QUANTITY,
               "antenna_position,prf_cycle_time", 4, {
      { UNIT_DN, "dn/pri", DATA_UINT2_100, 0,
                             ExtractAntSpinRateDN, pr_uint2_100 },
      { UNIT_DEGREES, "degrees/pri", DATA_FLOAT4_100, 0,
                             ExtractAntSpinRateDegree, pr_float4_6_100 },
      { UNIT_DEG_SEC, "degrees/sec", DATA_FLOAT4_100, 0,
                             ExtractAntSpinRateDegSec, pr_float4_6_100 },
      { UNIT_ROT_MIN, "rotation/min", DATA_FLOAT4_100, 0,
                             ExtractAntSpinRateRotMin, pr_float4_6_100 }
    }
  },

};

const int L1AParTabSize = ElementNumber(L1AParTab);
