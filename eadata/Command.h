//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.26   29 Jan 1999 15:04:14   sally
// added LASP proc commands
// 
//    Rev 1.25   08 Jan 1999 16:37:26   sally
// add LASP proc commands
// 
//    Rev 1.24   20 Oct 1998 15:47:18   sally
// change some relay commands to be realtime only
// 
//    Rev 1.23   20 Oct 1998 10:52:40   sally
// add static QPF commands (table macro commands)
// 
//    Rev 1.22   19 Oct 1998 10:35:24   sally
// added command IDs for new table macro commands
// 
//    Rev 1.21   08 Oct 1998 16:18:46   sally
// add REQQ like macro commands
// 
//    Rev 1.20   01 Sep 1998 11:14:12   daffer
// Added ReadShort and made other changes to accomodate new PBI commanding 
// 
//    Rev 1.19   01 Sep 1998 10:03:50   sally
//  separate CmdHex from CmdId
// 
//    Rev 1.18   31 Aug 1998 14:41:38   sally
// added REQQ-like PBI commands
// 
//    Rev 1.17   21 Aug 1998 11:20:06   daffer
// Added EffectId method
// 
//    Rev 1.16   19 Aug 1998 14:28:16   sally
// add 3 commands - SCANXA1, SCANXA2 and SCORBSYNC
// 
//    Rev 1.15   19 Aug 1998 10:28:38   daffer
// Added new effects to EffectE enumeration. Added cmdParams member
// and SetCmdParams method
// 
//    Rev 1.14   13 Jul 1998 14:21:50   sally
// add the datafile format for SCGATEWID
// 
//    Rev 1.13   09 Jul 1998 16:22:54   sally
// catch the output of process_reqi in an output file
// 
//    Rev 1.12   01 Jul 1998 16:51:36   sally
// added table upload repetition
// 
//    Rev 1.11   29 Jun 1998 16:51:54   sally
// 
// added embedded commands checking
// 
//    Rev 1.10   29 May 1998 15:26:46   sally
// adapted to the new REQI datafile format, per Lee's memo
// 
//    Rev 1.9   28 May 1998 11:05:38   sally
// re-order the commands and fixed some typoes
// 
//    Rev 1.8   28 May 1998 09:26:24   sally
// update the formats for REQQ, QPF and RTCF
// 
//    Rev 1.7   22 May 1998 16:35:02   daffer
// Added/modified code to do cmdlp/effect processing.
// Updated commanding and effect mnemonics/enums.
// 
//    Rev 1.6   27 Mar 1998 09:57:28   sally
// added L1A Derived data
// 
//    Rev 1.5   17 Mar 1998 14:41:08   sally
// changed for REQQ
// 
//    Rev 1.4   16 Mar 1998 10:52:22   sally
// ReadReqi() are split into two methods
// 
//    Rev 1.3   12 Mar 1998 17:15:50   sally
// adapt to the new QSCAT REQI format
// 
//    Rev 1.2   09 Mar 1998 16:34:16   sally
// adapt to the new REQI format
// 
//    Rev 1.1   20 Feb 1998 10:56:02   sally
// L1 to L1A
// 
//    Rev 1.0   04 Feb 1998 14:15:00   daffer
// Initial checking
// Revision 1.3  1998/02/03 00:12:33  sally
// change NRT to L1AP
//
// Revision 1.2  1998/01/30 22:28:11  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef COMMAND_H
#define COMMAND_H

static const char rcs_id_command_h[] =
    "@(#) $Header$";

#include <strings.h>

#include "CommonDefs.h"
#include "Tpg.h"

//-----------
// CONSTANTS 
//-----------

#define CMD_START_MARKER    "----START----"
#define COMMAND_ID_LABEL    "Cmd_ID:"
#define PLANNED_TPG_LABEL   "Planned_TPG:"
#define DATA_FILENAME_LABEL "Data_Filename:"
#define CMD_PARAMS_LABEL    "Cmd_Params:"
#define ORIGINATOR_LABEL    "Originator:"
#define COMMENTS_LABEL      "Comments:"
#define EFFECT_ID_LABEL     "Effect_ID:"
#define STATUS_ID_LABEL     "Status_ID:"
#define EXPECTED_TIME_LABEL "Expected_Time:"
#define L1A_TIME_LABEL      "L1A_Time:"
#define HK2_TIME_LABEL      "HK2_Time:"
#define L1AP_TIME_LABEL     "L1AP_Time:"
#define L1A_VERIFY_LABEL    "L1A_Verify:"
#define HK2_VERIFY_LABEL    "HK2_Verify:"
#define L1AP_VERIFY_LABEL    "L1AP_Verify:"
#define EFFECT_VALUE_LABEL   "Effect_Value:"
#define CMD_END_MARKER      "-----END-----"

