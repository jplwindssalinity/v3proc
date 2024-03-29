//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.21   07 Oct 1999 13:58:58   sally
// added L2Ahr file type
// 
//    Rev 1.20   02 Jun 1999 16:20:10   sally
// add leap second adjustment
// 
//    Rev 1.19   25 May 1999 14:04:14   sally
// add L2Ax for Bryan Stiles
// 
//    Rev 1.18   10 May 1999 16:50:30   sally
// add "Append" option
// 
//    Rev 1.17   07 May 1999 13:10:38   sally
// add memory check for CDS and SES
// 
//    Rev 1.16   12 Apr 1999 11:10:02   sally
// add new options for statistics extraction
// 
//    Rev 1.15   25 Mar 1999 14:02:40   daffer
// Added UQPX directory args
// .
// 
//    Rev 1.14   07 Dec 1998 15:39:08   sally
// add "-firstOnly" option
// 
//    Rev 1.13   02 Dec 1998 12:53:56   sally
// add option to suppress the printing of XMGR headers
// 
//    Rev 1.12   13 Oct 1998 15:32:42   sally
// added L1B file
// 
//    Rev 1.11   02 Oct 1998 14:22:18   sally
// add UNIT_TYPE arg
// 
//    Rev 1.10   21 Sep 1998 15:06:00   sally
// added Qpa
// 
//    Rev 1.9   28 Aug 1998 16:30:08   sally
// add REQA directory and filename args
// 
//    Rev 1.8   09 Jul 1998 09:28:48   sally
// change format per Lee's memo
// 
//    Rev 1.7   29 Jun 1998 16:51:24   sally
// added embedded commands checking
// 
//    Rev 1.6   26 May 1998 10:54:44   sally
// added QPF
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

#define LEAP_SECOND_TABLE_KEYWORD   "LEAP_SECOND_TABLE"
#define LEAP_SECOND_TABLE_OPTION    "-leapSecTable"
#define LEAP_SECOND_TABLE_ARGUMENT  "leapSecTable"

#define LIMIT_FILE_KEYWORD          "LIMIT_FILE"
#define LIMIT_FILE_OPTION           "-l"
#define LIMIT_FILE_ARGUMENT         "limit_file"

#define LOG_FILE_KEYWORD            "LOG_FILE"
#define LOG_FILE_OPTION             "-log"
#define LOG_FILE_ARGUMENT           "Log_file"

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

#define L1B_FILES_KEYWORD           "L1B_FILES"
#define L1B_FILES_OPTION            "-l1b"
#define L1B_FILES_ARGUMENT          "L1B_file..."
 
#define L2A_FILES_KEYWORD           "L2A_FILES"
#define L2A_FILES_OPTION            "-l2a"
#define L2A_FILES_ARGUMENT          "L2A_file..."
 
#define L2A_LIMIT_FILE_KEYWORD      "L2A_LIMIT_FILE"
#define L2A_LIMIT_FILE_OPTION       "-l2alim"
#define L2A_LIMIT_FILE_ARGUMENT     "L1A_limit_file"

#define L2Ax_FILES_KEYWORD           "L2Ax_FILES"
#define L2Ax_FILES_OPTION            "-l2ax"
#define L2Ax_FILES_ARGUMENT          "L2Ax_file..."
 
#define L2Ax_LIMIT_FILE_KEYWORD      "L2Ax_LIMIT_FILE"
#define L2Ax_LIMIT_FILE_OPTION       "-l2axlim"
#define L2Ax_LIMIT_FILE_ARGUMENT     "L1Ax_limit_file"

#define L2Ahr_FILES_KEYWORD          "L2Ahr_FILES"
#define L2Ahr_FILES_OPTION           "-l2ahr"
#define L2Ahr_FILES_ARGUMENT         "L2Ahr_file..."
 
#define L2Ahr_LIMIT_FILE_KEYWORD     "L2Ahr_LIMIT_FILE"
#define L2Ahr_LIMIT_FILE_OPTION      "-l2ahrlim"
#define L2Ahr_LIMIT_FILE_ARGUMENT    "L1Ahr_limit_file"

#define L2B_FILES_KEYWORD           "L2B_FILES"
#define L2B_FILES_OPTION            "-l2b"
#define L2B_FILES_ARGUMENT          "L2B_file..."
 
