//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.5   13 Apr 1998 15:04:22   daffer
// Took out mail_drn args
// 
//    Rev 1.4   13 Apr 1998 15:03:12   sally
// add L2A and L2B stuff
// 
//    Rev 1.3   10 Apr 1998 14:04:06   daffer
//   Added stage_nonrtc stuff.
// 
//    Rev 1.2   27 Mar 1998 09:56:44   sally
// added L1A Derived data
// 
//    Rev 1.1   20 Feb 1998 10:55:32   sally
// L1 to L1A
// 
//    Rev 1.0   04 Feb 1998 14:14:34   daffer
// Initial checking
// Revision 1.5  1998/02/03 00:12:33  sally
// change NRT to L1AP
//
// Revision 1.4  1998/01/30 22:28:04  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef ARGDEFS_H
#define ARGDEFS_H

static const char rcs_argdefs_h[] =
    "@(#) $Header$";

//===========================================
// The config file environment variable name 
//===========================================

#define ENV_CONFIG_FILENAME         "EA_CONFIG_FILE"

//==================
// Argument defines 
//==================

#define ALERT_FILE_KEYWORD          "ALERT_FILE"
#define ALERT_FILE_OPTION           "-a"
#define ALERT_FILE_ARGUMENT         "alert_file"

#define ALERT_MAIL_ADDRESS_KEYWORD  "ALERT_MAIL_ADDRESS"
#define ALERT_MAIL_ADDRESS_OPTION   "-amaddr"
#define ALERT_MAIL_ADDRESS_ARGUMENT "alert_mail_address"

#define AUTO_DAEMON_LOG_FILE_KEYWORD    "AUTO_DAEMON_LOG_FILE"
#define AUTO_DAEMON_LOG_FILE_OPTION     "-autodlog"
#define AUTO_DAEMON_LOG_FILE_ARGUMENT   "auto_daemon_log_file"

#define CMDLP_FILE_KEYWORD          "CMDLP_FILE"
#define CMDLP_FILE_OPTION           "-cmdlp"
#define CMDLP_FILE_ARGUMENT         "CMDLP_file"

#define DECIMATE_NUMBER_KEYWORD     "DECIMATE_NUMBER"
#define DECIMATE_NUMBER_OPTION      "-dnum"
#define DECIMATE_NUMBER_ARGUMENT    "decimate_#"

#define END_TIME_KEYWORD            "END_TIME"
#define END_TIME_OPTION             "-e"
#define END_TIME_ARGUMENT           "end_time"

#define EFFECT_FILE_KEYWORD         "EFFECT_FILE"
#define EFFECT_FILE_OPTION          "-effect"
#define EFFECT_FILE_ARGUMENT        "EFFECT_file"

#define EQX_FILE_KEYWORD            "EQX_FILE"
#define EQX_FILE_OPTION             "-eqx"
#define EQX_FILE_ARGUMENT           "EQX_file"

#define FILTER_KEYWORD              "FILTER"
#define FILTER_OPTION               "-filter"
#define FILTER_ARGUMENT             "filter_exp"

#define HK2_FILES_KEYWORD          "HK2_FILES"
#define HK2_FILES_OPTION           "-hk2"
#define HK2_FILES_ARGUMENT         "HK2_file..."

#define HK2_LIMIT_FILE_KEYWORD     "HK2_LIMIT_FILE"
#define HK2_LIMIT_FILE_OPTION      "-hk2lim"
#define HK2_LIMIT_FILE_ARGUMENT    "HK2_limit_file"

#define INPUT_FILE_KEYWORD          "INPUT_FILE"
#define INPUT_FILE_OPTION           "-i"
#define INPUT_FILE_ARGUMENT         "input_file"

#define LIMIT_FILE_KEYWORD          "LIMIT_FILE"
#define LIMIT_FILE_OPTION           "-l"
#define LIMIT_FILE_ARGUMENT         "limit_file"

#define L1A_FILES_KEYWORD           "L1A_FILES"
#define L1A_FILES_OPTION            "-l1a"
#define L1A_FILES_ARGUMENT          "L1A_file..."

#define L1A_LIMIT_FILE_KEYWORD      "L1A_LIMIT_FILE"
#define L1A_LIMIT_FILE_OPTION       "-l1alim"
#define L1A_LIMIT_FILE_ARGUMENT     "L1A_limit_file"

#define L1AP_FILES_KEYWORD          "L1AP_FILES"
#define L1AP_FILES_OPTION           "-l1ap"
#define L1AP_FILES_ARGUMENT         "L1AP_file..."
 
#define L1AP_LIMIT_FILE_KEYWORD     "L1AP_LIMIT_FILE"
#define L1AP_LIMIT_FILE_OPTION      "-l1aplim"
#define L1AP_LIMIT_FILE_ARGUMENT    "L1AP_limit_file"