#define INVALID_INT         -1
#define MAX_STRING_SIZE     1024
#define MAX_LINE_SIZE       1024

#define EA_IS_LASP_PROC(x)  (((x) & 0x7100) == 0x7100 ? 1 : 0)

//--------
// MACROS 
//--------

// return "" if the string pointer is NULL
#define CONVERT_TO_STRING(A)  A?A:""

//-------
// ENUMS 
//-------

enum EACommandE
{
    EA_CMD_NONE      = -1,    
    EA_CMD_UNKNOWN   = 0 ,     

    //-----------------------------
    // Pulse Discrete Commands
    //-----------------------------
    EA_CMD_SCK1RST,
    EA_CMD_SCK2RST,
    EA_CMD_SCK3RST,
    EA_CMD_SCK1SET,
    EA_CMD_SCK2SET,
    EA_CMD_SCK3SET,
    EA_CMD_SCK4RST,
    EA_CMD_SCK5RST,
    EA_CMD_SCK6RST,
    EA_CMD_SCK4SET,
    EA_CMD_SCK5SET,
    EA_CMD_SCK6SET,
    EA_CMD_SCK7RST,
    EA_CMD_SCK7SET,
    EA_CMD_SCK8RST,
    EA_CMD_SCK8SET,
    EA_CMD_SCK13RST,
    EA_CMD_SCK13SET,
    EA_CMD_SCK14RST,
    EA_CMD_SCK14SET,
    EA_CMD_SCK17RST,
    EA_CMD_SCK17SET,
    EA_CMD_SCK18RST,
    EA_CMD_SCK18SET,

    //-----------------------------
    // Serial Magnitude Commands
    //-----------------------------
    EA_CMD_SCTLDFPEN,
    EA_CMD_SCTLDFPDS,
    EA_CMD_SCRGAWID,
    EA_CMD_SCRGBWID,
    EA_CMD_SCTRPWID,
    EA_CMD_SCPRFCLK,
    EA_CMD_SCRCVGAN,
    EA_CMD_SCGRDNOR,
    EA_CMD_SCGRDDS,
    EA_CMD_SCRCVPON,
    EA_CMD_SCRCVPNR,
    EA_CMD_SCTOVRON,
    EA_CMD_SCTOVROFF,
    EA_CMD_SCTMDONN,
    EA_CMD_SCTMDOFF,
    EA_CMD_SCTWTSEL,
    EA_CMD_SCSESRST,
    EA_CMD_SCSMLDS,
    EA_CMD_SCHTRDS,
    EA_CMD_SCTWTMDS,
    EA_CMD_SCCALPER,
    EA_CMD_SCRSRLPR,
    EA_CMD_SCSWPEN,
    EA_CMD_SCSWPDS,
    EA_CMD_SCGATEWID,
    EA_CMD_SCORBTIC,
    EA_CMD_SCITMSYN,
    EA_CMD_SCSSTSW,
    EA_CMD_SCSCLSW,
    EA_CMD_SCSWOSW,
    EA_CMD_SCSALLSW,
    EA_CMD_SCSRCSW,
    EA_CMD_SCMROSTR,
    EA_CMD_SCTBLSTR,
    EA_CMD_SCMRUPLM,
    EA_CMD_SCMRLOLM,
    EA_CMD_SCBMOFST,
    EA_CMD_SCSASOFS,
    EA_CMD_SCMSLEN,
    EA_CMD_SCMSLDS,
    EA_CMD_SCBRASW,
    EA_CMD_SCBRBSW,
    EA_CMD_SCBDASW,
    EA_CMD_SCBDBSW,
    EA_CMD_SCENGSW,
    EA_CMD_SCSTASW,
    EA_CMD_SCPSTSW,
    EA_CMD_SCPCLSW,
    EA_CMD_SCPRCSW,
    EA_CMD_SCPWOSW,
    EA_CMD_SCNOOP,
    EA_CMD_SCSMLEN,
    EA_CMD_SCHTREN,
    EA_CMD_SCTWTMEN,
    EA_CMD_SCSSTTBL,
    EA_CMD_SCSCLTBL,
    EA_CMD_SCSRCTBL,
    EA_CMD_SCSWOTBL,
    EA_CMD_SCSALLTBL,
    EA_CMD_SCBRATBL,
    EA_CMD_SCBRBTBL,
    EA_CMD_SCBDATBL,
    EA_CMD_SCBDBTBL,
    EA_CMD_SCENGTBL,
    EA_CMD_SCSTATBL,
    EA_CMD_SCPSTTBL,
    EA_CMD_SCPCLTBL,
    EA_CMD_SCPRCTBL,
    EA_CMD_SCPWOTBL,
    EA_CMD_SCK9RST,
    EA_CMD_SCK10RST,
    EA_CMD_SCK11RST,
    EA_CMD_SCK19RST,
    EA_CMD_SCK12RST,
    EA_CMD_SCK25RST,
    EA_CMD_SCSSBRST,
    EA_CMD_SCK20RST,
    EA_CMD_SCK15RST,
    EA_CMD_SCK26RST,
    EA_CMD_SCK21RST,
    EA_CMD_SCK23RST,
    EA_CMD_SCK16RST,
    EA_CMD_SCSSARST,
    EA_CMD_SCK22RST,
    EA_CMD_SCK24RST,
    EA_CMD_SCK9SET,
    EA_CMD_SCK10SET,
    EA_CMD_SCK11SET,
    EA_CMD_SCK19SET,
    EA_CMD_SCK12SET,
    EA_CMD_SCK25SET,
    EA_CMD_SCSSBSET,
    EA_CMD_SCK20SET,
    EA_CMD_SCK15SET,
    EA_CMD_SCK26SET,
    EA_CMD_SCK21SET,
    EA_CMD_SCK23SET,
    EA_CMD_SCK16SET,
    EA_CMD_SCSSASET,
    EA_CMD_SCK22SET,
    EA_CMD_SCK24SET,
    EA_CMD_SCSWPATCH,
    EA_CMD_SCMODCAL,
    EA_CMD_SCMODWOM,
    EA_CMD_SCMODSTB,
    EA_CMD_SCCDSRST,
    EA_CMD_SCMODRCV,