#define L2B_LIMIT_FILE_KEYWORD      "L2B_LIMIT_FILE"
#define L2B_LIMIT_FILE_OPTION       "-l2blim"
#define L2B_LIMIT_FILE_ARGUMENT     "L1A_limit_file"

#define CDS_MEMORY_FILE_KEYWORD     "CDS_MEMORY_FILE"
#define CDS_MEMORY_FILE_OPTION      "-cdsmem"
#define CDS_MEMORY_FILE_ARGUMENT    "cds_memory_file"

#define SES_MEMORY_FILE_KEYWORD     "SES_MEMORY_FILE"
#define SES_MEMORY_FILE_OPTION      "-sesmem"
#define SES_MEMORY_FILE_ARGUMENT    "ses_memory_file"

#define QPF_DIRECTORY_KEYWORD       "QPF_DIRECTORY"
#define QPF_DIRECTORY_OPTION        "-qpfdir"
#define QPF_DIRECTORY_ARGUMENT      "QPF_directory"

#define QPF_FILE_KEYWORD            "QPF_FILE"
#define QPF_FILE_OPTION             "-qpf"
#define QPF_FILE_ARGUMENT           "QPF_file"

#define QPA_DIRECTORY_KEYWORD       "QPA_DIRECTORY"
#define QPA_DIRECTORY_OPTION        "-qpadir"
#define QPA_DIRECTORY_ARGUMENT      "QPA_directory"

#define QPA_FILE_KEYWORD            "QPA_FILE"
#define QPA_FILE_OPTION             "-qpa"
#define QPA_FILE_ARGUMENT           "QPA_file"

#define UQPX_DIRECTORY_KEYWORD            "UQPX_DIRECTORY"
#define UQPX_DIRECTORY_OPTION             "-uqpx"
#define UQPX_DIRECTORY_ARGUMENT           "UQPX_Directory"

#define OUTPUT_FILE_KEYWORD         "OUTPUT_FILE"
#define OUTPUT_FILE_OPTION          "-o"
#define OUTPUT_FILE_ARGUMENT        "output_file"

#define POLY_TABLE_KEYWORD          "POLY_TABLE"
#define POLY_TABLE_OPTION           "-polytable"
#define POLY_TABLE_ARGUMENT         "polytable"

#define REQA_FILE_KEYWORD           "REQA_FILE"
#define REQA_FILE_OPTION            "-reqa"
#define REQA_FILE_ARGUMENT          "REQA_file"

#define REQA_DIRECTORY_KEYWORD      "REQA_DIRECTORY"
#define REQA_DIRECTORY_OPTION       "-reqadir"
#define REQA_DIRECTORY_ARGUMENT     "REQA_directory"

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

#define UNIT_TYPE_KEYWORD            "UNIT_TYPE"
#define UNIT_TYPE_OPTION             "-unit"
#define UNIT_TYPE_ARGUMENT           "unit_type"

#define X_PARAMETER_KEYWORD         "X_PARAMETER"
#define X_PARAMETER_OPTION          "-x"
#define X_PARAMETER_ARGUMENT        "x_param"

#define Y_PARAMETERS_KEYWORD        "Y_PARAMETERS"
#define Y_PARAMETERS_OPTION         "-y"
#define Y_PARAMETERS_ARGUMENT       "y_param..."

#define STAT_NUM_FRAMES_KEYWORD     "STAT_NUM_FRAMES"
#define STAT_NUM_FRAMES_OPTION      "-statNumFrames"
#define STAT_NUM_FRAMES_ARGUMENT    "numFrames"

//==============================
// toggle options (no argument) 
//==============================

#define ALERT_MAIL_ENABLE_KEYWORD   "ALERT_MAIL_ENABLE"
#define ALERT_MAIL_ENABLE_OPTION    "-am"
#define CHECK_EMBEDDED_KEYWORD      "CHECK_EMBEDDED"
#define CHECK_EMBEDDED_OPTION       "-ckembed"
#define USE_CURR_REQQ_NUM_KEYWORD   "USE_CURR_REQQ_NUM"
#define USE_CURR_REQQ_NUM_OPTION    "-useCurrReqqNum"
#define NO_GR_HEADER_KEYWORD        "NO_GR_HEADER"
#define NO_GR_HEADER_OPTION         "-h"               // jim H. picked this
#define FIRST_DATA_ONLY_KEYWORD     "FIRST_DATA_ONLY"
#define FIRST_DATA_ONLY_OPTION      "-firstOnly"

