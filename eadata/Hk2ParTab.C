//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.11   06 Oct 1998 15:50:48   sally
// separate odd frame parameters from even frame
// 
//    Rev 1.10   28 Sep 1998 16:19:36   sally
// correct some units
// 
//    Rev 1.9   24 Sep 1998 15:54:22   sally
// corrected some print functions
// 
//    Rev 1.8   09 Sep 1998 15:06:40   sally
// add minor_frame_count as the first SDS ID
// 
//    Rev 1.7   08 Sep 1998 16:25:06   sally
// added HK2 FSW subcoms
// 
//    Rev 1.6   01 Jul 1998 09:43:12   sally
// un-expand the fields
// 
//    Rev 1.5   11 May 1998 14:33:14   sally
// fixed some HK2 data
// 
//    Rev 1.4   04 May 1998 10:52:44   sally
// added HK2 filters
// 
//    Rev 1.3   01 May 1998 15:31:56   sally
// fixed typo
// 
//    Rev 1.2   01 May 1998 14:47:34   sally
// added HK2 file
// 
//    Rev 1.1   28 Apr 1998 15:57:24   sally
// added scatterometer housekeeping (1553) data for HK2
// 
//    Rev 1.0   27 Apr 1998 15:50:18   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#include "ParTab.h"
#include "L1AExtract.h"
#include "Hk2Extract.h"
#include "Print.h"

//-********************************************************************
//     defines a giant table to hold all HK2 parameters.
//     Each parameter has up to "NUM_MAX_UNIT_ENTRIES"  unit entries
//     which defines how to extract and interpret the data according
//     the unit type.
//-********************************************************************

static const char rcs_id_HK2ParTab_C[] = "@(#) $Header$";

