/*=========================================================
** Copyright  (C)1996, California Institute of Technology. 
** U.S. Government sponsorship under 
** NASA Contract NAS7-1260 is acknowledged
**
**
** CM Log
** $Log$
// 
//    Rev 1.0   18 Aug 1998 11:04:10   sally
// Initial revision.
** 
**    Rev 1.5   01 Jul 1998 09:42:48   sally
** un-expand the fields
** 
**    Rev 1.4   01 May 1998 14:45:02   sally
** added HK2 file
** 
**    Rev 1.3   28 Apr 1998 15:57:22   sally
** added scatterometer housekeeping (1553) data for HK2
** 
**    Rev 1.2   27 Apr 1998 15:49:28   sally
** update HK2 data
** 
**    Rev 1.1   17 Apr 1998 16:48:38   sally
** add L2A and L2B parameter tables
** 
**    Rev 1.0   14 Apr 1998 15:09:14   sally
** Initial revision.
** 
** $Date$
** $Revision$
** $Author$
**
=========================================================*/

static const char Hk2HdfMap_C_id[] =
    "@(#) $Header$";

#include "hk2_hdf_map.h"
#include "frame_read.h"

#define ElementNumber(arr) ((unsigned int) (sizeof(arr) / sizeof(arr[0])))

/*---------------------------------------------------------
** bit order: from right to left, i.e. 7 6 5 4 3 2 1 0
---------------------------------------------------------*/