    EA_CMD_SCANXA1,
    EA_CMD_SCANXA2,
    EA_CMD_SCORBSYNC,

    //PBI commands
    EA_CMD_SCPBHDATA,
    EA_CMD_SCPBSWORD,
    EA_CMD_SCPBSTEST,
    EA_CMD_SCPBXMOFF,
    EA_CMD_SCPBXMOR,
    EA_CMD_SCPBRESET,
    EA_CMD_SCPBLCMD,
    EA_CMD_SCPBBWORD,
    EA_CMD_SCTIME,
    EA_CMD_SCTIMERB,
    EA_CMD_SCPBRST,
    EA_CMD_SCPBRSTCL,
    EA_CMD_SCWDTEN,
    EA_CMD_SCWDTDS,
    EA_CMD_SCWDTDSCL,
    EA_CMD_SCWDTCL,
    EA_CMD_SCWDTST,
    EA_CMD_SCWDTRST,
    EA_CMD_SCWPVCL,
    EA_CMD_SCWPVST,
    EA_CMD_SCWPVEN,
    EA_CMD_SCWPVDS,
    EA_CMD_SCPBIRB,
    EA_CMD_SCRSAMOD,
    EA_CMD_SCRSARB,
    EA_CMD_SCDWRAP,
    EA_CMD_SCDWRAPRB,
    EA_CMD_SCMEMRB,
    EA_CMD_SCMEMLD,
    EA_CMD_SCBMOFST81,
    EA_CMD_SCCALPER0,
    EA_CMD_SCCALPER1,
    EA_CMD_SCGATEWID16,
    EA_CMD_SCGATEWID17,
    EA_CMD_SCGATEWID18,
    EA_CMD_SCGATEWID19,
    EA_CMD_SCGATEWID20,
    EA_CMD_SCGATEWID21,
    EA_CMD_SCMRLOLM01,
    EA_CMD_SCMROSTR01,
    EA_CMD_SCMRUPLM01,
    EA_CMD_SCMRLOLM00,
    EA_CMD_SCMROSTR00,
    EA_CMD_SCMRUPLM00,
    EA_CMD_SCORBTIC0,
    EA_CMD_SCORBTIC2,
    EA_CMD_SCPRFCLK54,
    EA_CMD_SCPRFCLK60,
    EA_CMD_SCRCVGAN1,
    EA_CMD_SCRCVGAN10,
    EA_CMD_SCRCVGAN11,
    EA_CMD_SCRCVGAN12,
    EA_CMD_SCRCVGAN13,
    EA_CMD_SCRCVGAN14,
    EA_CMD_SCRCVGAN15,
    EA_CMD_SCRCVGAN16,
    EA_CMD_SCRCVGAN17,
    EA_CMD_SCRCVGAN18,
    EA_CMD_SCRCVGAN19,
    EA_CMD_SCRCVGAN2,
    EA_CMD_SCRCVGAN20,
    EA_CMD_SCRCVGAN3,
    EA_CMD_SCRCVGAN4,
    EA_CMD_SCRCVGAN5,
    EA_CMD_SCRCVGAN6,
    EA_CMD_SCRCVGAN7,
    EA_CMD_SCRCVGAN8,
    EA_CMD_SCRCVGAN9,
    EA_CMD_SCRGAWID14,
    EA_CMD_SCRGAWID15,
    EA_CMD_SCRGAWID16,
    EA_CMD_SCRGAWID20,
    EA_CMD_SCRGAWID21,
    EA_CMD_SCRGBWID14,
    EA_CMD_SCRGBWID15,
    EA_CMD_SCRGBWID16,
    EA_CMD_SCRGBWID20,
    EA_CMD_SCRGBWID21,
    EA_CMD_SCRSRLPR0,
    EA_CMD_SCRSRLPR30,
    EA_CMD_SCRSRLPR45,
    EA_CMD_SCSASOFS57,
    EA_CMD_SCSASOFS81,
    EA_CMD_SCSASOFS16,
    EA_CMD_SCSWPEN1D,
    EA_CMD_SCSWPDS1D,
    EA_CMD_SCTBLSTR15,
    EA_CMD_SCTLDFPEN5F,
    EA_CMD_SCTRPWID09,
    EA_CMD_SCTRPWID10,
    EA_CMD_SCTRPWID11,
    EA_CMD_SCTRPWID12,
    EA_CMD_SCTRPWID13,
    EA_CMD_SCTRPWID14,
    EA_CMD_SCTRPWID15,
    EA_CMD_SCTWT1SEL,
    EA_CMD_SCTWT2SEL,

