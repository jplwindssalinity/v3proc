//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_l1ah_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <math.h>
#include "L1AH.h"
#include "ETime.h"
#include "Sds.h"
#include "Qscat.h"
#include "mfhdf.h"

//=========================//
// The table of Attributes //
//=========================//

Attribute* long_name = new Attribute("LongName", "char", "1",
    "SeaWinds Level 1A Engineering Unit Converted Telemetry");
Attribute* short_name = new Attribute("ShortName", "char", "1",
    "SWSL1A");
Attribute* producer_agency = new Attribute("producer_agency", "char", "1",
    "NASA");
Attribute* producer_institution = new Attribute("producer_institution",
    "char", "1", "JPL");
Attribute* instrument_short_name = new Attribute("InstrumentShortName",
    "char", "1", "SeaWinds");
Attribute* platform_long_name = new Attribute("PlatformLongName", "char",
    "1", "Advanced Earth Observing Satellite II");
Attribute* platform_short_name = new Attribute("PlatformShortName", "char",
    "1", "ADEOS-II");
Attribute* platform_type = new Attribute("PlatformType", "char", "1",
    "spacecraft");
Attribute* project_id = new Attribute("project_id", "char", "1", "SeaWinds");
Attribute* data_format_type = new Attribute("data_format_type", "char", "1",
    "NCSA HDF");
Attribute* qa_percent_out_of_bounds_data =
    new Attribute("QAPercentOutOfBoundsData", "int", "1", "0");
Attribute* qa_percent_missing_data = new Attribute("QAPercentMissingData",
    "int", "1", "0");
Attribute* build_id = new Attribute("build_id", "char", "1", "1.0/Sim");
Attribute* hdf_version_id = new Attribute("HDF_version_id", "char", "1",
    "4.1r3");
Attribute* production_date_time = new Attribute("production_date_time",
    "char", "1", "<missing>");
Attribute* sis_id = new Attribute("sis_id", "char", "1",
    "686-644-5/2000-04-01");
Attribute* operation_mode = new Attribute("OperationMode", "char", "1",
    "Wind Observation");
Attribute* start_orbit_number = new Attribute("StartOrbitNumber", "int", "1",
    "<missing>");
Attribute* stop_orbit_number = new Attribute("StopOrbitNumber", "int", "1",
    "<missing>");
Attribute* equator_crossing_longitude =
    new Attribute("EquatorCrossingLongitude", "float", "1", "<missing>");
Attribute* equator_crossing_date = new Attribute("EquatorCrossingDate",
    "char", "1", "<missing>");
Attribute* equator_crossing_time = new Attribute("EquatorCrossingTime",
    "char", "1", "<missing>");
Attribute* rev_number = new Attribute("rev_number", "int", "1", "<missing>");
Attribute* rev_orbit_period = new Attribute("rev_orbit_period", "float", "1",
    "<missing>");
Attribute* orbit_inclination = new Attribute("orbit_inclination", "float", "1",
    "<missing>");
Attribute* orbit_semi_major_axis = new Attribute("orbit_semi_major_axis",
    "float", "1", "<missing>");
Attribute* orbit_eccentricity = new Attribute("orbit_eccentricity", "float",
    "1", "<missing>");
Attribute* range_beginning_date = new Attribute("RangeBeginningDate", "char",
    "1", "<missing>");
Attribute* range_beginning_time = new Attribute("RangeBeginningTime", "char",
    "1", "<missing>");
Attribute* range_ending_date = new Attribute("RangeEndingDate", "char",
    "1", "<missing>");
Attribute* range_ending_time = new Attribute("RangeEndingTime", "char",
    "1", "<missing>");
Attribute* ephemeris_type = new Attribute("ephemeris_type", "char", "1",
    "Sim");
Attribute* parameter_name = new Attribute("ParameterName", "char", "1",
    "power_dn");
Attribute* attitude_type = new Attribute("attitude_type", "char", "1",
    "Sim");
Attribute* maximum_pulses_per_frame =
    new Attribute("maximum_pulses_per_frame", "int", "1", "<missing>");
Attribute* l1a_expected_frames = new Attribute("l1a_expected_frames", "int",
    "1", "<missing>");
Attribute* l1a_actual_frames = new Attribute("l1a_actual_frames", "int",
    "1", "<missing>");
Attribute* product_span = new Attribute("product_span", "int", "1",
    "Pass");

Attribute* g_attribute_table[] =
{
    long_name,
    short_name,
    producer_agency,
    producer_institution,
    platform_type,
    instrument_short_name,
    platform_long_name,
    platform_short_name,
    project_id,
    data_format_type,
    qa_percent_out_of_bounds_data,
    qa_percent_missing_data,
    build_id,
    hdf_version_id,
    production_date_time,
    sis_id,
    operation_mode,
    start_orbit_number,
    stop_orbit_number,
    equator_crossing_longitude,
    equator_crossing_date,
    equator_crossing_time,
    rev_number,
    rev_orbit_period,
    orbit_inclination,
    orbit_semi_major_axis,
    orbit_eccentricity,
    range_beginning_date,
    range_beginning_time,
    range_ending_date,
    range_ending_time,
    ephemeris_type,
    parameter_name,
    attitude_type,
    maximum_pulses_per_frame,
    l1a_expected_frames,
    l1a_actual_frames,
    product_span,
    NULL
};

//====================//
// The table of SDS's //
//====================//

int32 dim_sizes_frame[] = { SD_UNLIMITED };
int32 dim_sizes_frame_3[] = { SD_UNLIMITED, 3 };
int32 dim_sizes_frame_4[] = { SD_UNLIMITED, 4 };
int32 dim_sizes_frame_5[] = { SD_UNLIMITED, 5 };
int32 dim_sizes_frame_8[] = { SD_UNLIMITED, 8 };
int32 dim_sizes_frame_13[] = { SD_UNLIMITED, 13 };
int32 dim_sizes_frame_100[] = { SD_UNLIMITED, 100 };
int32 dim_sizes_frame_2_8[] = { SD_UNLIMITED, 2, 8 };
int32 dim_sizes_frame_100_12[] = { SD_UNLIMITED, 100, 12 };
const char* dim_names_frame[] = { "Telemetry_Frame" };
const char* dim_names_frame_packet_header[] = { "Telemetry_Frame",
    "Small_Integer" };
const char* dim_names_frame_err_msg_hist[] = { "Telemetry_Frame", "Message" };
const char* dim_names_frame_cmd_history[] = { "Telemetry_Frame", "Command" };
const char* dim_names_frame_fill[] = { "Telemetry_Frame", "Fill_Entries" };
const char* dim_names_frame_word[] = { "Telemetry_Frame", "Word" };
const char* dim_names_frame_pcd[] = { "Telemetry_Frame", "PCD_Group",
    "Entry" };
const char* dim_names_frame_pulse[] = { "Telemetry_Frame",
    "Scatterometer_Pulse" };
const char* dim_names_frame_byte[] = { "Telemetry_Frame", "Byte" };
const char* dim_names_frame_pulse_slice[] = { "Telemetry_Frame",
    "Scatterometer_Pulse", "Chirp" };