#define L1A_DERIVED_FILES_KEYWORD          "L1A_DERIVED_FILES"
#define L1A_DERIVED_FILES_OPTION           "-l1adrv"
#define L1A_DERIVED_FILES_ARGUMENT         "L1A_drv_file..."
 
#define L1A_DERIVED_LIMIT_FILE_KEYWORD     "L1A_DERIVED_LIMIT_FILE"
#define L1A_DERIVED_LIMIT_FILE_OPTION      "-l1adrvlim"
#define L1A_DERIVED_LIMIT_FILE_ARGUMENT    "L1A_drv_limit_file"

#define L2A_FILES_KEYWORD           "L2A_FILES"
#define L2A_FILES_OPTION            "-l2a"
#define L2A_FILES_ARGUMENT          "L2A_file..."
 
#define L2A_LIMIT_FILE_KEYWORD      "L2A_LIMIT_FILE"
#define L2A_LIMIT_FILE_OPTION       "-l2alim"
#define L2A_LIMIT_FILE_ARGUMENT     "L1A_limit_file"

#define L2B_FILES_KEYWORD           "L2B_FILES"
#define L2B_FILES_OPTION            "-l2b"
#define L2B_FILES_ARGUMENT          "L2B_file..."
 
#define L2B_LIMIT_FILE_KEYWORD      "L2B_LIMIT_FILE"
#define L2B_LIMIT_FILE_OPTION       "-l2blim"
#define L2B_LIMIT_FILE_ARGUMENT     "L1A_limit_file"

#define LOG_FILE_KEYWORD            "LOG_FILE"
#define LOG_FILE_OPTION             "-log"
#define LOG_FILE_ARGUMENT           "Log_file"

#define MEMORY_FILE_KEYWORD         "MEMORY_FILE"
#define MEMORY_FILE_OPTION          "-mem"
#define MEMORY_FILE_ARGUMENT        "memory_file"

#define NPF_DIRECTORY_KEYWORD       "NPF_DIRECTORY"
#define NPF_DIRECTORY_OPTION        "-npfdir"
#define NPF_DIRECTORY_ARGUMENT      "NPF_directory"

#define OUTPUT_FILE_KEYWORD         "OUTPUT_FILE"
#define OUTPUT_FILE_OPTION          "-o"
#define OUTPUT_FILE_ARGUMENT        "output_file"

#define POLY_TABLE_KEYWORD          "POLY_TABLE"
#define POLY_TABLE_OPTION           "-polytable"
#define POLY_TABLE_ARGUMENT         "polytable"

#define REQI_DIRECTORY_KEYWORD      "REQI_DIRECTORY"
#define REQI_DIRECTORY_OPTION       "-reqidir"
#define REQI_DIRECTORY_ARGUMENT     "REQI_directory"

#define REQI_FILE_KEYWORD           "REQI_FILE"
#define REQI_FILE_OPTION            "-reqi"
#define REQI_FILE_ARGUMENT          "REQI_file"

#define REQQ_DIRECTORY_KEYWORD      "REQQ_DIRECTORY"
#define REQQ_DIRECTORY_OPTION       "-reqqdir"
#define REQQ_DIRECTORY_ARGUMENT     "REQQ_directory"

#define RTCF_DIRECTORY_KEYWORD      "RTCF_DIRECTORY"
#define RTCF_DIRECTORY_OPTION       "-rtcfdir"
#define RTCF_DIRECTORY_ARGUMENT     "RTCF_directory"

#define SEQ_NUMBER_FILE_KEYWORD     "SEQ_NUMBER_FILE"
#define SEQ_NUMBER_FILE_OPTION      "-seq"
#define SEQ_NUMBER_FILE_ARGUMENT    "seq_#_file"

#define STAGE_NONRTC_CONFIG_FILE_KEYWORD    "STAGE_NONRTC_CONFIG_FILE"
#define STAGE_NONRTC_CONFIG_FILE_OPTION     "-snc"
#define STAGE_NONRTC_CONFIG_FILE_ARGUMENT   "stage_NONRTC_config_file"

#define STAGE_NONRTC_LOG_FILE_KEYWORD   "STAGE_NONRTC_LOG_FILE"
#define STAGE_NONRTC_LOG_FILE_OPTION    "-snlog"
#define STAGE_NONRTC_LOG_FILE_ARGUMENT  "stage_nonrtc_log_file"

#define START_TIME_KEYWORD          "START_TIME"
#define START_TIME_OPTION           "-s"
#define START_TIME_ARGUMENT         "start_time"

#define TLM_FILES_KEYWORD           "TLM_FILES"
#define TLM_FILES_OPTION            "-tlm"
#define TLM_FILES_ARGUMENT          "tlm_file..."

