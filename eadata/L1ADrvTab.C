//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.13   11 Sep 1998 10:29:20   sally
// add mWatts for all dBm units
// 
//    Rev 1.12   18 Aug 1998 15:06:26   sally
// mv mWatts for transmit power to L1ADrvTab.C
// 
//    Rev 1.11   18 Aug 1998 10:56:40   sally
// make L1ADrvExtract return any number of values
// 
//    Rev 1.10   27 Jul 1998 14:00:06   sally
// passing polynomial table to extraction function
// 
//    Rev 1.9   29 Jun 1998 16:52:00   sally
// added embedded commands checking
// 
//    Rev 1.8   19 Jun 1998 16:54:14   sally
// added "Orbit Period" in L1A Derived Data  
// 
//    Rev 1.7   15 Jun 1998 11:29:34   sally
// change loop_back_cal_power to loop_back_cal_A_power loop_back_cal_B_power
// 
//    Rev 1.5   06 Apr 1998 16:27:56   sally
// merged with SVT
// 
//    Rev 1.4   01 Apr 1998 13:36:12   sally
// for L1A Derived table
// 
//    Rev 1.3   30 Mar 1998 15:13:50   sally
// added L2A parameter table
// 
//    Rev 1.2   27 Mar 1998 14:51:28   sally
// fixed some L1A_Derived stuff
// 
//    Rev 1.1   27 Mar 1998 09:58:50   sally
// added L1A Derived data
// 
//    Rev 1.0   24 Mar 1998 16:02:30   sally
// Initial revision.
// 
// $Date4
// $Revision4
// $Author$
//
//=========================================================

#include "ParTab.h"
#include "L1AExtract.h"
#include "L1ADrvExtract.h"
#include "Print.h"

//********************************************************************
//     defines a giant table to hold all L1A derived parameters.
//     Each parameter has up to 8 unit entries
//     which defines how to extract and interpret the data according
//     the unit type.
//
//     NOTE: the extraction function returns number of elements
//           which are actually returned.
//********************************************************************

static const char rcs_id_L1AParTab_C[] = "@(#) $Header$";