    //------------------------
    // expansion needed macros
    //------------------------
    EA_CMD_SCBDA_DEF,
    EA_CMD_SCBDA_DTC,
    EA_CMD_SCBDA_PBL,
    EA_CMD_SCBDA_FLT,
    EA_CMD_SCBDB_DEF,
    EA_CMD_SCBDB_DTC,
    EA_CMD_SCBDB_PBL,
    EA_CMD_SCBDB_FLT,
    EA_CMD_SCBRA_DEF,
    EA_CMD_SCBRA_RGC,
    EA_CMD_SCBRA_FLT,
    EA_CMD_SCBRB_DEF,
    EA_CMD_SCBRB_RGC,
    EA_CMD_SCBRB_FLT,
    EA_CMD_SCENG_T,
    EA_CMD_SCPCL_T,
    EA_CMD_SCPRC_T,
    EA_CMD_SCPST_T,
    EA_CMD_SCPWO_T,
    EA_CMD_SCSALL_CT1,
    EA_CMD_SCSALL_CT2,
    EA_CMD_SCSALL_DT1,
    EA_CMD_SCSALL_DT2,
    EA_CMD_SCSALL_RT1,
    EA_CMD_SCSALL_RT2,
    EA_CMD_SCSALL_ST1,
    EA_CMD_SCSALL_ST2,
    EA_CMD_SCSALL_WT1,
    EA_CMD_SCSALL_WT2,
    EA_CMD_SCSCL_T,
    EA_CMD_SCSRC_T,
    EA_CMD_SCSST_T,
    EA_CMD_SCSTA_T,
    EA_CMD_SCSWO_T,
    EA_CMD_SCSTATBLE,
    EA_CMD_SCENGTBLC,
    EA_CMD_SCSWPATCH_T,
    EA_CMD_VBAND_RELEASE,
    EA_CMD_PLB_TURN_ON,
    EA_CMD_PLB_TURN_OFF,
    EA_CMD_PPS_TURN_ON,
    EA_CMD_PPS_TURN_OFF,
    EA_CMD_CDS_TURN_ON,
    EA_CMD_CDS_TURN_OFF,
    EA_CMD_SAS_TURN_ON,
    EA_CMD_SAS_TURN_OFF,
    EA_CMD_SAS_SPIN_VFY,
    EA_CMD_SAS_SPIN_VFY2,
    EA_CMD_SES_TURN_ON,
    EA_CMD_SES_TURN_OFF,
    EA_CMD_TWTA_TURN_ON,
    EA_CMD_TWTA_TURN_OFF,
    EA_CMD_TWTA_VFY,
    EA_CMD_INST_MODE,
    EA_CMD_SES_RESET_ENBL,
    EA_CMD_SES_RESET_DSBL,
    EA_CMD_INST_QUIK_ON,
    EA_CMD_SES_RESET,
    EA_CMD_INST_TURN_OFF,
    EA_CMD_SCAT_FST_ACQ,
    EA_CMD_RAD_PAR_UPDATE,
    EA_CMD_SCAT_MOD_ON,
    EA_CMD_SCAT_MOD_OFF,
    EA_CMD_CDS_TMON_ENBL,
    EA_CMD_SES_TMON_ENBL,
    EA_CMD_SAS_TMON_ENBL,
    EA_CMD_INST_TEMP_MON
};