#define TLM_TYPE_KEYWORD            "TLM_TYPE"
#define TLM_TYPE_OPTION             "-t"
#define TLM_TYPE_ARGUMENT           "tlm_type"

#define TODO_CMD_DIRECTORY_KEYWORD  "TODO_CMD_DIRECTORY"
#define TODO_CMD_DIRECTORY_OPTION   "-cmddir"
#define TODO_CMD_DIRECTORY_ARGUMENT "todo_cmd_directory"

#define TODO_FILE_KEYWORD           "TODO_FILE"
#define TODO_FILE_OPTION            "-todo"
#define TODO_FILE_ARGUMENT          "todo_file"

#define X_PARAMETER_KEYWORD         "X_PARAMETER"
#define X_PARAMETER_OPTION          "-x"
#define X_PARAMETER_ARGUMENT        "x_param"

#define Y_PARAMETERS_KEYWORD        "Y_PARAMETERS"
#define Y_PARAMETERS_OPTION         "-y"
#define Y_PARAMETERS_ARGUMENT       "y_param..."

//==============================
// toggle options (no argument) 
//==============================

#define ALERT_MAIL_ENABLE_KEYWORD   "ALERT_MAIL_ENABLE"
#define ALERT_MAIL_ENABLE_OPTION    "-am"

//============
// Structures 
//============

#define ALERT_FILE_ARG  \
    {ALERT_FILE_KEYWORD, ALERT_FILE_OPTION, ALERT_FILE_ARGUMENT}
#define AUTO_DAEMON_LOG_FILE_ARG    \
    {AUTO_DAEMON_LOG_FILE_KEYWORD, AUTO_DAEMON_LOG_FILE_OPTION,\
     AUTO_DAEMON_LOG_FILE_ARGUMENT}
#define CMDLP_FILE_ARG  \
    {CMDLP_FILE_KEYWORD, CMDLP_FILE_OPTION, CMDLP_FILE_ARGUMENT}
#define DECIMATE_NUMBER_ARG \
    {DECIMATE_NUMBER_KEYWORD, DECIMATE_NUMBER_OPTION, DECIMATE_NUMBER_ARGUMENT}
#define END_TIME_ARG    \
    {END_TIME_KEYWORD, END_TIME_OPTION, END_TIME_ARGUMENT}
#define EFFECT_FILE_ARG \
    {EFFECT_FILE_KEYWORD, EFFECT_FILE_OPTION, EFFECT_FILE_ARGUMENT}
#define EQX_FILE_ARG    \
    {EQX_FILE_KEYWORD, EQX_FILE_OPTION, EQX_FILE_ARGUMENT}
#define FILTER_ARG  \
    {FILTER_KEYWORD, FILTER_OPTION, FILTER_ARGUMENT}
#define HK2_FILES_ARG  \
    {HK2_FILES_KEYWORD, HK2_FILES_OPTION, HK2_FILES_ARGUMENT}
#define HK2_LIMIT_FILE_ARG \
    {HK2_LIMIT_FILE_KEYWORD, HK2_LIMIT_FILE_OPTION, HK2_LIMIT_FILE_ARGUMENT}
#define INPUT_FILE_ARG  \
    {INPUT_FILE_KEYWORD, INPUT_FILE_OPTION, INPUT_FILE_ARGUMENT}
#define LIMIT_FILE_ARG  \
    {LIMIT_FILE_KEYWORD, LIMIT_FILE_OPTION, LIMIT_FILE_ARGUMENT}
#define L1A_FILES_ARG    \
    {L1A_FILES_KEYWORD, L1A_FILES_OPTION, L1A_FILES_ARGUMENT}
#define L1A_LIMIT_FILE_ARG   \
    {L1A_LIMIT_FILE_KEYWORD, L1A_LIMIT_FILE_OPTION, L1A_LIMIT_FILE_ARGUMENT}
#define L1A_DERIVED_FILES_ARG    \
    {L1A_DERIVED_FILES_KEYWORD, L1A_DERIVED_FILES_OPTION, L1A_DERIVED_FILES_ARGUMENT}
#define L1A_DERIVED_LIMIT_FILE_ARG   \
    {L1A_DERIVED_LIMIT_FILE_KEYWORD, L1A_DERIVED_LIMIT_FILE_OPTION, L1A_DERIVED_LIMIT_FILE_ARGUMENT}
#define L2A_FILES_ARG    \
    {L2A_FILES_KEYWORD, L2A_FILES_OPTION, L2A_FILES_ARGUMENT}
#define L2A_LIMIT_FILE_ARG   \
    {L2A_LIMIT_FILE_KEYWORD, L2A_LIMIT_FILE_OPTION, L2A_LIMIT_FILE_ARGUMENT}
#define L2B_FILES_ARG    \
    {L2B_FILES_KEYWORD, L2B_FILES_OPTION, L2B_FILES_ARGUMENT}
