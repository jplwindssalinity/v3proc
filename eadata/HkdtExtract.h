//=========================================================//
// Copyright  (C)1997, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef HKDTEXTRACT_H
#define HKDTEXTRACT_H

static const char rcs_id_hkdt_extract_h[] =
    "@(#) $Id$";

#include "CommonDefs.h"
#include "Itime.h"

#define DIU_THRESH      108     // DIU 15 volt threshold for instrument on


//----------------------------//
// HKDT FILE HEADER Functions //
//----------------------------//

char HKFH_File_Name(const char* frame, char* data);
char HKFH_SC_Name(const char* frame, char* data);
char HKFH_Deliv_Name(const char* frame, char* data);
char HKFH_Sensor_Name(const char* frame, char* data);
char HKFH_Create_Date(const char* frame, char* data);
char HKFH_Create_Time(const char* frame, char* data);
char HKFH_Start_Date(const char* frame, char* data);
char HKFH_Start_Date_c(const char* frame, char* data);
char HKFH_End_Date(const char* frame, char* data);
char HKFH_End_Date_c(const char* frame, char* data);
char HKFH_Data_Type(const char* frame, char* data);
char HKFH_Record_Amt(const char* frame, char* data);
char HKFH_Record_Amt_n(const char* frame, char* data);
char HKFH_Record_Len(const char* frame, char* data);
char HKFH_Record_Len_n(const char* frame, char* data);

//----------------------------------//
// HKDT DATA BLOCK HEADER Functions //
//----------------------------------//

char HKDBH_Start_Time(const char* frame, char* data);
char HKDBH_End_Time(const char* frame, char* data);
char HKDBH_Data_Type(const char* frame, char* data);
char HKDBH_Record_Amt(const char* frame, char* data);
char HKDBH_Record_Len(const char* frame, char* data);
char HKDBH_Format_ID(const char* frame, char* data);
char HKDBH_Extract_Map(const char* frame, char* data);
char HKDBH_Dwell_Flag(const char* frame, char* data);
char HKDBH_ND_Word(const char* frame, char* data);
char HKDBH_SD_Frame(const char* frame, char* data);

//------------------------//
// HKDT Special Functions //
//------------------------//

Itime HK_Itime(const char* frame);
int HK_HVPS_On(const char* frame);
int HK_Instrument_On(const char* frame);

//----------------------------//
// HKDT DATA RECORD Functions //
//----------------------------//