#define STATISTICS_KEYWORD          "STATISTICS"
#define STATISTICS_OPTION           "-statistics"
 
#define USE_AVG_STAT_KEYWORD        "AVG_STAT"
#define USE_AVG_STAT_OPTION         "-useAvgStat"

#define APPEND_OUTPUT_KEYWORD        "APPEND_OUTPUT"
#define APPEND_OUTPUT_OPTION         "-appendOutput"

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
#define LEAP_SECOND_TABLE_ARG \
    {LEAP_SECOND_TABLE_KEYWORD, LEAP_SECOND_TABLE_OPTION, LEAP_SECOND_TABLE_ARGUMENT}
#define L1A_FILES_ARG    \
    {L1A_FILES_KEYWORD, L1A_FILES_OPTION, L1A_FILES_ARGUMENT}
#define L1A_LIMIT_FILE_ARG   \
    {L1A_LIMIT_FILE_KEYWORD, L1A_LIMIT_FILE_OPTION, L1A_LIMIT_FILE_ARGUMENT}
#define L1A_DERIVED_FILES_ARG    \
    {L1A_DERIVED_FILES_KEYWORD, L1A_DERIVED_FILES_OPTION, L1A_DERIVED_FILES_ARGUMENT}
#define L1A_DERIVED_LIMIT_FILE_ARG   \
    {L1A_DERIVED_LIMIT_FILE_KEYWORD, L1A_DERIVED_LIMIT_FILE_OPTION, L1A_DERIVED_LIMIT_FILE_ARGUMENT}
#define L1B_FILES_ARG    \
    {L1B_FILES_KEYWORD, L1B_FILES_OPTION, L1B_FILES_ARGUMENT}
#define L2A_FILES_ARG    \
    {L2A_FILES_KEYWORD, L2A_FILES_OPTION, L2A_FILES_ARGUMENT}
#define L2A_LIMIT_FILE_ARG   \
    {L2A_LIMIT_FILE_KEYWORD, L2A_LIMIT_FILE_OPTION, L2A_LIMIT_FILE_ARGUMENT}
#define L2Ax_FILES_ARG    \
    {L2Ax_FILES_KEYWORD, L2Ax_FILES_OPTION, L2Ax_FILES_ARGUMENT}
#define L2Ax_LIMIT_FILE_ARG   \
    {L2Ax_LIMIT_FILE_KEYWORD, L2Ax_LIMIT_FILE_OPTION, L2Ax_LIMIT_FILE_ARGUMENT}
#define L2Ahr_FILES_ARG    \
    {L2Ahr_FILES_KEYWORD, L2Ahr_FILES_OPTION, L2Ahr_FILES_ARGUMENT}
#define L2Ahr_LIMIT_FILE_ARG   \
    {L2Ahr_LIMIT_FILE_KEYWORD, L2Ahr_LIMIT_FILE_OPTION, L2Ahr_LIMIT_FILE_ARGUMENT}
#define L2B_FILES_ARG    \
    {L2B_FILES_KEYWORD, L2B_FILES_OPTION, L2B_FILES_ARGUMENT}
#define L2B_LIMIT_FILE_ARG   \
    {L2B_LIMIT_FILE_KEYWORD, L2B_LIMIT_FILE_OPTION, L2B_LIMIT_FILE_ARGUMENT}
#define LOG_FILE_ARG	\
	{LOG_FILE_KEYWORD, LOG_FILE_OPTION, LOG_FILE_ARGUMENT}
#define CDS_MEMORY_FILE_ARG \
    {CDS_MEMORY_FILE_KEYWORD, CDS_MEMORY_FILE_OPTION, CDS_MEMORY_FILE_ARGUMENT}
#define SES_MEMORY_FILE_ARG \
    {SES_MEMORY_FILE_KEYWORD, SES_MEMORY_FILE_OPTION, SES_MEMORY_FILE_ARGUMENT}