const ParTabEntry HK2ParTab[] =
{
  { UTC_TIME, "UTC Time", SOURCE_HK2, MEAS_TIME, "hk2_time", 6, {
      { UNIT_AUTOTIME, "(auto)",DATA_ITIME, 0, ExtractTaiTime, 0 },
      { UNIT_CODE_A, "Code A",  DATA_ITIME, 0, ExtractTaiTime, pr_itime_codea },
      { UNIT_DAYS,   "days",    DATA_ITIME, 0, ExtractTaiTime, pr_itime_d },
      { UNIT_HOURS,  "hours",   DATA_ITIME, 0, ExtractTaiTime, pr_itime_h },
      { UNIT_MINUTES,"minutes", DATA_ITIME, 0, ExtractTaiTime, pr_itime_m },
      { UNIT_SECONDS,"seconds", DATA_ITIME, 0, ExtractTaiTime, pr_itime_s }
    }
  },
  { TAI_TIME, "TAI Time", SOURCE_HK2, MEAS_TIME, "hk2_time", 1, {
      { UNIT_TAI_SECONDS, "seconds after TAI", DATA_FLOAT8, 0,
                                               ExtractData1D, pr_float8_10 },
    }
  },
  { HK2_FRAME_COUNT, "Hk2 Minor Frame Count", SOURCE_HK2,
                              MEAS_TIME, "hk2_minor_frame_count", 1, {
      { UNIT_COUNTS, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
    }
  },
  { TORQUE_ROD_1_STATUS, "Torque Rod 1 Status", SOURCE_HK2,
                                  MEAS_STATUS, "torque_rod_status", 1, {
      { UNIT_MAP, "0=Off,1=Off,2=Neg,3=Pos", DATA_UINT1,
                                  0, Extract8Bit6_7, pr_uint1 }
    }
  },
  { TORQUE_ROD_2_STATUS, "Torque Rod 2 Status", SOURCE_HK2,
                                  MEAS_STATUS, "torque_rod_status", 1, {
      { UNIT_MAP, "0=Off, 1=Off, 2=Neg, 3=Pos", DATA_UINT1,
                                  0, Extract8Bit4_5, pr_uint1 }
    }
  },
  { TORQUE_ROD_3_STATUS, "Torque Rod 3 Status", SOURCE_HK2,
                                  MEAS_STATUS, "torque_rod_status", 1, {
      { UNIT_MAP, "0=Off, 1=Off, 2=Neg, 3=Pos", DATA_UINT1, 
                                  0, Extract8Bit2_3, pr_uint1 }
    }
  },
  { CSM_STATUS, "CSM Status", SOURCE_HK2, MEAS_STATUS, "SBW05", 1, {
      { UNIT_MAP, "0=Dsbl, 1=Enbl", DATA_UINT1, 0, Extract8Bit6, pr_bit }
    }
  },
  { CBM_STATUS, "CBM Status", SOURCE_HK2, MEAS_STATUS, "SBW05", 1, {
      { UNIT_MAP, "0=Dsbl, 1=Enbl", DATA_UINT1, 0, Extract8Bit5, pr_bit }
    }
  },
  { TABLE_UPLOAD_IN_PROG, "Table Upload in Progress",
                        SOURCE_HK2, MEAS_STATUS, "SBW05", 1, {
      { UNIT_MAP, "0=NoUpld, 1=Active", DATA_UINT1, 0, Extract8Bit4, pr_bit }
    }
  },
  { TABLE_DOWNLOAD_IN_PROG, "Table Download in Progress",
                        SOURCE_HK2, MEAS_STATUS, "SBW05", 1, {
      { UNIT_MAP, "0=NoUpld, 1=Active", DATA_UINT1, 0, Extract8Bit3, pr_bit }
    }
  },
  { EEPROM_PROG_STATUS, "EEPROM Programming Status",
                        SOURCE_HK2, MEAS_STATUS, "SBW05", 1, {
      { UNIT_MAP, "0=Active, 1=Success, 2=Failed", DATA_UINT1,
                                        0, Extract8Bit1_2, pr_uint1 }
    }
  },
  { EEPROM_PROG_TOGGLE, "EEPROM Programming Toggle",
                        SOURCE_HK2, MEAS_STATUS, "SBW05", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, Extract8Bit0, pr_bit }
    }
  },
  { FLTSW_CMD_ACC_CNT, "FltSW Command Accept Counter",
                        SOURCE_HK2, MEAS_QUANTITY, "fltsw_cmd_acc_cnt", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
    }
  },
  { FLTSW_CMD_REJ_CNT, "FltSW Command Reject Counter",
                        SOURCE_HK2, MEAS_QUANTITY, "fltsw_cmd_rej_cnt", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
    }
  },
  { CMD_STATUS, "Command Status", SOURCE_HK2, MEAS_QUANTITY, "cmd_status", 1, {
      { UNIT_DN, "0=NA, 1=OK, 2=BadOp, ...", DATA_UINT1, 
                                     0, ExtractData1D, pr_uint1 },
    }
  },
  { CSM_EXEC_CMD_CNT, "CSM Executed Command Count",
                        SOURCE_HK2, MEAS_QUANTITY, "csm_exec_cmd_cnt", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 },
    }
  },
  { CBM_EXEC_CMD_CNT, "CBM Executed Command Count",
                        SOURCE_HK2, MEAS_QUANTITY, "cbm_exec_cmd_cnt", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 },
    }
  },
  { TABLE_UPLOAD_RX_CNT, "Table Upload Bytes Received Count",
                        SOURCE_HK2, MEAS_QUANTITY, "table_upload_rx_cnt", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 },
    }
  },
  { TABLE_CHECKSUM_ACC_CNT, "Table Checksum Accept Counter",
                     SOURCE_HK2, MEAS_QUANTITY, "table_checksum_acc_cnt", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
    }
  },
  { TABLE_CHECKSUM_REJ_CNT, "Table Checksum Reject Counter",
                     SOURCE_HK2, MEAS_QUANTITY, "table_checksum_rej_cnt", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
    }
  },
  { AUTO_WAIT_STATUS, "Autonomous Wait Transition Status",
                     SOURCE_HK2, MEAS_STATUS, "SBW02", 1, {
      { UNIT_MAP, "0=Enbl, 1=Dsbl", DATA_UINT1, 0, Extract8Bit7, pr_bit },
    }
  },
  { STAR_TRACKER_ATT, "Star Tracker Attitude Status",
                     SOURCE_HK2, MEAS_STATUS, "SBW02", 1, {
      { UNIT_MAP, "0=Good, 1=Coarse, 2=Bad", DATA_UINT1, 0,
                                         Extract8Bit5_6, pr_uint1 },
    }
  },
  { SOLAR_ARRAY_1_ENBL, "Solar Array 1 Autonomous Control Enabled",
                     SOURCE_HK2, MEAS_STATUS, "SBW02", 1, {
      { UNIT_MAP, "0=Off, 1=Auto", DATA_UINT1, 0, Extract8Bit4, pr_bit },
    }
  },
  { SOLAR_ARRAY_2_ENBL, "Solar Array 2 Autonomous Control Enabled",
                     SOURCE_HK2, MEAS_STATUS, "SBW02", 1, {
      { UNIT_MAP, "0=Off, 1=Auto", DATA_UINT1, 0, Extract8Bit3, pr_bit },
    }
  },
  { GYRO_DATA_QUALITY, "Gyro Data Quality",
                     SOURCE_HK2, MEAS_STATUS, "SBW02", 1, {
      { UNIT_MAP, "0=Good, 1=Coarse, 2=Bad", DATA_UINT1, 0,
                                         Extract8Bit1_2, pr_uint1 },
    }
  },
  { PROC_RESET_FLAG, "Processor Reset Flag",
                     SOURCE_HK2, MEAS_STATUS, "SBW02", 1, {
      { UNIT_MAP, "0=Clear, 1=Reset", DATA_UINT1, 0, Extract8Bit0, pr_bit },
    }
  },
  { FLTSW_MINOR_FRAME_TIME, "FltSW Minor Frame Time Tag",
                     SOURCE_HK2, MEAS_TIME, "fltsw_minor_frame_time", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 },
    }
  },
  { SAS_EA_A_PWR_TEMP, "SAS EA-A Power Supply Temperature, PA-1",
                        SOURCE_HK2, MEAS_TEMPERATURE, "sas_ea_a_pwr_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                               ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { SAS_DUPLEX_BEARING_TEMP, "SAS Duplex Bearing Temperature, PA-2",
                   SOURCE_HK2, MEAS_TEMPERATURE, "sas_duplex_bearing_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                               ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { SES_TEMP_1, "SES Temperature 1, PA-4",
                   SOURCE_HK2, MEAS_TEMPERATURE, "ses_temp_1", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                               ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { TWT1_BASE_TEMP, "TWTA Temperature 1, PA-5",
                   SOURCE_HK2, MEAS_TEMPERATURE, "twt1_base_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                               ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { CDS_A_TEMP, "CDS-A Temperature, PA-7",
                   SOURCE_HK2, MEAS_TEMPERATURE, "cds_a_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                               ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { HK2_PSU_TEMP, "CDS PSU-1 Temperature, PA-8",
                   SOURCE_HK2, MEAS_TEMPERATURE, "hk2_psu_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                               ExtractData1D_uint1_float, pr_float4_6 }
    }
  },
  { RATE_ERROR_DETECTED, "Rate Error Detected",
                   SOURCE_HK2, MEAS_STATUS, "SBW201", 1, {
      { UNIT_MAP, "0=Clear, 1=Error", DATA_UINT1, 0, Extract8Bit7, pr_bit }
    }
  },
  { ATT_ERROR_DETECTED, "Attitude Error Detected",
                   SOURCE_HK2, MEAS_STATUS, "SBW201", 1, {
      { UNIT_MAP, "0=Clear, 1=Error", DATA_UINT1, 0, Extract8Bit6, pr_bit }
    }
  },
  { ATT_DETERM_OVERRIDE, "Attitude Determination Override",
                   SOURCE_HK2, MEAS_STATUS, "SBW201", 1, {
      { UNIT_MAP, "0=Nominal, 1=Rsensor, 2=MeasAtt, 3=Jamload", DATA_UINT1, 0,
                                   Extract8Bit4_5, pr_uint1 }
    }
  },
  { CBM_ACTIVE_BLOCK_CNT, "CBM Active Block Count",
                   SOURCE_HK2, MEAS_QUANTITY, "SBW201", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, Extract8Bit0_3, pr_uint1 }
    }
  },
  { AUTO_SCAT_MSG_STATUS, "Automatic Scatterometer Messaging Status",
                   SOURCE_HK2, MEAS_STATUS, "SBW202", 1, {
      { UNIT_MAP, "0=Off, 1=Auto", DATA_UINT1, 0, Extract8Bit7, pr_uint1 }
    }
  },
  { AUTO_SCAT_TEMP_MON, "Automatic Scatterometer Temp Monitor Status",
                   SOURCE_HK2, MEAS_STATUS, "SBW202", 1, {
      { UNIT_MAP, "0=Off, 1=Auto", DATA_UINT1, 0, Extract8Bit6, pr_bit }
    }
  },
  { CDS_SAFE_SEQ_STATUS, "CDS Safing Sequence Status",
                   SOURCE_HK2, MEAS_STATUS, "SBW202", 1, {
      { UNIT_MAP, "0=Off, 1=Active, 2=Complete", DATA_UINT1, 0,
                                        Extract8Bit4_5, pr_uint1 }
    }
  },
  { SES_SAFE_SEQ_STATUS, "SES Safing Sequence Status",
                   SOURCE_HK2, MEAS_STATUS, "SBW202", 1, {
      { UNIT_MAP, "0=Off, 1=Active, 2=Complete", DATA_UINT1, 0,
                                        Extract8Bit2_3, pr_uint1 }
    }
  },
  { SAS_SAFE_SEQ_STATUS, "SAS Safing Sequence Status",
                   SOURCE_HK2, MEAS_STATUS, "SBW202", 1, {
      { UNIT_MAP, "0=Off, 1=Active, 2=Complete", DATA_UINT1, 0,
                                        Extract8Bit0_1, pr_uint1 }
    }
  },

  // Scatterometer Housekeeping Data (1553)
  { CDS_PSU_BUS_VOLT, "CDS PSU Electronic Bus Voltage",
                   SOURCE_HK2, MEAS_VOLTAGE, "cds_psu_bus_volt", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { CDS_PRIM_BUS_CURRENT, "CDS Primary Bus Current",
                   SOURCE_HK2, MEAS_CURRENT, "cds_prim_bus_curr", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { SASA_PRIM_BUS_CURRENT, "SAS A Primary Bus Current",
                   SOURCE_HK2, MEAS_CURRENT, "sasa_prim_bus_curr", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { SASB_PRIM_BUS_CURRENT, "SAS B Primary Bus Current",
                   SOURCE_HK2, MEAS_CURRENT, "sasb_prim_bus_curr", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { SESA_PRIM_BUS_CURRENT, "SES A Primary Bus Current",
                   SOURCE_HK2, MEAS_CURRENT, "sesa_prim_bus_curr", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { SESB_PRIM_BUS_CURRENT, "SES B Primary Bus Current",
                   SOURCE_HK2, MEAS_CURRENT, "sesb_prim_bus_curr", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { TWTA1_PRIM_BUS_CURRENT, "TWTA 1 Primary Bus Current",
                   SOURCE_HK2, MEAS_CURRENT, "twta1_prim_bus_curr", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { TWTA2_PRIM_BUS_CURRENT, "TWTA 2 Primary Bus Current",
                   SOURCE_HK2, MEAS_CURRENT, "twta2_prim_bus_curr", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { TWTA1_BODY_REG_CURR, "TWTA 1 Body Regulation Current",
                   SOURCE_HK2, MEAS_CURRENT, "twta1_body_reg_curr", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { TWTA2_BODY_REG_CURR, "TWTA 2 Body Regulation Current",
                   SOURCE_HK2, MEAS_CURRENT, "twta2_body_reg_curr", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { TWTA1_ION_PUMP_CURR, "TWTA 1 Ion Pump Current",
                   SOURCE_HK2, MEAS_CURRENT, "twta1_ion_pump_curr", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { TWTA2_ION_PUMP_CURR, "TWTA 2 Ion Pump Current",
                   SOURCE_HK2, MEAS_CURRENT, "twta2_ion_pump_curr", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { TWTA_DRIVE_PWR, "TWTA Drive Power",
                   SOURCE_HK2, MEAS_POWER, "twta_drive_pwr", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_WATTS, "watts", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { SES_TRS_XMIT_PWR, "SES TRS RF Transmit Power",
                   SOURCE_HK2, MEAS_POWER, "ses_trs_xmit_pwr", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_WATTS, "watts", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { SASA_EAA_MOTOR_CURR, "SAS A EA-A Motor Current",
                   SOURCE_HK2, MEAS_CURRENT, "sasa_eaa_motor_curr", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { SASB_EAB_MOTOR_CURR, "SAS B EA-B Motor Current",
                   SOURCE_HK2, MEAS_CURRENT, "sasb_eab_motor_curr", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_AMPS, "amps", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { SASA_EAA_SPIN_RATE, "SAS A EA-A Spin Rate",
                   SOURCE_HK2, MEAS_RATE, "sasa_eaa_spin_rate", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_RPM, "rpm", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { SASB_EAB_SPIN_RATE, "SAS B EA-B Spin Rate",
                   SOURCE_HK2, MEAS_RATE, "sasb_eab_spin_rate", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_RPM, "rpm", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { SASA_EAA_SAA_SEC_VOLT, "SAS A EA-A SAA Secondary Voltage",
                   SOURCE_HK2, MEAS_VOLTAGE, "sasa_eaa_saa_sec_volt", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { SASB_EAB_SAA_SEC_VOLT, "SAS B EA-B SAA Secondary Voltage",
                   SOURCE_HK2, MEAS_VOLTAGE, "sasb_eab_saa_sec_volt", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { CDS_PSU_3_TEMP, "CDS PSU #3 Temperature",
                  SOURCE_HK2, MEAS_TEMPERATURE, "cds_psu_3_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { CDS_IDP_A_TEMP, "CDS IDP-A (SFC-A) Temperature",
                  SOURCE_HK2, MEAS_TEMPERATURE, "cds_idp_a_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { CDS_IDP_B_TEMP, "CDS IDP-B (SFC-B) Temperature",
                  SOURCE_HK2, MEAS_TEMPERATURE, "cds_idp_b_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { TWTA_1_HVPS_CHAS_TEMP, "TWTA 1 HVPS Chassis Temperature",
                  SOURCE_HK2, MEAS_TEMPERATURE, "twta_1_hvps_chas_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { TWTA_2_HVPS_CHAS_TEMP, "TWTA 2 HVPS Chassis Temperature",
                  SOURCE_HK2, MEAS_TEMPERATURE, "twta_2_hvps_chas_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { TWTA_1_BASE_TEMP, "TWTA 1 Base Temperature",
                  SOURCE_HK2, MEAS_TEMPERATURE, "twta_1_base_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { TWTA_2_BASE_TEMP, "TWTA 2 Base Temperature",
                  SOURCE_HK2, MEAS_TEMPERATURE, "twta_2_base_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { SES_DC_CONV_TEMP, "SES DC/DC Converter Temperature",
                  SOURCE_HK2, MEAS_TEMPERATURE, "ses_dc_conv_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { SES_PREC_COUPLER_TEMP, "SES Precision Coupler Temperature",
                  SOURCE_HK2, MEAS_TEMPERATURE, "ses_prec_coupler_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { SES_GAIN_ATTEN_TEMP, "SES Gain Attenuator Temperature",
                  SOURCE_HK2, MEAS_TEMPERATURE, "ses_gain_atten_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { SES_RX_PROTECT_SW_TEMP, "SES Receive Protect Switch Temperature",
                  SOURCE_HK2, MEAS_TEMPERATURE, "ses_rx_protect_sw_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { SAS_RJ_TEMP, "SAS RJ Temperature",
                  SOURCE_HK2, MEAS_TEMPERATURE, "sas_rj_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint1_float, pr_float4_6 },
    }
  },
  { CDS_ERR_RESET_FLAG, "CDS Error System Reset Flag",
                        SOURCE_HK2, MEAS_STATUS, "SCBW1", 1, {
      { UNIT_MAP, "0=Normal, 1=Reset", DATA_UINT1, 0, Extract8Bit7, pr_bit }
    }
  },
  { STALE_DATA_TOGGLE, "Stale Data Toggle Flag",
                        SOURCE_HK2, MEAS_STATUS, "SCBW1", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, Extract8Bit6, pr_bit }
    }
  },
  { A2D_TIMEOUT_FLAG, "A2D or Pulse Discrete Timeout Flag",
                        SOURCE_HK2, MEAS_STATUS, "SCBW1", 1, {
      { UNIT_MAP, "0=Normal, 1=Timeout",DATA_UINT1, 0, Extract8Bit5, pr_bit }
    }
  },
  { MISS_SC_TIME_HLDC, "Missing S/C Time Flag",
                        SOURCE_HK2, MEAS_STATUS, "SCBW1", 1, {
      { UNIT_MAP, "0=Normal, 1=Missing",DATA_UINT1, 0, Extract8Bit4, pr_bit }
    }
  },
  { MTLM_STLM_ERR, "MTLM or STLM Error Flag",
                        SOURCE_HK2, MEAS_STATUS, "SCBW1", 1, {
      { UNIT_MAP, "0=Normal, 1=Error",DATA_UINT1, 0, Extract8Bit3, pr_bit }
    }
  },
  { FAULT_PROTECT_EVENT, "Fault Protection Event Flag",
                        SOURCE_HK2, MEAS_STATUS, "SCBW1", 1, {
      { UNIT_MAP, "0=Normal, 1=Fault",DATA_UINT1, 0, Extract8Bit2, pr_bit }
    }
  },
  { WATCHDOG_TIMEOUT, "Watchdog Timeout Flag",
                        SOURCE_HK2, MEAS_STATUS, "SCBW1", 1, {
      { UNIT_MAP, "0=Normal, 1=Timeout",DATA_UINT1, 0, Extract8Bit1, pr_bit }
    }
  },
  { POWER_ON_RESET, "Power-On Reset Flag",
                        SOURCE_HK2, MEAS_STATUS, "SCBW1", 1, {
      { UNIT_MAP, "0=Normal, 1=POR",DATA_UINT1, 0, Extract8Bit0, pr_bit }
    }
  },
  { OPERATIONAL_MODE, "Instrument Mode", SOURCE_HK2,
                          MEAS_STATUS, "inst_mode", 2, {
      { UNIT_MAP, "14=WOM, 7=CBM, 112=SBM, 224=ROM", DATA_UINT1,
                0, ExtractData1D, pr_uint1 },
      { UNIT_HEX_BYTES, "0E=WOM, 07=CBM, 70=SBM, E0=ROM", DATA_UINT1,
                0, ExtractData1D, pr_char1x }
    }
  },
  { VALID_COMMAND_COUNT, "Valid Command Count", SOURCE_HK2,
                                MEAS_QUANTITY, "valid_cmd_cnt", 1, {
      { UNIT_DN, "dn",DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { INVALID_COMMAND_COUNT, "Invalid Command Count", SOURCE_HK2,
                                MEAS_QUANTITY, "invalid_cmd_cnt", 1, {
      { UNIT_DN, "dn",DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { TWTA_1_CONV_OC, "TWTA 1 Converter Over-Current Status",
                        SOURCE_HK2, MEAS_STATUS, "SCBW2", 1, {
      { UNIT_MAP, "0=Off, 1=On",DATA_UINT1, 0, Extract8Bit7, pr_bit }
    }
  },
  { TWTA_1_UNDER_VOLT, "TWTA 1 Under-Voltage Status",
                        SOURCE_HK2, MEAS_STATUS, "SCBW2", 1, {
      { UNIT_MAP, "0=Off, 1=On",DATA_UINT1, 0, Extract8Bit6, pr_bit }
    }
  },
  { TWTA_1_BODY_OC, "TWTA 1 Body Over-Current Status",
                        SOURCE_HK2, MEAS_STATUS, "SCBW2", 1, {
      { UNIT_MAP, "0=Off, 1=On",DATA_UINT1, 0, Extract8Bit5, pr_bit }
    }
  },
  { TWTA_2_CONV_OC, "TWTA 2 Converter Over-Current Status",
                        SOURCE_HK2, MEAS_STATUS, "SCBW2", 1, {
      { UNIT_MAP, "0=Off, 1=On",DATA_UINT1, 0, Extract8Bit4, pr_bit }
    }
  },
  { TWTA_2_UNDER_VOLT, "TWTA 2 Under-Voltage Status",
                        SOURCE_HK2, MEAS_STATUS, "SCBW2", 1, {
      { UNIT_MAP, "0=Off, 1=On",DATA_UINT1, 0, Extract8Bit3, pr_bit }
    }
  },
  { TWTA_2_BODY_OC, "TWTA 2 Body Over-Current Status",
                        SOURCE_HK2, MEAS_STATUS, "SCBW2", 1, {
      { UNIT_MAP, "0=Off, 1=On",DATA_UINT1, 0, Extract8Bit2, pr_bit }
    }
  },
  { PLL_OUT_OF_LOCK, "PLL Out-of-Lock Flag",
                        SOURCE_HK2, MEAS_STATUS, "SCBW2", 1, {
      { UNIT_MAP, "0=Off, 1=On",DATA_UINT1, 0, Extract8Bit1, pr_bit }
    }
  },
  { BEAM_SELECT, "Beam Select Status",
                        SOURCE_HK2, MEAS_STATUS, "SCBW2", 1, {
      { UNIT_MAP, "0=Off, 1=On",DATA_UINT1, 0, Extract8Bit0, pr_bit }
    }
  },
  { SES_WATCHDOG_TIMER_EVENT, "SES Watchdog Timer Event Flag",
                   SOURCE_HK2, MEAS_STATUS, "SCBW3", 1, {
      { UNIT_MAP, "0=OK, 1=Error",DATA_UINT1, 0, Extract8Bit6, pr_bit }
    }
  },
  { SES_TRS_CMD_SUCC, "SES TRS Command Success Flag",
                   SOURCE_HK2, MEAS_STATUS, "SCBW3", 1, {
      { UNIT_MAP, "0=Off, 1=On",DATA_UINT1, 0, Extract8Bit4, pr_bit }
    }
  },
  { SES_RX_PROTECT, "SES Receive Protect On/Off Flag",
                   SOURCE_HK2, MEAS_STATUS, "SCBW3", 1, {
      { UNIT_MAP, "0=Off, 1=On",DATA_UINT1, 0, Extract8Bit3, pr_bit }
    }
  },
  { SES_SERIAL_PRT_PARITY, "SES Seial Port Parity Error Flag",
                   SOURCE_HK2, MEAS_STATUS, "SCBW3", 1, {
      { UNIT_MAP, "0=Normal, 1=Dsbl",DATA_UINT1, 0, Extract8Bit2, pr_bit }
    }
  },
  { GRID_INHIBIT, "Grid Inhibit Flag",
                   SOURCE_HK2, MEAS_STATUS, "SCBW3", 1, {
      { UNIT_MAP, "0=Normal, 1=Dsbl",DATA_UINT1, 0, Extract8Bit1, pr_bit }
    }
  },
  { TWT_BODY_OC_TRIP, "Body Over-Current Trip Control Flag",
                   SOURCE_HK2, MEAS_STATUS, "SCBW3", 1, {
      { UNIT_MAP, "0=Off, 1=On",DATA_UINT1, 0, Extract8Bit0, pr_bit }
    }
  },
  { K15_SES_SELECT, "SES-A/B Relay K15 Status",
                   SOURCE_HK2, MEAS_STATUS, "SCBW4", 1, {
      { UNIT_MAP, "0=Set, 1=Reset", DATA_UINT1, 0, Extract8Bit7, pr_bit }
    }
  },
  { K26_SPARE, "Spare Relay K26 Status",
                   SOURCE_HK2, MEAS_STATUS, "SCBW4", 1, {
      { UNIT_MAP, "0=Set, 1=Reset", DATA_UINT1, 0, Extract8Bit6, pr_bit }
    }
  },
  { K25_SPARE, "Spare Relay K25 Status",
                   SOURCE_HK2, MEAS_STATUS, "SCBW4", 1, {
      { UNIT_MAP, "0=Set, 1=Reset", DATA_UINT1, 0, Extract8Bit5, pr_bit }
    }
  },
  { K12_TWTA_SELECT, "TWTA 1,2 Relay K12 Status",
                   SOURCE_HK2, MEAS_STATUS, "SCBW4", 1, {
      { UNIT_MAP, "0=Set, 1=Reset", DATA_UINT1, 0, Extract8Bit4, pr_bit }
    }
  },
  { K11_TWTA_SELECT, "TWTA 1,2 Relay K11 Status",
                   SOURCE_HK2, MEAS_STATUS, "SCBW4", 1, {
      { UNIT_MAP, "0=Set, 1=Reset", DATA_UINT1, 0, Extract8Bit3, pr_bit }
    }
  },
  { K10_TWTA_POWER, "TWT On,Off Relay K10 Status",
                   SOURCE_HK2, MEAS_STATUS, "SCBW4", 1, {
      { UNIT_MAP, "0=Set, 1=Reset", DATA_UINT1, 0, Extract8Bit2, pr_bit }
    }
  },
  { K9_TWTA_POWER, "TWT On,Off Relay K9 Status",
                   SOURCE_HK2, MEAS_STATUS, "SCBW4", 1, {
      { UNIT_MAP, "0=Set, 1=Reset", DATA_UINT1, 0, Extract8Bit1, pr_bit }
    }
  },
  { K20_SAS_SELECT, "SAS A,B Select Relay K20 Status",
                   SOURCE_HK2, MEAS_STATUS, "SCBW4", 1, {
      { UNIT_MAP, "0=Set, 1=Reset", DATA_UINT1, 0, Extract8Bit0, pr_bit }
    }
  },
  { K19_SAS_SELECT, "SAS A,B Select Relay K19 Status",
                   SOURCE_HK2, MEAS_STATUS, "SCBW5", 1, {
      { UNIT_MAP, "0=Set, 1=Reset", DATA_UINT1, 0, Extract8Bit7, pr_bit }
    }
  },
  { K16_SES_SELECT, "SES-A,B Relay K16 Status",
                   SOURCE_HK2, MEAS_STATUS, "SCBW5", 1, {
      { UNIT_MAP, "0=Set, 1=Reset", DATA_UINT1, 0, Extract8Bit6, pr_bit }
    }
  },
  { K24_SPARE, "Spare Relay K24 Status",
                   SOURCE_HK2, MEAS_STATUS, "SCBW5", 1, {
      { UNIT_MAP, "0=Set, 1=Reset", DATA_UINT1, 0, Extract8Bit5, pr_bit }
    }
  },
  { SAS_B_SPIN_RATE, "SAS B Spin Rate Flag",
                   SOURCE_HK2, MEAS_STATUS, "SCBW5", 1, {
      { UNIT_MAP, "0 = 19.8 RPM, 1 = 18.0 RPM", DATA_UINT1, 0,
                                Extract8Bit4, pr_bit }
    }
  },
  { K23_SPARE, "Spare Relay K23 Status",
                   SOURCE_HK2, MEAS_STATUS, "SCBW5", 1, {
      { UNIT_MAP, "0=Set, 1=Reset", DATA_UINT1, 0, Extract8Bit3, pr_bit }
    }
  },
  { K22_SES_SUPP_HTR_PWR, "SES Supl Heater On,Off Relay K22 Status",
                   SOURCE_HK2, MEAS_STATUS, "SCBW5", 1, {
      { UNIT_MAP, "0=Set, 1=Reset", DATA_UINT1, 0, Extract8Bit2, pr_bit }
    }
  },
  { K21_SES_SUPP_HTR_PWR, "SES Supl Heater On,Off Relay K21 Status",
                   SOURCE_HK2, MEAS_STATUS, "SCBW5", 1, {
      { UNIT_MAP, "0=Set, 1=Reset", DATA_UINT1, 0, Extract8Bit1, pr_bit }
    }
  },
  { SAS_A_SPIN_RATE, "SAS A Spin Rate Flag",
                   SOURCE_HK2, MEAS_STATUS, "SCBW5", 1, {
      { UNIT_MAP, "0 = 19.8 RPM, 1 = 18.0 RPM", DATA_UINT1, 0,
                                Extract8Bit0, pr_bit }
    }
  },

  // GPS Receivers
  { GPS_TIME_TAG_MONTH, "GPS Time Tag Month",
                   SOURCE_HK2, MEAS_TIME, "gps_time_tag_month", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { GPS_TIME_TAG_DAY, "GPS Time Tag Day",
                   SOURCE_HK2, MEAS_TIME, "gps_time_tag_day", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { GPS_TIME_TAG_YEAR, "GPS Time Tag Year",
                   SOURCE_HK2, MEAS_TIME, "gps_time_tag_year", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { GPS_TIME_TAG_HOURS, "GPS Time Tag Hours",
                   SOURCE_HK2, MEAS_TIME, "gps_time_tag_hour", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { GPS_TIME_TAG_MINUTES, "GPS Time Tag Minutes",
                   SOURCE_HK2, MEAS_TIME, "gps_time_tag_min", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { GPS_TIME_TAG_SECONDS, "GPS Time Tag Seconds",
                   SOURCE_HK2, MEAS_TIME, "gps_time_tag_sec", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { GPS_TIME_TAG_NSECONDS, "GPS Time Tag Nanoseconds",
                   SOURCE_HK2, MEAS_TIME, "gps_time_tag_nsec", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { CURRENT_DOP, "Current DOP", SOURCE_HK2, MEAS_STATUS, "current_dop", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { NUM_VISIBLE_SAT, "Number of Visible Satellites",
                   SOURCE_HK2, MEAS_QUANTITY, "num_visible_sat", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { NUM_SAT_TRACKED, "Number of Satellites Tracked",
                   SOURCE_HK2, MEAS_QUANTITY, "num_sat_tracked", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_1_ID, "Satellite 1 ID", SOURCE_HK2, MEAS_ID, "sat_1_id", 1, {
      { UNIT_ID, "id", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_1_TRACK_MODE, "Sat 1 Channel Tracking Mode",
                   SOURCE_HK2, MEAS_STATUS, "sat_1_track_mode", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_2_ID, "Satellite 2 ID", SOURCE_HK2, MEAS_ID, "sat_2_id", 1, {
      { UNIT_ID, "id", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_2_TRACK_MODE, "Sat 2 Channel Tracking Mode",
                   SOURCE_HK2, MEAS_STATUS, "sat_2_track_mode", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_3_ID, "Satellite 3 ID", SOURCE_HK2, MEAS_ID, "sat_3_id", 1, {
      { UNIT_ID, "id", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_3_TRACK_MODE, "Sat 3 Channel Tracking Mode",
                   SOURCE_HK2, MEAS_STATUS, "sat_3_track_mode", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_4_ID, "Satellite 4 ID", SOURCE_HK2, MEAS_ID, "sat_4_id", 1, {
      { UNIT_ID, "id", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_4_TRACK_MODE, "Sat 4 Channel Tracking Mode",
                   SOURCE_HK2, MEAS_STATUS, "sat_4_track_mode", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_5_ID, "Satellite 5 ID", SOURCE_HK2, MEAS_ID, "sat_5_id", 1, {
      { UNIT_ID, "id", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_5_TRACK_MODE, "Sat 5 Channel Tracking Mode",
                   SOURCE_HK2, MEAS_STATUS, "sat_5_track_mode", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_6_ID, "Satellite 6 ID", SOURCE_HK2, MEAS_ID, "sat_6_id", 1, {
      { UNIT_ID, "id", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_6_TRACK_MODE, "Sat 6 Channel Tracking Mode",
                   SOURCE_HK2, MEAS_STATUS, "sat_6_track_mode", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_7_ID, "Satellite 7 ID", SOURCE_HK2, MEAS_ID, "sat_7_id", 1, {
      { UNIT_ID, "id", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_7_TRACK_MODE, "Sat 7 Channel Tracking Mode",
                   SOURCE_HK2, MEAS_STATUS, "sat_7_track_mode", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_8_ID, "Satellite 8 ID", SOURCE_HK2, MEAS_ID, "sat_8_id", 1, {
      { UNIT_ID, "id", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_8_TRACK_MODE, "Sat 8 Channel Tracking Mode",
                   SOURCE_HK2, MEAS_STATUS, "sat_8_track_mode", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_9_ID, "Satellite 9 ID", SOURCE_HK2, MEAS_ID, "sat_9_id", 1, {
      { UNIT_ID, "id", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_9_TRACK_MODE, "Sat 9 Channel Tracking Mode",
                   SOURCE_HK2, MEAS_STATUS, "sat_9_track_mode", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_10_ID, "Satellite 10 ID", SOURCE_HK2, MEAS_ID, "sat_10_id", 1, {
      { UNIT_ID, "id", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_10_TRACK_MODE, "Sat 10 Channel Tracking Mode",
                   SOURCE_HK2, MEAS_STATUS, "sat_10_track_mode", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_11_ID, "Satellite 11 ID", SOURCE_HK2, MEAS_ID, "sat_11_id", 1, {
      { UNIT_ID, "id", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_11_TRACK_MODE, "Sat 11 Channel Tracking Mode",
                   SOURCE_HK2, MEAS_STATUS, "sat_11_track_mode", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_12_ID, "Satellite 12 ID", SOURCE_HK2, MEAS_ID, "sat_12_id", 1, {
      { UNIT_ID, "id", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SAT_12_TRACK_MODE, "Sat 12 Channel Tracking Mode",
                   SOURCE_HK2, MEAS_STATUS, "sat_12_track_mode", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { POS_PROP_MODE, "Receiver Status -- Position Propagate Mode",
                   SOURCE_HK2, MEAS_STATUS, "SBW11", 1, {
      { UNIT_MAP, "1=Prop/Bad, 0=No Prop", DATA_UINT1, 0,
                                   Extract8Bit7, pr_bit }
    }
  },
  { POOR_GEOMETRY, "Receiver Status -- Poor Geometry",
                   SOURCE_HK2, MEAS_STATUS, "SBW11", 1, {
      { UNIT_MAP, "1=Poor/Bad, 0=Good", DATA_UINT1, 0, Extract8Bit6, pr_bit }
    }
  },
  { THREE_D_FIX, "Receiver Status -- 3D Fix",
                   SOURCE_HK2, MEAS_STATUS, "SBW11", 1, {
      { UNIT_MAP, "1=Fix, 0=NoFix/Bad", DATA_UINT1, 0, Extract8Bit5, pr_bit }
    }
  },
  { ALTITUDE_HOLD, "Receiver Status -- Altitude Hold",
                   SOURCE_HK2, MEAS_STATUS, "SBW11", 1, {
      { UNIT_MAP, "1=Fix2D/Bad, 0=Fix3D", DATA_UINT1, 0, Extract8Bit4, pr_bit}
    }
  },
  { ACQUIRE_SAT, "Receiver Status -- Acquiring Satellites",
                   SOURCE_HK2, MEAS_STATUS, "SBW11", 1, {
      { UNIT_MAP, "1=Acquiring/Bad, 0=Clear", DATA_UINT1, 0,
                                     Extract8Bit3, pr_bit }
    }
  },
  { STORE_NEW_ALMANAC, "Receiver Status -- Storing New Almanac",
                   SOURCE_HK2, MEAS_STATUS, "SBW11", 1, {
      { UNIT_MAP, "1=Storing/Bad, 0=Clear", DATA_UINT1, 0,
                                     Extract8Bit2, pr_bit }
    }
  },
  { INSUFF_VISIBLE_SAT, "Receiver Status -- Insufficient Visible Satellites",
                   SOURCE_HK2, MEAS_STATUS, "SBW11", 1, {
      { UNIT_MAP, "1=TooFew/Bad, 0=OK", DATA_UINT1, 0, Extract8Bit1, pr_bit }
    }
  },
  { BAD_ALMANAC, "Receiver Status -- Bad Almanac",
                   SOURCE_HK2, MEAS_STATUS, "SBW11", 1, {
      { UNIT_MAP, "1=TooFew/Bad, 0=OK", DATA_UINT1, 0, Extract8Bit0, pr_bit }
    }
  },
  { GPS_POS_X_RAW_1, "GPS Position, X, Raw Segment 1",
                   SOURCE_HK2, MEAS_LOCATION, "gps_pos_x_raw_1", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { GPS_POS_X_RAW_2, "GPS Position, X, Raw Segment 2",
                   SOURCE_HK2, MEAS_LOCATION, "gps_pos_x_raw_2", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { GPS_POS_Y_RAW_1, "GPS Position, Y, Raw Segment 1",
                   SOURCE_HK2, MEAS_LOCATION, "gps_pos_y_raw_1", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { GPS_POS_Y_RAW_2, "GPS Position, Y, Raw Segment 2",
                   SOURCE_HK2, MEAS_LOCATION, "gps_pos_y_raw_2", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { GPS_POS_Z_RAW_1, "GPS Position, Z, Raw Segment 1",
                   SOURCE_HK2, MEAS_LOCATION, "gps_pos_z_raw_1", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { GPS_POS_Z_RAW_2, "GPS Position, Z, Raw Segment 2",
                   SOURCE_HK2, MEAS_LOCATION, "gps_pos_z_raw_2", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { GPS_VEL_X_RAW_1, "GPS Velocity, X, Raw Segment 1",
                   SOURCE_HK2, MEAS_VELOCITY, "gps_vel_x_raw_1", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { GPS_VEL_X_RAW_2, "GPS Velocity, X, Raw Segment 2",
                   SOURCE_HK2, MEAS_VELOCITY, "gps_vel_x_raw_2", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { GPS_VEL_Y_RAW_1, "GPS Velocity, Y, Raw Segment 1",
                   SOURCE_HK2, MEAS_VELOCITY, "gps_vel_y_raw_1", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { GPS_VEL_Y_RAW_2, "GPS Velocity, Y, Raw Segment 2",
                   SOURCE_HK2, MEAS_VELOCITY, "gps_vel_y_raw_2", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { GPS_VEL_Z_RAW_1, "GPS Velocity, Z, Raw Segment 1",
                   SOURCE_HK2, MEAS_VELOCITY, "gps_vel_z_raw_1", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { GPS_VEL_Z_RAW_2, "GPS Velocity, Z, Raw Segment 2",
                   SOURCE_HK2, MEAS_VELOCITY, "gps_vel_z_raw_2", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { GPS_VTCW_MS_SEG, "GPS VTCW, MS Segment",
                   SOURCE_HK2, MEAS_DATA, "gps_vtcw_1", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { GPS_VTCW_MID_SEG, "GPS VTCW, Middle Segment",
                   SOURCE_HK2, MEAS_DATA, "gps_vtcw_2", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { GPS_VTCW_LS_SEG, "GPS VTCW, LS Segment",
                   SOURCE_HK2, MEAS_DATA, "gps_vtcw_3", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { GYRO_FRAME_COUNTER, "Gyro Frame Counter",
                   SOURCE_HK2, MEAS_QUANTITY, "SBW07", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, Extract8Bit0_6, pr_uint1 }
    }
  },
  { GYRO_HIGH_VOLT_STAT, "Gyro High Voltage Status",
                   SOURCE_HK2, MEAS_QUANTITY, "SBW07", 1, {
      { UNIT_MAP, "0=OK, 1=Fail", DATA_UINT1, 0, Extract8Bit7, pr_bit }
    }
  },
  { GYRO_X_DITH_STAT, "Gyro X Dither Status",
                   SOURCE_HK2, MEAS_STATUS, "SBW06", 1, {
      { UNIT_MAP, "0=OK, 1=Fail", DATA_UINT1, 0, Extract8Bit0, pr_bit }
    }
  },
  { GYRO_Y_DITH_STAT, "Gyro Y Dither Status",
                   SOURCE_HK2, MEAS_STATUS, "SBW06", 1, {
      { UNIT_MAP, "0=OK, 1=Fail", DATA_UINT1, 0, Extract8Bit1, pr_bit }
    }
  },
  { GYRO_Z_DITH_STAT, "Gyro Z Dither Status",
                   SOURCE_HK2, MEAS_STATUS, "SBW06", 1, {
      { UNIT_MAP, "0=OK, 1=Fail", DATA_UINT1, 0, Extract8Bit2, pr_bit }
    }
  },
  { GYRO_X_INTENSITY, "Gyro X Intensity",
                   SOURCE_HK2, MEAS_STATUS, "SBW06", 1, {
      { UNIT_MAP, "0=OK, 1=Fail", DATA_UINT1, 0, Extract8Bit3, pr_bit }
    }
  },
  { GYRO_Y_INTENSITY, "Gyro Y Intensity",
                   SOURCE_HK2, MEAS_STATUS, "SBW06", 1, {
      { UNIT_MAP, "0=OK, 1=Fail", DATA_UINT1, 0, Extract8Bit4, pr_bit }
    }
  },
  { GYRO_Z_INTENSITY, "Gyro Z Intensity",
                   SOURCE_HK2, MEAS_STATUS, "SBW06", 1, {
      { UNIT_MAP, "0=OK, 1=Fail", DATA_UINT1, 0, Extract8Bit5, pr_bit }
    }
  },
  { GYRO_PWR_SUPP_P15V_STAT, "Gyro Power Supply +15V Status",
                   SOURCE_HK2, MEAS_STATUS, "SBW06", 1, {
      { UNIT_MAP, "0=OK, 1=Fail", DATA_UINT1, 0, Extract8Bit6, pr_bit }
    }
  },
  { GYRO_PWR_SUPP_P120V_STAT, "Gyro Power Supply +120V Status",
                   SOURCE_HK2, MEAS_STATUS, "SBW06", 1, {
      { UNIT_MAP, "0=OK, 1=Fail", DATA_UINT1, 0, Extract8Bit7, pr_bit }
    }
  },
  { WHEEL_1_SPEED_DIR, "Wheel 1 Speed/Direction",
                   SOURCE_HK2, MEAS_QUANTITY, "wheel_1_speed_dir", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D, pr_int2 },
      { UNIT_RAD_SEC, "rad/sec", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { WHEEL_2_SPEED_DIR, "Wheel 2 Speed/Direction",
                   SOURCE_HK2, MEAS_QUANTITY, "wheel_2_speed_dir", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D, pr_int2 },
      { UNIT_RAD_SEC, "rad/sec", DATA_FLOAT4, 1, 
                               ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { WHEEL_3_SPEED_DIR, "Wheel 3 Speed/Direction",
                   SOURCE_HK2, MEAS_QUANTITY, "wheel_3_speed_dir", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D, pr_int2 },
      { UNIT_RAD_SEC, "rad/sec", DATA_FLOAT4, 1, 
                               ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { WHEEL_4_SPEED_DIR, "Wheel 4 Speed/Direction",
                   SOURCE_HK2, MEAS_QUANTITY, "wheel_4_speed_dir", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D, pr_int2 },
      { UNIT_RAD_SEC, "rad/sec", DATA_FLOAT4, 1, 
                               ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { STATUS_CODE, "Status Code", SOURCE_HK2, MEAS_STATUS, "status_code", 1, {
      { UNIT_MAP,"0=OK, 1=CSUMerr/bad, 2=SWerr/bad, 3=REQerr/bad, 4/HWerr/bad", 
                           DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { COMMAND_ID, "Command ID", SOURCE_HK2, MEAS_ID, "command_id", 1, {
      { UNIT_ID,"id", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { TLM_FORMAT, "Telemetry Format", SOURCE_HK2, MEAS_ID, "tlm_format", 1, {
      { UNIT_MAP, "0=Fmt0, 1=Fmt1", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { INIT_COMPLETE, "Initialization Complete", SOURCE_HK2,
                   MEAS_STATUS, "SSR04-06", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, Extract8Bit7, pr_bit }
    }
  },
  { CURRENT_STATE, "Current State", SOURCE_HK2,
                   MEAS_STATUS, "SSR04-06", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, Extract8Bit4_6, pr_uint1 }
    }
  },
  { CURRENT_MODE, "Current Mode", SOURCE_HK2,
                   MEAS_STATUS, "SSR04-06", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, Extract8Bit0_3, pr_uint1 }
    }
  },
  { SSR1_SELECTED_PWR, "SSR Selected Power", SOURCE_HK2,
                   MEAS_STATUS, "SSR07-12", 1, {
      { UNIT_MAP, "0=Primary, 1=Redundant",DATA_UINT1,0,Extract8Bit7, pr_bit}
    }
  },
  { SSR2_SELECTED_PWR, "SSR2 Selected Power", SOURCE_HK2,
                   MEAS_STATUS, "SSR07-12", 1, {
      { UNIT_MAP, "0=Primary, 1=Redundant",DATA_UINT1,0,Extract8Bit6, pr_bit}
    }
  },
  { SSR1_EDAC_ENABLED, "SSR EDAC Enabled", SOURCE_HK2,
                   MEAS_STATUS, "SSR07-12", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, Extract8Bit5, pr_bit }
    }
  },
  { SSR2_EDAC_ENABLED, "SSR2 EDAC Enabled", SOURCE_HK2,
                   MEAS_STATUS, "SSR07-12", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, Extract8Bit4, pr_bit }
    }
  },
  { SSR1_CURR_PWR_STATE, "SSR Current Power State", SOURCE_HK2,
                   MEAS_STATUS, "SSR07-12", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, Extract8Bit2_3, pr_uint1 }
    }
  },
  { SSR2_CURR_PWR_STATE, "SSR2 Current Power State", SOURCE_HK2,
                   MEAS_STATUS, "SSR07-12", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, Extract8Bit0_1, pr_uint1 }
    }
  },
  { ADDR_SW_EXCEPTION, "Address of Software Exception", SOURCE_HK2,
                   MEAS_ADDRESS, "addr_sw_exception", 1, {
      { UNIT_HEX_ADDR, "hex", DATA_UINT2, 0, ExtractData1D, pr_char2x }
    }
  },
  { CORR_MEM_ERR_CNT, "Corrected Processer Memory Error Count", SOURCE_HK2,
                   MEAS_QUANTITY, "corr_mem_err_cnt", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { UNCORR_MEM_ERR_CNT, "Uncorrected Processer Memory Error Count", SOURCE_HK2,
                   MEAS_QUANTITY, "uncorr_mem_err_cnt", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SSR1_RX_STATUS, "SSR Receiver Status", SOURCE_HK2,
                   MEAS_STATUS, "ssr1_rx_status", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SSR1_XMIT_STATUS, "SSR Transmitter Status", SOURCE_HK2,
                   MEAS_STATUS, "ssr1_xmit_status", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SSR2_RX_STATUS, "SSR2 Receiver Status", SOURCE_HK2,
                   MEAS_STATUS, "ssr2_rx_status", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { SSR2_XMIT_STATUS, "SSR2 Transmitter Status", SOURCE_HK2,
                   MEAS_STATUS, "ssr2_xmit_status", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { PART_0_SSR1_REC_ADDR, "Partition 0, SSR Record Pointer Address",
                  SOURCE_HK2, MEAS_ADDRESS, "part_0_ssr1_rec_addr", 1, {
      { UNIT_HEX_ADDR, "hex", DATA_UINT4, 0, ExtractData1D, pr_char4x }
    }
  },
  { PART_0_SSR2_REC_ADDR, "Partition 0, SSR2 Record Pointer Address",
                  SOURCE_HK2, MEAS_ADDRESS, "part_0_ssr2_rec_addr", 1, {
      { UNIT_HEX_ADDR, "hex", DATA_UINT4, 0, ExtractData1D, pr_char4x }
    }
  },
  { PART_0_SSR1_UNFIX_ERR, "Pratition 0, SSR Unfixable Errors", SOURCE_HK2,
                   MEAS_STATUS, "part_0_ssr1_unfix_err", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { PART_0_SSR1_PBK_ADDR, "Partition 0, SSR Playback Pointer Address",
                  SOURCE_HK2, MEAS_ADDRESS, "part_0_ssr1_pbk_addr", 1, {
      { UNIT_HEX_ADDR, "hex", DATA_UINT4, 0, ExtractData1D, pr_char4x }
    }
  },
  { PART_0_SSR2_UNFIX_ERR, "Pratition 0, SSR2 Unfixable Errors", SOURCE_HK2,
                   MEAS_STATUS, "part_0_ssr2_unfix_err", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { PART_0_SSR2_PBK_ADDR, "Partition 0, SSR2 Playback Pointer Address",
                  SOURCE_HK2, MEAS_ADDRESS, "part_0_ssr2_pbk_addr", 1, {
      { UNIT_HEX_ADDR, "hex", DATA_UINT4, 0, ExtractData1D, pr_char4x }
    }
  },
  { PART_1_SSR1_REC_ADDR, "Partition 1, SSR Record Pointer Address",
                  SOURCE_HK2, MEAS_ADDRESS, "part_1_ssr1_rec_addr", 1, {
      { UNIT_HEX_ADDR, "hex", DATA_UINT4, 0, ExtractData1D, pr_char4x }
    }
  },
  { PART_1_SSR2_REC_ADDR, "Partition 1, SSR2 Record Pointer Address",
                  SOURCE_HK2, MEAS_ADDRESS, "part_1_ssr2_rec_addr", 1, {
      { UNIT_HEX_ADDR, "hex", DATA_UINT4, 0, ExtractData1D, pr_char4x }
    }
  },
  { PART_1_SSR1_UNFIX_ERR, "Pratition 1, SSR Unfixable Errors", SOURCE_HK2,
                   MEAS_STATUS, "part_1_ssr1_unfix_err", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { PART_1_SSR1_PBK_ADDR, "Partition 1, SSR Playback Pointer Address",
                  SOURCE_HK2, MEAS_ADDRESS, "part_1_ssr1_pbk_addr", 1, {
      { UNIT_HEX_ADDR, "hex", DATA_UINT4, 0, ExtractData1D, pr_char4x }
    }
  },
  { PART_1_SSR2_UNFIX_ERR, "Pratition 1, SSR2 Unfixable Errors", SOURCE_HK2,
                   MEAS_STATUS, "part_1_ssr2_unfix_err", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D, pr_uint1 }
    }
  },
  { PART_1_SSR2_PBK_ADDR, "Partition 1, SSR2 Playback Pointer Address",
                  SOURCE_HK2, MEAS_ADDRESS, "part_1_ssr2_pbk_addr", 1, {
      { UNIT_HEX_ADDR, "hex", DATA_UINT4, 0, ExtractData1D, pr_char4x }
    }
  },
  { SSR1_CORR_MEM_FLAG, "SSR Corrected Mem. Error Rollover Flag",
                  SOURCE_HK2, MEAS_STATUS, "ssr1_corr_mem_flag", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, Extract8Bit0, pr_bit }
    }
  },
  { SSR1_CORR_MEM_ERR_CNT, "SSR Corrected Mem. Error Count",
                  SOURCE_HK2, MEAS_QUANTITY, "ssr1_corr_mem_err_cnt", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { SSR2_CORR_MEM_FLAG, "SSR2 Corrected Mem. Error Rollover Flag",
                  SOURCE_HK2, MEAS_STATUS, "ssr2_corr_mem_flag", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, Extract8Bit0, pr_bit }
    }
  },
  { SSR2_CORR_MEM_ERR_CNT, "SSR2 Corrected Mem. Error Count",
                  SOURCE_HK2, MEAS_QUANTITY, "ssr2_corr_mem_err_cnt", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { SSR1_UNCORR_MEM_FLAG, "SSR Uncorrected Mem. Error Rollover Flag",
                  SOURCE_HK2, MEAS_STATUS, "ssr1_uncorr_mem_flag", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, Extract8Bit0, pr_bit }
    }
  },
  { SSR1_UNCORR_MEM_ERR_CNT, "SSR Uncorrected Mem. Error Count",
                  SOURCE_HK2, MEAS_QUANTITY, "ssr1_uncorr_mem_err_cnt", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { SSR2_UNCORR_MEM_FLAG, "SSR2 Uncorrected Mem. Error Rollover Flag",
                  SOURCE_HK2, MEAS_STATUS, "ssr2_uncorr_mem_flag", 1, {
      { UNIT_DN, "dn", DATA_UINT1, 0, Extract8Bit0, pr_bit }
    }
  },
  { SSR2_UNCORR_MEM_ERR_CNT, "SSR2 Uncorrected Mem. Error Count",
                  SOURCE_HK2, MEAS_QUANTITY, "ssr2_uncorr_mem_err_cnt", 1, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractData1D, pr_uint4 }
    }
  },
  { MESSAGE_CHECKSUM, "Message Checksum",
                  SOURCE_HK2, MEAS_QUANTITY, "msg_checksum", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { ADCS_CYCLE_START_1, "VTCW @ ADCS Cycle Start, MS Segment",
                  SOURCE_HK2, MEAS_QUANTITY, "adcs_cycle_start_1", 1, {
      { UNIT_HEX_BYTES, "hex", DATA_UINT2, 0, ExtractData1D, pr_char2x }
    }
  },
  { ADCS_CYCLE_START_2, "VTCW @ ADCS Cycle Start, Mid Segment",
                  SOURCE_HK2, MEAS_QUANTITY, "adcs_cycle_start_2", 1, {
      { UNIT_HEX_BYTES, "hex", DATA_UINT2, 0, ExtractData1D, pr_char2x }
    }
  },
  { ADCS_CYCLE_START_3, "VTCW @ ADCS Cycle Start, LS Segment",
                  SOURCE_HK2, MEAS_QUANTITY, "adcs_cycle_start_3", 1, {
      { UNIT_HEX_BYTES, "hex", DATA_UINT2, 0, ExtractData1D, pr_char2x }
    }
  },
  { STAR_TRKR_ATT_Q1, "Star Tracker Attitude Q1",
                  SOURCE_HK2, MEAS_QUANTITY, "star_trkr_att_q1", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                        ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { STAR_TRKR_ATT_Q2, "Star Tracker Attitude Q2",
                  SOURCE_HK2, MEAS_QUANTITY, "star_trkr_att_q2", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                        ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { STAR_TRKR_ATT_Q3, "Star Tracker Attitude Q3",
                  SOURCE_HK2, MEAS_QUANTITY, "star_trkr_att_q3", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                        ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { STAR_TRKR_ATT_Q4, "Star Tracker Attitude Q4",
                  SOURCE_HK2, MEAS_QUANTITY, "star_trkr_att_q4", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                        ExtractData1D_int2_float, pr_float4_6 }
    }
  },
  { AXIS1_AVG_PRD1, "Axis 1 Data Average, Period 1",
                  SOURCE_HK2, MEAS_QUANTITY, "axis1_avg_prd1", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { AXIS2_AVG_PRD1, "Axis 2 Data Average, Period 1",
                  SOURCE_HK2, MEAS_QUANTITY, "axis2_avg_prd1", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { AXIS3_AVG_PRD1, "Axis 3 Data Average, Period 1",
                  SOURCE_HK2, MEAS_QUANTITY, "axis3_avg_prd1", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { AXIS1_AVG_PRD2, "Axis 1 Data Average, Period 2",
                  SOURCE_HK2, MEAS_QUANTITY, "axis1_avg_prd2", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { AXIS2_AVG_PRD2, "Axis 2 Data Average, Period 2",
                  SOURCE_HK2, MEAS_QUANTITY, "axis2_avg_prd2", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { AXIS3_AVG_PRD2, "Axis 3 Data Average, Period 2",
                  SOURCE_HK2, MEAS_QUANTITY, "axis3_avg_prd2", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { AXIS1_AVG_PRD3, "Axis 1 Data Average, Period 3",
                  SOURCE_HK2, MEAS_QUANTITY, "axis1_avg_prd3", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { AXIS2_AVG_PRD3, "Axis 2 Data Average, Period 3",
                  SOURCE_HK2, MEAS_QUANTITY, "axis2_avg_prd3", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { AXIS3_AVG_PRD3, "Axis 3 Data Average, Period 3",
                  SOURCE_HK2, MEAS_QUANTITY, "axis3_avg_prd3", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { AXIS1_AVG_PRD4, "Axis 1 Data Average, Period 4",
                  SOURCE_HK2, MEAS_QUANTITY, "axis1_avg_prd4", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { AXIS2_AVG_PRD4, "Axis 2 Data Average, Period 4",
                  SOURCE_HK2, MEAS_QUANTITY, "axis2_avg_prd4", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { AXIS3_AVG_PRD4, "Axis 3 Data Average, Period 4",
                  SOURCE_HK2, MEAS_QUANTITY, "axis3_avg_prd4", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { AXIS1_AVG_PRD5, "Axis 1 Data Average, Period 5",
                  SOURCE_HK2, MEAS_QUANTITY, "axis1_avg_prd5", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { AXIS2_AVG_PRD5, "Axis 2 Data Average, Period 5",
                  SOURCE_HK2, MEAS_QUANTITY, "axis2_avg_prd5", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },
  { AXIS3_AVG_PRD5, "Axis 3 Data Average, Period 5",
                  SOURCE_HK2, MEAS_QUANTITY, "axis3_avg_prd5", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 }
    }
  },

  // FSW Subcoms
  { ATT_UPDATE_RATE_X, "Attitude Update Rate Vector X", SOURCE_HK2, MEAS_RATE,
         "hk2_minor_frame_count,att_update_rate_x", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_RAD_SEC, "radians/sec", DATA_FLOAT4, 1,
                           ExtractData1D_int2_float_Even, pr_float4_6 },
    }
  },
  { CONTROL_TORQUE_X, "Control Torque X", SOURCE_HK2, MEAS_RATE,
         "hk2_minor_frame_count,control_torque_x", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_NM, "Nm", DATA_FLOAT4, 1,
                           ExtractData1D_int2_float_Odd, pr_float4_6 },
    }
  },
  { ATT_UPDATE_RATE_Y, "Attitude Update Rate Vector Y", SOURCE_HK2, MEAS_RATE,
          "hk2_minor_frame_count,att_update_rate_y", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_RAD_SEC, "radians/sec", DATA_FLOAT4, 1,
                           ExtractData1D_int2_float_Even, pr_float4_6 },
    }
  },
  { CONTROL_TORQUE_Y, "Control Torque Y", SOURCE_HK2, MEAS_RATE,
          "hk2_minor_frame_count,control_torque_y", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_NM, "Nm", DATA_FLOAT4, 1,
                           ExtractData1D_int2_float_Odd, pr_float4_6 },
    }
  },
  { ATT_UPDATE_RATE_Z, "Attitude Update Rate Vector Z", SOURCE_HK2, MEAS_RATE,
          "hk2_minor_frame_count,att_update_rate_z", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_RAD_SEC, "radians/sec", DATA_FLOAT4, 1,
                           ExtractData1D_int2_float_Even, pr_float4_6 },
    }
  },
  { CONTROL_TORQUE_Z, "Control Torque Z", SOURCE_HK2, MEAS_RATE,
          "hk2_minor_frame_count,control_torque_z", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_NM, "Nm", DATA_FLOAT4, 1,
                           ExtractData1D_int2_float_Odd, pr_float4_6 },
    }
  },
  { MAX_RESIDUAL, "Maximum residual", SOURCE_HK2, MEAS_QUANTITY,
          "hk2_minor_frame_count,max_residual", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                           ExtractData1D_int2_float_Even, pr_float4_6 },
    }
  },
  { SUN_SENSOR1_INTENSITY, "Sun Sensor 1 Raw Intensity",
                  SOURCE_HK2, MEAS_QUANTITY,
                  "hk2_minor_frame_count,sun_sensor1_intensity", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },

  { WHEEL_1_TORQUE_CMD, "Wheel 1 Torque Command", SOURCE_HK2, MEAS_QUANTITY,
           "hk2_minor_frame_count,wheel_1_torque_cmd", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_NM, "Nm", DATA_FLOAT4, 1,
                           ExtractData1D_int2_float_Even, pr_float4_6 },
    }
  },
  { SUN_SENSOR2_INTENSITY, "Sun Sensor 2 Raw Intensity",
                  SOURCE_HK2, MEAS_QUANTITY,
                  "hk2_minor_frame_count,sun_sensor2_intensity", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },
  { WHEEL_2_TORQUE_CMD, "Wheel 2 Torque Command", SOURCE_HK2, MEAS_QUANTITY,
                  "hk2_minor_frame_count,wheel_2_torque_cmd", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_NM, "Nm", DATA_FLOAT4, 1,
                           ExtractData1D_int2_float_Even, pr_float4_6 },
    }
  },
  { SUN_SENSOR3_INTENSITY, "Sun Sensor 3 Raw Intensity",
                  SOURCE_HK2, MEAS_QUANTITY,
                  "hk2_minor_frame_count,sun_sensor3_intensity", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },
  { WHEEL_3_TORQUE_CMD, "Wheel 3 Torque Command", SOURCE_HK2, MEAS_QUANTITY,
                  "hk2_minor_frame_count,wheel_3_torque_cmd", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_NM, "Nm", DATA_FLOAT4, 1,
                           ExtractData1D_int2_float_Even, pr_float4_6 },
    }
  },
  { SUN_SENSOR4_INTENSITY, "Sun Sensor 4 Raw Intensity",
                  SOURCE_HK2, MEAS_QUANTITY,
                  "hk2_minor_frame_count,sun_sensor4_intensity", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },
  { WHEEL_4_TORQUE_CMD, "Wheel 4 Torque Command", SOURCE_HK2, MEAS_QUANTITY,
                 "hk2_minor_frame_count,wheel_4_torque_cmd", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_NM, "Nm", DATA_FLOAT4, 1,
                           ExtractData1D_int2_float_Even, pr_float4_6 },
    }
  },
  { SUN_SENSOR5_INTENSITY, "Sun Sensor 5 Raw Intensity",
              SOURCE_HK2, MEAS_QUANTITY,
              "hk2_minor_frame_count,sun_sensor5_intensity", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },
  { CURRENT_ADCS_STATE, "Current ADCS State", SOURCE_HK2,
                   MEAS_STATUS, "hk2_minor_frame_count,SBW03", 1, {
      { UNIT_MAP, "0=Wait,1=Detumble,2=AcqSun,3=Point,4=DeltaV", DATA_UINT1,
                                  0, Extract8Bit5_7_Even, pr_uint1 }
    }
  },
  { ACT_CNTL_ENB_DSB, "Actuator Control Enbl/Dsbl", SOURCE_HK2,
                   MEAS_STATUS, "hk2_minor_frame_count,SBW03", 1, {
      { UNIT_MAP, "0=Off,1=Auto", DATA_UINT1, 0, Extract8Bit4_Even, pr_bit }
    }
  },
  { WHELL_1_OVERSPEED, "Wheel 1 Overspeed", SOURCE_HK2,
                   MEAS_STATUS, "hk2_minor_frame_count,SBW03", 1, {
      { UNIT_MAP, "0=OK,1=Fault", DATA_UINT1, 0, Extract8Bit3_Even, pr_bit }
    }
  },
  { WHELL_2_OVERSPEED, "Wheel 2 Overspeed", SOURCE_HK2,
                   MEAS_STATUS, "hk2_minor_frame_count,SBW03", 1, {
      { UNIT_MAP, "0=OK,1=Fault", DATA_UINT1, 0, Extract8Bit2_Even, pr_bit }
    }
  },
  { WHELL_3_OVERSPEED, "Wheel 3 Overspeed", SOURCE_HK2,
                   MEAS_STATUS, "hk2_minor_frame_count,SBW03", 1, {
      { UNIT_MAP, "0=OK,1=Fault", DATA_UINT1, 0, Extract8Bit1_Even, pr_bit }
    }
  },
  { WHELL_4_OVERSPEED, "Wheel 4 Overspeed", SOURCE_HK2,
                   MEAS_STATUS, "hk2_minor_frame_count,SBW03", 1, {
      { UNIT_MAP, "0=OK,1=Fault", DATA_UINT1, 0, Extract8Bit0_Even, pr_bit }
    }
  },
  { SUN_SENSOR6_INTENSITY, "Sun Sensor 6 Raw Intensity",
                  SOURCE_HK2, MEAS_QUANTITY,
                  "hk2_minor_frame_count,sun_sensor6_intensity", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },
  { STAR_TRACK_1_DATA_REQ_ENB, "Star Tracker #1 Data Requests Enabled",
               SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW04", 1, {
      { UNIT_MAP, "0=Dsbl,1=Enbl", DATA_UINT1, 0, Extract8Bit7_Even, pr_bit }
    }
  },
  { STAR_TRACK_2_DATA_REQ_ENB, "Star Tracker #2 Data Requests Enabled",
               SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW04", 1, {
      { UNIT_MAP, "0=Dsbl,1=Enbl", DATA_UINT1, 0, Extract8Bit6_Even, pr_bit }
    }
  },
  { ATT_UPDATE_RATE_STATUS, "Attitude Update Rate Status",
               SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW04", 1, {
      { UNIT_MAP, "0=Good,1=Coarse,2=Bad", DATA_UINT1,
                                  0, Extract8Bit4_5_Even, pr_uint1 }
    }
  },
  { INT_PROPAGATOR_VALID, "Internal Propagator Valid",
               SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW04", 1, {
      { UNIT_MAP, "0=Invalid,1=Valid", DATA_UINT1,
                                   0, Extract8Bit2_Even, pr_bit }
    }
  },
  { GPS_DATA_VALID_FLAG, "GPS Data Valid Flag",
               SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW04", 1, {
      { UNIT_MAP, "0=Invalid,1=Valid", DATA_UINT1,
                                   0, Extract8Bit1_Even, pr_bit }
    }
  },
  { START_GPS_CARRIER_DATA, "Start of GPS Carrier Phase Data",
               SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW04", 1, {
      { UNIT_MAP, "0=Mid,1=Start", DATA_UINT1,
                                   0, Extract8Bit0_Even, pr_bit }
    }
  },
  { STAR_TRACK_1_CCD_TEMP, "Star Tracker #1 CCD Temperature",
          SOURCE_HK2, MEAS_TEMPERATURE, 
          "hk2_minor_frame_count,star_1_ccd_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Even, pr_uint2 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                             1, ExtractData1D_uint2_float_Even, pr_float4_6 }
    }
  },
  { SUN_SENSOR7_INTENSITY, "Sun Sensor 7 Raw Intensity",
          SOURCE_HK2, MEAS_TEMPERATURE,
          "hk2_minor_frame_count,sun_sensor7_intensity", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },
  { STAR_TRACK_1_BASE_TEMP, "Star Tracker #1 Baseplate Temperature",
          SOURCE_HK2, MEAS_TEMPERATURE,
          "hk2_minor_frame_count,star_1_base_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Even, pr_uint2 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                             1, ExtractData1D_uint2_float_Even, pr_float4_6 }
    }
  },
  { SUN_SENSOR8_INTENSITY, "Sun Sensor 8 Raw Intensity",
          SOURCE_HK2, MEAS_TEMPERATURE,
          "hk2_minor_frame_count,sun_sensor8_intensity", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },
  { STAR_TRACK_1_LENS_TEMP, "Star Tracker #1 Lens Temperature",
         SOURCE_HK2, MEAS_TEMPERATURE,
         "hk2_minor_frame_count,star_1_lens_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Even, pr_uint2 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4,
                             1, ExtractData1D_uint2_float_Even, pr_float4_6 }
    }
  },
  { SUN_SENSOR9_INTENSITY, "Sun Sensor 9 Raw Intensity",
         SOURCE_HK2, MEAS_TEMPERATURE,
         "hk2_minor_frame_count,sun_sensor9_intensity", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },
  { STAR_1_P2_VOLT, "Star Tracker #1 +2 Volt Supply",
          SOURCE_HK2, MEAS_VOLTAGE,
          "hk2_minor_frame_count,star_1_p2_volt", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D_Even, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4,
                             1, ExtractData1D_uint1_float_Even, pr_float4_6 }
    }
  },
  { SUN_SENSOR10_INTENSITY, "Sun Sensor 10 Raw Intensity",
               SOURCE_HK2, MEAS_QUANTITY,
               "hk2_minor_frame_count,sun_sensor10_intensity", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },
  { STAR_1_M8_VOLT, "Star Tracker #1 -8 Volt Supply", SOURCE_HK2, MEAS_VOLTAGE,
               "hk2_minor_frame_count,star_1_m8_volt", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D_Even, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4,
                             1, ExtractData1D_uint1_float_Even, pr_float4_6 }
    }
  },
  { STAR_1_P5_VOLT, "Star Tracker #1 +5 Volt Supply", SOURCE_HK2, MEAS_VOLTAGE,
               "hk2_minor_frame_count,star_1_p5_volt", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D_Even, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4,
                             1, ExtractData1D_uint1_float_Even, pr_float4_6 }
    }
  },
  { SUN_SENSOR11_INTENSITY, "Sun Sensor 11 Raw Intensity",
               SOURCE_HK2, MEAS_QUANTITY,
               "hk2_minor_frame_count,sun_sensor11_intensity", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },
  { STAR_1_M5_VOLT, "Star Tracker #1 -5 Volt Supply", SOURCE_HK2, MEAS_VOLTAGE,
               "hk2_minor_frame_count,star_1_m5_volt", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D_Even, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4,
                             1, ExtractData1D_uint1_float_Even, pr_float4_6 }
    }
  },
  { STAR_1_BG_READ, "Star Tracker #1 Background Reading",
          SOURCE_HK2, MEAS_QUANTITY,
          "hk2_minor_frame_count,star_1_bg_read", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Even, pr_uint2 },
    }
  },
  { SUN_SENSOR12_INTENSITY, "Sun Sensor 12 Raw Intensity",
           SOURCE_HK2, MEAS_QUANTITY,
           "hk2_minor_frame_count,sun_sensor12_intensity", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },
  { STAR_1_FF_CNT, "Star Tracker #1 Full Field Search Count",
           SOURCE_HK2, MEAS_QUANTITY,
           "hk2_minor_frame_count,star_1_ff_cnt", 1, {
      { UNIT_COUNTS, "counts", DATA_UINT1, 0, ExtractData1D_Even, pr_uint1 },
    }
  },
  { STAR_1_FALALRM_CNT, "Star Tracker #1 False Alarms Count",
            SOURCE_HK2, MEAS_QUANTITY,
            "hk2_minor_frame_count,star_1_falalrm_cnt", 1, {
      { UNIT_COUNTS, "counts", DATA_UINT1, 0, ExtractData1D_Even, pr_uint1 },
    }
  },
  { SUN_SENSOR13_INTENSITY, "Sun Sensor 13 Raw Intensity",
            SOURCE_HK2, MEAS_QUANTITY,
            "hk2_minor_frame_count,sun_sensor13_intensity", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },
  { STAR_1_TLM_OFFSET, "Star Tracker #1 Telemetry Table Offset",
        SOURCE_HK2, MEAS_QUANTITY,
        "hk2_minor_frame_count,star_1_tlm_offset", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Even, pr_uint2 },
    }
  },
  { SUN_SENSOR14_INTENSITY, "Sun Sensor 14 Raw Intensity",
        SOURCE_HK2, MEAS_QUANTITY,
        "hk2_minor_frame_count,sun_sensor14_intensity", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },
  { STAR_2_CCD_TEMP, "Star Tracker #2 CCD Temperature",
                SOURCE_HK2, MEAS_TEMPERATURE,
                "hk2_minor_frame_count,star_2_ccd_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Even, pr_uint2 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                               ExtractData1D_uint2_float_Even, pr_float4_6 }
    }
  },
  { MEASURED_MAG_FIELD_X, "Measured Mag Field X", SOURCE_HK2, MEAS_QUANTITY,
                    "hk2_minor_frame_count,measured_mag_field_x", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_TELSA, "Tesla", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { STAR_2_BASE_TEMP, "Star Tracker #2 Baseplate Temperature",
                     SOURCE_HK2, MEAS_TEMPERATURE,
                     "hk2_minor_frame_count,star_2_base_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Even, pr_uint2 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                               ExtractData1D_uint2_float_Even, pr_float4_6 }
    }
  },
  { MEASURED_MAG_FIELD_Y, "Measured Mag Field Y", SOURCE_HK2, MEAS_QUANTITY,
                     "hk2_minor_frame_count,measured_mag_field_y", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_TELSA, "Tesla", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { STAR_2_LENS_TEMP, "Star Tracker #2 Lens Temperature",
                 SOURCE_HK2, MEAS_TEMPERATURE,
                 "hk2_minor_frame_count,star_2_lens_temp", 2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Even, pr_uint2 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                               ExtractData1D_uint2_float_Even, pr_float4_6 }
    }
  },
  { MEASURED_MAG_FIELD_Z, "Measured Mag Field Z", SOURCE_HK2, MEAS_QUANTITY,
                 "hk2_minor_frame_count,measured_mag_field_z", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_TELSA, "Tesla", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { STAR_2_P2_VOLT, "Star Tracker #2 +2 Volt Supply", SOURCE_HK2, MEAS_VOLTAGE,
                 "hk2_minor_frame_count,star_2_p2_volt", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D_Even, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4,
                             1, ExtractData1D_uint1_float_Even, pr_float4_6 }
    }
  },
  { STAR_2_M8_VOLT, "Star Tracker #2 -8 Volt Supply", SOURCE_HK2, MEAS_VOLTAGE,
                  "hk2_minor_frame_count,star_2_m8_volt", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D_Even, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4,
                             1, ExtractData1D_uint1_float_Even, pr_float4_6 }
    }
  },
  { MEASURED_SUN_VECTOR_X, "Measured Sun Vector X", SOURCE_HK2, MEAS_QUANTITY,
                  "hk2_minor_frame_count,measured_sun_vector_x", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { STAR_2_P5_VOLT, "Star Tracker #2 +5 Volt Supply", SOURCE_HK2, MEAS_VOLTAGE,
                  "hk2_minor_frame_count,star_2_p5_volt", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D_Even, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4,
                             1, ExtractData1D_uint1_float_Even, pr_float4_6 }
    }
  },
  { STAR_2_M5_VOLT, "Star Tracker #2 -5 Volt Supply", SOURCE_HK2, MEAS_VOLTAGE,
                  "hk2_minor_frame_count,star_2_m5_volt", 2, {
      { UNIT_DN, "dn", DATA_UINT1, 0, ExtractData1D_Even, pr_uint1 },
      { UNIT_VOLTS, "volts", DATA_FLOAT4,
                             1, ExtractData1D_uint1_float_Even, pr_float4_6 }
    }
  },
  { MEASURED_SUN_VECTOR_Y, "Measured Sun Vector Y", SOURCE_HK2, MEAS_QUANTITY,
                  "hk2_minor_frame_count,measured_sun_vector_y", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { STAR_2_BG_READ, "Tracker #2 Background Reading", SOURCE_HK2, MEAS_QUANTITY,
                   "hk2_minor_frame_count,star_2_bg_read", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Even, pr_uint2 },
    }
  },
  { MEASURED_SUN_VECTOR_Z, "Measured Sun Vector Z", SOURCE_HK2, MEAS_QUANTITY,
                   "hk2_minor_frame_count,measured_sun_vector_z", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { STAR_2_FF_CNT, "Star Tracker #2 Full Field Search Count",
                  SOURCE_HK2, MEAS_QUANTITY,
                  "hk2_minor_frame_count,star_2_ff_cnt", 1, {
      { UNIT_COUNTS, "counts", DATA_UINT1, 0, ExtractData1D_Even, pr_uint1 },
    }
  },
  { STAR_2_FALARM_CNT, "Star Tracker #2 False Alarms Count",
                  SOURCE_HK2, MEAS_QUANTITY,
                  "hk2_minor_frame_count,star_2_falarm_cnt", 1, {
      { UNIT_COUNTS, "counts", DATA_UINT1, 0, ExtractData1D_Even, pr_uint1 },
    }
  },
  { ECFF_TARGET_ID, "ECEF Target Id", SOURCE_HK2, MEAS_ID,
                  "hk2_minor_frame_count,ecff_target_id", 1, {
      { UNIT_ID, "id", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },
  { STAR_2_TLM_OFFSET, "Star Tracker #2 Telemetry Table Offset",
             SOURCE_HK2, MEAS_QUANTITY,
             "hk2_minor_frame_count,star_2_tlm_offset", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Even, pr_uint2 },
    }
  },
  { FFT_TARGET_ID, "FFT (Fixed Frame Table) Target Id",
             SOURCE_HK2, MEAS_QUANTITY,
             "hk2_minor_frame_count,fft_target_id", 1, {
      { UNIT_ID, "dn", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },
  { RATE_SEN_ATT_Q1, "Rate Sensor Attitude Q1", SOURCE_HK2, MEAS_QUANTITY,
             "hk2_minor_frame_count,rate_sen_att_q1", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { DESIRED_ATT_Q1, "Desired Attitude Q1", SOURCE_HK2, MEAS_QUANTITY,
           "hk2_minor_frame_count,desired_att_q1", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { RATE_SEN_ATT_Q2, "Rate Sensor Attitude Q2", SOURCE_HK2, MEAS_QUANTITY,
           "hk2_minor_frame_count,rate_sen_att_q2", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { DESIRED_ATT_Q2, "Desired Attitude Q2", SOURCE_HK2, MEAS_QUANTITY,
           "hk2_minor_frame_count,desired_att_q2", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { RATE_SEN_ATT_Q3, "Rate Sensor Attitude Q3", SOURCE_HK2, MEAS_QUANTITY,
           "hk2_minor_frame_count,rate_sen_att_q3", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { DESIRED_ATT_Q3, "Desired Attitude Q3", SOURCE_HK2, MEAS_QUANTITY, 
           "hk2_minor_frame_count,desired_att_q3", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { RATE_SEN_ATT_Q4, "Rate Sensor Attitude Q4", SOURCE_HK2, MEAS_QUANTITY,
           "hk2_minor_frame_count,rate_sen_att_q4", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { DESIRED_ATT_Q4, "Desired Attitude Q4", SOURCE_HK2, MEAS_QUANTITY,
          "hk2_minor_frame_count,desired_att_q4", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { CONTROL_FRAME_ATT_Q1, "Control Frame Attitude Q1",
         SOURCE_HK2, MEAS_QUANTITY,
         "hk2_minor_frame_count,control_frame_att_q1", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { NEXT_ORBIT_POS_X, "Next Orbital Position X", SOURCE_HK2, MEAS_QUANTITY,
         "hk2_minor_frame_count,next_orbit_pos_x", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_KILOMETERS, "km", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { CONTROL_FRAME_ATT_Q2, "Control Frame Attitude Q2",
         SOURCE_HK2, MEAS_QUANTITY,
         "hk2_minor_frame_count,control_frame_att_q2", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { NEXT_ORBIT_POS_Y, "Next Orbital Position Y", SOURCE_HK2, MEAS_QUANTITY,
       "hk2_minor_frame_count,next_orbit_pos_y", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_KILOMETERS, "km", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { CONTROL_FRAME_ATT_Q3, "Control Frame Attitude Q3",
         SOURCE_HK2, MEAS_QUANTITY,
         "hk2_minor_frame_count,control_frame_att_q3", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { NEXT_ORBIT_POS_Z, "Next Orbital Position Z", SOURCE_HK2, MEAS_QUANTITY,
          "hk2_minor_frame_count,next_orbit_pos_z", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_KILOMETERS, "km", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { CONTROL_FRAME_ATT_Q4, "Control Frame Attitude Q4",
         SOURCE_HK2, MEAS_QUANTITY,
         "hk2_minor_frame_count,control_frame_att_q4", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { NEXT_ORBIT_VEL_X, "Next Orbital Velocity X", SOURCE_HK2, MEAS_QUANTITY,
         "hk2_minor_frame_count,next_orbit_vel_x", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_KM_SEC, "km/sec", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { CONTROL_FRAME_RATE_X, "Control Frame Rate X", SOURCE_HK2, MEAS_RATE,
         "hk2_minor_frame_count,control_frame_rate_x", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_RAD_SEC, "radians/sec", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { NEXT_ORBIT_VEL_Y, "Next Orbital Velocity Y", SOURCE_HK2, MEAS_RATE,
          "hk2_minor_frame_count,next_orbit_vel_y", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_KM_SEC, "km/sec", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { CONTROL_FRAME_RATE_Y, "Control Frame Rate Y", SOURCE_HK2, MEAS_RATE,
          "hk2_minor_frame_count,control_frame_rate_y", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_RAD_SEC, "radians/sec", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { NEXT_ORBIT_VEL_Z, "Next Orbital Velocity Z", SOURCE_HK2, MEAS_RATE,
         "hk2_minor_frame_count,next_orbit_vel_z", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_KM_SEC, "km/sec", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { CONTROL_FRAME_RATE_Z, "Control Frame Rate Z", SOURCE_HK2, MEAS_RATE,
        "hk2_minor_frame_count,control_frame_rate_z", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_RAD_SEC, "radians/sec", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { MODEL_MAG_FIELD_VX, "Modeled Mag Field Vx", SOURCE_HK2, MEAS_RATE,
         "hk2_minor_frame_count,model_mag_field_vx", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_EU, "mGauss", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { MEASURED_ATT_Q1, "Measured Attitude Q1", SOURCE_HK2, MEAS_RATE,
           "hk2_minor_frame_count,measured_att_q1", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { MODEL_MAG_FIELD_VY, "Modeled Mag Field Vy", SOURCE_HK2, MEAS_RATE,
            "hk2_minor_frame_count,model_mag_field_vy", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_EU, "mGauss", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { MEASURED_ATT_Q2, "Measured Attitude Q2", SOURCE_HK2, MEAS_RATE,
          "hk2_minor_frame_count,measured_att_q2", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { MODEL_MAG_FIELD_VZ, "Modeled Mag Field Vz", SOURCE_HK2, MEAS_RATE,
             "hk2_minor_frame_count,model_mag_field_vz", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_EU, "mGauss", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { MEASURED_ATT_Q3, "Measured Attitude Q3", SOURCE_HK2, MEAS_RATE,
              "hk2_minor_frame_count,measured_att_q3", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { MODEL_SUN_VX, "Modeled Sun Vx", SOURCE_HK2, MEAS_RATE,
              "hk2_minor_frame_count,model_sun_vx", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { MEASURED_ATT_Q4, "Measured Attitude Q4", SOURCE_HK2, MEAS_RATE,
            "hk2_minor_frame_count,measured_att_q4", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { MODEL_SUN_VY, "Modeled Sun Vy", SOURCE_HK2, MEAS_RATE,
             "hk2_minor_frame_count,model_sun_vy", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { TOTAL_MOMENTUM_1, "Total System Momentum 1", SOURCE_HK2, MEAS_RATE,
               "hk2_minor_frame_count,total_momentum_1", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_NM, "Nm", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { MODEL_SUN_VZ, "Modeled Sun Vz", SOURCE_HK2, MEAS_RATE,
             "hk2_minor_frame_count,model_sun_vz", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { TOTAL_MOMENTUM_2, "Total System Momentum 2", SOURCE_HK2, MEAS_RATE,
          "hk2_minor_frame_count,total_momentum_2", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_NM, "Nm", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { CALC_SOLAR_ARRAY1_POS, "Calculated Solar Array 1 Position",
          SOURCE_HK2, MEAS_RATE,
          "hk2_minor_frame_count,calc_solar_array1_pos", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { TOTAL_MOMENTUM_3, "Total System Momentum 3", SOURCE_HK2, MEAS_RATE,
         "hk2_minor_frame_count,total_momentum_3", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Even, pr_int2 },
      { UNIT_NM, "Nm", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Even, pr_float4_6 }
    }
  },
  { CALC_SOLAR_ARRAY2_POS, "Calculated Solar Array 2 Position",
          SOURCE_HK2, MEAS_RATE,
          "hk2_minor_frame_count,calc_solar_array2_pos", 2, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D_Odd, pr_int2 },
      { UNIT_EU, "eu", DATA_FLOAT4, 1,
                               ExtractData1D_int2_float_Odd, pr_float4_6 }
    }
  },
  { PRI_SEC_ANG_STATUS, "Primary_Secondary Angle Status",
           SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW08", 1, {
      { UNIT_MAP, "0=Invalid, 1=Valid", DATA_UINT1, 0,
                               Extract8Bit7_Even, pr_bit }
    }
  },
  { PRI_TER_ANG_STATUS, "Primary_Tertiary Angle Status",
           SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW08", 1, {
      { UNIT_MAP, "0=Invalid, 1=Valid", DATA_UINT1, 0,
                               Extract8Bit6_Even, pr_bit }
    }
  },
  { PRI_FOUR_ANG_STATUS, "Primary_Fourth Angle Status",
           SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW08", 1, {
      { UNIT_MAP, "0=Invalid, 1=Valid", DATA_UINT1, 0,
                               Extract8Bit5_Even, pr_bit }
    }
  },
  { PRI_ORBIT_ANG_STATUS, "Primary_Orbit Angle Status",
           SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW08", 1, {
      { UNIT_MAP, "0=Invalid, 1=Valid", DATA_UINT1, 0,
                               Extract8Bit4_Even, pr_bit }
    }
  },
  { SEC_TER_ANG_STATUS, "Secondary_Tertiary Angle Status",
           SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW08", 1, {
      { UNIT_MAP, "0=Invalid, 1=Valid", DATA_UINT1, 0,
                               Extract8Bit3_Even, pr_bit }
    }
  },
  { SEC_FOUR_ANG_STATUS, "Secondary_Fourth Angle Status",
           SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW08", 1, {
      { UNIT_MAP, "0=Invalid, 1=Valid", DATA_UINT1, 0,
                               Extract8Bit2_Even, pr_bit }
    }
  },
  { SEC_ORBIT_ANG_STATUS, "Secondary_Orbit Angle Status",
           SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW08", 1, {
      { UNIT_MAP, "0=Invalid, 1=Valid", DATA_UINT1, 0,
                               Extract8Bit1_Even, pr_bit }
    }
  },
  { TER_FOUR_ANG_STATUS, "Tertiary_Fourth Angle Status",
           SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW08", 1, {
      { UNIT_MAP, "0=Invalid, 1=Valid", DATA_UINT1, 0,
                               Extract8Bit0_Even, pr_bit }
    }
  },
  { TER_ORBIT_ANG_STATUS, "Tertiary_Orbit Angle Status",
           SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW09", 1, {
      { UNIT_MAP, "0=Invalid, 1=Valid", DATA_UINT1, 0,
                               Extract8Bit7_Even, pr_bit }
    }
  },
  { FOUR_ORBIT_ANG_STATUS, "Fourth_Orbit Angle Status",
           SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW09", 1, {
      { UNIT_MAP, "0=Invalid, 1=Valid", DATA_UINT1, 0,
                               Extract8Bit6_Even, pr_bit }
    }
  },
  { ORBIT_INTERP_METHOD, "Orbit Interpolation Method Used",
           SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW09", 1, {
      { UNIT_MAP, "0=Internal, 1=GPS, 2=LAP", DATA_UINT1, 
                                  0, Extract8Bit4_5_Even, pr_uint1 }
    }
  },
  { VECTOR_PAIR_SELECTION, "Vector Pair Selection",
           SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW09", 1, {
      { UNIT_MAP, "0=V12, 1=V13,...", DATA_UINT1, 
                                  0, Extract8Bit0_3_Even, pr_uint1 }
    }
  },
  { SOLAR_ARRAY1_POT_READING, "Solar Array 1 Potentiometer Reading",
                 SOURCE_HK2, MEAS_QUANTITY,
                 "hk2_minor_frame_count,solar_array1_pot_reading", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },
  { ATT_DERTERM_METHOD, "Attitude Determination Method",
                SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW10", 1, {
      { UNIT_MAP, "0=A3RA, 1=A3R, 2=A2R, 3=None", DATA_UINT1, 0,
                               Extract8Bit6_7_Even, pr_uint1 }
    }
  },
  { ATT_UPDATE_METHOD, "Attitude Update Method",
                SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW10", 1, {
      { UNIT_MAP, "0=A3RA, 1=A3R, 2=A2R, 3=None", DATA_UINT1, 0,
                               Extract8Bit4_5_Even, pr_uint1 }
    }
  },
  { SOLUTION_STATUS, "Solution Status",
                SOURCE_HK2, MEAS_STATUS, "hk2_minor_frame_count,SBW200", 1, {
      { UNIT_MAP, "0=Both, 1=OneEach, 2=V12R, 3=V14R,...", DATA_UINT1, 0,
                               Extract8Bit3_7_Even, pr_uint1 }
    }
  },
  { SOLAR_ARRAY2_POT_READING, "Solar Array 2 Potentiometer Reading",
                 SOURCE_HK2, MEAS_QUANTITY,
                 "hk2_minor_frame_count,solar_array2_pot_reading", 1, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D_Odd, pr_uint2 },
    }
  },
  { ACTIVE_CDU, "Active CDU", SOURCE_HK2, MEAS_STATUS,
                 "hk2_minor_frame_count,active_cdu", 1, {
      { UNIT_MAP, "24=CDU1, 25=CDU2", DATA_UINT1, 0,
                               Extract8Bit3_7_Even, pr_uint1 }
    }
  },
  { CMD_ORBIT_INTERP_METHOD, "Commanded Orbit Interpolation Method",
                          SOURCE_HK2, MEAS_STATUS,
                          "hk2_minor_frame_count,SBW12", 1, {
      { UNIT_MAP, "0=Internal, 1=GPS, 2=LAP", DATA_UINT1, 0,
                               Extract8Bit5_6_Odd, pr_uint1 }
    }
  },
  { RADIUS_VIOLATION, "Radius Violation",
                          SOURCE_HK2, MEAS_STATUS,
                          "hk2_minor_frame_count,SBW12", 1, {
      { UNIT_MAP, "0=OK, 1=Fault", DATA_UINT1, 0,
                               Extract8Bit4_Odd, pr_bit }
    }
  },
  { SPEED_VIOLATION, "Speed Violation",
                          SOURCE_HK2, MEAS_STATUS,
                          "hk2_minor_frame_count,SBW12", 1, {
      { UNIT_MAP, "0=OK, 1=Fault", DATA_UINT1, 0,
                               Extract8Bit3_Odd, pr_bit }
    }
  },
  { ORBIT_STATE_VALID, "Orbital State Valid",
                          SOURCE_HK2, MEAS_STATUS,
                          "hk2_minor_frame_count,SBW12", 1, {
      { UNIT_MAP, "0=OK, 1=Fault", DATA_UINT1, 0,
                               Extract8Bit2_Odd, pr_bit }
    }
  },
  { TARGET_TABLE_SELECT, "Target Table Selection",
                          SOURCE_HK2, MEAS_STATUS,
                          "hk2_minor_frame_count,SBW12", 1, {
      { UNIT_MAP, "0=ECEF, 1=Fixed, 2=STBY", DATA_UINT1, 0,
                               Extract8Bit0_1_Odd, pr_uint1 }
    }
  },
  { MEAS_MAG_FIELD, "Measured Mag Field Status", SOURCE_HK2, MEAS_STATUS,
               "hk2_minor_frame_count,meas_mag_field+meas_sun_vector", 1, {
      { UNIT_MAP, "0=Good, 1=Coarse, 2=Bad", DATA_UINT1, 0,
                               Extract8Bit6_7_Odd, pr_uint1 }
    }
  },
  { MEAS_SUN_VECTOR, "Measured Sun Vector Status", SOURCE_HK2, MEAS_STATUS,
               "hk2_minor_frame_count,meas_mag_field+meas_sun_vector", 1, {
      { UNIT_MAP, "0=Good, 1=Coarse, 2=Bad", DATA_UINT1, 0,
                               Extract8Bit4_5_Odd, pr_uint1 }
    }
  },

  { GYRO_TEMP_1, "Gyro Temperature 1",
                  SOURCE_HK2, MEAS_TEMPERATURE, "gyro_temp_1", 2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint2_float, pr_float4_6 },
    }
  },
  { GYRO_TEMP_2, "Gyro Temperature 2",
                  SOURCE_HK2, MEAS_TEMPERATURE, "gyro_temp_2", 2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint2_float, pr_float4_6 },
    }
  },
  { GYRO_TEMP_3, "Gyro Temperature 3",
                  SOURCE_HK2, MEAS_TEMPERATURE, "gyro_temp_3", 2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractData1D, pr_uint2 },
      { UNIT_DEGREES_C, "degC", DATA_FLOAT4, 1,
                           ExtractData1D_uint2_float, pr_float4_6 },
    }
  },
 
};
 
const int HK2ParTabSize = ElementNumber(HK2ParTab);