enum EffectE
{
    EFF_UNKNOWN = 0,
    EFF_ALL_OFF, 
    EFF_ELECTRONICS_ON, 
    EFF_RHM,
    EFF_TWTA_ON,
    EFF_TWTA_REPL_HEATER_ON,
    EFF_TWTA_REPL_HEATER_OFF,
    EFF_CDS_A, 
    EFF_CDS_B,
    EFF_SES_ELECTRONICS_ON,
    EFF_SES_REPL_HEATER_ON,
    EFF_SES_REPL_HEATER_OFF,
    EFF_SES_SUPPL_HEATER_ON,
    EFF_SES_SUPPL_HEATER_OFF,
    EFF_SAS_ELECTRONICS_ON,
    EFF_SAS_REPL_HEATER_ON,
    EFF_REPLACEMENT_HEATERS_ON,
    EFF_REPLACEMENT_HEATERS_OFF,
    EFF_STB, 
    EFF_RCV, 
    EFF_CAL, 
    EFF_WOM,
    EFF_TWTA_1, 
    EFF_TWTA_2,
    //    EFF_HVPS_ON, 
    //    EFF_HVPS_OFF,
    //    EFF_NEW_BINNING_CONSTANTS, 
    //    EFF_NEW_ANTENNA_SEQUENCE,
    EFF_TRS_CHANGE, 
    EFF_TWT_TRIP_OVERRIDE_ENABLE,
    EFF_TWT_TRIP_OVERRIDE_DISABLE,
    EFF_TWTA_MONITOR_CHANGE,
    EFF_TWTA_LOWDRIVE_POWER_FP_ON,
    EFF_TWTA_LOWDRIVE_POWER_FP_OFF,
    EFF_SES_PARAMS_TABLE_UPDATE,
    EFF_PRF_TABLE_UPDATE,
    EFF_RANGEGATE_TABLE_UPDATE,
    EFF_DOPPLER_TABLE_UPDATE,
    EFF_SERIAL_TELEM_TABLE_UPDATE,
    EFF_MISSION_TELEM_TABLE_UPDATE,
    EFF_SER_DIG_ENG_TLM_TABLE_UPDATE,
    EFF_SER_DIG_ST_TLM_TABLE_UPDATE,
    EFF_SES_PARAMS_TABLE_CHANGE,
    EFF_PRF_TABLE_CHANGE,
    EFF_RANGEGATE_TABLE_CHANGE,
    EFF_DOPPLER_TABLE_CHANGE,
    EFF_SERIAL_TELEM_TABLE_CHANGE,
    EFF_MISSION_TELEM_TABLE_CHANGE,
    EFF_SER_DIG_ENG_TLM_TABLE_CHANGE,
    EFF_SER_DIG_ST_TLM_TABLE_CHANGE,
    EFF_VALID_COMMAND_CNTR_CHANGE,
    EFF_INVALID_COMMAND_CNTR_CHANGE,
    EFF_COMMAND_HISTORY_CHANGE,
    EFF_COMMAND_HISTORY_REPEAT,
    EFF_SAS_A,
    EFF_SAS_B,
    EFF_SES_A,
    EFF_SES_B,
    EFF_SAS_A_SPIN_RATE_198,
    EFF_SAS_A_SPIN_RATE_180,
    EFF_SAS_B_SPIN_RATE_198,
    EFF_SAS_B_SPIN_RATE_180,
    EFF_RANGE_GATE_A_WIDTH_CHANGE,
    EFF_RANGE_GATE_B_WIDTH_CHANGE,
    EFF_TRANSMIT_PULSE_WIDTH_CHANGE,
    EFF_RECEIVER_GAIN_CHANGE,
    EFF_GRID_NORMAL,
    EFF_GRID_DISABLED,
    EFF_MODULATION_ON,
    EFF_MODULATION_OFF,
    EFF_PRF_CLOCK_CHANGE,
    EFF_RECEIVE_PROTECT_NORMAL,
    EFF_RECEIVE_PROTECT_ON,
    EFF_SES_RESET,
//     EFF_SES_MULTISEQ_DLE_RESPONSE_ENABLED,
//     EFF_SES_MULTISEQ_DLE_RESPONSE_DISABLED,
//     EFF_SAS_MULTISEQ_DLE_RESPONSE_ENABLED,
//     EFF_SAS_MULTISEQ_DLE_RESPONSE_DISABLED
//     EFF_SES_SUPPL_HEATER_CNTRL_ENABLED,
//     EFF_SES_SUPPL_HEATER_CNTRL_DISABLED,
     EFF_SES_MULTISEQ_DLE_RESPONSE_CHANGE,
     EFF_SAS_MULTISEQ_DLE_RESPONSE_CHANGE,
     EFF_SES_SUPPL_HEATER_CNTRL_CHANGE,
    EFF_CAL_PULSE_PERIOD_CHANGE,
    EFF_SES_RESET_RELOAD_PERIOD,
    EFF_CDS_SW_PATCH,
    EFF_CDS_SW_PATCH_ENABLE,
    EFF_CDS_SW_PATCH_DISABLE,
    EFF_TRANSMITPULSE_RANGEGATE_WIDTH_CHANGE,
    EFF_ORBIT_TIME_TICKS,
    EFF_INST_TIME_SYNC_INTERVAL,
    EFF_CDS_MEMORY_RDOUT_START_ADDR,
    EFF_CDS_MEMORY_RDOUT_UP_LIM,
    EFF_CDS_MEMORY_RDOUT_LO_LIM,
    EFF_TABLE_READOUT_START,
    EFF_BEAM_OFFSETS,
    EFF_SAS_OFFSETS,
    EFF_CDS_SOFT_RESET,
    EFF_NONE,   // nothing happened
    EFF_MOOT,   // something happened, but no effect
    NUM_EFFECTS
};