char HK_Time(const char* frame, char* data);
char HK_Time_Dif(const char* frame, char* data);
char HK_Frame_Id(const char* frame, char* data);
char HK_Dwell_Flag(const char* frame, char* data);
char HK_Format_Flag(const char* frame, char* data);
char HK_RIU_A_Mode_1(const char* frame, char* data);
char HK_RIU_A_Mode_2(const char* frame, char* data);
char HK_RIU_B_Mode_1(const char* frame, char* data);
char HK_RIU_B_Mode_2(const char* frame, char* data);
char HK_Ant_2_Deploy(const char* frame, char* data);
char HK_Ant_5_Deploy(const char* frame, char* data);
char HK_Ant_14_Deploy(const char* frame, char* data);
char HK_Ant_36_Deploy(const char* frame, char* data);
char HK_K12(const char* frame, char* data);
char HK_K11(const char* frame, char* data);
char HK_WTS_1(const char* frame, char* data);
char HK_WTS_2(const char* frame, char* data);
char HK_K10(const char* frame, char* data);
char HK_K09(const char* frame, char* data);
char HK_K04(const char* frame, char* data);
char HK_K05(const char* frame, char* data);
char HK_K06(const char* frame, char* data);
char HK_K14(const char* frame, char* data);
char HK_K13(const char* frame, char* data);
char HK_K08(const char* frame, char* data);
char HK_K07(const char* frame, char* data);
char HK_RIU_Ref_Voltage(const char* frame, char* data);
char HK_RIU_Ref_Voltage_v(const char* frame, char* data);
char HK_RIU_5V(const char* frame, char* data);
char HK_RIU_5V_v(const char* frame, char* data);
char HK_DSS_A_5V(const char* frame, char* data);
char HK_DSS_A_5V_v(const char* frame, char* data);
char HK_DSS_A_15V(const char* frame, char* data);
char HK_DSS_A_15V_v(const char* frame, char* data);
char HK_DSS_B_5V(const char* frame, char* data);
char HK_DSS_B_5V_v(const char* frame, char* data);
char HK_DSS_B_15V(const char* frame, char* data);
char HK_DSS_B_15V_v(const char* frame, char* data);
char HK_Main_Bus_Current(const char* frame, char* data);
char HK_Main_Bus_Current_a(const char* frame, char* data);
char HK_Main_Bus_Voltage(const char* frame, char* data);
char HK_Main_Bus_Voltage_v(const char* frame, char* data);
char HK_Main_Bus_Power(const char* frame, char* data);
char HK_DIU_15V(const char* frame, char* data);
char HK_DIU_15V_v(const char* frame, char* data);
char HK_Instrument_On(const char* frame, char* data);
char HK_TWT_Drive_Pwr(const char* frame, char* data);
char HK_TWT_Drive_Pwr_d(const char* frame, char* data);
char HK_TWT_Drive_Pwr_m(const char* frame, char* data);
char HK_Transmit_Pwr(const char* frame, char* data);
char HK_Transmit_Pwr_d(const char* frame, char* data);
char HK_Transmit_Pwr_w(const char* frame, char* data);
char HK_TWT_1_Body_Reg_Voltage(const char* frame, char* data);
char HK_TWT_1_Body_Reg_Voltage_v(const char* frame, char* data);
char HK_TWT_1_Ion_Pump_Current(const char* frame, char* data);
char HK_TWT_1_Ion_Pump_Current_u(const char* frame, char* data);
char HK_TWT_1_Cathode_Current(const char* frame, char* data);
char HK_TWT_1_Cathode_Current_m(const char* frame, char* data);
char HK_TWT_1_Body_Current(const char* frame, char* data);
char HK_TWT_1_Body_Current_m(const char* frame, char* data);
char HK_TWT_2_Body_Reg_Voltage(const char* frame, char* data);
char HK_TWT_2_Body_Reg_Voltage_v(const char* frame, char* data);
char HK_TWT_2_Ion_Pump_Current(const char* frame, char* data);
char HK_TWT_2_Ion_Pump_Current_u(const char* frame, char* data);
char HK_TWT_2_Cathode_Current(const char* frame, char* data);
char HK_TWT_2_Cathode_Current_m(const char* frame, char* data);
char HK_TWT_2_Body_Current(const char* frame, char* data);
char HK_TWT_2_Body_Current_m(const char* frame, char* data);
char HK_RIU_A_Temp(const char* frame, char* data);
char HK_RIU_A_Temp_d(const char* frame, char* data);
char HK_RIU_B_Temp(const char* frame, char* data);
char HK_RIU_B_Temp_d(const char* frame, char* data);
char HK_RIU_AB_Const_I(const char* frame, char* data);
char HK_RIU_AB_Const_I_m(const char* frame, char* data);
char HK_RIU_AB_Const_I_mx(const char* frame, char* data);
char HK_HVPS_1_Temp(const char* frame, char* data);
char HK_HVPS_1_Temp_d(const char* frame, char* data);
char HK_TWTA_1_Base_Temp(const char* frame, char* data);
char HK_TWTA_1_Base_Temp_d(const char* frame, char* data);
char HK_HVPS_2_Temp(const char* frame, char* data);
char HK_HVPS_2_Temp_d(const char* frame, char* data);
char HK_TWTA_2_Base_Temp(const char* frame, char* data);
char HK_TWTA_2_Base_Temp_d(const char* frame, char* data);
char HK_IPM_Temp(const char* frame, char* data);
char HK_IPM_Temp_d(const char* frame, char* data);
char HK_TPM_Temp(const char* frame, char* data);
char HK_TPM_Temp_d(const char* frame, char* data);
char HK_ASM_Temp(const char* frame, char* data);
char HK_ASM_Temp_d(const char* frame, char* data);
char HK_REU_Base_Temp(const char* frame, char* data);
char HK_REU_Base_Temp_d(const char* frame, char* data);
char HK_DIU_Temp(const char* frame, char* data);
char HK_DIU_Temp_d(const char* frame, char* data);
char HK_DSS_Base_A_Temp(const char* frame, char* data);
char HK_DSS_Base_A_Temp_d(const char* frame, char* data);
char HK_DSS_Base_B_Temp(const char* frame, char* data);
char HK_DSS_Base_B_Temp_d(const char* frame, char* data);
char HK_DSS_Base_C_Temp(const char* frame, char* data);
char HK_DSS_Base_C_Temp_d(const char* frame, char* data);
char HK_Ant_2_Dep_Temp(const char* frame, char* data);
char HK_Ant_2_Dep_Temp_d(const char* frame, char* data);
char HK_Ant_5_Dep_Temp(const char* frame, char* data);
char HK_Ant_5_Dep_Temp_d(const char* frame, char* data);
char HK_Ant_14_Dep_Temp(const char* frame, char* data);
char HK_Ant_14_Dep_Temp_d(const char* frame, char* data);
char HK_Ant_36_Dep_Temp(const char* frame, char* data);
char HK_Ant_36_Dep_Temp_d(const char* frame, char* data);
char HK_HVPS_Backup_Off(const char* frame, char* data);
char HK_TWTA_UV_Trip(const char* frame, char* data);
char HK_TWTA_OC_Trip(const char* frame, char* data);
char HK_TWTA_Body_OC_Trip(const char* frame, char* data);
char HK_Bin_Param_Err(const char* frame, char* data);
char HK_Def_Bin_Const(const char* frame, char* data);
char HK_Lack_Start_Reqs(const char* frame, char* data);
char HK_Err_Queue_Full(const char* frame, char* data);
char HK_Fault_Counter(const char* frame, char* data);
char HK_NSCAT_Mode(const char* frame, char* data);
char HK_Ext_Mode(const char* frame, char* data);
char HK_Cmd_Counter(const char* frame, char* data);
char HK_Rx_Pro(const char* frame, char* data);
char HK_ULM_Lock(const char* frame, char* data);
char HK_SLM_Lock(const char* frame, char* data);
char HK_DSS(const char* frame, char* data);
char HK_TWTA(const char* frame, char* data);
char HK_HVPS(const char* frame, char* data);
char HK_TWTA_Trip_Override(const char* frame, char* data);
char HK_TWT_Mon_En(const char* frame, char* data);
char HK_HVPS_Shut_En(const char* frame, char* data);
/*
char HK_ESA_Radiance_1(const char* frame, char* data);
char HK_ESA_Radiance_2(const char* frame, char* data);
char HK_ESA_Radiance_3(const char* frame, char* data);
char HK_ESA_Radiance_4(const char* frame, char* data);
*/
char HK_FSSA_Rdata(const char* frame, char* data);
char HK_FSSA_Rdata_p(const char* frame, char* data);
char HK_IRU_Rate_Roll(const char* frame, char* data);
char HK_IRU_Rate_Roll_ds(const char* frame, char* data);
char HK_IRU_Rate_Pitch(const char* frame, char* data);
char HK_IRU_Rate_Pitch_ds(const char* frame, char* data);
char HK_IRU_Rate_Yaw(const char* frame, char* data);
char HK_IRU_Rate_Yaw_ds(const char* frame, char* data);
char HK_ESA_Edge_Angle_1(const char* frame, char* data);
char HK_ESA_Edge_Angle_2(const char* frame, char* data);
char HK_ESA_Edge_Angle_3(const char* frame, char* data);
char HK_ESA_Edge_Angle_4(const char* frame, char* data);
char HK_Sun_Angle_d(const char* frame, char* data);

float IPM_To_mW_SC(float monitor_voltage, float thermistor_resistance);
float TPM_To_W_SC(float monitor_voltage, float thermistor_resistance);
double line_fit(double x1, double x2, double y1, double y2, double x);

//---------------------------------------//
// HKDT DATA RECORD Functions: Validated //
//---------------------------------------//

char HK_Valid_TWTA(const char* frame, char* data);
char HK_Valid_TWTA_Body_OC_Trip(const char* frame, char* data);
char HK_Valid_TWTA_OC_Trip(const char* frame, char* data);
char HK_Valid_TWTA_UV_Trip(const char* frame, char* data);

#endif //HKDTEXTRACT_H
