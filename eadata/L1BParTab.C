//=========================================================
// Copyright  (C)1995, California Institute of Technology.
// U.S. Government sponsorship under
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   16 Oct 1998 09:05:06   sally
// extract frame_time using V data
// 
//    Rev 1.0   13 Oct 1998 15:33:30   sally
// Initial revision.
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#include "ParTab.h"
#include "L1AExtract.h"
#include "Print.h"

static const char rcs_id_L2AParTab_C[] =
    "@(#) $Header$";

const ParTabEntry L1BParTab[] =
{
  { FRAME_TIME, "Frame Time", SOURCE_L1B, MEAS_TIME, "v:frame_time", 6, {
      { UNIT_AUTOTIME, "(auto)", DATA_ITIME, 0, ExtractL1Time, NULL },
      { UNIT_CODE_A,   "Code A", DATA_ITIME,0,ExtractL1Time,pr_itime_codea},
      { UNIT_DAYS,     "days", DATA_ITIME, 0, ExtractL1Time, pr_itime_d },
      { UNIT_HOURS,    "hours", DATA_ITIME, 0, ExtractL1Time, pr_itime_h },
      { UNIT_MINUTES,  "minutes", DATA_ITIME, 0, ExtractL1Time, pr_itime_m },
      { UNIT_SECONDS,  "seconds", DATA_ITIME, 0, ExtractL1Time, pr_itime_s }
    }
  },
  { ORBIT_TIME, "Orbit Time", SOURCE_L1A, MEAS_TIME, "orbit_time", 1, {
      { UNIT_COUNTS, "counts", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },

  { FRAME_INST_STATUS, "Frame Inst Status Flags", SOURCE_L1A,
            MEAS_MAP, "frame_inst_status", 2, {
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

  { FRAME_ERR_STATUS, "L1A Software Error Status Flags", SOURCE_L1A,
            MEAS_MAP, "frame_err_status", 2, {
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

  { FRAME_QUALITY_FLAG, "Frame Quality", SOURCE_L1A,
            MEAS_MAP, "frame_qual_flag", 2, {
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

  { NUM_PULSES, "Number of Pulses", SOURCE_L1A, MEAS_QUANTITY,
            "num_pulses", 1, {
      { UNIT_DN, "dn", DATA_INT1, 0, ExtractData1D, pr_int1 }
    }
  },
  { SC_LAT, "S/C Latitude", SOURCE_L1A, MEAS_LOCATION, "sc_lat", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4, 0, ExtractData1D, pr_float4_6 }
    }
  },
  { SC_LON, "S/C Longitude", SOURCE_L1A, MEAS_LOCATION, "sc_lon", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4, 0, ExtractData1D, pr_float4_6 }
    }
  },
  { SC_ALT, "S/C Altitude", SOURCE_L1A, MEAS_DISTANCE, "sc_alt", 2, {
      { UNIT_METERS, "meters", DATA_FLOAT4, 0, ExtractData1D, pr_float4_6 },
      { UNIT_KILOMETERS, "km", DATA_FLOAT4, 0, ExtractData1D_m_km, pr_float4_6 }
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
                "x_vel", 1, {
      { UNIT_MPS, "m/s", DATA_FLOAT4, 0, ExtractData1D, pr_float4_6 }
    }
  },
  { Y_VEL, "Y Component of S/C Velocity", SOURCE_L1A, MEAS_VELOCITY,
                "y_vel", 1, {
      { UNIT_MPS, "m/s", DATA_FLOAT4, 0, ExtractData1D, pr_float4_6 }
    }
  },
  { Z_VEL, "Z Component of S/C Velocity", SOURCE_L1A, MEAS_VELOCITY,
                "z_vel", 1, {
      { UNIT_MPS, "m/s", DATA_FLOAT4, 0, ExtractData1D, pr_float4_6 }
    }
  },
  { ROLL, "S/C Roll", SOURCE_L1A, MEAS_ORIENTATION, "roll", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4,
                           0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { PITCH, "S/C Pitch", SOURCE_L1A, MEAS_ORIENTATION, "pitch", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4,
                           0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { YAW, "S/C Yaw", SOURCE_L1A, MEAS_ORIENTATION, "yaw", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4,
                           0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { BANDWIDTH_RATIO, "Bandwidth Ratio", SOURCE_L1A, MEAS_POWER,
             "bandwidth_ratio", 1, {
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { X_CAL_A, "Cal Value for A beam pulses", SOURCE_L1A, MEAS_POWER,
             "x_cal_A", 1, {
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { X_CAL_B, "Cal Value for B beam pulses", SOURCE_L1A, MEAS_POWER,
             "x_cal_B", 1, {
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { CELL_LAT, "Center Latitude for whole pulse", SOURCE_L1A,
              MEAS_DATA, "cell_lat", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_100,
                   0, ExtractData2D_100, pr_float4_6_100 }
    }
  },
  { CELL_LON, "Center Longitude for whole pulse", SOURCE_L1A,
              MEAS_DATA, "cell_lon", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_100,
                   0, ExtractData2D_100, pr_float4_6_100 }
    }
  },
  { SIGMA0_MODE_FLAG, "Sigma 0 Mode Flag", SOURCE_L1A, MEAS_STATUS,
             "sigma0_mode_flag", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData2D_100, pr_uint2_100 }
    }
  },
  { SIGMA0_QUAL_FLAG, "Sigma 0 Quality Flag", SOURCE_L1A, MEAS_STATUS,
             "sigma0_qual_flag", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData2D_100, pr_uint2_100 }
    }
  },
  { SIGMA0, "Sigma 0 for Entire Pulse", SOURCE_L1A, MEAS_POWER, "sigma0", 1, {
      { UNIT_DB, "dB", DATA_FLOAT4_100, 0,
                      ExtractData2D_100_int2_float, pr_float4_6_100 }
    }
  },
  { FREQUENCY_SHIFT, "Shift in Baseband Frequency", SOURCE_L1A,
                MEAS_POWER, "frequency_shift", 1, {
      { UNIT_HZ, "Hz", DATA_INT2_100, 0, ExtractData2D_100, pr_int2_100 }
    }
  },
  { CELL_AZIMUTH, "Azimuth for whole pulse", SOURCE_L1A,
                       MEAS_DATA, "cell_azimuth", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_100, 0,
                      ExtractData2D_100_uint2_float, pr_float4_6_100 }
    }
  },
  { CELL_INCIDENCE, "Incidence for whole pulse", SOURCE_L1A,
                       MEAS_DATA, "cell_incidence", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_100, 0,
                      ExtractData2D_100_int2_float, pr_float4_6_100 }
    }
  },
  { ANTENNA_AZIMUTH, "Antenna Azimuth", SOURCE_L1A,
                       MEAS_DATA, "antenna_azimuth", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_100, 0,
                      ExtractData2D_100_uint2_float, pr_float4_6_100 }
    }
  },
  { SNR, "Signal to Noise Ratio", SOURCE_L1A, MEAS_POWER, "snr", 1, {
      { UNIT_DB, "dB", DATA_FLOAT4_100, 0,
                      ExtractData2D_100_int2_float, pr_float4_6_100 }
    }
  },
  { KPC_A, "Zero Order Coeff to Calc Kpc", SOURCE_L1A,
                       MEAS_DATA, "kpc_a", 1, {
      { UNIT_DN, "dn", DATA_FLOAT4_100, 0,
                      ExtractData2D_100_int2_float, pr_float4_6_100 }
    }
  },
  { SLICE_QUAL_FLAG, "Slice Quality Flag", SOURCE_L1A,
                       MEAS_STATUS, "slice_qual_flag", 1, {
      { UNIT_DN, "dn", DATA_UINT4_100, 0, ExtractData1D, pr_uint4_100 }
    }
  },
  { SLICE_LAT, "Difference of Slice Latitude", SOURCE_L1A,
                       MEAS_DATA, "slice_lat", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_100_8, 0,
                      ExtractData3D_100_8_int2_float, pr_100_8_float4_6 }
    }
  },
  { SLICE_LON, "Difference of Slice Longitude", SOURCE_L1A,
                       MEAS_DATA, "slice_lon", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_100_8, 0,
                      ExtractData3D_100_8_int2_float, pr_100_8_float4_6 }
    }
  },
  { SLICE_SIGMA0, "Sigma 0 for each Slice", SOURCE_L1A,
                       MEAS_POWER, "slice_sigma0", 1, {
      { UNIT_DB, "dB", DATA_FLOAT4_100_8, 0,
                      ExtractData3D_100_8_int2_float, pr_100_8_float4_6 }
    }
  },
  { X_FACTOR, "X Factor for each Slice", SOURCE_L1A,
                       MEAS_POWER, "x_factor", 1, {
      { UNIT_DB, "dB", DATA_FLOAT4_100_8, 0,
                      ExtractData3D_100_8_int2_float, pr_100_8_float4_6 }
    }
  },
  { SLICE_AZIMUTH, "Azimuth for each Slice", SOURCE_L1A,
                       MEAS_POWER, "slice_azimuth", 1, {
      { UNIT_DEGREES, "degress", DATA_FLOAT4_100_8, 0,
                      ExtractData3D_100_8_uint2_float, pr_100_8_float4_6 }
    }
  },
  { SLICE_INCIDENCE, "Incidence for each Slice", SOURCE_L1A,
                       MEAS_DATA, "slice_incidence", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_100_8, 0,
                      ExtractData3D_100_8_int2_float, pr_100_8_float4_6 }
    }
  },
  { SLICE_SNR, "Slice Signal to Noise Ratio", SOURCE_L1A,
                       MEAS_DATA, "slice_snr", 1, {
      { UNIT_DB, "dB", DATA_FLOAT4_100_8, 0,
                      ExtractData3D_100_8_int2_float, pr_100_8_float4_6 }
    }
  },
  { SLICE_KPC_A, "KPC for Each Slice", SOURCE_L1A,
                       MEAS_DATA, "slice_kpc_a", 1, {
      { UNIT_DN, "dn", DATA_FLOAT4_100_8, 0,
                      ExtractData3D_100_8_int2_float, pr_100_8_float4_6 }
    }
  }
};

const int L1BParTabSize = ElementNumber(L1BParTab);