enum CmdTypeE
{
    TYP_UNKNOWN = 0,
    TYP_AUTOMATIC, TYP_REAL_TIME, TYP_UNPLANNED,
    NUM_TYPES
};

enum CmdStatusE
{
    STA_UNKNOWN = 0,
    STA_OK, STA_UNEXPECTED, STA_BOGUS, STA_CANCELED, STA_MISSED, STA_REJECTED,
    STA_GHOST, STA_ANOMALOUS, STA_IDIOSYNCRATIC,
    NUM_STATUS
};

enum VerifyE
{
    VER_NO = 0, VER_YES, VER_NA, VER_OK,
    NUM_VERIFYS
};

#if 0
struct CmdMnemonics
{
    EACommandE  commandId;
    char*       mnemonic;
};
#endif

struct CmdTypeMnemonics
{
    CmdTypeE    commandType;
    char*       mnemonic;
};

struct EffectMnemonics
{
    EffectE     effectId;
    char*       mnemonic;
};

struct CmdStatusMnemonics
{
    CmdStatusE  cmdStatusId;
    char*       mnemonic;
    char*       abbreviation;
};

struct CmdVerifyMnemonics
{
    VerifyE     cmdVerifyId;
    char*       mnemonic;
};

enum EADataFileFormat
{
    EA_DATAFILE_NONE = 0,
    EA_DATAFILE_UDEC2_ASCII,
    EA_DATAFILE_HEX2_ASCII,
    EA_DATAFILE_HEX4_TO_2HEX2,
    EA_DATAFILE_UDEC4_TO_2HEX2,
    EA_DATAFILE_2UDEC2_ASCII,
    EA_DATAFILE_ASCII
};