#define L2B_LIMIT_FILE_ARG   \
    {L2B_LIMIT_FILE_KEYWORD, L2B_LIMIT_FILE_OPTION, L2B_LIMIT_FILE_ARGUMENT}
#define LOG_FILE_ARG	\
	{LOG_FILE_KEYWORD, LOG_FILE_OPTION, LOG_FILE_ARGUMENT}
#define MEMORY_FILE_ARG \
    {MEMORY_FILE_KEYWORD, MEMORY_FILE_OPTION, MEMORY_FILE_ARGUMENT}
#define ALERT_MAIL_ADDRESS_ARG  \
    {ALERT_MAIL_ADDRESS_KEYWORD, ALERT_MAIL_ADDRESS_OPTION, \
     ALERT_MAIL_ADDRESS_ARGUMENT}
#define NPF_DIRECTORY_ARG   \
    {NPF_DIRECTORY_KEYWORD, NPF_DIRECTORY_OPTION, NPF_DIRECTORY_ARGUMENT}
#define L1AP_FILES_ARG   \
    {L1AP_FILES_KEYWORD, L1AP_FILES_OPTION, L1AP_FILES_ARGUMENT}
#define L1AP_LIMIT_FILE_ARG  \
    {L1AP_LIMIT_FILE_KEYWORD, L1AP_LIMIT_FILE_OPTION, L1AP_LIMIT_FILE_ARGUMENT}
#define OUTPUT_FILE_ARG \
    {OUTPUT_FILE_KEYWORD, OUTPUT_FILE_OPTION, OUTPUT_FILE_ARGUMENT}
#define POLY_TABLE_ARG  \
    {POLY_TABLE_KEYWORD, POLY_TABLE_OPTION, POLY_TABLE_ARGUMENT}
#define REQI_DIRECTORY_ARG  \
    {REQI_DIRECTORY_KEYWORD, REQI_DIRECTORY_OPTION, REQI_DIRECTORY_ARGUMENT}
#define REQI_FILE_ARG   \
    {REQI_FILE_KEYWORD, REQI_FILE_OPTION, REQI_FILE_ARGUMENT}
#define REQQ_DIRECTORY_ARG  \
    {REQQ_DIRECTORY_KEYWORD, REQQ_DIRECTORY_OPTION, REQQ_DIRECTORY_ARGUMENT}
#define RTCF_DIRECTORY_ARG  \
    {RTCF_DIRECTORY_KEYWORD, RTCF_DIRECTORY_OPTION, RTCF_DIRECTORY_ARGUMENT}
#define SEQ_NUMBER_FILE_ARG \
    {SEQ_NUMBER_FILE_KEYWORD, SEQ_NUMBER_FILE_OPTION, SEQ_NUMBER_FILE_ARGUMENT}
#define STAGE_NONRTC_CONFIG_FILE_ARG    \
    {STAGE_NONRTC_CONFIG_FILE_KEYWORD, STAGE_NONRTC_CONFIG_FILE_OPTION, \
    STAGE_NONRTC_CONFIG_FILE_ARGUMENT}
#define STAGE_NONRTC_LOG_FILE_ARG   \
    {STAGE_NONRTC_LOG_FILE_KEYWORD, STAGE_NONRTC_LOG_FILE_OPTION, \
    STAGE_NONRTC_LOG_FILE_ARGUMENT}
#define START_TIME_ARG  \
    {START_TIME_KEYWORD, START_TIME_OPTION, START_TIME_ARGUMENT}
#define TLM_FILES_ARG   \
    {TLM_FILES_KEYWORD, TLM_FILES_OPTION, TLM_FILES_ARGUMENT}
#define TLM_TYPE_ARG    \
    {TLM_TYPE_KEYWORD, TLM_TYPE_OPTION, TLM_TYPE_ARGUMENT}
#define TODO_CMD_DIRECTORY_ARG  \
    {TODO_CMD_DIRECTORY_KEYWORD, TODO_CMD_DIRECTORY_OPTION, \
    TODO_CMD_DIRECTORY_ARGUMENT}
#define TODO_FILE_ARG   \
    {TODO_FILE_KEYWORD, TODO_FILE_OPTION, TODO_FILE_ARGUMENT}
#define X_PARAMETER_ARG \
    {X_PARAMETER_KEYWORD, X_PARAMETER_OPTION, X_PARAMETER_ARGUMENT}
#define Y_PARAMETERS_ARG    \
    {Y_PARAMETERS_KEYWORD, Y_PARAMETERS_OPTION, Y_PARAMETERS_ARGUMENT}

#define ALERT_MAIL_ENABLE_ARG   \
    {ALERT_MAIL_ENABLE_KEYWORD, ALERT_MAIL_ENABLE_OPTION, 0}

#endif