const char* dim_names_frame_[] = { "Telemetry_Frame" };

//=======//
// SDSes //
//=======//

SdsFloat64* frame_time_secs = new SdsFloat64("frame_time_secs", 1,
    dim_sizes_frame, "sec", 1.0, 0.0, dim_names_frame, 5.0E9, 0.0);
SdsFloat64* instrument_time = new SdsFloat64("instrument_time", 1,
    dim_sizes_frame, "counts", 1.0, 0.0, dim_names_frame, pow(2.0, 36.0), 0.0);
SdsUInt32* orbit_time = new SdsUInt32("orbit_time", 1, dim_sizes_frame,
    "counts", 1.0, 0.0, dim_names_frame, 4294967295, 0);
SdsFloat32* x_pos = new SdsFloat32("x_pos", 1, dim_sizes_frame, "m", 1.0, 0.0,
    dim_names_frame, 9999999.0, -9999999.0);
SdsFloat32* y_pos = new SdsFloat32("y_pos", 1, dim_sizes_frame, "m", 1.0, 0.0,
    dim_names_frame, 9999999.0, -9999999.0);
SdsFloat32* z_pos = new SdsFloat32("z_pos", 1, dim_sizes_frame, "m", 1.0, 0.0,
    dim_names_frame, 9999999.0, -9999999.0);
SdsFloat32* x_vel = new SdsFloat32("x_vel", 1, dim_sizes_frame, "m/s",
    1.0, 0.0, dim_names_frame, 8000.0, -8000.0);
SdsFloat32* y_vel = new SdsFloat32("y_vel", 1, dim_sizes_frame, "m/s",
    1.0, 0.0, dim_names_frame, 8000.0, -8000.0);
SdsFloat32* z_vel = new SdsFloat32("z_vel", 1, dim_sizes_frame, "m/s",
    1.0, 0.0, dim_names_frame, 8000.0, -8000.0);
SdsInt16* roll = new SdsInt16("roll", 1, dim_sizes_frame, "deg", 0.001, 0.0,
    dim_names_frame, 3.0, -3.0);
SdsInt16* pitch = new SdsInt16("pitch", 1, dim_sizes_frame, "deg", 0.001, 0.0,
    dim_names_frame, 3.0, -3.0);
SdsInt16* yaw = new SdsInt16("yaw", 1, dim_sizes_frame, "deg", 0.001, 0.0,
    dim_names_frame, 3.0, -3.0);
SdsUInt16* first_packet_header = new SdsUInt16("first_packet_header", 2,
    dim_sizes_frame_3, "n/a", 1.0, 0.0, dim_names_frame_packet_header,
    0xffff, 0x0000);
