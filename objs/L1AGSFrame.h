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

struct GSL1APcd
{
    // methods
    Read(char*   frameStartByte);

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
    Read(char*   frameStartByte);

    // member variables
    char            pad1[11];
    unsigned char   prf_count;
    char            pad2[17];
    unsigned char   prf_cycle_time;
    char            pad3;
    unsigned char   range_gate_a_width;
    char            pad4;
    unsigned char   range_gate_b_width;
    char            pad5[8];
    unsigned char   pulse_width;
    char            pad6[4];
    unsigned char   pred_antenna_pos_count;
    char            pad7[3];
    unsigned char   doppler_orbit_step;
    unsigned char   prf_orbit_step_change;
    char            pad8[9];
    char            vtcw[8];
    char            pad9[10];
};

struct GSL1AEngData
{
    // methods
    Read(char*   frameStartByte);

    // member variables
    char            pad1[46];
    unsigned char   precision_coupler_temp;
    char            pad2[4];
    unsigned char   rcv_protect_sw_temp;
    char            pad3[2];
    unsigned char   beam_select_sw_temp;
    char            pad4;
    unsigned char   receiver_temp;
    char            pad5[5];
};

struct GSL1AEu
{
    float           prf_cycle_time_eu;
    char            pad1[8];
    float           range_gate_width_inner;
    float           range_gate_width_outer;
    float           transmit_pulse_width;
    int             true_cal_pulse_pos;
    char            pad2[8];
    float           precision_coupler_temp_eu;
    float           rcv_protect_sw_temp_eu;
    float           beam_select_sw_temp_eu;
    float           receiver_temp_eu;

};

struct GSL1ASci
{
    // methods
    Read(char*   frameStartByte);

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

    int  ReadL1AFile(FILE* ifp);
    int  WriteGSL1AFile(FILE* ofp);

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
    int           la1_frame_inst_status;
    int           l1a_frame_err_status;
    short         l1a_frame_qual_flag;
    char          l1a_pulse_qual_flag[13];
    char          pad3;
};

#endif