//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_l1a_gs_frame_c[] =
    "@(#) $Id$";

#include <string.h>

#include "L1AGSFrame.h"

L1AGSFrame::L1AGSFrame()
{
    (void)memset(&in_pcd, 0, sizeof(in_pcd));
    (void)memset(&status, 0, sizeof(status));
    (void)memset(&engdata, 0, sizeof(engdata));
    (void)memset(pad1, 0, 2);
    (void)memset(SESerrflags, 0, 13);
    (void)memset(&pad2, 0, 1);
    (void)memset(memro, 0, 26);
    (void)memset(pcd, 0, 32);
    (void)memset(&in_eu, 0, sizeof(in_eu));
    (void)memset(&in_science, 0, sizeof(in_science));
    (void)memset(&l1a_frame_inst_status, 0, sizeof(l1a_frame_inst_status));
    (void)memset(&l1a_frame_err_status, 0, sizeof(l1a_frame_err_status));
    (void)memset(&l1a_frame_qual_flag, 0, sizeof(l1a_frame_qual_flag));
    (void)memset(&l1a_pulse_qual_flag, 0, 13);
    (void)memset(&pad3, 0, 1);
}

int
L1AGSFrame::Pack(
char*      buffer)
{
    char* ptr = buffer;
    (void)memset(ptr, 0, GS_L1A_FRAME_SIZE);
    // write byte size header for fortran header
    int gsL1AFrameSize = GS_L1A_FRAME_SIZE - 8;
    (void)memcpy(ptr, &gsL1AFrameSize, sizeof(int)); ptr += sizeof(int);

    (void)memcpy(ptr, &in_pcd, sizeof(GSL1APcd)); ptr += sizeof(GSL1APcd);

    (void)memcpy(ptr, &status, sizeof(GSL1AStatus));
    ptr += sizeof(GSL1AStatus);
    (void)memcpy(ptr, &engdata, sizeof(GSL1AEngData));
    ptr += sizeof(GSL1AEngData);
    ptr += 2;
    (void)memcpy(ptr, SESerrflags, 13); ptr += 13;
    ptr++;
    (void)memcpy(ptr, memro, 26); ptr += 26;
    (void)memcpy(ptr, pcd, 32); ptr += 32;
    (void)memcpy(ptr, &in_eu, sizeof(GSL1AEu));
    ptr += sizeof(GSL1AEu);
    (void)memcpy(ptr, &in_science, sizeof(GSL1ASci));
    ptr += sizeof(GSL1ASci);
    (void)memcpy(ptr, &l1a_frame_inst_status, sizeof(int));
    ptr += sizeof(int);
    (void)memcpy(ptr, &l1a_frame_err_status, sizeof(int));
    ptr += sizeof(int);
    (void)memcpy(ptr, &l1a_frame_qual_flag, sizeof(short));
    ptr += sizeof(short);
    (void)memcpy(ptr, l1a_pulse_qual_flag, 13); ptr += 13;
    ptr++;
    (void)memcpy(ptr, &gsL1AFrameSize, sizeof(int)); ptr += sizeof(int);
    return 1;

} // L1AGSFrame::Pack

int
L1AGSFrame::Unpack(
char*      buffer)
{
    int gsL1AFrameSize;
    char* ptr = buffer;
    (void)memcpy(&gsL1AFrameSize, ptr, sizeof(int)); ptr += sizeof(int);
    if (gsL1AFrameSize != GS_L1A_FRAME_SIZE - 8) return 0;
    (void)memcpy(&in_pcd, ptr, sizeof(GSL1APcd));
    ptr += sizeof(GSL1APcd);
    (void)memcpy(&status, ptr, sizeof(GSL1AStatus));
    ptr += sizeof(GSL1AStatus);
    (void)memcpy(&engdata, ptr, sizeof(GSL1AEngData));
    ptr += sizeof(GSL1AEngData);
    ptr += 2;
    (void)memcpy(SESerrflags, ptr, 13); ptr += 13;
    ptr++;
    (void)memcpy(memro, ptr, 26); ptr += 26;
    (void)memcpy(pcd, ptr, 32); ptr += 32;
    (void)memcpy(&in_eu, ptr, sizeof(GSL1AEu));
    ptr += sizeof(GSL1AEu);
    (void)memcpy(&in_science, ptr, sizeof(GSL1ASci));
    ptr += sizeof(GSL1ASci);
    (void)memcpy(&l1a_frame_inst_status, ptr, sizeof(int));
    ptr += sizeof(int);
    (void)memcpy(&l1a_frame_err_status, ptr, sizeof(int));
    ptr += sizeof(int);
    (void)memcpy(&l1a_frame_qual_flag, ptr, sizeof(short));
    ptr += sizeof(short);
    (void)memcpy(l1a_pulse_qual_flag, ptr, 13); ptr += 13;
    return 1;

} // L1AGSFrame::Unpack