SdsUInt16* telemetry_table_id = new SdsUInt16("telemetry_table_id", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 65535, 0);
SdsUInt8* status_error_flags = new SdsUInt8("status_error_flags", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 0xff, 0x00);
SdsUInt8* table_readout_type = new SdsUInt8("table_readout_type", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 0x1e, 0x00);
SdsUInt16* table_readout_offset = new SdsUInt16("table_readout_offset", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 0xffff, 0x0000);
SdsUInt32* table_readout_data = new SdsUInt32("table_readout_data", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 0xffffffff, 0x00000000);
SdsUInt8* operational_mode = new SdsUInt8("operational_mode", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 0xe0, 0x00);
SdsUInt8* prf_count = new SdsUInt8("prf_count", 1, dim_sizes_frame, "counts",
    1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt16* status_change_flags = new SdsUInt16("prf_count", 1, dim_sizes_frame,
    "n/a", 1.0, 0.0, dim_names_frame, 0xffff, 0x0000);
SdsUInt16* error_message = new SdsUInt16("error_message", 1, dim_sizes_frame,
    "n/a", 1.0, 0.0, dim_names_frame, 65535, 0);
SdsUInt16* error_message_history = new SdsUInt16("error_message_history", 2,
    dim_sizes_frame_5, "n/a", 1.0, 0.0, dim_names_frame_err_msg_hist,
    65535, 0);
SdsUInt8* valid_command_count = new SdsUInt8("valid_command_count", 1,
    dim_sizes_frame, "counts", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* invalid_command_count = new SdsUInt8("invalid_command_count", 1,
    dim_sizes_frame, "counts", 1.0, 0.0, dim_names_frame, 255, 0);
SdsInt8* specified_cal_pulse_pos = new SdsInt8("specified_cal_pulse_pos", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 96, -1);
SdsUInt8* prf_cycle_time = new SdsUInt8("prf_cycle_time", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* range_gate_a_delay = new SdsUInt8("range_gate_a_delay", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* range_gate_a_width = new SdsUInt8("range_gate_a_width", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* range_gate_b_delay = new SdsUInt8("range_gate_b_delay", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* range_gate_b_width = new SdsUInt8("range_gate_b_width", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt32* doppler_shift_command_1 = new SdsUInt32("doppler_shift_command_1",
    1, dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 0x00ffffff,
    0x00000000);
SdsUInt32* doppler_shift_command_2 = new SdsUInt32("doppler_shift_command_2",
    1, dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 0x00ffffff,
    0x00000000);
SdsUInt8* pulse_width = new SdsUInt8("pulse_width", 1, dim_sizes_frame,
    "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* receiver_gain = new SdsUInt8("receiver_gain", 1, dim_sizes_frame,
    "dB", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* ses_configuration_flags = new SdsUInt8("ses_configuration_flags", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 0x0f, 0);
SdsUInt8* ses_data_overrun_count = new SdsUInt8("ses_data_overrun_count", 1,
    dim_sizes_frame, "counts", 1.0, 0.0, dim_names_frame, 100, 0);
SdsUInt8* ses_data_underrun_count = new SdsUInt8("ses_data_underrun_count", 1,
    dim_sizes_frame, "counts", 1.0, 0.0, dim_names_frame, 100, 0);
SdsUInt8* pred_antenna_pos_count = new SdsUInt8("pred_antenna_pos_count", 1,
    dim_sizes_frame, "counts", 1.0, 0.0, dim_names_frame, 100, 0);
SdsUInt16* running_error_count = new SdsUInt16("running_error_count", 1,
    dim_sizes_frame, "counts", 1.0, 0.0, dim_names_frame, 65535, 0);
SdsInt8* ses_reset_position = new SdsInt8("ses_reset_position", 1,
    dim_sizes_frame, "pulse", 1.0, 0.0, dim_names_frame, 100, -1);
SdsUInt8* doppler_orbit_step = new SdsUInt8("doppler_orbit_step", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 255, 0);
SdsInt8* prf_orbit_step_change = new SdsInt8("prf_orbit_step_change", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 100, -1);
SdsUInt16* cmd_history_queue = new SdsUInt16("cmd_history_queue", 2,
    dim_sizes_frame_4, "n/a", 1.0, 0.0, dim_names_frame_cmd_history, 65535, 0);
SdsUInt8* calc_ant_max_grp_count = new SdsUInt8("calc_ant_max_grp_count", 1,
    dim_sizes_frame, "counts", 1.0, 0.0, dim_names_frame, 100, 0);
SdsFloat64* vtcw = new SdsFloat64("vtcw", 1, dim_sizes_frame, "counts", 1.0,
    0.0, dim_names_frame, 28147497.0, 0);
SdsFloat64* corres_instr_time = new SdsFloat64("corres_instr_time", 1,
    dim_sizes_frame, "counts", 1.0, 0.0, dim_names_frame, 68719476736.0, 0);
SdsUInt8* fsw_mission_version_num = new SdsUInt8("fsw_mission_version_num", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 0x3f, 0x00);
SdsUInt8* fsw_build_number = new SdsUInt8("fsw_build_number", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* pbi_flag = new SdsUInt8("pbi_flag", 1, dim_sizes_frame, "n/a",
    1.0, 0.0, dim_names_frame, 0x3f, 0);
SdsFloat32* fill = new SdsFloat32("fill", 1, dim_sizes_frame_8, "n/a", 1.0,
    0.0, dim_names_frame_fill, 0.0, 0.0);
SdsUInt8* psu_elec_bus_volt = new SdsUInt8("psu_elec_bus_volt", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* cds_current = new SdsUInt8("cds_current", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* ses_a_current = new SdsUInt8("ses_a_current", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* ses_b_current = new SdsUInt8("ses_b_current", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* twta_1_current = new SdsUInt8("twta_1_current", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* twta_2_current = new SdsUInt8("twta_2_current", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* sas_a_current = new SdsUInt8("sas_a_current", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* sas_b_current = new SdsUInt8("sas_b_current", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* pcu_sec_volt_p12 = new SdsUInt8("pcu_sec_volt_p12", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* pcu_sec_volt_n12 = new SdsUInt8("pcu_sec_volt_n12", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* pcu_sec_volt_p30 = new SdsUInt8("pcu_sec_volt_p30", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* pcu_sec_volt_vme_p3 = new SdsUInt8("pcu_sec_volt_vme_p3", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* pcu_elec_bus_volt = new SdsUInt8("pcu_elec_bus_volt", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* idp_a_temp = new SdsUInt8("idp_a_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* idp_b_temp = new SdsUInt8("idp_b_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* psu_temp = new SdsUInt8("psu_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt16* relay_status = new SdsUInt16("relay_status", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 0xffff, 0x0000);
SdsUInt8* ea_a_motor_current = new SdsUInt8("ea_a_motor_current", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* ea_a_sec_volt_p5 = new SdsUInt8("ea_a_sec_volt_p5", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* ea_a_sec_volt_p14 = new SdsUInt8("ea_a_sec_volt_p14", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* ea_a_spin_rate = new SdsUInt8("ea_a_spin_rate", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* ea_a_saa_torque_cmd = new SdsUInt8("ea_a_saa_torque_cmd", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* ea_b_motor_current = new SdsUInt8("ea_b_motor_current", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* ea_b_sec_volt_p5 = new SdsUInt8("ea_b_sec_volt_p5", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* ea_b_sec_volt_p14 = new SdsUInt8("ea_b_sec_volt_p14", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* ea_b_spin_rate = new SdsUInt8("ea_b_spin_rate", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* ea_b_saa_torque_cmd = new SdsUInt8("ea_b_saa_torque_cmd", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* drive_motor_temp = new SdsUInt8("drive_motor_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* ea_a_power_supply_temp = new SdsUInt8("ea_a_power_supply_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* ea_b_power_supply_temp = new SdsUInt8("ea_b_power_supply_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* duplex_bearing_temp = new SdsUInt8("duplex_bearing_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* simplex_bearing_temp = new SdsUInt8("simplex_bearing_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* rj_temp = new SdsUInt8("rj_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* a2d_p12v_xcpl = new SdsUInt8("a2d_p12v_xcpl", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* twt1_body_reg_volt = new SdsUInt8("twt1_body_reg_volt", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* twt1_ion_pump_current = new SdsUInt8("twt1_ion_pump_current", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* twt1_body_current = new SdsUInt8("twt1_body_current", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* twt1_drive_power = new SdsUInt8("twt1_drive_power", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* twt2_body_reg_volt = new SdsUInt8("twt2_body_reg_volt", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* twt2_ion_pump_current = new SdsUInt8("twt2_ion_pump_current", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* twt2_body_current = new SdsUInt8("twt2_body_current", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* twt2_drive_power = new SdsUInt8("twt2_drive_power", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* transmit_power_a = new SdsUInt8("transmit_power_a", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* transmit_power_b = new SdsUInt8("transmit_power_b", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* power_convert_current = new SdsUInt8("power_convert_current", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* precision_coupler_temp = new SdsUInt8("precision_coupler_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* twt1_hvps_chassis_temp = new SdsUInt8("twt1_hvps_chassis_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* twt1_base_temp = new SdsUInt8("twt1_base_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* twt2_hvps_chassis_temp = new SdsUInt8("twt2_hvps_chassis_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* twt2_base_temp = new SdsUInt8("twt2_base_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* rcv_protect_sw_temp = new SdsUInt8("rcv_protect_sw_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* power_converter_temp = new SdsUInt8("power_converter_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* gain_atten_temp = new SdsUInt8("gain_atten_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* beam_select_sw_temp = new SdsUInt8("beam_select_sw_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* scp_temp = new SdsUInt8("scp_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* receiver_temp = new SdsUInt8("receiver_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* exciter_a_temp = new SdsUInt8("exciter_a_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* exciter_b_temp = new SdsUInt8("exciter_b_temp", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* eng_status_c1 = new SdsUInt8("eng_status_c1", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* eng_status_c2 = new SdsUInt8("eng_status_c2", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* eng_status_c3 = new SdsUInt8("eng_status_c3", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt8* ses_data_error_flags = new SdsUInt8("ses_data_error_flags", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 255, 0);
SdsUInt32* cds_memory_dump_addr = new SdsUInt32("cds_memory_dump_addr", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 0xffffffff, 0x00000000);
SdsUInt32* cds_memory_dump_data = new SdsUInt32("cds_memory_dump_data", 2,
    dim_sizes_frame_4, "n/a", 1.0, 0.0, dim_names_frame_word, 0xffffffff,
    0x00000000);
SdsUInt16* ses_memory_dump_addr = new SdsUInt16("ses_memory_dump_addr", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 0xffff, 0x0000);
SdsUInt32* ses_memory_dump_data = new SdsUInt32("ses_memory_dump_data", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 0xffffffff,
    0x00000000);
SdsUInt16* pcd_entry = new SdsUInt16("pcd_entry", 3, dim_sizes_frame_2_8,
    "DN", 1.0, 0.0, dim_names_frame_pcd, 0xffff, 0x0000);
SdsUInt16* pcd_entry2 = new SdsUInt16("pcd_entry", 3, dim_sizes_frame_2_8,
    "DN", 1.0, 0.0, dim_names_frame_pcd, 0xffff, 0x0000);
SdsInt16* range_gate_delay_inner = new SdsInt16("range_gate_delay_inner", 1,
    dim_sizes_frame, "sec", 0.000001, 0.0, dim_names_frame, 12750, 0);
SdsInt16* range_gate_delay_outer = new SdsInt16("range_gate_delay_outer", 1,
    dim_sizes_frame, "sec", 0.000001, 0.0, dim_names_frame, 12750, 0);
SdsInt16* range_gate_width_inner = new SdsInt16("range_gate_width_inner", 1,
    dim_sizes_frame, "sec", 0.000001, 0.0, dim_names_frame, 12750, 0);
SdsInt16* range_gate_width_outer = new SdsInt16("range_gate_width_outer", 1,
    dim_sizes_frame, "sec", 0.000001, 0.0, dim_names_frame, 12750, 0);
SdsInt16* transmit_pulse_width = new SdsInt16("transmit_pulse_width", 1,
    dim_sizes_frame, "sec", 0.000001, 0.0, dim_names_frame, 12750, 0);
SdsInt8* true_cal_pulse_pos = new SdsInt8("true_cal_pulse_pos", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 95, -1);
SdsInt16* transmit_power_inner = new SdsInt16("transmit_power_inner", 1,
    dim_sizes_frame, "dBm", 0.01, 0.0, dim_names_frame, 7000, 3000);
SdsInt16* transmit_power_outer = new SdsInt16("transmit_power_outer", 1,
    dim_sizes_frame, "dBm", 0.01, 0.0, dim_names_frame, 7000, 3000);
SdsInt16* rj_temp_eu = new SdsInt16("rj_temp_eu", 1,
    dim_sizes_frame, "deg_C", 0.01, 0.0, dim_names_frame, 8314, -1706);
SdsInt16* precision_coupler_temp_eu =
    new SdsInt16("precision_coupler_temp_eu", 1, dim_sizes_frame, "deg_C",
    0.01, 0.0, dim_names_frame, 8314, -1706);
SdsInt16* rcv_protect_sw_temp_eu = new SdsInt16("rcv_protect_sw_temp_eu", 1,
    dim_sizes_frame, "deg_C", 0.01, 0.0, dim_names_frame, 8314, -1706);
SdsInt16* beam_select_sw_temp_eu = new SdsInt16("beam_select_sw_temp_eu", 1,
    dim_sizes_frame, "deg_C", 0.01, 0.0, dim_names_frame, 8314, -1706);
SdsInt16* receiver_temp_eu = new SdsInt16("receiver_temp_eu", 1,
    dim_sizes_frame, "deg_C", 0.01, 0.0, dim_names_frame, 8314, -1706);
SdsUInt32* loop_back_cal_A_power = new SdsUInt32("loop_back_cal_A_power", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 8355840, 0);
SdsUInt32* loop_back_cal_B_power = new SdsUInt32("loop_back_cal_B_power", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 8355840, 0);
SdsUInt32* load_cal_A_power = new SdsUInt32("load_cal_A_power", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 8355840, 0);
SdsUInt32* load_cal_B_power = new SdsUInt32("load_cal_B_power", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 8355840, 0);
SdsUInt32* loop_back_cal_noise = new SdsUInt32("loop_back_cal_noise", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 4294967295, 0);
SdsUInt32* load_cal_noise = new SdsUInt32("load_cal_noise", 1,
    dim_sizes_frame, "DN", 1.0, 0.0, dim_names_frame, 4294967295, 0);
SdsUInt16* antenna_position = new SdsUInt16("antenna_position", 2,
    dim_sizes_frame_100, "DN", 1.0, 0.0, dim_names_frame_pulse, 65535, 0);
SdsUInt32* power_dn = new SdsUInt32("power_dn", 3, dim_sizes_frame_100_12,
    "DN", 1.0, 0.0, dim_names_frame_pulse_slice, 8355840, 0);
SdsUInt32* noise_dn = new SdsUInt32("noise_dn", 2, dim_sizes_frame_100,
    "DN", 1.0, 0.0, dim_names_frame_pulse, 4294967295, 0);
SdsUInt32* frame_inst_status = new SdsUInt32("frame_inst_status", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 0x3fffffff, 0x00000000);
SdsUInt32* frame_err_status = new SdsUInt32("frame_err_status", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 0x0fffffff, 0x00000000);
SdsUInt16* frame_qual_flag = new SdsUInt16("frame_qual_flag", 1,
    dim_sizes_frame, "n/a", 1.0, 0.0, dim_names_frame, 0x001f, 0x0000);
SdsUInt8* pulse_qual_flag = new SdsUInt8("pulse_qual_flag", 2,
    dim_sizes_frame_13, "n/a", 1.0, 0.0, dim_names_frame_byte, 0xff, 0x00);

Sds* g_sds_table[] =
{
    frame_time_secs,
    instrument_time,
    orbit_time,
    x_pos,
    y_pos,
    z_pos,
    x_vel,
    y_vel,
    z_vel,
    roll,
    pitch,
    yaw,
    first_packet_header,
    telemetry_table_id,
    status_error_flags,
    table_readout_type,
    table_readout_offset,
    table_readout_data,
    operational_mode,
    prf_count,
    status_change_flags,
    error_message,
    error_message_history,
    valid_command_count,
    invalid_command_count,
    specified_cal_pulse_pos,
    prf_cycle_time,
    range_gate_a_delay,
    range_gate_a_width,
    range_gate_b_delay,
    range_gate_b_width,
    doppler_shift_command_1,
    doppler_shift_command_2,
    pulse_width,
    receiver_gain,
    ses_configuration_flags,
    ses_data_overrun_count,
    ses_data_underrun_count,
    pred_antenna_pos_count,
    running_error_count,
    ses_reset_position,
    doppler_orbit_step,
    prf_orbit_step_change,
    cmd_history_queue,
    calc_ant_max_grp_count,
    vtcw,
    corres_instr_time,
    fsw_mission_version_num,
    fsw_build_number,
    pbi_flag,
    fill,
    psu_elec_bus_volt,
    cds_current,
    ses_a_current,
    ses_b_current,
    twta_1_current,
    twta_2_current,
    sas_a_current,
    sas_b_current,
    pcu_sec_volt_p12,
    pcu_sec_volt_n12,
    pcu_sec_volt_p30,
    pcu_sec_volt_vme_p3,
    pcu_elec_bus_volt,
    idp_a_temp,
    idp_b_temp,
    psu_temp,
    relay_status,
    ea_a_motor_current,
    ea_a_sec_volt_p5,
    ea_a_sec_volt_p14,
    ea_a_spin_rate,
    ea_a_saa_torque_cmd,
    ea_b_motor_current,
    ea_b_sec_volt_p5,
    ea_b_sec_volt_p14,
    ea_b_spin_rate,
    ea_b_saa_torque_cmd,
    drive_motor_temp,
    ea_a_power_supply_temp,
    ea_b_power_supply_temp,
    duplex_bearing_temp,
    simplex_bearing_temp,
    rj_temp,
    a2d_p12v_xcpl,
    twt1_body_reg_volt,
    twt1_ion_pump_current,
    twt1_body_current,
    twt1_drive_power,
    twt2_body_reg_volt,
    twt2_ion_pump_current,
    twt2_body_current,
    twt2_drive_power,
    transmit_power_a,
    transmit_power_b,
    power_convert_current,
    precision_coupler_temp,
    twt1_hvps_chassis_temp,
    twt1_base_temp,
    twt2_hvps_chassis_temp,
    twt2_base_temp,
    rcv_protect_sw_temp,
    power_converter_temp,
    gain_atten_temp,
    beam_select_sw_temp,
    scp_temp,
    receiver_temp,
    exciter_a_temp,
    exciter_b_temp,
    eng_status_c1,
    eng_status_c2,
    eng_status_c3,
    ses_data_error_flags,
    cds_memory_dump_addr,
    cds_memory_dump_data,
    pcd_entry,
    range_gate_delay_inner,
    range_gate_delay_outer,
    range_gate_width_inner,
    range_gate_width_outer,
    transmit_pulse_width,
    true_cal_pulse_pos,
    transmit_power_inner,
    transmit_power_outer,
    rj_temp_eu,
    precision_coupler_temp_eu,
    rcv_protect_sw_temp_eu,
    beam_select_sw_temp_eu,
    receiver_temp_eu,
    loop_back_cal_A_power,
    loop_back_cal_B_power,
    load_cal_A_power,
    load_cal_B_power,
    loop_back_cal_noise,
    load_cal_noise,
    antenna_position,
    power_dn,
    noise_dn,
    frame_inst_status,
    frame_err_status,
    frame_qual_flag,
    pulse_qual_flag,
    NULL
};

//======//
// L1AH //
//======//

L1AH::L1AH()
:   _eqxTime(0.0), _rangeBeginningTime(0.0), _rangeEndingTime(0.0),
    _eqxLongitude(0.0), _hdfInputFileId(0), _hdfOutputFileId(0),
    _sdsInputFileId(0), _sdsOutputFileId(0), _currentRecordIdx(0)
{
    ETime ref;
    ref.FromCodeA("2005-01-01");
    _referenceTime = ref.GetSec();
    return;
}

//------------------//
// L1AH::NextRecord //
//------------------//

int
L1AH::NextRecord()
{
    _currentRecordIdx++;
    return (_currentRecordIdx);
}

//-------------------------//
// L1AH::OpenHdfForWriting //
//-------------------------//

int
L1AH::OpenHdfForWriting()
{
    _hdfOutputFileId = Hopen(_outputFilename, DFACC_CREATE, 0);
    if (_hdfOutputFileId == FAIL)
    {
        return (0);
    }
    return (1);
}

//-------------------------//
// L1AH::OpenHdfForReading //
//-------------------------//

int
L1AH::OpenHdfForReading()
{
    _hdfInputFileId = Hopen(_inputFilename, DFACC_READ, 0);
    if (_hdfInputFileId == FAIL)
    {
        return (0);
    }
    return (1);
}

//--------------------//
// L1AH::CreateVdatas //
//--------------------//

int
L1AH::CreateVdatas()
{
    // initialize the Vdata interface
    if (Vstart(_hdfOutputFileId) != SUCCEED)
    {
        fprintf(stderr, "CreateVdatas: error with Vstart\n");
        return(0);
    }

    // create a new vdata (the -1 means create)
    int32 vdata_id = VSattach(_hdfOutputFileId, -1, "w");

    // set the name and class
    if (VSsetname(vdata_id, FRAME_TIME_NAME) != SUCCEED)
    {
        fprintf(stderr, "CreateVdatas: error with VSsetname\n");
        return(0);
    }
    if (VSsetclass(vdata_id, FRAME_TIME_NAME) != SUCCEED)
    {
        fprintf(stderr, "CreateVdatas: error with VSsetclass\n");
        return(0);
    }

    // define
    if (VSfdefine(vdata_id, FRAME_TIME_NAME, 21, 21) != SUCCEED)
    {
        fprintf(stderr, "CreateVdatas: error with VSfdefine\n");
        return(0);
    }

    // set fields
    if (VSsetfields(vdata_id, FRAME_TIME_NAME) != SUCCEED)
    {
        fprintf(stderr, "CreateVdatas: error with VSsetfields\n");
        return(0);
    }

    // detach
    if (VSdetach(vdata_id) != SUCCEED)
    {
        fprintf(stderr, "CreateVdatas: error with VSdetach\n");
        return(0);
    }

    return (1);
}

//-------------------//
// L1AH::WriteVdatas //
//-------------------//
// coverts the frame time into the appropriate format for writing
// to a vdata

int
L1AH::WriteVdatas()
{
    // add the reference time to the delta time to get the "real" time
    ETime real_time;
    real_time.SetTime(frame.time + _referenceTime);

    // convert to a string
    char string[CODE_B_TIME_LENGTH];
    real_time.ToCodeB(string);

    // get the reference
    int32 vdata_ref = VSfind(_hdfOutputFileId, FRAME_TIME_NAME);
    if (vdata_ref == 0)
    {
        fprintf(stderr, "WriteVdatas: error with VSfind\n");
        return(0);
    }

    // attach
    int32 vdata_id = VSattach(_hdfOutputFileId, vdata_ref, "w");
    if (vdata_id == FAIL)
    {
        fprintf(stderr, "WriteVdatas: error with VSattach\n");
        return(0);
    }

    // seek
    if (_currentRecordIdx > 0)
    {
        // HDF seek function can't seek to the end
        // you are "supposed" to seek to one before the end...
        if (VSseek(vdata_id, _currentRecordIdx - 1) == FAIL)
        {
            fprintf(stderr, "WriteVdatas: error with VSseek\n");
            return(0);
        }
        // ...and then read the last one
        unsigned char dummy[CODE_B_TIME_LENGTH];
        if (VSread(vdata_id, dummy, 1, FULL_INTERLACE) != 1)
        {
            fprintf(stderr, "WriteVdatas: error with VSread\n");
            return(0);
        }
    }

    // write
    if (VSwrite(vdata_id, (unsigned char *)string, 1, FULL_INTERLACE) != 1)
    {
        fprintf(stderr, "WriteVdatas: error with VSwrite\n");
        return(0);
    }

    // detach
    if (VSdetach(vdata_id) != SUCCEED)
    {
        fprintf(stderr, "WriteVdatas: error with VSdetach\n");
        return(0);
    }

    return(1);
}

//----------------------//
// L1AH::EndVdataOutput //
//----------------------//

int
L1AH::EndVdataOutput()
{
    if (Vend(_hdfOutputFileId) != SUCCEED)
    {
        fprintf(stderr, "EndVdataOutput: error wind Vend\n");
        return(0);
    }
    return(1);
}

//-------------------------//
// L1AH::OpenSDSForWriting //
//-------------------------//

int
L1AH::OpenSDSForWriting()
{
    _sdsOutputFileId = SDstart(_outputFilename, DFACC_WRITE);
    if (_sdsOutputFileId == FAIL)
    {
        return (0);
    }
    return (1);
}

//------------------//
// L1AH::CreateSDSs //
//------------------//

int
L1AH::CreateSDSs()
{
    for (int idx = 0; g_sds_table[idx] != NULL; idx++)
    {
        Sds* sds = g_sds_table[idx];
        if (! sds->Create(_sdsOutputFileId))
        {
            fprintf(stderr, "L1AH::CreateSDSs: error creating SDS %d\n", idx);
            return(0);
        }
    }
    return (1);
}

//-----------------//
// L1AH::WriteSDSs //
//-----------------//

int
L1AH::WriteSDSs()
{
    GSL1AStatus* status = &(frame.status);
    GSL1AEu* in_eu = &(frame.in_eu);

    //------------------------------------------------//
    // set all of the values that will get used later //
    //------------------------------------------------//

    // frame time secs
    frame_time_secs->SetFromDouble(&(frame.time));

    // orbit time
    orbit_time->SetWithUnsignedInt(&(frame.orbitTicks));

    // doppler orbit step
    doppler_orbit_step->SetWithUnsignedChar(&(status->doppler_orbit_step));

    // prf orbit step change
	char posc;
    posc = -1;
    if (status->prf_orbit_step_change < 127)
        posc = (char)status->prf_orbit_step_change;
    prf_orbit_step_change->SetWithChar(&posc);

    // prf cycle time
    prf_cycle_time->SetWithUnsignedChar(&(status->prf_cycle_time));

    // range gate a width
    range_gate_a_width->SetWithUnsignedChar(&(status->range_gate_a_width));

    // range gate b width
    range_gate_b_width->SetWithUnsignedChar(&(status->range_gate_b_width));

    // pulse width
    pulse_width->SetWithUnsignedChar(&(status->pulse_width));

    // true cal pulse pos
    unsigned char tcpp = (unsigned char)in_eu->true_cal_pulse_pos;
    true_cal_pulse_pos->SetWithChar((char *)&tcpp);

    // precision coupler temp eu
    precision_coupler_temp_eu->SetFromFloat(
        &(in_eu->precision_coupler_temp_eu));

    // rcv protect sw temp eu
    rcv_protect_sw_temp_eu->SetFromFloat(&(in_eu->rcv_protect_sw_temp_eu));

    // beam select sw temp eu
    beam_select_sw_temp_eu->SetFromFloat(&(in_eu->beam_select_sw_temp_eu));

    // receiver temp eu
    receiver_temp_eu->SetFromFloat(&(in_eu->receiver_temp_eu));

    // antenna position
    antenna_position->SetWithUnsignedShort(frame.antennaPosition);

    // power dn
    power_dn->SetWithUnsignedInt(frame.science);

    // noise dn
    noise_dn->SetWithUnsignedInt(frame.spotNoise);

    // frame inst status
    frame_inst_status->SetWithUnsignedInt(&(frame.frame_inst_status));

    // pulse qual flag
    pulse_qual_flag->SetWithUnsignedChar(frame.pulse_qual_flag);

    // frame err status
    frame_err_status->SetWithUnsignedInt(&(frame.frame_err_status));

    // frame qual flag
    frame_qual_flag->SetWithUnsignedShort(&(frame.frame_qual_flag));

/*
    instrument_time->SetFromUnsignedInt(&(frame.instrumentTicks));

    // convert km to m
    frame.gcX *= 1000.0;
    frame.gcY *= 1000.0;
    frame.gcZ *= 1000.0;
    x_pos->SetFromFloat(&(frame.gcX));
    y_pos->SetFromFloat(&(frame.gcY));
    z_pos->SetFromFloat(&(frame.gcZ));

    // convert km/s to m/s
    frame.velX *= 1000.0;
    frame.velY *= 1000.0;
    frame.velZ *= 1000.0;
    x_vel->SetFromFloat(&(frame.velX));
    y_vel->SetFromFloat(&(frame.velY));
    z_vel->SetFromFloat(&(frame.velZ));

    // convert radians to degrees
    float r, p, y;
    frame.attitude.GetRPY(&r, &p, &y);
    r *= rtd;
    p *= rtd;
    y *= rtd;
    roll->SetFromFloat(&r);
    pitch->SetFromFloat(&p);
    yaw->SetFromFloat(&y);

    // set the packet header to 1 2 3
    static unsigned short packet_header[3] = { 1, 2, 3 };
    first_packet_header->SetWithUnsignedShort(packet_header);

    // this value was copied from a SeaWinds data file
    static unsigned short table_id = 514;
    telemetry_table_id->SetWithUnsignedShort(&table_id);

    // no errors
    static unsigned char uchar_zero = 0;
    status_error_flags->SetWithUnsignedChar(&uchar_zero);

    // this indicates the CDS contents table
    static unsigned char readout_type = 0x0f;
    table_readout_type->SetWithUnsignedChar(&readout_type);

    // zero offset
    static unsigned short offset = 0x0000;
    table_readout_offset->SetWithUnsignedShort(&offset);

    // zero data
    static unsigned int data = 0x00000000;
    table_readout_data->SetWithUnsignedInt(&data);

    // this indicates wind observation mode
    static unsigned char wom = 0x0e;
    operational_mode->SetWithUnsignedChar(&wom);

    // the number of pulses per frame
    static unsigned char pulses = 100;
    prf_count->SetWithUnsignedChar(&pulses);

    // set the fault detection and protection flags
    static unsigned short flags = 0x0760;
    status_change_flags->SetWithUnsignedShort(&flags);

    // error message and history
    static unsigned short ushort_zero = 0;
    error_message->SetWithUnsignedShort(&ushort_zero);
    static unsigned short history[] = { 1, 2, 3, 4, 5 };
    error_message_history->SetWithUnsignedShort(history);

    // valid and invalid command counters
    static unsigned char count = 6;
    valid_command_count->SetWithUnsignedChar(&count);
    invalid_command_count->SetWithUnsignedChar(&count);

    // cal pulse specified position
    char pos = -1;
    if (frame.calPosition != 255)    // 255 flags no cal pulse in frame
    {
        pos = (char)frame.calPosition;
    }
    specified_cal_pulse_pos->SetWithChar(
        &(status->specified_cal_pulse_pos));

    range_gate_a_delay->SetWithUnsignedChar(
        &(status->range_gate_a_delay));
    range_gate_b_delay->SetWithUnsignedChar(
        &(status->range_gate_b_delay));

    unsigned int ds1 = 0;
    unsigned char* ds1_ptr = (unsigned char*)&ds1;
    *(ds1_ptr + 1) = status->doppler_shift_command_1[0];
    *(ds1_ptr + 2) = status->doppler_shift_command_1[1];
    *(ds1_ptr + 3) = status->doppler_shift_command_1[2];
    doppler_shift_command_1->SetWithUnsignedInt(&ds1);

    unsigned int ds2 = 0;
    unsigned char* ds2_ptr = (unsigned char*)&ds2;
    *(ds2_ptr + 1) = status->doppler_shift_command_2[0];
    *(ds2_ptr + 2) = status->doppler_shift_command_2[1];
    *(ds2_ptr + 3) = status->doppler_shift_command_2[2];
    doppler_shift_command_2->SetWithUnsignedInt(&ds2);

    receiver_gain->SetWithUnsignedChar(&(status->receiver_gain));
    ses_configuration_flags->SetWithUnsignedChar(
        &(status->ses_configuration_flags));
    ses_data_overrun_count->SetWithUnsignedChar(
        &(status->ses_data_overrun_count));
    ses_data_underrun_count->SetWithUnsignedChar(
        &(status->ses_data_underrun_count));
    pred_antenna_pos_count->SetWithUnsignedChar(
        &(status->pred_antenna_pos_count));

    unsigned short ushort;
    memcpy(status->running_error_count, &ushort, 2);
    running_error_count->SetWithUnsignedShort(&ushort);

    ses_reset_position->SetWithChar(&(status->ses_reset_position));

    cmd_history_queue->SetWithUnsignedShort(
        (unsigned short *)(status->cmd_history_queue));
    calc_ant_max_grp_count->SetWithUnsignedChar(
        &(status->calc_ant_max_grp_count));

    unsigned int vtcw_hi4 = 0;
    unsigned short vtcw_lo2 = 0;
    (void)memcpy(&vtcw_hi4, status->vtcw, sizeof(unsigned int));
    (void)memcpy(&vtcw_lo2, status->vtcw+4, sizeof(unsigned short));
    double vtcw_value = (double)(vtcw_hi4)*65536.0 + vtcw_lo2;
    vtcw->SetFromDouble(&vtcw_value);

    unsigned int ci_time = 0;
    unsigned char ci_frac = 0;
    (void)memcpy(&ci_time, status->corres_instr_time, sizeof(unsigned int));
    (void)memcpy(&ci_frac, status->corres_instr_time+4, sizeof(unsigned char));
    double corres_value = (double)ci_time * 256.0 + (double)ci_frac;
    corres_instr_time->SetFromDouble(&corres_value);

    fsw_mission_version_num->SetWithUnsignedChar(
        &(status->fsw_mission_version_num));
    fsw_build_number->SetWithUnsignedChar(&(status->fsw_build_number));
    pbi_flag->SetWithUnsignedChar(&(status->pbi_flag));

    float fill_values[8] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    fill->SetWithFloat(fill_values);

    unsigned char dummy_uchar = 64;
    psu_elec_bus_volt->SetWithUnsignedChar(&dummy_uchar);
    cds_current->SetWithUnsignedChar(&dummy_uchar);
    ses_a_current->SetWithUnsignedChar(&dummy_uchar);
    ses_b_current->SetWithUnsignedChar(&dummy_uchar);
    twta_1_current->SetWithUnsignedChar(&dummy_uchar);
    twta_2_current->SetWithUnsignedChar(&dummy_uchar);
    sas_a_current->SetWithUnsignedChar(&dummy_uchar);
    sas_b_current->SetWithUnsignedChar(&dummy_uchar);
    pcu_sec_volt_p12->SetWithUnsignedChar(&dummy_uchar);
    pcu_sec_volt_n12->SetWithUnsignedChar(&dummy_uchar);
    pcu_sec_volt_p30->SetWithUnsignedChar(&dummy_uchar);
    pcu_sec_volt_vme_p3->SetWithUnsignedChar(&dummy_uchar);
    pcu_elec_bus_volt->SetWithUnsignedChar(&dummy_uchar);
    idp_a_temp->SetWithUnsignedChar(&dummy_uchar);
    idp_b_temp->SetWithUnsignedChar(&dummy_uchar);
    psu_temp->SetWithUnsignedChar(&dummy_uchar);

    unsigned short relay_value;
    memcpy(&relay_value, engdata->relay_status, sizeof(unsigned short));
    relay_status->SetWithUnsignedShort(&relay_value);

    ea_a_motor_current->SetWithUnsignedChar(&dummy_uchar);
    ea_a_sec_volt_p5->SetWithUnsignedChar(&dummy_uchar);
    ea_a_sec_volt_p14->SetWithUnsignedChar(&dummy_uchar);
    ea_a_spin_rate->SetWithUnsignedChar(&(engdata->ea_a_spin_rate));
    ea_a_saa_torque_cmd->SetWithUnsignedChar(&dummy_uchar);
    ea_b_motor_current->SetWithUnsignedChar(&dummy_uchar);
    ea_b_sec_volt_p5->SetWithUnsignedChar(&dummy_uchar);
    ea_b_sec_volt_p14->SetWithUnsignedChar(&dummy_uchar);
    ea_b_spin_rate->SetWithUnsignedChar(&(engdata->ea_b_spin_rate));
    ea_b_saa_torque_cmd->SetWithUnsignedChar(&dummy_uchar);
    drive_motor_temp->SetWithUnsignedChar(&dummy_uchar);
    ea_a_power_supply_temp->SetWithUnsignedChar(&dummy_uchar);
    ea_b_power_supply_temp->SetWithUnsignedChar(&dummy_uchar);
    duplex_bearing_temp->SetWithUnsignedChar(&dummy_uchar);
    simplex_bearing_temp->SetWithUnsignedChar(&dummy_uchar);
    rj_temp->SetWithUnsignedChar(&dummy_uchar);
    a2d_p12v_xcpl->SetWithUnsignedChar(&(engdata->a2d_p12v_xcpl));
    twt1_body_reg_volt->SetWithUnsignedChar(&dummy_uchar);
    twt1_ion_pump_current->SetWithUnsignedChar(&dummy_uchar);
    twt1_body_current->SetWithUnsignedChar(&dummy_uchar);
    twt1_drive_power->SetWithUnsignedChar(&dummy_uchar);
    twt2_body_reg_volt->SetWithUnsignedChar(&dummy_uchar);
    twt2_ion_pump_current->SetWithUnsignedChar(&dummy_uchar);
    twt2_body_current->SetWithUnsignedChar(&dummy_uchar);
    twt2_drive_power->SetWithUnsignedChar(&dummy_uchar);
    transmit_power_a->SetWithUnsignedChar(&(engdata->transmit_power_a));
    transmit_power_b->SetWithUnsignedChar(&(engdata->transmit_power_b));
    power_convert_current->SetWithUnsignedChar(&dummy_uchar);
    twt1_hvps_chassis_temp->SetWithUnsignedChar(&dummy_uchar);
    twt1_base_temp->SetWithUnsignedChar(&dummy_uchar);
    twt2_hvps_chassis_temp->SetWithUnsignedChar(&dummy_uchar);
    twt2_base_temp->SetWithUnsignedChar(&dummy_uchar);
    power_converter_temp->SetWithUnsignedChar(&dummy_uchar);
    gain_atten_temp->SetWithUnsignedChar(&dummy_uchar);
    scp_temp->SetWithUnsignedChar(&dummy_uchar);
    receiver_temp->SetWithUnsignedChar(&(engdata->receiver_temp));
    exciter_a_temp->SetWithUnsignedChar(&dummy_uchar);
    exciter_b_temp->SetWithUnsignedChar(&dummy_uchar);
    eng_status_c1->SetWithUnsignedChar(&(engdata->eng_status_c1));
    eng_status_c2->SetWithUnsignedChar(&(engdata->eng_status_c2));
    eng_status_c3->SetWithUnsignedChar(&(engdata->eng_status_c3));
    ses_data_error_flags->SetWithUnsignedChar(&dummy_uchar);

    cds_memory_dump_addr->SetWithUnsignedInt(&dummy_uchar);
    unsigned int data[4] = { 0, 0, 0, 0 };
    cds_memory_dump_data->SetWithUnsignedInt(data);

    unsigned short dummy_ushort = 0;
    ses_memory_dump_addr->SetWithUnsignedShort(&dummy_ushort);
    ses_memory_dump_data->SetWithUnsignedInt(data);

    pcd_entry->SetWithUnsignedShort(&dummy_ushort);
*/

    //-------------------------------------------//
    // determine some information from the frame //
    //-------------------------------------------//

    EqxCheck();
    if (_currentRecordIdx == 0)
    {
        _rangeBeginningTime = frame.time;
    }
    else
    {
        _rangeEndingTime = frame.time;
    }

    //----------------//
    // and write them //
    //----------------//

    for (int idx = 0; g_sds_table[idx] != NULL; idx++)
    {
        Sds* sds = g_sds_table[idx];
        if (! sds->Write(_sdsOutputFileId, _currentRecordIdx))
        {
            fprintf(stderr, "L1AH::WriteSDSs: error writing SDS %s\n",
                sds->GetName());
            return(0);
        }
    }

    return(1);
}

//--------------------//
// L1AH::EndSDSOutput //
//--------------------//

int
L1AH::EndSDSOutput()
{
    for (int idx = 0; g_sds_table[idx] != NULL; idx++)
    {
        Sds* sds = g_sds_table[idx];
        if (! sds->EndAccess())
        {
            fprintf(stderr, "L1AH::EndSDSOutput: error ending SDS access\n");
            return(0);
        }
    }
    if (SDend(_sdsOutputFileId) != SUCCEED)
    {
        fprintf(stderr, "L1AH::EndSDSOutput: error wind SDend\n");
        return(0);
    }
    return(1);
}

//---------------------//
// L1AH::WriteHDFFrame //
//---------------------//

int
L1AH::WriteHDFFrame()
{
    if (! WriteVdatas())
    {
        fprintf(stderr, "L1AH::WriteHDFFrame: error with WriteVdatas\n");
        return(0);
    }
    if (! WriteSDSs())
    {
        fprintf(stderr, "L1AH::WriteHDFFrame: error with WriteSDSs\n");
        return(0);
    }
    return (1);
}

//----------------------//
// L1AH::WriteHDFHeader //
//----------------------//

int
L1AH::WriteHDFHeader(
    double  period,
    double  inclination,
    double  sma,
    double  eccentricity)
{
    //--------------------------------------------//
    // set up all attributes that need setting up //
    //--------------------------------------------//

    char buffer[1024];

    // production date time
    ETime etime;
    etime.CurrentTime();
    etime.ToCodeB(buffer);
    production_date_time->ReplaceContents(buffer);

    // start and stop orbit numbers
    int start_orbit = (int)(_rangeBeginningTime / period) + 1;
    sprintf(buffer, "%d", start_orbit);
    start_orbit_number->ReplaceContents(buffer);

    int stop_orbit = (int)(_rangeEndingTime / period) + 1;
    sprintf(buffer, "%d", stop_orbit);
    stop_orbit_number->ReplaceContents(buffer);

    // eqx longitude
    sprintf(buffer, "%.4f", _eqxLongitude);
    equator_crossing_longitude->ReplaceContents(buffer);

    // eqx date
    etime.SetTime(_eqxTime + _referenceTime);
    etime.ToCodeB(buffer);
    buffer[8] = '\0';
    equator_crossing_date->ReplaceContents(buffer);

    // eqx time
    equator_crossing_time->ReplaceContents(buffer + 9);

    // rev number
    sprintf(buffer, "%d", stop_orbit);
    rev_number->ReplaceContents(buffer);

    // rev orbit period
    sprintf(buffer, "%.3f", period);
    rev_orbit_period->ReplaceContents(buffer);

    // orbit inclination
    sprintf(buffer, "%.5f", inclination);
    orbit_inclination->ReplaceContents(buffer);

    // orbit semi major axis
    sprintf(buffer, "%d", (int)(1000.0 * sma));    // km to m
    orbit_semi_major_axis->ReplaceContents(buffer);

    // orbit eccentricity
    sprintf(buffer, "%.8f", eccentricity);
    orbit_eccentricity->ReplaceContents(buffer);

    // range beginning date
    etime.SetTime(_rangeBeginningTime + _referenceTime);
    etime.ToCodeB(buffer);
    buffer[8] = '\0';
    range_beginning_date->ReplaceContents(buffer);

    // range beginning time
    range_beginning_time->ReplaceContents(buffer + 9);

    // range ending date
    etime.SetTime(_rangeEndingTime + _referenceTime);
    etime.ToCodeB(buffer);
    buffer[8] = '\0';
    range_ending_date->ReplaceContents(buffer);

    // range ending time
    range_ending_time->ReplaceContents(buffer + 9);

    // maximum pulses per frame
    sprintf(buffer, "%d", frame.spotsPerFrame);
    maximum_pulses_per_frame->ReplaceContents(buffer);

    // l1a expected and actual frames
    sprintf(buffer, "%d", _currentRecordIdx);
    l1a_expected_frames->ReplaceContents(buffer);
    l1a_actual_frames->ReplaceContents(buffer);

    //----------------------//
    // write out attributes //
    //----------------------//

    for (int idx = 0; g_attribute_table[idx] != NULL; idx++)
    {
        if (! (g_attribute_table[idx])->Write(_sdsOutputFileId))
        {
            return(0);
        }
    }

    return (1);
}

//-------------------------//
// L1AH::CloseHdfInputFile //
//-------------------------//

int
L1AH::CloseHdfInputFile()
{
    if (Hclose(_hdfInputFileId) != SUCCEED)
        return (0);
    return(1);
}

//--------------------------//
// L1AH::CloseHdfOutputFile //
//--------------------------//

int
L1AH::CloseHdfOutputFile()
{
    if (Hclose(_hdfOutputFileId) != SUCCEED)
        return (0);
    return(1);
}

//----------------//
// L1AH::EqxCheck //
//----------------//

void
L1AH::EqxCheck()
{
    static unsigned int last_orbit_ticks = 0;
    if (frame.orbitTicks < last_orbit_ticks)
    {
        // orbit timer must have gotten reset
        // eqx time is the time at 0
        _eqxTime = frame.time -
            (double)frame.orbitTicks / ORBIT_TICKS_PER_SECOND;
        _eqxLongitude = frame.gcLongitude * rtd;
    }
    last_orbit_ticks = frame.orbitTicks;
    return;
}

/*
L1AH::L1AH()
{
    return;
}

L1AH::~L1AH()
{
    return;
}
*/