//-----------------------------------------------------------------
// if (numStaticParams > 3)
//     this string is a const char* where all arguments are stored
//     no checksum is calculated (last word in the string is the checksum).
// else if (numStaticParams <= 3  && numStaticParams >= 0)
//     all argument words are in staticParamString 
//               if _numWordsInParamFile == 0; else the argument is taken
//               from paramFile if _numWordsInParamFile != 0.
//     no checksum is calculated.
// else if datafile is specified (numWordsInParamFile is non-zero):
//     checksum is calculated.
//      if (numWordsInParamFile > 2 || < 0)  => QPF
//               else it is a REQQ.
//-----------------------------------------------------------------
struct EACommandArgsEntry
{
    EACommandE        cmdID;          // command ID
    unsigned short    cmdHex;
    char*             mnemonic;
    char*             description;
    int               realtimeOnly;
    int               isQPF;
    int               numWordsInParamFile; // 0=no datafile, -1=variable length
    EADataFileFormat  datafileFormat;
    int               numStaticParams;  // 0-3= static args, -1=non-static args
    char*             staticParamString;// param words (numStaticParams 0-3)
                                     // or table string name(numStaticParams>3)
};

//=========
// Command 
//=========

class Command
{
public:

    enum StatusE
    {
        OK,
        END_OF_FILE,
        ERROR_READING_CMD,
        ERROR_UPDATING_CMD
    };

    enum CommandTypeE
    {
        COMMAND_TYPE_UNKNOWN,
        AUTOMATIC_COMMAND,
        REALTIME_COMMAND
    };

    enum FormatE
    {
        FORMAT_UNKNOWN,
        USE_PATH_FORMAT,
        USE_TIME_FORMAT
    };

    enum FileFormatE
    {
        FILE_UNKNOWN,
        FILE_REQQ,
        FILE_QPF,
        FILE_RTCF,
        FILE_REQQ_STATIC,
        FILE_QPF_STATIC
    };

    //---------
    // methods 
    //---------

    Command(int lineNo, const char*  reqiCommandString);
    Command(EACommandE  cmdID);
    Command();
    virtual ~Command();

    StatusE     GetStatus() { return(_status); };
    StatusE     Read(FILE* ifp);
    StatusE     Write(FILE* ofp);
    StatusE     WriteForHumans(FILE* ofp);
    
    int         ReadReqiCommandString(int lineNo, const char* string);
    int         ReadReqiOptionalString(int lineNo, const char* string);
    int         SetDataFilename(const char* string);
    int         SetCmdParams(const char* string);
    int         SetOriginator(const char* string);
    int         SetComments(const char* string);

    void              SetCommandType(CommandTypeE commandType)
                          { _commandType = commandType; }
    CommandTypeE      GetCommandType(void) const {return _commandType; }

    void              SetFormat(FormatE format) { _format = format; }
    FormatE           GetFormat(void) const {return _format; }

    void              SetIsQPF(int isQPF)
                          { _isQPF = isQPF; }
    int               GetIsQPF(void) const
                          {return _isQPF; }

    void              SetNumWordsInParamFile(int numWords)
                          { _numWordsInParamFile = numWords; }
    int               GetNumWordsInParamFile(void) const
                          {return _numWordsInParamFile; }

    void              SetDatafileFormat(EADataFileFormat datafileFormat)
                          { _datafileFormat = datafileFormat; }
    EADataFileFormat  GetDatafileFormat(void) const
                          { return _datafileFormat; }

    void              SetNumStaticParams(int numStaticParams)
                          { _numStaticParams = numStaticParams; }
    int               GetNumStaticParams(void) const
                          { return _numStaticParams; }

    void              SetStaticParamString(const char* staticParamString)
                          { (void)strncpy(_staticParamString,
                                        staticParamString, BIG_4K_SIZE - 1); }
    const char*       GetStaticParamString(void) const
                          { return _staticParamString; }

    int               ToReqiString(char* string);

    const char* EffectString(); 
    int         EffectValue() {return (effect_value);};
    EffectE     EffectId() {return (effectId);};
    const char* GetEffectString(EffectE targetEffectId);
    const char* GetCmdTypeString(CmdTypeE targetCmdType);
    const char* CmdStatusString(); 
    const char* CmdStatusAbbrev(); 

    const char* L1AVerifyString(){return(_VerifyIdToMnemonic(l1aVerify));}; 
    const char* Hk2VerifyString(){return(_VerifyIdToMnemonic(hk2Verify));}; 
    const char* L1ApVerifyString(){return(_VerifyIdToMnemonic(l1apVerify));}; 

