//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef L1AGSFRAME_H
#define L1AGSFRAME_H

static const char rcs_id_l1a_gs_frame_h[] =
    "@(#) $Id$";

#include <stdio.h>

//======================================================================
// CLASSES
//    L1AGSFrame
//======================================================================

//======================================================================
// CLASS
//    L1AGSFrame
//
// DESCRIPTION
//    The L1AGSFrame object contains the contents of a Ground System
//    Level 1A frame as a structure.
//======================================================================

// frame 5972 + 4 + 4 (fortran records header and trailer paddings)
#define GS_L1A_FRAME_SIZE    5980

#define GS_CAL_PULSE_FRAME_SIZE  150

struct GSL1APcd
{
    // methods
    int Read(char*   frameStartByte);

    // member variables
    char         frame_time[24];
    double       time;
    double       instrument_time;
    int          orbit_time;
    float        x_pos, y_pos, z_pos;
    float        x_vel, y_vel, z_vel;
    float        roll, pitch, yaw;
};

struct GSL1AStatus
{
    // methods
    int Read(char*   frameStartByte);

    // member variables
    unsigned char   telemetry_table_id[2];
    unsigned char   status_error_flags;
    unsigned char   table_readout_type;
    unsigned char   table_readout_offset[2];
    unsigned char   table_readout_data[4];
    unsigned char   operational_mode;
    unsigned char   prf_count;
    unsigned char   status_change_flags[2];
    unsigned char   error_message[2];
    unsigned char   error_message_history[10];
    unsigned char   valid_command_count;
    unsigned char   invalid_command_count;
    char            specified_cal_pulse_pos;
    unsigned char   prf_cycle_time;
    unsigned char   range_gate_a_delay;
    unsigned char   range_gate_a_width;
    unsigned char   range_gate_b_delay;
    unsigned char   range_gate_b_width;
    char            doppler_shift_command_1[3];
    char            doppler_shift_command_2[3];
    unsigned char   pulse_width;
    unsigned char   receiver_gain;
    unsigned char   ses_configuration_flags;
    unsigned char   ses_data_overrun_count;
    unsigned char   ses_data_underrun_count;
    unsigned char   pred_antenna_pos_count;
    unsigned char   running_error_count[2];
    char            ses_reset_position;
    unsigned char   doppler_orbit_step;
    unsigned char   prf_orbit_step_change;
    unsigned char   cmd_history_queue[8];
    unsigned char   calc_ant_max_grp_count;
    char            vtcw[6];
    char            corres_instr_time[5];
    unsigned char   fsw_mission_version_num;
    unsigned char   fsw_build_number;
    unsigned char   pbi_flag;
    char            pad1[6];
};

struct GSL1AEngData
{
    // methods
    int Read(char*   frameStartByte);

    // member variables
    char            pad1[16];
    unsigned char   relay_status[2];
    char            pad2[3];
    unsigned char   ea_a_spin_rate;
    char            pad3[4];
    unsigned char   ea_b_spin_rate;
    char            pad4[7];
    unsigned char   a2d_p12v_xcpl;
    char            pad5[8];
    unsigned char   transmit_power_a;
    unsigned char   transmit_power_b;
    char            pad6;
    unsigned char   precision_coupler_temp;
    char            pad7[4];
    unsigned char   rcv_protect_sw_temp;
    char            pad8[2];
    unsigned char   beam_select_sw_temp;
    char            pad9;
    unsigned char   receiver_temp;
    char            pad10[2];
    unsigned char   eng_status_c1;
    unsigned char   eng_status_c2;
    unsigned char   eng_status_c3;
};

struct GSL1AEu
{
    float           prf_cycle_time_eu;
    float           range_gate_delay_inner;
    float           range_gate_delay_outer;
    float           range_gate_width_inner;
    float           range_gate_width_outer;
    float           transmit_pulse_width;
    int             true_cal_pulse_pos;
    float           transmit_power_inner;
    float           transmit_power_outer;
    float           precision_coupler_temp_eu;
    float           rcv_protect_sw_temp_eu;
    float           beam_select_sw_temp_eu;
    float           receiver_temp_eu;

};

struct GSL1ASci
{
    // methods
    int Read(char*   frameStartByte);

    // member variables
    short      antenna_position[100];
    float      loop_back_cal_A_power[12];
    float      loop_back_cal_B_power[12];
    float      loop_back_cal_noise;
    float      load_cal_A_power[12];
    float      load_cal_B_power[12];
    float      load_cal_noise;
    int        power_dn[12][100];
    int        noise_dn[100];
};

//---------------------------------
// struct for Cal Pulse
// this structure is for reference only,
// scott writes the frame member by member
//---------------------------------
struct GSCalPulse
{
    double     frame_time;
    int        loop_back_cal_power[12];
    int        loop_back_cal_noise;
    int        load_cal_power[12];
    int        load_cal_noise;
    float      precision_coupler_temp_eu;
    float      rcv_protect_sw_temp_eu;
    float      beam_select_sw_temp_eu;
    float      receiver_temp_eu;
    float      transmit_power_inner;
    float      transmit_power_outer;
    int        frame_inst_status;
    int        frame_err_status;
    char       true_cal_pulse_position;
    char       beam_identifier;
    int        spare;
};

class L1AGSFrame
{
public:
    //--------------//
    // construction //
    //--------------//

    L1AGSFrame() ;
    virtual ~L1AGSFrame() { };

    int  GSFrameSize() { return GS_L1A_FRAME_SIZE; }

    //-------------------//
    // data manipulation //
    //-------------------//

    int  Pack(char* buffer);
    int  Unpack(char* buffer);
    int  WriteAscii(FILE* ofp);

    //-------------------------------------------------------------
    // product variables
    // The following is taken from GS (readL1A).
    // The variable names are preserved for ease of maintenance.
    //-------------------------------------------------------------
    GSL1APcd      in_pcd;
    GSL1AStatus   status;
    GSL1AEngData  engdata;
    char          pad1[2];
    char          SESerrflags[13];
    char          pad2;
    char          memro[26];
    char          pcd[32];
    GSL1AEu       in_eu;
    GSL1ASci      in_science;
    int           l1a_frame_inst_status;
    int           l1a_frame_err_status;
    short         l1a_frame_qual_flag;
    char          l1a_pulse_qual_flag[13];
    char          pad3;

};

#endif