const ParTabEntry L1ADerivedParTab[] =
{
  { UTC_TIME, "UTC Time", SOURCE_L1A_DERIVED, MEAS_TIME, "frame_time_secs", 6, {
      { UNIT_AUTOTIME, "(auto)",DATA_ITIME, 0, ExtractTaiTime, 0 },
      { UNIT_CODE_A, "Code A",  DATA_ITIME, 0, ExtractTaiTime, pr_itime_codea },
      { UNIT_DAYS,   "days",    DATA_ITIME, 0, ExtractTaiTime, pr_itime_d },
      { UNIT_HOURS,  "hours",   DATA_ITIME, 0, ExtractTaiTime, pr_itime_h },
      { UNIT_MINUTES,"minutes", DATA_ITIME, 0, ExtractTaiTime, pr_itime_m },
      { UNIT_SECONDS,"seconds", DATA_ITIME, 0, ExtractTaiTime, pr_itime_s }
    }
  },
  { NOISE_POWER_BEAM_A, "Noise Power in Beam A", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,noise_dn,true_cal_pulse_pos,loop_back_cal_noise",
         2, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractBeamANoiseDN, pr_uint4 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamANoisedB, pr_float4_6 }
    }
  },
  { NOISE_POWER_BEAM_B, "Noise Power in Beam B", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,noise_dn,true_cal_pulse_pos,loop_back_cal_noise",
         2, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractBeamBNoiseDN, pr_uint4 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamBNoisedB, pr_float4_6 }
    }
  },
  { SLICE_1_POWER_BEAM_A, "Slice 1 Power in Beam A", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamASlice1DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamASlice1dB, pr_float4_6 }
    }
  },
  { SLICE_2_POWER_BEAM_A, "Slice 2 Power in Beam A", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamASlice2DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamASlice2dB, pr_float4_6 }
    }
  },
  { SLICE_3_POWER_BEAM_A, "Slice 3 Power in Beam A", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamASlice3DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamASlice3dB, pr_float4_6 }
    }
  },
  { SLICE_4_POWER_BEAM_A, "Slice 4 Power in Beam A", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamASlice4DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamASlice4dB, pr_float4_6 }
    }
  },
  { SLICE_5_POWER_BEAM_A, "Slice 5 Power in Beam A", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamASlice5DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamASlice5dB, pr_float4_6 }
    }
  },
  { SLICE_6_POWER_BEAM_A, "Slice 6 Power in Beam A", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamASlice6DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamASlice6dB, pr_float4_6 }
    }
  },
  { SLICE_7_POWER_BEAM_A, "Slice 7 Power in Beam A", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamASlice7DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamASlice7dB, pr_float4_6 }
    }
  },
  { SLICE_8_POWER_BEAM_A, "Slice 8 Power in Beam A", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamASlice8DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamASlice8dB, pr_float4_6 }
    }
  },
  { SLICE_9_POWER_BEAM_A, "Slice 9 Power in Beam A", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamASlice9DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamASlice9dB, pr_float4_6 }
    }
  },
  { SLICE_10_POWER_BEAM_A, "Slice 10 Power in Beam A", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamASlice10DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamASlice10dB, pr_float4_6 }
    }
  },
  { SLICE_11_POWER_BEAM_A, "Slice 11 Power in Beam A", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamASlice11DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamASlice11dB, pr_float4_6 }
    }
  },
  { SLICE_12_POWER_BEAM_A, "Slice 12 Power in Beam A", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamASlice12DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamASlice12dB, pr_float4_6 }
    }
  },
  { SLICE_1_POWER_BEAM_B, "Slice 1 Power in Beam B", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_B_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamBSlice1DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamBSlice1dB, pr_float4_6 }
    }
  },
  { SLICE_2_POWER_BEAM_B, "Slice 2 Power in Beam B", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_B_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamBSlice2DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamBSlice2dB, pr_float4_6 }
    }
  },
  { SLICE_3_POWER_BEAM_B, "Slice 3 Power in Beam B", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_B_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamBSlice3DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamBSlice3dB, pr_float4_6 }
    }
  },
  { SLICE_4_POWER_BEAM_B, "Slice 4 Power in Beam B", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_B_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamBSlice4DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamBSlice4dB, pr_float4_6 }
    }
  },
  { SLICE_5_POWER_BEAM_B, "Slice 5 Power in Beam B", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_B_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamBSlice5DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamBSlice5dB, pr_float4_6 }
    }
  },
  { SLICE_6_POWER_BEAM_B, "Slice 6 Power in Beam B", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_B_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamBSlice6DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamBSlice6dB, pr_float4_6 }
    }
  },
  { SLICE_7_POWER_BEAM_B, "Slice 7 Power in Beam B", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_B_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamBSlice7DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamBSlice7dB, pr_float4_6 }
    }
  },
  { SLICE_8_POWER_BEAM_B, "Slice 8 Power in Beam B", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_B_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamBSlice8DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamBSlice8dB, pr_float4_6 }
    }
  },
  { SLICE_9_POWER_BEAM_B, "Slice 9 Power in Beam B", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_B_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamBSlice9DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamBSlice9dB, pr_float4_6 }
    }
  },
  { SLICE_10_POWER_BEAM_B, "Slice 10 Power in Beam B", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_B_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamBSlice10DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamBSlice10dB, pr_float4_6 }
    }
  },
  { SLICE_11_POWER_BEAM_B, "Slice 11 Power in Beam B", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_B_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamBSlice11DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamBSlice11dB, pr_float4_6 }
    }
  },
  { SLICE_12_POWER_BEAM_B, "Slice 12 Power in Beam B", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_B_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractBeamBSlice12DN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamBSlice12dB, pr_float4_6 }
    }
  },
  { TOTAL_12_SLICES_BEAM_A, "Total Power in Beam A", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractBeamAPowerDN, pr_uint4 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamAPowerdB, pr_float4_6 }
    }
  },
  { TOTAL_12_SLICES_BEAM_B, "Total Power in Beam B", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,loop_back_cal_B_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractBeamBPowerDN, pr_uint4 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBeamBPowerdB, pr_float4_6 }
    }
  },
  { NOISE_LOAD, "Noise Load Power", SOURCE_L1A_DERIVED, MEAS_QUANTITY,
         "operational_mode,noise_dn,true_cal_pulse_pos,load_cal_noise",
         2, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractNoiseLoadDN, pr_uint4 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractNoiseLoaddB, pr_float4_6 }
    }
  },
  { SLICE_1_LOAD_POWER, "Slice 1 Echo Load Power", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,load_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractSlice1LoadPowerDN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractSlice1LoadPowerdB,pr_float4_6}
    }
  },
  { SLICE_2_LOAD_POWER, "Slice 2 Echo Load Power", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,load_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractSlice2LoadPowerDN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractSlice2LoadPowerdB,pr_float4_6}
    }
  },
  { SLICE_3_LOAD_POWER, "Slice 3 Echo Load Power", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,load_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractSlice3LoadPowerDN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractSlice3LoadPowerdB,pr_float4_6}
    }
  },
  { SLICE_4_LOAD_POWER, "Slice 4 Echo Load Power", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,load_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractSlice4LoadPowerDN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractSlice4LoadPowerdB,pr_float4_6}
    }
  },
  { SLICE_5_LOAD_POWER, "Slice 5 Echo Load Power", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,load_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractSlice5LoadPowerDN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractSlice5LoadPowerdB,pr_float4_6}
    }
  },
  { SLICE_6_LOAD_POWER, "Slice 6 Echo Load Power", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,load_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractSlice6LoadPowerDN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractSlice6LoadPowerdB,pr_float4_6}
    }
  },
  { SLICE_7_LOAD_POWER, "Slice 7 Echo Load Power", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,load_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractSlice7LoadPowerDN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractSlice7LoadPowerdB,pr_float4_6}
    }
  },
  { SLICE_8_LOAD_POWER, "Slice 8 Echo Load Power", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,load_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractSlice8LoadPowerDN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractSlice8LoadPowerdB,pr_float4_6}
    }
  },
  { SLICE_9_LOAD_POWER, "Slice 9 Echo Load Power", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,load_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractSlice9LoadPowerDN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractSlice9LoadPowerdB,pr_float4_6}
    }
  },
  { SLICE_10_LOAD_POWER, "Slice 10 Echo Load Power", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,load_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractSlice10LoadPowerDN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractSlice10LoadPowerdB,pr_float4_6}
    }
  },
  { SLICE_11_LOAD_POWER, "Slice 11 Echo Load Power", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,load_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractSlice11LoadPowerDN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractSlice11LoadPowerdB,pr_float4_6}
    }
  },
  { SLICE_12_LOAD_POWER, "Slice 12 Echo Load Power", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,load_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT2, 0, ExtractSlice12LoadPowerDN, pr_uint2 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractSlice12LoadPowerdB,pr_float4_6}
    }
  },
  { TOTAL_LOAD_POWER, "Total Echo Load Power", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,power_dn,true_cal_pulse_pos,load_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_UINT4, 0, ExtractTotalLoadPowerDN, pr_uint4 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractTotalLoadPowerdB,pr_float4_6}
    }
  },
  { BANDWIDTH_RATIO_NOISE_ECHO_ALPHA,
         "Bandwidth ratio of Noise to Echo - Alpha", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,noise_dn,power_dn,true_cal_pulse_pos,load_cal_noise,load_cal_power",
         2, {
      { UNIT_DN, "dn", DATA_FLOAT4, 0, ExtractBandwidthRatioDN, pr_float4_6 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractBandwidthRatiodB, pr_float4_6 }
    }
  },
  { GAIN_RATIO_NOISE_ECHO_BEAM_A_BETA,
         "Gain ratio of Noise to Echo in Beam A - Beta", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,noise_dn,power_dn,true_cal_pulse_pos,loop_back_cal_noise,loop_back_cal_A_power",
         2, {
      { UNIT_DN, "dn", DATA_FLOAT4, 0, ExtractGainRatioBeamADN, pr_float4_6 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractGainRatioBeamAdB, pr_float4_6 }
    }
  },
  { GAIN_RATIO_NOISE_ECHO_BEAM_B_BETA,
         "Gain ratio of Noise to Echo in Beam B - Beta", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,noise_dn,power_dn,true_cal_pulse_pos,loop_back_cal_noise,loop_back_cal_B_power",
         2, {
      { UNIT_DN, "dn", DATA_FLOAT4, 0, ExtractGainRatioBeamBDN, pr_float4_6 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractGainRatioBeamBdB, pr_float4_6 }
    }
  },
  { GAIN_RATIO_NOISE_ECHO_BEAM_B_BETA,
         "Gain ratio of Noise to Echo in Beam B - Beta", SOURCE_L1A_DERIVED,
         MEAS_QUANTITY,
         "operational_mode,noise_dn,power_dn,true_cal_pulse_pos,loop_back_cal_noise,loop_back_cal_B_power",
         2, {
      { UNIT_DN, "dn", DATA_FLOAT4, 0, ExtractGainRatioBeamBDN, pr_float4_6 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractGainRatioBeamBdB, pr_float4_6 }
    }
  },
  { TRANSMIT_PWR_A, "Transmit Power A", SOURCE_L1A_DERIVED, MEAS_POWER,
            "transmit_power_a", 1, {
      // this won't be in polynomial directly, but it needs
      // the polynomial applied to dBm first
      { UNIT_MWATTS, "mWatts", DATA_FLOAT4, 0,
                                ExtractXmitPowerAmWatts, pr_float4_6 }
    }
  },
  { TRANSMIT_PWR_B, "Transmit Power B", SOURCE_L1A_DERIVED, MEAS_POWER,
            "transmit_power_b", 1, {
      // this won't be in polynomial directly, but it needs
      // the polynomial applied to dBm first
      { UNIT_MWATTS, "mWatts", DATA_FLOAT4, 0,
                                ExtractXmitPowerBmWatts, pr_float4_6 }
    }
  },
  { TWT_1_DRIVE_PWR, "TWT 1 Drive Power", SOURCE_L1A_DERIVED, MEAS_POWER,
            "twt1_drive_power", 1, {
      // this won't be in polynomial directly, but it needs
      // the polynomial applied to dBm first
      { UNIT_MWATTS, "mWatts", DATA_FLOAT4, 0,
                                ExtractTwt1PowermWatts, pr_float4_6 }
    }
  },
  { TWT_2_DRIVE_PWR, "TWT 2 Drive Power", SOURCE_L1A_DERIVED, MEAS_POWER,
            "twt2_drive_power", 1, {
      // this won't be in polynomial directly, but it needs
      // the polynomial applied to dBm first
      { UNIT_MWATTS, "mWatts", DATA_FLOAT4, 0,
                                ExtractTwt2PowermWatts, pr_float4_6 }
    }
  },
  { PWR_CONVERT_CURRENT, "PWR Convert Current", SOURCE_L1A_DERIVED,
            MEAS_CURRENT, "power_convert_current", 1, {
      // this won't be in polynomial directly, but it needs
      // the polynomial applied to dBm first
      { UNIT_MWATTS, "mWatts", DATA_FLOAT4, 0,
                                ExtractPowerCnvtCurrmWatts, pr_float4_6 }
    }
  },
  { TRANSMIT_POWER_INNER, "Transmit Power Inner", SOURCE_L1A_DERIVED,
            MEAS_POWER, "transmit_power_inner", 1, {
      // this won't be in polynomial directly, but it needs
      // the polynomial applied to dBm first
      { UNIT_MWATTS, "mWatts", DATA_FLOAT4, 0,
                                ExtractXmitPwrInnermWatts, pr_float4_6 }
    }
  },
  { TRANSMIT_POWER_OUTER, "Transmit Power Outer", SOURCE_L1A_DERIVED,
            MEAS_POWER, "transmit_power_outer", 1, {
      // this won't be in polynomial directly, but it needs
      // the polynomial applied to dBm first
      { UNIT_MWATTS, "mWatts", DATA_FLOAT4, 0,
                                ExtractXmitPwrOutermWatts, pr_float4_6 }
    }
  },
  { RECEIVER_GAIN_A, "Receiver Gain A", SOURCE_L1A_DERIVED, MEAS_QUANTITY,
         "operational_mode,noise_dn,true_cal_pulse_pos,loop_back_cal_noise,transmit_power_a",
         2, {
      { UNIT_DN, "dn", DATA_FLOAT4, 0, ExtractReceiverGainADN, pr_float4_6 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractReceiverGainAdB, pr_float4_6 }
    }
  },
  { RECEIVER_GAIN_B, "Receiver Gain B", SOURCE_L1A_DERIVED, MEAS_QUANTITY,
         "operational_mode,noise_dn,true_cal_pulse_pos,loop_back_cal_noise,transmit_power_b",
         2, {
      { UNIT_DN, "dn", DATA_FLOAT4, 0, ExtractReceiverGainBDN, pr_float4_6 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractReceiverGainBdB, pr_float4_6 }
    }
  },
  { NOISE_FIGURE_A, "Noise Figure A", SOURCE_L1A_DERIVED, MEAS_QUANTITY,
         "operational_mode,noise_dn,true_cal_pulse_pos,load_cal_noise,loop_back_cal_noise,transmit_power_a",
         2, {
      { UNIT_DN, "dn", DATA_FLOAT4, 0, ExtractNoiseFigureADN, pr_float4_6 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractNoiseFigureAdB, pr_float4_6 }
    }
  },
  { NOISE_FIGURE_B, "Noise Figure B", SOURCE_L1A_DERIVED, MEAS_QUANTITY,
         "operational_mode,noise_dn,true_cal_pulse_pos,load_cal_noise,loop_back_cal_noise,transmit_power_b",
         2, {
      { UNIT_DN, "dn", DATA_FLOAT4, 0, ExtractNoiseFigureBDN, pr_float4_6 },
      { UNIT_DB, "dB", DATA_FLOAT4, 0, ExtractNoiseFigureBdB, pr_float4_6 }
    }
  },

  { ORBIT_PERIOD, "Orbit Period", SOURCE_L1A_DERIVED, MEAS_QUANTITY,
               "orbit_time", 1, {
      { UNIT_COUNTS, "counts", DATA_UINT4, 0, ExtractOrbitPeriod, pr_uint4 }
    }
  },
  { ANT_SPIN_RATE, "Antenna Spin Rate", SOURCE_L1A_DERIVED, MEAS_QUANTITY,
               "antenna_position,prf_cycle_time", 4, {
      { UNIT_DN, "dn", DATA_UINT2_100, 0, ExtractAntSpinRateDN, pr_uint2_100 },
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_100, 0,
                             ExtractAntSpinRateDegree, pr_float4_6_100 },
      { UNIT_DEG_SEC, "degrees/sec", DATA_FLOAT4_100, 0,
                             ExtractAntSpinRateDegSec, pr_float4_6_100 },
      { UNIT_ROT_MIN, "rotation/min", DATA_FLOAT4_100, 0,
                             ExtractAntSpinRateRotMin, pr_float4_6_100 }
    }
  },
};

const int L1ADerivedTabSize = ElementNumber(L1ADerivedParTab);