    static const char* CmdIdToMnemonic(EACommandE cmdId);
    static const char* CmdIdToDescript(EACommandE cmdId );
    static EACommandE MnemonicToCmdId(const char* string,
                                      unsigned short& p_cmdHex);
    static EACommandE CmdHexToCmdId(unsigned short p_cmdHex);
    static unsigned short CmdIdToCmdHex(EACommandE p_cmdId);
    static EffectE    MnemonicToEffectId(char* string); // mnemonic -> effectId
    static CmdStatusE MnemonicToStatusId(char* string); // mnemonic -> statusId
    static VerifyE    MnemonicToVerifyId(char* string); // mnemonic -> verifyId
    static int        Matchable(EACommandE command_id, EffectE effect_id);

    static FileFormatE  GetFileFormat(const Command* cmd);


    Itime       EarliestTime() const;
    Itime       LatestTime() const;
    Itime       BestTime() const;

    CmdTypeE    CmdType();

    int         UpdateWithEffect(Command* effect_cmd);
    int         Effective();

    static int  NeedDataFile(EACommandE commandID);
    static int  RealtimeOnly(EACommandE commandID);
    static int  IsQPF(EACommandE commandID);
    static int  NumWordsInParamFile(EACommandE commandID);
    static EADataFileFormat  DatafileFormat(EACommandE commandID);
    static int  NumStaticParams(EACommandE commandID);
    static const char* StaticParamString(EACommandE commandID);


                // return 1 if the word is a command, else 0
    static int  CheckForCommands(
                                 char*           array,      // IN
                                 int             array_size, // IN
                                 int&            byte1,      // OUT
                                 int&            bit1,       // OUT
                                 int&            byte2,      // OUT
                                 int&            bit2,       // OUT
                                 EACommandE&     cmd_code,   // OUT
                                 const char*&    cmd_name,   // OUT
                                 FILE*           outputFP);  // IN

    // int CmdParamsToEU( float values[2] );

    //-----------
    // variables 
    //-----------
    EACommandE      commandId;
    unsigned short  cmdHex;
    char            mnemonic[STRING_LEN];
    Tpg             plannedTpg;
    char*           dataFilename;
    char*           cmdParams;  // if Reqq or Rtcf.
    char*           originator;
    char*           comments;

    EffectE         effectId;
    unsigned short int effect_value; // For Command History Queue, stores 
                                     // cmdHex value of command in history queue.

    CmdStatusE  statusId;
    Itime       expectedTime;

    Itime       l1aTime;
    Itime       hk2Time;
    Itime       l1apTime;

    VerifyE     l1aVerify;
    VerifyE     hk2Verify;
    VerifyE     l1apVerify;



    unsigned char   tableRepetition;      // only used for reqi processing

    static const EffectMnemonics    effect_mnemonics[];
    static const CmdStatusMnemonics cmd_status_mnemonics[];
    static const CmdVerifyMnemonics cmd_verify_mnemonics[];
    static const CmdTypeMnemonics   cmd_type_mnemonics[];

    static const EACommandArgsEntry   cmdArgsTable[];
    static const int                  numCmdArgsEntries;

private:
    //---------
    // methods 
    //---------

    void        _WriteMarker(FILE* ofp, char* label);
    void        _WriteInt(FILE* ofp, char* label, int value);
    void        _WriteTpg(FILE* ofp, char* label, Tpg tpg);
    void        _WriteString(FILE* ofp, char* label, char* string);
    void        _WriteTime(FILE* ofp, char* label, Itime itime);

    int         _ReadMarker(FILE* ifp, char* marker);
    int         _ReadInt(FILE* ifp, char* label, int* value);
    int         _ReadShort(FILE* ifp, char* label, short int* value);
    int         _ReadTpg(FILE* ifp, char* label, Tpg* tpg);
    int         _ReadString(FILE* ifp, char* label, char** string);
    int         _ReadTime(FILE* ifp, char* label, Itime* itime);

                //Get Optional REQI string (repitition, originator and comments)
    int         _GetOptionalReqiString(char* string);

    static const char*  _VerifyIdToMnemonic(const VerifyE id);

    //-----------
    // variables 
    //-----------

    StatusE          _status;
    CommandTypeE     _commandType;
    FormatE          _format;
    int              _isQPF;
    int              _numWordsInParamFile; // 0=no datafile, -1=variable length
    EADataFileFormat _datafileFormat;
    int              _numStaticParams; // 0-3= static args, -1=non-static args
    char             _staticParamString[BIG_4K_SIZE];//static param string,if any
};

int operator<(const Command&, const Command&);
int operator<=(const Command&, const Command&);
int operator>(const Command&, const Command&);
int operator>=(const Command&, const Command&);
int operator==(const Command&, const Command&);

#endif