int
L1AGSFrame::WriteAscii(
FILE*   ofp)
{
    if (ofp == NULL) return(0);

    //-----------------------------
    // write PCD
    //-----------------------------
    fprintf(ofp, "\n####################  PCD ####################\n\n");
    char ftime[25];
    (void)memcpy(ftime, in_pcd.frame_time, 24);
    ftime[24] = '\0';
    fprintf(ofp, "frame_time: %s\n", ftime);
    fprintf(ofp, "time: %g, instrument_time: %g, orbit_time: %d\n",
                     in_pcd.time, in_pcd.instrument_time, in_pcd.orbit_time);
    fprintf(ofp, "x_pos: %g, y_pos: %g, z_pos: %g\n",
                     in_pcd.x_pos, in_pcd.y_pos, in_pcd.z_pos);
    fprintf(ofp, "x_vel: %g, y_vel: %g, z_vel: %g\n",
                     in_pcd.x_vel, in_pcd.y_vel, in_pcd.z_vel);
    fprintf(ofp, "roll: %g, pitch: %g, yaw: %g\n",
                     in_pcd.roll, in_pcd.pitch, in_pcd.yaw);
  
    fprintf(ofp,"\n ============== GS Status Block ==============\n\n");
    fprintf(ofp,"telemetry_table_id = %d\n", *((short*)status.telemetry_table_id));
    fprintf(ofp,"status_error_flags = %d\n", status.status_error_flags);
    fprintf(ofp,"table_readout_type = %d\n", status.table_readout_type);
    fprintf(ofp,"table_readout_offset = %d\n", *((short*)status.table_readout_offset));
    fprintf(ofp,"table_readout_data = %d %d %d %d\n",
      status.table_readout_data[0],
      status.table_readout_data[1],
      status.table_readout_data[2],
      status.table_readout_data[3]);
    fprintf(ofp,"operational_mode = %d\n", status.operational_mode);
    fprintf(ofp,"prf_count = %d\n", status.prf_count);
    fprintf(ofp,"status_change_flags = %d\n", *((short*)status.status_change_flags));
    fprintf(ofp,"error_message = %d\n", *((unsigned short*)status.error_message));
    fprintf(ofp,"error_message_history = ");
    for (int i=0; i < 5; i++)
      fprintf(ofp," %d", *((unsigned short*)(status.error_message_history + i*2)));
    fprintf(ofp,"\n");
    fprintf(ofp,"valid_command_count %d\n", status.valid_command_count);
    fprintf(ofp,"invalid_command_count %d\n", status.invalid_command_count);
    fprintf(ofp,"specified_cal_pulse_pos = %d\n",
      status.specified_cal_pulse_pos);
    fprintf(ofp,"prf_cycle_time = %d\n", status.prf_cycle_time);
    fprintf(ofp,"range_gate_a_delay = %d\n", status.range_gate_a_delay);
    fprintf(ofp,"range_gate_a_width = %d\n", status.range_gate_a_width);
    fprintf(ofp,"range_gate_b_delay = %d\n", status.range_gate_b_delay);
    fprintf(ofp,"range_gate_b_width = %d\n", status.range_gate_b_width);
    fprintf(ofp,"doppler_shift_command_1 = %d %d %d\n",
      status.doppler_shift_command_1[0],
      status.doppler_shift_command_1[1],
      status.doppler_shift_command_1[2]);
    fprintf(ofp,"doppler_shift_command_2 = %d %d %d\n",
      status.doppler_shift_command_2[0],
      status.doppler_shift_command_2[1],
      status.doppler_shift_command_2[2]);
    fprintf(ofp,"pulse_width = %d\n", status.pulse_width);
    fprintf(ofp,"receiver_gain (attenuation) = %d dB\n", status.receiver_gain);
    fprintf(ofp,"ses_configuration_flags = 0x%02x\n",
      status.ses_configuration_flags);
    fprintf(ofp,"ses_data_overrun_count = %d\n", status.ses_data_overrun_count);
    fprintf(ofp,"ses_data_underrun_count = %d\n",
      status.ses_data_underrun_count);
    fprintf(ofp,"pred_antenna_pos_count = %d\n", status.pred_antenna_pos_count);
    fprintf(ofp,"running_error_count = %d\n", *((unsigned short*)status.running_error_count));
    fprintf(ofp,"ses_reset_position = %d\n", status.ses_reset_position);
    fprintf(ofp,"doppler_orbit_step = %d\n", status.doppler_orbit_step);
    fprintf(ofp,"prf_orbit_step_change = %d\n", status.prf_orbit_step_change);
    fprintf(ofp,"cmd_history_queue = %d %d %d %d %d %d %d %d\n",
      status.cmd_history_queue[0],
      status.cmd_history_queue[1],
      status.cmd_history_queue[2],
      status.cmd_history_queue[3],
      status.cmd_history_queue[4],
      status.cmd_history_queue[5],
      status.cmd_history_queue[6],
      status.cmd_history_queue[7]);
    fprintf(ofp,"calc_ant_max_grp_count = %d\n", status.calc_ant_max_grp_count);
    unsigned int vtcw_hi4 = 0;
    unsigned short vtcw_lo2 = 0;
    (void)memcpy(&vtcw_hi4, status.vtcw, sizeof(unsigned int));
    (void)memcpy(&vtcw_lo2, status.vtcw+4, sizeof(unsigned short));
    double vtcw = (double)(vtcw_hi4)*65536.0 + vtcw_lo2;
    fprintf(ofp,"vtcw = %14.2f\n",vtcw);
    unsigned int corres_instr_time = 0;
    unsigned char frac = 0;
    (void)memcpy(&corres_instr_time, status.corres_instr_time,
                 sizeof(unsigned int));
    (void)memcpy(&frac, status.corres_instr_time+4, sizeof(unsigned char));
    fprintf(ofp,"corres_instr_time = %d, (frac) %d\n",corres_instr_time,frac);
    fprintf(ofp,"fsw_mission_version_num = %d\n",
      status.fsw_mission_version_num);
    fprintf(ofp,"fsw_build_number = %d\n",
      status.fsw_build_number);
    fprintf(ofp,"pbi_flag = %d\n",
      status.pbi_flag);
  
    fprintf(ofp,"\n ============== GS Engineering Block ==============\n\n");
    fprintf(ofp,"relay_status = %d\n",
      *((unsigned short *)engdata.relay_status));
    fprintf(ofp,"ea_a_spin_rate = %d\n",engdata.ea_a_spin_rate);
    fprintf(ofp,"ea_b_spin_rate = %d\n",engdata.ea_b_spin_rate);
    fprintf(ofp,"a2d_p12v_xcpl = %d\n",engdata.a2d_p12v_xcpl);
    fprintf(ofp,"transmit_power_a = %d\n",engdata.transmit_power_a);
    fprintf(ofp,"transmit_power_b = %d\n",engdata.transmit_power_b);
    fprintf(ofp,"precision_coupler_temp = %d\n",engdata.precision_coupler_temp);
    fprintf(ofp,"rcv_protect_sw_temp = %d\n", engdata.rcv_protect_sw_temp);
    fprintf(ofp,"beam_select_sw_temp = %d\n", engdata.beam_select_sw_temp);
    fprintf(ofp,"receiver_temp = %d\n", engdata.receiver_temp);
    fprintf(ofp,"a2d_p12v_xcpl voltage = %d\n", engdata.a2d_p12v_xcpl);
  
    fprintf(ofp,"\n ============== GS EU Block ==============\n\n");
    fprintf(ofp,"prf_cycle_time_eu = %g\n", in_eu.prf_cycle_time_eu);
    fprintf(ofp,"range_gate_delay_inner = %g\n", in_eu.range_gate_delay_inner);
    fprintf(ofp,"range_gate_delay_outer = %g\n", in_eu.range_gate_delay_outer);
    fprintf(ofp,"range_gate_width_inner = %g\n", in_eu.range_gate_width_inner);
    fprintf(ofp,"range_gate_width_outer = %g\n", in_eu.range_gate_width_outer);
    fprintf(ofp,"transmit_pulse_width = %g\n", in_eu.transmit_pulse_width);
    fprintf(ofp,"true_cal_pulse_pos = %d\n", in_eu.true_cal_pulse_pos);
    fprintf(ofp,"transmit_power_inner = %g\n", in_eu.transmit_power_inner);
    fprintf(ofp,"transmit_power_outer = %g\n", in_eu.transmit_power_outer);
    fprintf(ofp,"precision_coupler_temp_eu = %g\n",
                           in_eu.precision_coupler_temp_eu);
    fprintf(ofp,"rcv_protect_sw_temp_eu = %g\n", in_eu.rcv_protect_sw_temp_eu);
    fprintf(ofp,"beam_select_sw_temp_eu = %g\n", in_eu.beam_select_sw_temp_eu);
    fprintf(ofp,"receiver_temp_eu = %g\n", in_eu.receiver_temp_eu);
    fprintf(ofp,"\n ============== GS Science Block ==============\n\n");
    int i=0, j=0, k=0;
    fprintf(ofp,"\n antenna_position[100]: \n");
    for (i=0; i < 10; i++)
    {
        for (j=0; j < 10; j++)
            fprintf(ofp, " %d", in_science.antenna_position[i*10+j]);
        fprintf(ofp, "\n");
    }

    fprintf(ofp,"\nloop_back_cal_A_power[12]: \n");
    for (i=0; i < 1; i++)
    {
        for (j=0; j < 12; j++)
            fprintf(ofp, " %g", in_science.loop_back_cal_A_power[i*12+j]);
        fprintf(ofp, "\n");
    }

    fprintf(ofp,"\nloop_back_cal_B_power[12]: \n");
    for (i=0; i < 1; i++)
    {
        for (j=0; j < 12; j++)
            fprintf(ofp, " %g", in_science.loop_back_cal_B_power[i*12+j]);
        fprintf(ofp, "\n");
    }
    fprintf(ofp,"\nloop_back_cal_noise: %g, load_cal_noise: %g\n",
                 in_science.loop_back_cal_noise, in_science.load_cal_noise);
    fprintf(ofp,"\nload_cal_A_power[12]: \n");
    for (i=0; i < 1; i++)
    {
        for (j=0; j < 12; j++)
            fprintf(ofp, " %g", in_science.load_cal_A_power[i*12+j]);
        fprintf(ofp, "\n");
    }
    fprintf(ofp,"\nload_cal_B_power[12]: \n");
    for (i=0; i < 1; i++)
    {
        for (j=0; j < 12; j++)
            fprintf(ofp, " %g", in_science.load_cal_B_power[i*12+j]);
        fprintf(ofp, "\n");
    }

    fprintf(ofp,"\npower_dn[12][100]: \n");
    int count = 0;
    for (k=0; k < 12; k++)
    {
      for (j=0; j < 100; j++)
      {
        fprintf(ofp, " %d", in_science.power_dn[k][j]);
        if (count >= 11)
        {
          fprintf(ofp, "\n");
          count = 0;
        }
        else
        {
          count++;
        }
      }
    }
    fprintf(ofp,"\nl1a_frame_inst_status: 0x%08x\n", l1a_frame_inst_status);
    fprintf(ofp,"l1a_frame_err_status: 0x%08x\n", l1a_frame_err_status);
    fprintf(ofp,"l1a_frame_qual_flag: 0x%08x\n", l1a_frame_qual_flag);
    fprintf(ofp, "l1a_pulse_qual_flag:\n");
    for (int i=0; i < 13; i++)
    {
        fprintf(ofp, "%02x", l1a_pulse_qual_flag[i]);
    }
    fprintf(ofp, "\n");
    return 1;

} // L1AGSFrame::WriteAscii


#ifdef test_L1AGSFrame

int
main(
int     ,
char**  )
{
    L1AGSFrame  gsFrame;
    return(0);
}

#endif