const Hk2HdfMapEntry Hk2HdfMapTable[] =
{
    { "hk2_time", "TAITIME", DATA_FLOAT8, 1.0, 0, FrameRead8Bytes },
    { "torque_rod_status", "SBW13", DATA_UINT1, 1.0, 14, FrameRead1Byte },
    { "SBW05", "SBW05", DATA_UINT1, 1.0, 15, FrameRead1Byte },
    { "fltsw_cmd_acc_cnt", "SWACPT", DATA_UINT1, 1.0, 16, FrameRead1Byte },
    { "fltsw_cmd_rej_cnt", "SWRJCT", DATA_UINT1, 1.0, 17, FrameRead1Byte },
    { "cmd_status", "SWCMDST", DATA_UINT2, 1.0, 18, FrameRead2Bytes },
    { "csm_exec_cmd_cnt", "SWCSMCNT", DATA_UINT2, 1.0, 20, FrameRead2Bytes },
    { "cbm_exec_cmd_cnt", "SWCBMCNT", DATA_UINT2, 1.0, 22, FrameRead2Bytes },
    { "table_upload_rx_cnt", "SWTBYTES", DATA_UINT2, 1.0, 24, FrameRead2Bytes },
    { "table_checksum_acc_cnt", "SWTCACPT", DATA_UINT1,1.0,26,FrameRead1Byte},
    { "table_checksum_rej_cnt", "SWTCRJCT", DATA_UINT1,1.0,27,FrameRead1Byte},
    { "SBW02", "SBW02", DATA_UINT1, 1.0, 28, FrameRead1Byte},
    { "fltsw_minor_frame_time","SWMFTIME",DATA_UINT4, 1.0, 29, FrameRead3Bytes},

    { "sas_ea_a_pwr_temp", "SCSASPST", DATA_UINT1, 1.0, 32, FrameRead1Byte },
    { "sas_duplex_bearing_temp","SCSASDBRGT",DATA_UINT1, 1.0,33,FrameRead1Byte},
    { "ses_temp_1", "SCSEST1", DATA_UINT1, 1.0, 34, FrameRead1Byte },
    { "twt1_base_temp", "SCTWTAT1", DATA_UINT1, 1.0, 35, FrameRead1Byte },
    { "cds_a_temp", "SCCDSAT", DATA_UINT1, 1.0, 36, FrameRead1Byte },
    { "hk2_psu_temp", "SCPSU1T", DATA_UINT1, 1.0, 37, FrameRead1Byte },

    { "SBW201", "SBW201", DATA_UINT1, 1.0, 38, FrameRead1Byte },
    { "SBW202", "SBW202", DATA_UINT1, 1.0, 39, FrameRead1Byte },

    /*---------------------------------------------
    ** Scatterometer Housekeeping Data (1553)
    ---------------------------------------------*/
    { "cds_psu_bus_volt", "SCCDSPSUEV", DATA_UINT1, 1.0, 40, FrameRead1Byte },
    { "cds_prim_bus_curr","SCCDSBUSI", DATA_UINT1, 1.0, 41, FrameRead1Byte },
    { "sasa_prim_bus_curr","SCSASABUSI", DATA_UINT1, 1.0, 42,FrameRead1Byte},
    { "sasb_prim_bus_curr","SCSASBBUSI", DATA_UINT1, 1.0, 43,FrameRead1Byte},
    { "sesa_prim_bus_curr","SCSESABUSI", DATA_UINT1, 1.0, 44,FrameRead1Byte},
    { "sesb_prim_bus_curr","SCSESBBUSI", DATA_UINT1, 1.0, 45,FrameRead1Byte},
    { "twta1_prim_bus_curr","SCTW1BUSI", DATA_UINT1, 1.0, 46,FrameRead1Byte},
    { "twta2_prim_bus_curr","SCTW2BUSI", DATA_UINT1, 1.0, 47,FrameRead1Byte},
    { "twta1_body_reg_curr","SCTW1REGV", DATA_UINT1, 1.0, 48,FrameRead1Byte},
    { "twta2_body_reg_curr","SCTW2REGV", DATA_UINT1, 1.0, 49,FrameRead1Byte},
    { "twta1_ion_pump_curr","SCTW1PMPC", DATA_UINT1, 1.0, 50,FrameRead1Byte},
    { "twta2_ion_pump_curr","SCTW2PMPC", DATA_UINT1, 1.0, 51,FrameRead1Byte},
    { "twta_drive_pwr","SCTWDRVPW", DATA_UINT1, 1.0, 52,FrameRead1Byte},
    { "ses_trs_xmit_pwr","SCSESXMPW", DATA_UINT1, 1.0, 53,FrameRead1Byte},
    { "sasa_eaa_motor_curr","SCEAAMTRI", DATA_UINT1, 1.0, 54,FrameRead1Byte},
    { "sasb_eab_motor_curr","SCEABMTRI", DATA_UINT1, 1.0, 55,FrameRead1Byte},
    { "sasa_eaa_spin_rate","SCSASASPIN", DATA_UINT1, 1.0, 56,FrameRead1Byte},
    { "sasb_eab_spin_rate","SCSASBSPIN", DATA_UINT1, 1.0, 57,FrameRead1Byte},
    { "sasa_eaa_saa_sec_volt","SCEAAP14V", DATA_UINT1, 1.0, 58,FrameRead1Byte},
    { "sasb_eab_saa_sec_volt","SCEABP14V", DATA_UINT1, 1.0, 59,FrameRead1Byte},
    { "cds_psu_3_temp","SCPSU3T", DATA_UINT1, 1.0, 60, FrameRead1Byte },
    { "cds_idp_a_temp","SCIDPAT", DATA_UINT1, 1.0, 61, FrameRead1Byte },
    { "cds_idp_b_temp","SCIDPBT", DATA_UINT1, 1.0, 62, FrameRead1Byte },
    { "twta_1_hvps_chas_temp","SCTW1HVPST",DATA_UINT1, 1.0, 63, FrameRead1Byte},
    { "twta_2_hvps_chas_temp","SCTW2HVPST",DATA_UINT1, 1.0, 64, FrameRead1Byte},
    { "twta_1_base_temp","SCTW1BASET",DATA_UINT1, 1.0, 65, FrameRead1Byte},
    { "twta_2_base_temp","SCTW2BASET",DATA_UINT1, 1.0, 66, FrameRead1Byte},
    { "ses_dc_conv_temp","SCSESCNVT",DATA_UINT1, 1.0, 67, FrameRead1Byte},
    { "ses_prec_coupler_temp","SCSESCPLRT",DATA_UINT1, 1.0, 68, FrameRead1Byte},
    { "ses_gain_atten_temp","SCSESATTNT",DATA_UINT1, 1.0, 69, FrameRead1Byte},
    { "ses_rx_protect_sw_temp","SCSESRPST",DATA_UINT1, 1.0, 70, FrameRead1Byte},
    { "sas_rj_temp","SCSASRJT",DATA_UINT1, 1.0, 71, FrameRead1Byte},
    { "SCBW1","SCBW1",DATA_UINT1, 1.0, 72, FrameRead1Byte},
    { "inst_mode","SCINSTMODE",DATA_UINT1, 1.0, 73, FrameRead1Byte},
    { "valid_cmd_cnt","SCVCMDCNT",DATA_UINT1, 1.0, 74, FrameRead1Byte},
    { "invalid_cmd_cnt","SCICMDCNT",DATA_UINT1, 1.0, 75, FrameRead1Byte},
    { "SCBW2", "SCBW2", DATA_UINT1, 1.0, 76, FrameRead1Byte},
    { "SCBW3","SCBW3", DATA_UINT1, 1.0, 77, FrameRead1Byte},
    { "SCBW4", "SCBW4", DATA_UINT1, 1.0, 78, FrameRead1Byte},
    { "SCBW5", "SCBW5", DATA_UINT1, 1.0, 79, FrameRead1Byte},

    /*---------------------------------------------
    ** GPS Receivers
    ---------------------------------------------*/
    { "gps_time_tag_month", "ADGPSMNTH", DATA_UINT1, 1.0, 80, FrameRead1Byte },
    { "gps_time_tag_day", "ADGPSDAY", DATA_UINT1, 1.0, 81, FrameRead1Byte },
    { "gps_time_tag_year", "ADGPSYEAR", DATA_UINT2, 1.0, 82, FrameRead2Bytes },
    { "gps_time_tag_hour", "ADGPSHOUR", DATA_UINT1, 1.0, 84, FrameRead1Byte },
    { "gps_time_tag_min", "ADGPSMIN", DATA_UINT1, 1.0, 85, FrameRead1Byte },
    { "gps_time_tag_sec", "ADGPSSEC", DATA_UINT1, 1.0, 86, FrameRead1Byte },
    { "gps_time_tag_nsec", "ADGPSNANO", DATA_UINT4, 1.0, 87, FrameRead4Bytes },
    { "current_dop", "ADGPSDOP", DATA_UINT2, 1.0, 91, FrameRead2Bytes },
    { "num_visible_sat", "ADGPSVSAT", DATA_UINT1, 1.0, 93, FrameRead1Byte },
    { "num_sat_tracked", "ADGPSNSAT", DATA_UINT1, 1.0, 94, FrameRead1Byte },
    { "sat_1_id", "ADGPS1ID", DATA_UINT1, 1.0, 95, FrameRead1Byte },
    { "sat_1_track_mode", "ADGPS1MOD", DATA_UINT1, 1.0, 96, FrameRead1Byte},
    { "sat_2_id", "ADGPS2ID", DATA_UINT1, 1.0, 97, FrameRead1Byte },
    { "sat_2_track_mode", "ADGPS2MOD", DATA_UINT1, 1.0, 98, FrameRead1Byte},
    { "sat_3_id", "ADGPS3ID", DATA_UINT1, 1.0, 99, FrameRead1Byte },
    { "sat_3_track_mode", "ADGPS3MOD", DATA_UINT1, 1.0, 100, FrameRead1Byte},
    { "sat_4_id", "ADGPS4ID", DATA_UINT1, 1.0, 101, FrameRead1Byte },
    { "sat_4_track_mode", "ADGPS4MOD", DATA_UINT1, 1.0, 102, FrameRead1Byte},
    { "sat_5_id", "ADGPS5ID", DATA_UINT1, 1.0, 103, FrameRead1Byte },
    { "sat_5_track_mode", "ADGPS5MOD", DATA_UINT1, 1.0, 104, FrameRead1Byte},
    { "sat_6_id", "ADGPS6ID", DATA_UINT1, 1.0, 105, FrameRead1Byte },
    { "sat_6_track_mode", "ADGPS6MOD", DATA_UINT1, 1.0, 106, FrameRead1Byte},
    { "sat_7_id", "ADGPS7ID", DATA_UINT1, 1.0, 107, FrameRead1Byte },
    { "sat_7_track_mode", "ADGPS7MOD", DATA_UINT1, 1.0, 108, FrameRead1Byte},
    { "sat_8_id", "ADGPS8ID", DATA_UINT1, 1.0, 109, FrameRead1Byte },
    { "sat_8_track_mode", "ADGPS8MOD", DATA_UINT1, 1.0, 110, FrameRead1Byte},
    { "sat_9_id", "ADGPS9ID", DATA_UINT1, 1.0, 111, FrameRead1Byte },
    { "sat_9_track_mode", "ADGPS9MOD", DATA_UINT1, 1.0, 112, FrameRead1Byte},
    { "sat_10_id", "ADGPS10ID", DATA_UINT1, 1.0, 113, FrameRead1Byte },
    { "sat_10_track_mode", "ADGPS10MOD", DATA_UINT1, 1.0, 114, FrameRead1Byte},
    { "sat_11_id", "ADGPS11ID", DATA_UINT1, 1.0, 115, FrameRead1Byte },
    { "sat_11_track_mode", "ADGPS11MOD", DATA_UINT1, 1.0, 116, FrameRead1Byte},
    { "sat_12_id", "ADGPS12ID", DATA_UINT1, 1.0, 117, FrameRead1Byte },
    { "sat_12_track_mode", "ADGPS12MOD", DATA_UINT1, 1.0, 118, FrameRead1Byte},
    { "SBW11","SBW11", DATA_UINT1, 1.0, 119, FrameRead1Byte},

    { "gps_pos_x_raw_1", "ADGPSPOSX1", DATA_UINT4, 1.0, 120, FrameRead4Bytes },
    { "gps_pos_x_raw_2", "ADGPSPOSX2", DATA_UINT4, 1.0, 124, FrameRead4Bytes },
    { "gps_pos_y_raw_1", "ADGPSPOSY1", DATA_UINT4, 1.0, 128, FrameRead4Bytes },
    { "gps_pos_y_raw_2", "ADGPSPOSY2", DATA_UINT4, 1.0, 132, FrameRead4Bytes },
    { "gps_pos_z_raw_1", "ADGPSPOSZ1", DATA_UINT4, 1.0, 136, FrameRead4Bytes },
    { "gps_pos_z_raw_2", "ADGPSPOSZ2", DATA_UINT4, 1.0, 140, FrameRead4Bytes },
    { "gps_vel_x_raw_1", "ADGPSVELX1", DATA_UINT4, 1.0, 144, FrameRead4Bytes },
    { "gps_vel_x_raw_2", "ADGPSVELX2", DATA_UINT4, 1.0, 148, FrameRead4Bytes },
    { "gps_vel_y_raw_1", "ADGPSVELY1", DATA_UINT4, 1.0, 152, FrameRead4Bytes },
    { "gps_vel_y_raw_2", "ADGPSVELY2", DATA_UINT4, 1.0, 156, FrameRead4Bytes },
    { "gps_vel_z_raw_1", "ADGPSVELZ1", DATA_UINT4, 1.0, 160, FrameRead4Bytes },
    { "gps_vel_z_raw_2", "ADGPSVELZ2", DATA_UINT4, 1.0, 164, FrameRead4Bytes },

    { "gps_vtcw_1", "ADGPSVTCW1", DATA_UINT2, 1.0, 168, FrameRead2Bytes },
    { "gps_vtcw_2", "ADGPSVTCW2", DATA_UINT2, 1.0, 170, FrameRead2Bytes },
    { "gps_vtcw_3", "ADGPSVTCW3", DATA_UINT2, 1.0, 172, FrameRead2Bytes },
    { "SBW06", "SBW06", DATA_UINT1, 1.0, 174, FrameRead1Byte },
    { "SBW07", "SBW07", DATA_UINT1, 1.0, 175, FrameRead1Byte},

    { "wheel_1_speed_dir", "ADW1SDIR", DATA_INT2, 1.0, 176, FrameRead2Bytes},
    { "wheel_2_speed_dir", "ADW2SDIR", DATA_INT2, 1.0, 178, FrameRead2Bytes},
    { "wheel_3_speed_dir", "ADW3SDIR", DATA_INT2, 1.0, 180, FrameRead2Bytes},
    { "wheel_4_speed_dir", "ADW4SDIR", DATA_INT2, 1.0, 182, FrameRead2Bytes},
    { "status_code", "SSRSTATUS", DATA_UINT1, 1.0, 186, FrameRead1Byte},
    { "command_id", "SSRCMDID", DATA_UINT1, 1.0, 187, FrameRead1Byte},
    { "tlm_format", "SSRTLMFMT", DATA_UINT1, 1.0, 188, FrameRead1Byte},

    { "ssr_resv_1", "SSRSVD1", DATA_UINT1, 1.0, 189, FrameRead1Byte},
    { "SSR04-06", "SSR04-06", DATA_UINT1, 1.0, 190, FrameRead1Byte },
    { "SSR07-12", "SSR07-12", DATA_UINT1, 1.0, 191, FrameRead1Byte },
    { "addr_sw_exception", "SSRSEADDR", DATA_UINT2, 1.0, 192, FrameRead2Bytes },
    { "corr_mem_err_cnt", "SSRCPMERR", DATA_UINT1, 1.0, 194, FrameRead1Byte },
    { "uncorr_mem_err_cnt", "SSRUPMERR", DATA_UINT1, 1.0, 195, FrameRead1Byte },
    { "ssr1_rx_status", "SSRRXST", DATA_UINT1, 1.0, 196, FrameRead1Byte },
    { "ssr1_xmit_status", "SSRXMST", DATA_UINT1, 1.0, 197, FrameRead1Byte },
    { "ssr2_rx_status", "SSR2RXST", DATA_UINT1, 1.0, 198, FrameRead1Byte },
    { "ssr2_xmit_status", "SSR2XMST", DATA_UINT1, 1.0, 199, FrameRead1Byte },
    { "part_0_ssr1_rec_addr", "SSRP0REC", DATA_UINT4, 1.0, 201,FrameRead3Bytes},
    { "part_0_ssr2_rec_addr", "SSR2P0REC",DATA_UINT4, 1.0, 204,FrameRead3Bytes},
    { "part_0_ssr1_unfix_err", "SSRP0UE",DATA_UINT1, 1.0, 208,FrameRead1Byte},
    { "part_0_ssr1_pbk_addr", "SSRP0PBK", DATA_UINT4, 1.0, 209,FrameRead3Bytes},
    { "part_0_ssr2_unfix_err", "SSR2P0UE",DATA_UINT1, 1.0, 212,FrameRead1Byte},
    { "part_0_ssr2_pbk_addr", "SSR2P0PBK",DATA_UINT4, 1.0, 213,FrameRead3Bytes},
    { "part_1_ssr1_rec_addr", "SSRP1REC", DATA_UINT4, 1.0, 217,FrameRead3Bytes},
    { "part_1_ssr2_rec_addr", "SSR2P1REC",DATA_UINT4, 1.0, 221,FrameRead3Bytes},
    { "part_1_ssr1_unfix_err", "SSRP1UE",DATA_UINT1, 1.0, 224,FrameRead1Byte},
    { "part_1_ssr1_pbk_addr", "SSRP1PBK", DATA_UINT4, 1.0, 225,FrameRead3Bytes},
    { "part_1_ssr2_unfix_err", "SSR2P1UE",DATA_UINT1, 1.0, 228,FrameRead1Byte},
    { "part_1_ssr2_pbk_addr", "SSR2P1PBK",DATA_UINT4, 1.0, 229,FrameRead3Bytes},

    { "ssr1_corr_mem_flag", "SSRCMFLG", DATA_UINT1, 1.0, 232, FrameRead1Byte},
    { "ssr1_corr_mem_err_cnt", "SSRCMERR",DATA_UINT4, 1.0,233,FrameRead3Bytes},
    { "ssr2_corr_mem_flag", "SSR2CMFLG", DATA_UINT1, 1.0, 236, FrameRead1Byte},
    { "ssr2_corr_mem_err_cnt", "SSR2CMERR",DATA_UINT4, 1.0,237,FrameRead3Bytes},
    { "ssr1_uncorr_mem_flag", "SSR1UMFLG", DATA_UINT1, 1.0, 240,FrameRead1Byte},
    { "ssr1_uncorr_mem_err_cnt","SSRUMERR",DATA_UINT4,1.0,241,FrameRead3Bytes},
    { "ssr2_uncorr_mem_flag", "SSR2UMFLG", DATA_UINT1, 1.0, 244,FrameRead1Byte},
    { "ssr2_uncorr_mem_err_cnt","SSR2UMERR",DATA_UINT4,1.0,245,FrameRead3Bytes},

    { "msg_checksum", "SSRMCSUM", DATA_UINT2, 1.0, 248, FrameRead2Bytes },
    { "adcs_cycle_start_1", "ADCSVTCW1", DATA_UINT2, 1.0, 250,FrameRead2Bytes },
    { "adcs_cycle_start_2", "ADCSVTCW2", DATA_UINT2, 1.0, 252,FrameRead2Bytes },
    { "adcs_cycle_start_3", "ADCSVTCW3", DATA_UINT2, 1.0, 254,FrameRead2Bytes },
    { "star_trkr_att_q1", "ADSTAQ1", DATA_INT2, 1.0, 262, FrameRead2Bytes },
    { "star_trkr_att_q2", "ADSTAQ2", DATA_INT2, 1.0, 264, FrameRead2Bytes },
    { "star_trkr_att_q3", "ADSTAQ3", DATA_INT2, 1.0, 266, FrameRead2Bytes },
    { "star_trkr_att_q4", "ADSTAQ4", DATA_INT2, 1.0, 268, FrameRead2Bytes },

    { "axis1_avg_prd1", "ADGYA1D1", DATA_UINT2, 1.0, 270, FrameRead2Bytes },
    { "axis2_avg_prd1", "ADGYA2D1", DATA_UINT2, 1.0, 272, FrameRead2Bytes },
    { "axis3_avg_prd1", "ADGYA3D1", DATA_UINT2, 1.0, 274, FrameRead2Bytes },
    { "axis1_avg_prd2", "ADGYA1D2", DATA_UINT2, 1.0, 276, FrameRead2Bytes },
    { "axis2_avg_prd2", "ADGYA2D2", DATA_UINT2, 1.0, 278, FrameRead2Bytes },
    { "axis3_avg_prd2", "ADGYA3D2", DATA_UINT2, 1.0, 280, FrameRead2Bytes },
    { "axis1_avg_prd3", "ADGYA1D3", DATA_UINT2, 1.0, 282, FrameRead2Bytes },
    { "axis2_avg_prd3", "ADGYA2D3", DATA_UINT2, 1.0, 284, FrameRead2Bytes },

    /*---------------------------------------------
    ** FSW Subcoms
    ---------------------------------------------*/
    { "control_torque_x", "ADCTX",   DATA_INT2, 1.0, 286, FrameRead2Bytes },
    { "control_torque_y", "ADCTY",   DATA_INT2, 1.0, 288, FrameRead2Bytes },
    { "control_torque_z", "ADCTZ",   DATA_INT2, 1.0, 290, FrameRead2Bytes },
    { "max_residual",  "ADMAXRES",   DATA_INT2, 1.0, 292, FrameRead2Bytes },
    { "wheel_1_torque_cmd", "ADW1TCMD", DATA_INT2, 1.0, 294, FrameRead2Bytes },
    { "wheel_2_torque_cmd", "ADW2TCMD", DATA_INT2, 1.0, 296, FrameRead2Bytes },
    { "wheel_3_torque_cmd", "ADW3TCMD", DATA_INT2, 1.0, 298, FrameRead2Bytes },
    { "wheel_4_torque_cmd", "ADW4TCMD", DATA_INT2, 1.0, 300, FrameRead2Bytes },
    { "wheel_4_torque_cmd", "ADW4TCMD", DATA_INT2, 1.0, 300, FrameRead2Bytes },
    { "SBW03", "SBW03", DATA_UINT1, 1.0, 302, FrameRead1Byte },
    { "SBW04", "SBW04", DATA_UINT1, 1.0, 303, FrameRead1Byte },
    { "star_1_ccd_temp", "ADST1CCDT", DATA_UINT2, 1.0, 304, FrameRead2Bytes },
    { "star_1_base_temp", "ADST1BPT", DATA_UINT2, 1.0, 306, FrameRead2Bytes },
    { "star_1_lens_temp", "ADST1LT", DATA_UINT2, 1.0, 308, FrameRead2Bytes },
    { "star_1_p2_volt", "ADST1P2V", DATA_UINT1, 1.0, 310, FrameRead1Byte },
    { "star_1_m8_volt", "ADST1M8V", DATA_UINT1, 1.0, 311, FrameRead1Byte },
    { "star_1_p5_volt", "ADST1P5V", DATA_UINT1, 1.0, 312, FrameRead1Byte },
    { "star_1_m5_volt", "ADST1M5V", DATA_UINT1, 1.0, 313, FrameRead1Byte },
    { "star_1_bg_read", "ADST1BG", DATA_UINT2, 1.0, 314, FrameRead2Bytes },
    { "star_1_ff_cnt", "ADST1FFCNT", DATA_UINT1, 1.0, 316, FrameRead1Byte },
    { "star_1_falarm_cnt", "ADST1FACNT", DATA_UINT1, 1.0,317,FrameRead1Byte },
    { "star_1_tlm_offset", "ADST1TOFF", DATA_UINT2, 1.0, 318, FrameRead2Bytes },
    { "star_2_ccd_temp", "ADST2CCDT", DATA_UINT2, 1.0, 320, FrameRead2Bytes },
    { "star_2_base_temp", "ADST2BPT", DATA_UINT2, 1.0, 322, FrameRead2Bytes },
    { "star_2_lens_temp", "ADST2LT", DATA_UINT2, 1.0, 324, FrameRead2Bytes },
    { "star_2_p2_volt", "ADST2P2V", DATA_UINT1, 1.0, 326, FrameRead1Byte },
    { "star_2_m8_volt", "cwADST2M8V", DATA_UINT1, 1.0, 327, FrameRead1Byte },
    { "star_2_p5_volt", "ADST2P5V", DATA_UINT1, 1.0, 328, FrameRead1Byte },
    { "star_2_m5_volt", "ADST2M5V", DATA_UINT1, 1.0, 329, FrameRead1Byte },
    { "star_2_bg_read", "ADST2BG", DATA_UINT2, 1.0, 330, FrameRead2Bytes },
    { "star_2_ff_cnt", "ADST2FFCNT", DATA_UINT1, 1.0, 332, FrameRead1Byte },
    { "star_2_falarm_cnt", "ADST2FACNT", DATA_UINT1, 1.0,333,FrameRead1Byte },
    { "star_2_tlm_offset", "ADST2TOFF", DATA_UINT2, 1.0, 334, FrameRead2Bytes },
    { "rate_sen_att_q1", "ADRSAQ1", DATA_INT2, 1.0, 336, FrameRead2Bytes },
    { "rate_sen_att_q2", "ADRSAQ2", DATA_INT2, 1.0, 338, FrameRead2Bytes },
    { "rate_sen_att_q3", "ADRSAQ3", DATA_INT2, 1.0, 340, FrameRead2Bytes },
    { "rate_sen_att_q4", "ADRSAQ4", DATA_INT2, 1.0, 342, FrameRead2Bytes },

    { "axis3_avg_prd3", "ADGYA3D3", DATA_UINT2, 1.0, 378, FrameRead2Bytes },
    { "axis1_avg_prd4", "ADGYA1D4", DATA_UINT2, 1.0, 380, FrameRead2Bytes },
    { "axis2_avg_prd4", "ADGYA2D4", DATA_UINT2, 1.0, 382, FrameRead2Bytes },
    { "axis3_avg_prd4", "ADGYA3D4", DATA_UINT2, 1.0, 384, FrameRead2Bytes },
    { "axis1_avg_prd5", "ADGYA1D5", DATA_UINT2, 1.0, 386, FrameRead2Bytes },
    { "axis2_avg_prd5", "ADGYA2D5", DATA_UINT2, 1.0, 388, FrameRead2Bytes },
    { "axis3_avg_prd5", "ADGYA3D5", DATA_UINT2, 1.0, 390, FrameRead2Bytes },

    { "gyro_temp_1", "ADGYT1", DATA_UINT2, 1.0, 392, FrameRead2Bytes },
    { "gyro_temp_2", "ADGYT2", DATA_UINT2, 1.0, 394, FrameRead2Bytes },
    { "gyro_temp_3", "ADGYT3", DATA_UINT2, 1.0, 396, FrameRead2Bytes }
};

const int Hk2HdfTabSize = ElementNumber(Hk2HdfMapTable);