#define ALERT_MAIL_ADDRESS_ARG  \
    {ALERT_MAIL_ADDRESS_KEYWORD, ALERT_MAIL_ADDRESS_OPTION, \
     ALERT_MAIL_ADDRESS_ARGUMENT}
#define QPA_FILE_ARG   \
    {QPA_FILE_KEYWORD, QPA_FILE_OPTION, QPA_FILE_ARGUMENT}
#define QPA_DIRECTORY_ARG   \
    {QPA_DIRECTORY_KEYWORD, QPA_DIRECTORY_OPTION, QPA_DIRECTORY_ARGUMENT}
#define UQPX_DIRECTORY_ARG   \
    {UQPX_DIRECTORY_KEYWORD, UQPX_DIRECTORY_OPTION, UQPX_DIRECTORY_ARGUMENT}
#define QPF_FILE_ARG   \
    {QPF_FILE_KEYWORD, QPF_FILE_OPTION, QPF_FILE_ARGUMENT}
#define QPF_DIRECTORY_ARG   \
    {QPF_DIRECTORY_KEYWORD, QPF_DIRECTORY_OPTION, QPF_DIRECTORY_ARGUMENT}
#define L1AP_FILES_ARG   \
    {L1AP_FILES_KEYWORD, L1AP_FILES_OPTION, L1AP_FILES_ARGUMENT}
#define L1AP_LIMIT_FILE_ARG  \
    {L1AP_LIMIT_FILE_KEYWORD, L1AP_LIMIT_FILE_OPTION, L1AP_LIMIT_FILE_ARGUMENT}
#define OUTPUT_FILE_ARG \
    {OUTPUT_FILE_KEYWORD, OUTPUT_FILE_OPTION, OUTPUT_FILE_ARGUMENT}
#define POLY_TABLE_ARG  \
    {POLY_TABLE_KEYWORD, POLY_TABLE_OPTION, POLY_TABLE_ARGUMENT}
#define REQA_DIRECTORY_ARG  \
    {REQA_DIRECTORY_KEYWORD, REQA_DIRECTORY_OPTION, REQA_DIRECTORY_ARGUMENT}
#define REQA_FILE_ARG   \
    {REQA_FILE_KEYWORD, REQA_FILE_OPTION, REQA_FILE_ARGUMENT}
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
#define UNIT_TYPE_ARG    \
    {UNIT_TYPE_KEYWORD, UNIT_TYPE_OPTION, UNIT_TYPE_ARGUMENT}
#define X_PARAMETER_ARG \
    {X_PARAMETER_KEYWORD, X_PARAMETER_OPTION, X_PARAMETER_ARGUMENT}
#define Y_PARAMETERS_ARG    \
    {Y_PARAMETERS_KEYWORD, Y_PARAMETERS_OPTION, Y_PARAMETERS_ARGUMENT}

#define ALERT_MAIL_ENABLE_ARG   \
    {ALERT_MAIL_ENABLE_KEYWORD, ALERT_MAIL_ENABLE_OPTION, "0"}
#define CHECK_EMBEDDED_ARG   \
    {CHECK_EMBEDDED_KEYWORD, CHECK_EMBEDDED_OPTION, "0"}
#define USE_CURR_REQQ_NUM_ARG   \
    {USE_CURR_REQQ_NUM_KEYWORD, USE_CURR_REQQ_NUM_OPTION, "0"}
#define NO_GR_HEADER_ARG   \
    {NO_GR_HEADER_KEYWORD, NO_GR_HEADER_OPTION, "0"}
#define FIRST_DATA_ONLY_ARG   \
    {FIRST_DATA_ONLY_KEYWORD, FIRST_DATA_ONLY_OPTION, "0"}
#define STATISTICS_ARG   \
    {STATISTICS_KEYWORD, STATISTICS_OPTION, "0"}
#define USE_AVG_STAT_ARG   \
    {USE_AVG_STAT_KEYWORD, USE_AVG_STAT_OPTION, "0"}
#define APPEND_OUTPUT_ARG   \
    {APPEND_OUTPUT_KEYWORD, APPEND_OUTPUT_OPTION, "0"}
#define STAT_NUM_FRAMES_ARG   \
    {STAT_NUM_FRAMES_KEYWORD, STAT_NUM_FRAMES_OPTION, STAT_NUM_FRAMES_ARGUMENT}

#endif
