//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   17 Apr 1998 16:48:38   sally
// add L2A and L2B parameter tables
// 
//    Rev 1.0   14 Apr 1998 15:09:14   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

static const char Hk2HdfMap_C_id[] =
    "@(#) $Header$";

#include "Parameter.h"
#include "Hk2HdfMap.h"
#include "FrameRead.h"

//---------------------------------------------------------
// bit order: from left to right, i.e. 0 1 2 3 4 5 6 7
//---------------------------------------------------------

const Hk2HdfMapEntry Hk2HdfMapTable[] =
{
    { "torque_rod_1_status", "ADTR1ST", DATA_UINT1, 1.0, 6, FrameReadBits6_7 },
    { "torque_rod_2_status", "ADTR2ST", DATA_UINT1, 1.0, 6, FrameReadBits4_5 },
    { "torque_rod_3_status", "ADTR3ST", DATA_UINT1, 1.0, 6, FrameReadBits2_3 },
    { "csm_status", "SWCSMST", DATA_UINT1, 1.0, 7, FrameReadBit6 },
    { "cbm_status", "SWCBMST", DATA_UINT1, 1.0, 7, FrameReadBit5 },
    { "table_upload_in_prog", "SWTLOAD", DATA_UINT1, 1.0, 7, FrameReadBit4 },
    { "table_download_in_prog", "SWTDUMP", DATA_UINT1, 1.0, 7, FrameReadBit3 },
    { "eeprom_prog_status", "SWEEPST", DATA_UINT1, 1.0, 7, FrameReadBits1_2 },
    { "eeprom_prog_toggle", "SWEEPTG", DATA_UINT1, 1.0, 7, FrameReadBit0 },
    { "sas_ea_a_pwr_temp", "scat0001", DATA_UINT1, 1.0, 24, FrameRead1Byte },
    { "sas_duplex_bearing_temp", "scat0002", DATA_UINT1, 1.0,25,FrameRead1Byte},
    { "ses_temp_1", "scat0003", DATA_UINT1, 1.0, 26, FrameRead1Byte },
    { "twt1_base_temp", "scat0004", DATA_UINT1, 1.0, 27, FrameRead1Byte },
    { "cds_a_temp", "scat0005", DATA_UINT1, 1.0, 28, FrameRead1Byte },
    { "psu_temp", "scat0006", DATA_UINT1, 1.0, 29, FrameRead1Byte },
    { "gps_time_tag_month", "ADGPMONTH", DATA_UINT1, 1.0, 72, FrameRead1Byte },
    { "gps_time_tag_day", "ADGPDAY", DATA_UINT1, 1.0, 73, FrameRead1Byte },
    { "gps_time_tag_year", "ADGPYEAR", DATA_UINT2, 1.0, 74, FrameRead2Bytes },
    { "gps_time_tag_hour", "ADGPHOUR", DATA_UINT1, 1.0, 76, FrameRead1Byte },
    { "gps_time_tag_min", "ADGPMIN", DATA_UINT1, 1.0, 77, FrameRead1Byte },
    { "gps_time_tag_sec", "ADGPSEC", DATA_UINT1, 1.0, 78, FrameRead1Byte },
    { "gps_time_tag_nsec", "ADGPNANO", DATA_UINT4, 1.0, 79, FrameRead4Bytes },
    { "current_dop", "ADGPDOP", DATA_UINT2, 1.0, 83, FrameRead2Bytes },
    { "num_visible_sat", "ADGPVSAT", DATA_UINT1, 1.0, 85, FrameRead1Byte },
    { "num_sat_tracked", "ADGPTSAT", DATA_UINT1, 1.0, 86, FrameRead1Byte },
    { "sat_1_id", "ADGP1ID", DATA_UINT1, 1.0, 87, FrameRead1Byte },
    { "sat_1_track_mode", "ADGP1MODE", DATA_UINT1, 1.0, 88, FrameRead1Byte},
    { "sat_2_id", "ADGP2ID", DATA_UINT1, 1.0, 89, FrameRead1Byte },
    { "sat_2_track_mode", "ADGP2MODE", DATA_UINT1, 1.0, 90, FrameRead1Byte},
    { "sat_3_id", "ADGP3ID", DATA_UINT1, 1.0, 91, FrameRead1Byte },
    { "sat_3_track_mode", "ADGP3MODE", DATA_UINT1, 1.0, 92, FrameRead1Byte},
    { "sat_4_id", "ADGP4ID", DATA_UINT1, 1.0, 93, FrameRead1Byte },
    { "sat_4_track_mode", "ADGP4MODE", DATA_UINT1, 1.0, 94, FrameRead1Byte},
    { "sat_5_id", "ADGP5ID", DATA_UINT1, 1.0, 95, FrameRead1Byte },
    { "sat_5_track_mode", "ADGP5MODE", DATA_UINT1, 1.0, 96, FrameRead1Byte},
    { "sat_6_id", "ADGP6ID", DATA_UINT1, 1.0, 97, FrameRead1Byte },
    { "sat_6_track_mode", "ADGP6MODE", DATA_UINT1, 1.0, 98, FrameRead1Byte},
    { "sat_7_id", "ADGP7ID", DATA_UINT1, 1.0, 99, FrameRead1Byte },
    { "sat_7_track_mode", "ADGP7MODE", DATA_UINT1, 1.0, 100, FrameRead1Byte},
    { "sat_8_id", "ADGP8ID", DATA_UINT1, 1.0, 101, FrameRead1Byte },
    { "sat_8_track_mode", "ADGP8MODE", DATA_UINT1, 1.0, 102, FrameRead1Byte},
    { "sat_9_id", "ADGP9ID", DATA_UINT1, 1.0, 103, FrameRead1Byte },
    { "sat_9_track_mode", "ADGP9MODE", DATA_UINT1, 1.0, 104, FrameRead1Byte},
    { "sat_10_id", "ADGP10ID", DATA_UINT1, 1.0, 105, FrameRead1Byte },
    { "sat_10_track_mode", "ADGP10MODE", DATA_UINT1, 1.0, 106, FrameRead1Byte},
    { "sat_11_id", "ADGP11ID", DATA_UINT1, 1.0, 107, FrameRead1Byte },
    { "sat_11_track_mode", "ADGP11MODE", DATA_UINT1, 1.0, 108, FrameRead1Byte},
    { "sat_12_id", "ADGP12ID", DATA_UINT1, 1.0, 109, FrameRead1Byte },
    { "sat_12_track_mode", "ADGP12MODE", DATA_UINT1, 1.0, 110, FrameRead1Byte},
    { "rx_stat_pos_prop_mode","ADGPRXPROP",DATA_UINT1, 1.0,111,FrameReadBit7},
    { "rx_stat_poor_geom","ADGPRXGEO",DATA_UINT1, 1.0,111,FrameReadBit6 },
    { "rx_stat_3d_fix","ADGPRX3D",DATA_UINT1, 1.0,111,FrameReadBit5 },
    { "rx_stat_alti_hold","ADGPRXALT",DATA_UINT1, 1.0,111,FrameReadBit4 },
    { "rx_stat_acq_sat","ADGPRXACQ",DATA_UINT1, 1.0,111,FrameReadBit3 },
    { "rx_stat_store_new_alm","ADGPRXSALM",DATA_UINT1, 1.0,111,FrameReadBit2 },
    { "rx_stat_insuff_sat","ADGPRXVSAT",DATA_UINT1, 1.0,111,FrameReadBit1 },
    { "rx_stat_bad_alm","ADGPRXALM",DATA_UINT1, 1.0,111,FrameReadBit0 },
    { "gps_pos_x_raw_1", "ADGPPOSX1", DATA_UINT4, 1.0, 112, FrameRead4Bytes },
    { "gps_pos_x_raw_2", "ADGPPOSX2", DATA_UINT4, 1.0, 116, FrameRead4Bytes },
    { "gps_pos_y_raw_1", "ADGPPOSY1", DATA_UINT4, 1.0, 120, FrameRead4Bytes },
    { "gps_pos_y_raw_2", "ADGPPOSY2", DATA_UINT4, 1.0, 124, FrameRead4Bytes },
    { "gps_pos_z_raw_1", "ADGPPOSZ1", DATA_UINT4, 1.0, 128, FrameRead4Bytes },
    { "gps_pos_z_raw_2", "ADGPPOSZ2", DATA_UINT4, 1.0, 132, FrameRead4Bytes },
    { "gps_vel_x_raw_1", "ADGPVELX1", DATA_UINT4, 1.0, 136, FrameRead4Bytes },
    { "gps_vel_x_raw_2", "ADGPVELX2", DATA_UINT4, 1.0, 140, FrameRead4Bytes },
    { "gps_vel_y_raw_1", "ADGPVELY1", DATA_UINT4, 1.0, 144, FrameRead4Bytes },
    { "gps_vel_y_raw_2", "ADGPVELY2", DATA_UINT4, 1.0, 148, FrameRead4Bytes },
    { "gps_vel_z_raw_1", "ADGPVELZ1", DATA_UINT4, 1.0, 152, FrameRead4Bytes },
    { "gps_vel_z_raw_2", "ADGPVELZ2", DATA_UINT4, 1.0, 156, FrameRead4Bytes },
    { "gps_vtcw_1", "ADGPVTCW1", DATA_UINT2, 1.0, 160, FrameRead2Bytes },
    { "gps_vtcw_2", "ADGPVTCW2", DATA_UINT2, 1.0, 162, FrameRead2Bytes },
    { "gps_vtcw_3", "ADGPVTCW3", DATA_UINT2, 1.0, 164, FrameRead2Bytes },
    { "gyro_frame_counter", "ADGYFCNT", DATA_UINT1, 1.0, 166, FrameReadBits1_7},
    { "gyro_high_volt_stat", "ADGYHVST", DATA_UINT1, 1.0, 166, FrameReadBit0},
    { "gyro_x_dither_stat", "ADGYDSX", DATA_UINT1, 1.0, 167, FrameReadBit7},
    { "gyro_y_dither_stat", "ADGYDSY", DATA_UINT1, 1.0, 167, FrameReadBit6},
    { "gyro_z_dither_stat", "ADGYDSZ", DATA_UINT1, 1.0, 167, FrameReadBit5},
    { "gyro_x_intensity", "ADGYINTX", DATA_UINT1, 1.0, 167, FrameReadBit4},
    { "gyro_y_intensity", "ADGYINTY", DATA_UINT1, 1.0, 167, FrameReadBit3},
    { "gyro_z_intensity", "ADGYINTZ", DATA_UINT1, 1.0, 167, FrameReadBit2},
    { "gyro_pwr_p15v_stat", "ADGYP15V", DATA_UINT1, 1.0, 167, FrameReadBit1},
    { "gyro_pwr_p120v_stat", "ADGYP120V", DATA_UINT1, 1.0, 167,FrameReadBit0},
    { "wheel_1_speed_dir", "ADW1SDIR", DATA_INT2, 1.0, 168, FrameRead2Bytes},
    { "wheel_2_speed_dir", "ADW2SDIR", DATA_INT2, 1.0, 170, FrameRead2Bytes},
    { "wheel_3_speed_dir", "ADW3SDIR", DATA_INT2, 1.0, 172, FrameRead2Bytes},
    { "wheel_4_speed_dir", "ADW4SDIR", DATA_INT2, 1.0, 174, FrameRead2Bytes},
    { "status_code", "SSSTATUS", DATA_UINT1, 1.0, 178, FrameRead1Byte},
    { "command_id", "SSCMDID", DATA_UINT1, 1.0, 179, FrameRead1Byte},
    { "tlm_format", "SSTLMFMT", DATA_UINT1, 1.0, 180, FrameRead1Byte},
    { "ssr_resv_1", "SSRSVD1", DATA_UINT1, 1.0, 181, FrameRead1Byte},
    { "init_complete", "SSINIT", DATA_UINT1, 1.0, 182, FrameReadBit7 },
    { "current_state", "SSSTATE", DATA_UINT1, 1.0, 182, FrameReadBits4_6 },
    { "current_mode", "SSMODE", DATA_UINT1, 1.0, 182, FrameReadBits0_3 },
    { "ssr1_selected_pwr", "SS1PW", DATA_UINT1, 1.0, 183, FrameReadBit7 },
    { "ssr2_selected_pwr", "SS2PW", DATA_UINT1, 1.0, 183, FrameReadBit6 },
    { "ssr1_edac_enabled", "SS1EDAC", DATA_UINT1, 1.0, 183, FrameReadBit5 },
    { "ssr2_edac_enabled", "SS2EDAC", DATA_UINT1, 1.0, 183, FrameReadBit4 },
    { "ssr1_curr_pwr_state", "SS1PWST", DATA_UINT1, 1.0, 183, FrameReadBits2_3},
    { "ssr2_curr_pwr_state", "SS2PWST", DATA_UINT1, 1.0, 183, FrameReadBits0_1},
    { "addr_sw_exception", "SSSEADDR", DATA_UINT2, 1.0, 184, FrameRead2Bytes },
    { "corr_mem_err_cnt", "SSCPMECNT", DATA_UINT1, 1.0, 186, FrameRead1Byte },
    { "uncorr_mem_err_cnt", "SSUPMECNT", DATA_UINT1, 1.0, 187, FrameRead1Byte },
    { "ssr1_rx_status", "SS1RXST", DATA_UINT1, 1.0, 188, FrameRead1Byte },
    { "ssr1_xmit_status", "SS1XMST", DATA_UINT1, 1.0, 189, FrameRead1Byte },
    { "ssr2_rx_status", "SS2RXST", DATA_UINT1, 1.0, 190, FrameRead1Byte },
    { "ssr2_xmit_status", "SS2XMST", DATA_UINT1, 1.0, 191, FrameRead1Byte },
    { "part_0_ssr1_rec_addr", "SS1P0REC", DATA_UINT4, 1.0, 192,FrameRead4Bytes},
    { "part_0_ssr2_rec_addr", "SS2P0REC", DATA_UINT4, 1.0, 196,FrameRead4Bytes},
    { "part_1_ssr1_rec_addr", "SS1P1REC", DATA_UINT4, 1.0, 200,FrameRead4Bytes},
    { "part_1_ssr2_rec_addr", "SS2P1REC", DATA_UINT4, 1.0, 204,FrameRead4Bytes},
    { "part_0_ssr1_pbk_addr", "SS1P0PBK", DATA_UINT4, 1.0, 208,FrameRead4Bytes},
    { "part_0_ssr2_pbk_addr", "SS2P0PBK", DATA_UINT4, 1.0, 212,FrameRead4Bytes},
    { "part_1_ssr1_pbk_addr", "SS1P1PBK", DATA_UINT4, 1.0, 216,FrameRead4Bytes},
    { "part_1_ssr2_pbk_addr", "SS2P1PBK", DATA_UINT4, 1.0, 220,FrameRead4Bytes},
    { "ssr_resv_2", "SSRSVD2", DATA_UINT1, 1.0, 224, FrameReadBits1_7},
    { "ssr1_corr_mem_flag", "SS1CMFLG", DATA_UINT1, 1.0, 224, FrameReadBit0},
    { "ssr1_corr_mem_err_cnt", "SS1CMECNT",DATA_UINT4, 1.0,225,FrameRead3Bytes},
    { "ssr_resv_3", "SSRSVD3", DATA_UINT1, 1.0, 228, FrameReadBits1_7},
    { "ssr2_corr_mem_flag", "SS2CMFLG", DATA_UINT1, 1.0, 228, FrameReadBit0},
    { "ssr2_corr_mem_err_cnt", "SS2CMECNT",DATA_UINT4, 1.0,229,FrameRead3Bytes},
    { "ssr_resv_4", "SSRSVD4", DATA_UINT1, 1.0, 232, FrameReadBits1_7},
    { "ssr1_uncorr_mem_flag", "SS1UMFLG", DATA_UINT1, 1.0, 232, FrameReadBit0},
    { "ssr1_uncorr_mem_err_cnt","SS1UMECNT",DATA_UINT4,1.0,233,FrameRead3Bytes},
    { "ssr_resv_5", "SSRSVD5", DATA_UINT1, 1.0, 236, FrameReadBits1_7},
    { "ssr2_uncorr_mem_flag", "SS2UMFLG", DATA_UINT1, 1.0, 236, FrameReadBit0},
    { "ssr2_uncorr_mem_err_cnt","SS2UMECNT",DATA_UINT4,1.0,237,FrameRead3Bytes},
    { "msg_checksum", "SSMCSUM", DATA_UINT2, 1.0, 240, FrameRead2Bytes },
    { "adcs_cycle_start_1", "ADCSVTCW1", DATA_UINT2, 1.0, 242,FrameRead2Bytes },
    { "adcs_cycle_start_2", "ADCSVTCW2", DATA_UINT2, 1.0, 244,FrameRead2Bytes },
    { "adcs_cycle_start_3", "ADCSVTCW3", DATA_UINT2, 1.0, 246,FrameRead2Bytes },
    { "star_trkr_att_q1", "ADSTAQ1", DATA_INT2, 1.0, 254, FrameRead2Bytes },
    { "star_trkr_att_q2", "ADSTAQ2", DATA_INT2, 1.0, 256, FrameRead2Bytes },
    { "star_trkr_att_q3", "ADSTAQ3", DATA_INT2, 1.0, 258, FrameRead2Bytes },
    { "star_trkr_att_q4", "ADSTAQ4", DATA_INT2, 1.0, 260, FrameRead2Bytes },
    { "axis1_avg_prd1", "ADGYA1D1", DATA_UINT2, 1.0, 262, FrameRead2Bytes },
    { "axis2_avg_prd1", "ADGYA2D1", DATA_UINT2, 1.0, 264, FrameRead2Bytes },
    { "axis3_avg_prd1", "ADGYA3D1", DATA_UINT2, 1.0, 266, FrameRead2Bytes },
    { "axis1_avg_prd2", "ADGYA1D2", DATA_UINT2, 1.0, 268, FrameRead2Bytes },
    { "axis2_avg_prd2", "ADGYA2D2", DATA_UINT2, 1.0, 270, FrameRead2Bytes },
    { "axis3_avg_prd2", "ADGYA3D2", DATA_UINT2, 1.0, 272, FrameRead2Bytes },
    { "axis1_avg_prd3", "ADGYA1D3", DATA_UINT2, 1.0, 274, FrameRead2Bytes },
    { "axis2_avg_prd3", "ADGYA2D3", DATA_UINT2, 1.0, 276, FrameRead2Bytes },
    { "axis3_avg_prd3", "ADGYA3D3", DATA_UINT2, 1.0, 370, FrameRead2Bytes },
    { "axis1_avg_prd4", "ADGYA1D4", DATA_UINT2, 1.0, 372, FrameRead2Bytes },
    { "axis2_avg_prd4", "ADGYA2D4", DATA_UINT2, 1.0, 374, FrameRead2Bytes },
    { "axis3_avg_prd4", "ADGYA3D4", DATA_UINT2, 1.0, 376, FrameRead2Bytes },
    { "axis1_avg_prd5", "ADGYA1D5", DATA_UINT2, 1.0, 378, FrameRead2Bytes },
    { "axis2_avg_prd5", "ADGYA2D5", DATA_UINT2, 1.0, 380, FrameRead2Bytes },
    { "axis3_avg_prd5", "ADGYA3D5", DATA_UINT2, 1.0, 382, FrameRead2Bytes },
    { "gyro_temp_1", "ADGYT1", DATA_UINT2, 1.0, 384, FrameRead2Bytes },
    { "gyro_temp_2", "ADGYT2", DATA_UINT2, 1.0, 386, FrameRead2Bytes },
    { "gyro_temp_3", "ADGYT3", DATA_UINT2, 1.0, 388, FrameRead2Bytes }
};

const int Hk2HdfTabSize = ElementNumber(Hk2HdfMapTable);
