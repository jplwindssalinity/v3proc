//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.9   27 Mar 1998 09:57:24   sally
// added L1A Derived data
// 
//    Rev 1.8   24 Mar 1998 09:15:56   sally
// de-warn for GNU C++
// 
//    Rev 1.7   17 Mar 1998 14:41:04   sally
// changed for REQQ
// 
//    Rev 1.6   16 Mar 1998 13:46:16   sally
// fix some parsing errors
// 
//    Rev 1.5   16 Mar 1998 10:52:16   sally
// ReadReqi() are split into two methods
// 
//    Rev 1.4   12 Mar 1998 17:15:46   sally
// adapt to the new QSCAT REQI format
// 
//    Rev 1.3   09 Mar 1998 16:34:12   sally
// adapt to the new REQI format
// 
//    Rev 1.2   26 Feb 1998 09:54:32   sally
// to pacify GNU C++ compiler
// 
//    Rev 1.1   20 Feb 1998 10:55:58   sally
// L1 to L1A
// 
//    Rev 1.0   04 Feb 1998 14:14:56   daffer
// Initial checking
// Revision 1.3  1998/02/03 00:12:33  sally
// change NRT to L1AP
//
// Revision 1.2  1998/01/30 22:28:56  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================
 
static const char rcs_id_Command_C[] =
    "@(#) $Header$";

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "Command.h"
#include "Tpg.h"

const EACommandArgsEntry Command::cmdArgsTable[] =
{
    { EA_CMD_RGAWID, "RGAWID", "Range Gate A Width",                1, 0 },
    { EA_CMD_RGBWID, "RGBWID", "Range Gate B Width",                1, 0 },
    { EA_CMD_TRPWID, "TRPWID", "Transmit Pulse Width",              1, 0 },
    { EA_CMD_PRFCLK, "PRFCLK", "PRF Clock Rate",                    1, 0 },
    { EA_CMD_RCVGAN, "RCVGAN", "Receiver Gain",                     1, 0 },
    { EA_CMD_GRDNOR, "GRDNOR", "TWT Grid Normal",                   0, 0 },
    { EA_CMD_GRDDIS, "GRDDIS", "TWT Grid Disable",                  0, 0 },
    { EA_CMD_RCVPON, "RCVPON", "Receive Protect On",                0, 0 },
    { EA_CMD_RCVPNR, "RCVPNR", "Receive Protect Normal",            0, 0 },
    { EA_CMD_TOVRON, "TOVRON", "TWT Trip Override On",              0, 0 },
    { EA_CMD_TOVROF, "TOVROF", "TWT Trip Override Off",             0, 0 },
    { EA_CMD_TMDONN, "TMDONN", "TWT Modulation On",                 0, 0 },
    { EA_CMD_TMDOFF, "TMDOFF", "TWT Modulation Off",                0, 0 },
    { EA_CMD_TWTTRS, "TWTTRS", "TWTA T/R Select 1 or 2",            1, 0 },
    { EA_CMD_SESRES, "SESRES", "SES Reset",                         0, 0 },
    { EA_CMD_SMLDIS, "SMLDIS", "SES Multi Seq Loss Diable",         0, 0 },
    { EA_CMD_HTRDIS, "HTRDIS", "SES Suppl Heater Disable",          0, 0 },
    { EA_CMD_TWTMND, "TWTMND", "TWTA Trip Monitor Disable",         0, 0 },
    { EA_CMD_CALPER, "CALPER", "Cal Pulse Dequence Period",         1, 0 },
    { EA_CMD_RSRLPR, "RSRLPR", "SES Reset-Reload Period",           1, 0 },
    { EA_CMD_PATENB, "PATENB", "S/W Patch Enable",                  2, 0 },
    { EA_CMD_PATDIS, "PATDIS", "S/W Patch Disable",                 2, 0 },
    { EA_CMD_RGTPWD, "RGTPWD", "Recv Gate/Tran Pulse Width",        2, 0 },
    { EA_CMD_ORBTIC, "ORBTIC", "Timer Ticks Per Orbit",             2, 0 },
    { EA_CMD_ITMSYN, "ITMSYN", "Instrument Time Sync Interv",       1, 0 },
    { EA_CMD_SSTSWH, "SSTSWH", "SES Standby Param Tbl Switch",      0, 0 },
    { EA_CMD_SCLSWH, "SCLSWH", "SES Calib Param Tbl Switch",        0, 0 },
    { EA_CMD_SWOSWH, "SWOSWH", "SES Wind Obs Param Tbl Switch",     0, 0 },
    { EA_CMD_SCMSWH, "SCMSWH", "SES AllMod Param Tbl Switch",       0, 0 },
    { EA_CMD_SRCSWH, "SRCSWH", "SES Recv Only Param Tbl Switch",    0, 0 },
    { EA_CMD_MROSTR, "MROSTR", "CDS MRO Start Address",             2, 0 },
    { EA_CMD_TBLSTR, "TBLSTR", "Table RO Start Table Type",         1, 0 },
    { EA_CMD_SEQUPP, "SEQUPP", "CDS Segment MRO High Address",      2, 0 },
    { EA_CMD_SEQLOW, "SEQLOW", "CDS Segment MRO Low Address",       2, 0 },
    { EA_CMD_BMOFST, "BMOFST", "Beam A/B Offsets",                  2, 0 },
    { EA_CMD_SASOFS, "SASOFS", "SAS A/B Offsets",                   2, 0 },
    { EA_CMD_SSLENB, "SSLENB", "SAS Multi Seq Loss Enable",         0, 0 },
    { EA_CMD_SSLDIS, "SSLDIS", "SAS Multi Seq Loss Disable",        0, 0 },
    { EA_CMD_BRASWH, "BRASWH", "Beam A Range Table Switch",         0, 0 },
    { EA_CMD_BRBSWH, "BRBSWH", "Beam B Range Table Switch",         0, 0 },
    { EA_CMD_BDASWH, "BDASWH", "Beam A Doppler Table Switch",       0, 0 },
    { EA_CMD_BDBSWH, "BDBSWH", "Beam B Doppler Table Switch",       0, 0 },
    { EA_CMD_ENGSWH, "ENGSWH", "Serial Dig Eng TLM Switch",         0, 0 },
    { EA_CMD_STASWH, "STASWH", "Serial Dig STA TLM Switch",         0, 0 },
    { EA_CMD_PSTSWH, "PSTSWH", "PRF Standby Table Switch",          0, 0 },
    { EA_CMD_PCLSWH, "PCLSWH", "PRF Calibration Table Switch",      0, 0 },
    { EA_CMD_PRCSWH, "PRCSWH", "PRF Receive Only Table Switch",     0, 0 },
    { EA_CMD_PWOSWH, "PWOSWH", "PRF Wind Obs Table Switch",         0, 0 },
    { EA_CMD_NO__OP, "NO__OP", "No Operation Command",              0, 0 },
    { EA_CMD_SMLENB, "SMLENB", "SES Multi Seq Loss Enable",         0, 0 },
    { EA_CMD_HTRENB, "HTRENB", "SES Suppl Heater Enable",           0, 0 },
    { EA_CMD_TWTMNE, "TWTMNE", "TWTA Trip Monitor Enable",          1, 0 },
    { EA_CMD_SSTTBL, "SSTTBL", "SES Standby Param Tbl Update",      6, 1 },
    { EA_CMD_SCLTBL, "SCLTBL", "SES Cal Param Tbl Update",          6, 1 },
    { EA_CMD_SRCTBL, "SRCTBL", "SES Recv Only Param Tbl Update",    6, 1 },
    { EA_CMD_SWOTBL, "SWOTBL", "SES Wind Obs Param Tbl Update",     6, 1 },
    { EA_CMD_SCMTBL, "SCMTBL", "SES AllMod Param Tbl Update",      12, 1 },
    { EA_CMD_BRATBL, "BRATBL", "Beam A Range Table Update",        23, 1 },
    { EA_CMD_BRBTBL, "BRBTBL", "Beam B Range Table Update",        23, 1 },
    { EA_CMD_BDATBL, "BDATBL", "Beam A Doppler Table Update",     786, 1 },
    { EA_CMD_BDBTBL, "BDBTBL", "Beam B Doppler Table Update",     786, 1 },
    { EA_CMD_ENGTBL, "ENGTBL", "Serial Dig Eng TLM Table",         20, 1 },
    { EA_CMD_STATBL, "STATBL", "Serial Dig STA TLM Table",         20, 1 },
    { EA_CMD_PSTTBL, "PSTTBL", "PRF Standby Table Update",         -1, 1 },
    { EA_CMD_PCLTBL, "PCLTBL", "PRF Calibration Table Update",     -1, 1 },
    { EA_CMD_PRCTBL, "PRCTBL", "PRF Receive Only Table Update",    -1, 1 },
    { EA_CMD_PWOTBL, "PWOTBL", "PRF Wind Obs Table Update",        -1, 1 },
    { EA_CMD_K9RESS, "K9RESS", "TWTA Pwr On/Off K9 Primary",        0, 0 },
    { EA_CMD_K10RES, "K10RES", "TWTA Pwr On/Off K10 Secondary",     0, 0 },
    { EA_CMD_K11RES, "K11RES", "Select TWTA 1/2 K11 Primary",       0, 0 },
    { EA_CMD_K19RES, "K19RES", "Select SAS A/B K19 Primary",        0, 0 },
    { EA_CMD_K12RES, "K12RES", "Select TWTA 1/2 K12 Secondary",     0, 0 },
    { EA_CMD_K25RES, "K25RES", "Spare K25",                         0, 0 },
    { EA_CMD_SSBRES, "SSBRES", "Select SAS-B 18 RPM",               0, 0 },
    { EA_CMD_K20RES, "K20RES", "Select SAS A/B K20 Secondary",      0, 0 },
    { EA_CMD_K15RES, "K15RES", "Select SAS A/B K15 Primary",        0, 0 },
    { EA_CMD_K26RES, "K26RES", "Spare K26",                         0, 0 },
    { EA_CMD_K21RES, "K21RES", "SES Heater On/Off K21 Primary",     0, 0 },
    { EA_CMD_K23RES, "K23RES", "Spare K23",                         0, 0 },
    { EA_CMD_K16RES, "K16RES", "Select SES A/B K16 Secondary",      0, 0 },
    { EA_CMD_SSARES, "SSARES", "Select SES-A 18 RPM",               0, 0 },
    { EA_CMD_K22RES, "K22RES", "SES Heater On/Off K22 Secondary",   0, 0 },
    { EA_CMD_K24RES, "K24RES", "Spare K24",                         0, 0 },
    { EA_CMD_K9SETT, "K9SETT", "TWTA Pwr On/Off K9 Primary",        0, 0 },
    { EA_CMD_K10SET, "K10SET", "TWTA Pwr On/Off K10 Secondary",     0, 0 },
    { EA_CMD_K11SET, "K11SET", "Select TWTA 1/2 K11 Primary",       0, 0 },
    { EA_CMD_K19SET, "K19SET", "Select SAS A/B K19 Primary",        0, 0 },
    { EA_CMD_K12SET, "K12SET", "Select TWTA 1/2 K12 Secondary",     0, 0 },
    { EA_CMD_K25SET, "K25SET", "Spare K25",                         0, 0 },
    { EA_CMD_SSBSET, "SSBSET", "Select SAS-B 19.8 RPM",             0, 0 },
    { EA_CMD_K20SET, "K20SET", "Select SAS A/B K20 Secondary",      0, 0 },
    { EA_CMD_K15SET, "K15SET", "Select SES A/B K15 Primary",        0, 0 },
    { EA_CMD_K26SET, "K26SET", "Spare K26",                         0, 0 },
    { EA_CMD_K21SET, "K21SET", "SES Heater On/Off K21 Primary",     0, 0 },
    { EA_CMD_K23SET, "K23SET", "Spare K23",                         0, 0 },
    { EA_CMD_K16SET, "K16SET", "Select SES A/B K16 Secondary",      0, 0 },
    { EA_CMD_SSASET, "SSASET", "Select SAS-A 19.8 RPM",             0, 0 },
    { EA_CMD_K22SET, "K22SET", "SES Heater On/Off K22 Secondary",   0, 0 },
    { EA_CMD_K24SET, "K24SET", "Spare K24",                         0, 0 },
    { EA_CMD_SWPTCH, "SWPTCH", "CDS Flight S/W Patch",             -1, 1 },
    { EA_CMD_MODCAL, "MODCAL", "Mode Calibration",                  0, 0 },
    { EA_CMD_MODWOM, "MODWOM", "Mode Wind Observation",             0, 0 },
    { EA_CMD_MODSTB, "MODSTB", "Mode Standby",                      0, 0 },
    { EA_CMD_CDSRES, "CDSRES", "CDS System Reset",                  0, 0 },
    { EA_CMD_MODRCV, "MODRCV", "Mode Receive Only",                 0, 0 }
};

const int Command::numCmdArgsEntries = ElementNumber(cmdArgsTable);

//static
const CmdTypeMnemonics Command::cmd_type_mnemonics[] =
{
    { TYP_UNKNOWN, "Unknown" },
    { TYP_AUTOMATIC, "Automatic" },
    { TYP_REAL_TIME, "Real Time" },
    { TYP_UNPLANNED, "Unplanned" },
    { (CmdTypeE)0, (char*)0 }
};

//static
const EffectMnemonics Command::effect_mnemonics[] =
{
    { EFF_UNKNOWN, "Unknown" },
    { EFF_ALL_OFF, "Off" }, { EFF_ELECTRONICS_ON, "Elec On" },
    { EFF_RHM, "RHM" },
    { EFF_REPLACEMENT_HEATER_ENABLED, "Rep Htr Ena" },
    { EFF_REPLACEMENT_HEATER_DISABLED, "Rep Htr Dis" },
    { EFF_DSS_A, "DSS A" }, { EFF_DSS_B, "DSS B" },
    { EFF_SPARE_HEATER_ENABLED, "Spare Htr En" },
    { EFF_SPARE_HEATER_DISABLED, "Spare Htr Dis" },
    { EFF_SBM, "SBM" }, { EFF_ROM, "ROM" }, { EFF_CCM, "CCM" },
    { EFF_DBM, "DBM" }, { EFF_WOM, "WOM" },
    { EFF_TWTA_1, "TWTA #1" }, { EFF_TWTA_2, "TWTA #2" },
    { EFF_HVPS_ON, "HVPS On" }, { EFF_HVPS_OFF, "HVPS Off" },
    { EFF_NEW_BINNING_CONSTANTS, "New Bin Const" },
    { EFF_NEW_ANTENNA_SEQUENCE, "New Ant Seq" },
    { EFF_WTS_1, "WTS 1" }, { EFF_WTS_2, "WTS 2" },
    { EFF_TWTA_TRIP_OVERRIDE_ENABLED, "Trip Ovr En" },
    { EFF_RFS_MONITOR_CHANGE, "Trip Mon Change" },
    { EFF_NONE, "None" }, { EFF_MOOT, "Moot" },
    { (EffectE)0, (char*)0 }
};

//static
const CmdStatusMnemonics Command::cmd_status_mnemonics[] =
{
    { STA_UNKNOWN, "Unknown", "???" },
    { STA_OK, "OK", "OK" },
    { STA_UNEXPECTED, "Unexpected", "UnX" },
    { STA_BOGUS, "Bogus", "Bog" },
    { STA_CANCELED, "Canceled", "Can" },
    { STA_MISSED, "Missed", "Mis" },
    { STA_REJECTED, "Rejected", "Rej" },
    { STA_GHOST, "Ghost", "Gho" },
    { STA_ANOMALOUS, "Anomalous", "Ano" },
    { STA_IDIOSYNCRATIC, "Idiosyncratic", "Idi" },
    { (CmdStatusE)0, (char*)0, (char*)0 }
};

//static
const CmdVerifyMnemonics Command::cmd_verify_mnemonics[]=
{
    { VER_NO, "No" },
    { VER_YES, "Yes" },
    { VER_NA, "N/A" },
    { VER_OK, "OK" },
    { (VerifyE)0, (char*)0 }
};

//=================
// Command methods 
//=================

Command::Command(
int         lineNo,              // line number
const char* reqiCommandString)   // REQI command string
:   commandId(EA_CMD_UNKNOWN), plannedTpg(INVALID_TPG), dataFilename(NULL),
    originator(NULL), comments(NULL), effectId(EFF_UNKNOWN),
    statusId(STA_OK), expectedTime(INVALID_TIME), l1Time(INVALID_TIME),
    hkdtTime(INVALID_TIME), l1apTime(INVALID_TIME), l1Verify(VER_NO),
    hkdtVerify(VER_NO), l1apVerify(VER_NO), binRepetition(0),
    _status(OK), _commandType(COMMAND_TYPE_UNKNOWN), _format(FORMAT_UNKNOWN)
{
    if ( ! ReadReqiCommandString(lineNo, reqiCommandString))
        _status = ERROR_READING_CMD;

} // Command::Command

Command::Command()
:   commandId(EA_CMD_UNKNOWN), plannedTpg(INVALID_TPG), dataFilename(NULL),
    originator(NULL), comments(NULL), effectId(EFF_UNKNOWN),
    statusId(STA_OK), expectedTime(INVALID_TIME), l1Time(INVALID_TIME),
    hkdtTime(INVALID_TIME), l1apTime(INVALID_TIME), l1Verify(VER_NO),
    hkdtVerify(VER_NO), l1apVerify(VER_NO), binRepetition(0),
    _status(OK), _commandType(COMMAND_TYPE_UNKNOWN), _format(FORMAT_UNKNOWN)
{
}

Command::~Command()
{
    free(dataFilename);
    free(originator);
    free(comments);
    return;
}

//------
// Read 
//------

Command::StatusE
Command::Read(
    FILE*   ifp)
{
    int retval = _ReadMarker(ifp, CMD_START_MARKER);
    if (retval == -1)
        _status = END_OF_FILE;
    else if (retval == 0)
        _status = ERROR_READING_CMD;
    else if (_ReadInt(ifp, COMMAND_ID_LABEL, (int *)&commandId) &&
        _ReadTpg(ifp, PLANNED_TPG_LABEL, &plannedTpg) &&
        _ReadString(ifp, DATA_FILENAME_LABEL, &dataFilename) &&
        _ReadString(ifp, ORIGINATOR_LABEL, &originator) &&
        _ReadString(ifp, COMMENTS_LABEL, &comments) &&
        _ReadInt(ifp, EFFECT_ID_LABEL, (int *)&effectId) &&
        _ReadInt(ifp, STATUS_ID_LABEL, (int *)&statusId) &&
        _ReadTime(ifp, EXPECTED_TIME_LABEL, &expectedTime) &&
        _ReadTime(ifp, L1A_TIME_LABEL, &l1Time) &&
        _ReadTime(ifp, HK2_TIME_LABEL, &hkdtTime) &&
        _ReadTime(ifp, L1AP_TIME_LABEL, &l1apTime) &&
        _ReadInt(ifp, L1A_VERIFY_LABEL, (int *)&l1Verify) &&
        _ReadInt(ifp, HK2_VERIFY_LABEL, (int *)&hkdtVerify) &&
        _ReadInt(ifp, L1AP_VERIFY_LABEL, (int *)&l1apVerify) &&
        _ReadMarker(ifp, CMD_END_MARKER) )
    {
        _status = OK;
    }
    else
        _status = ERROR_READING_CMD;

    return (_status);
}

//-------------------------
// ReadReqiCommandString 
//-------------------------

int
Command::ReadReqiCommandString(
int           lineNo,    // line number
const char*   line)      // REQI command line
{
    if (line == 0) return 0;

    // take out the trailing '\n' and blanks first
    char* lastChar = (char*) line + strlen(line) - 1;
    if (*lastChar == '\n')
        *lastChar-- = '\0';
    while (*lastChar == ' ')
        *lastChar-- = '\0';

    const char* ptr = line;
    // look for a time and a command mnemonic
    char mnemonic[STRING_LEN];
    char timeString[STRING_LEN];
    int bytes=0;
    if (sscanf(ptr, " %s %s%n", timeString, mnemonic, &bytes) != 2)
        return 0;

    Itime::CodeAFormatE timeFormat =
                            Itime::CheckCodeAFormat(timeString);
                
    switch (timeFormat)
    {
        case Itime::CODE_A_FORMAT_UNKNOWN:
        case Itime::CODE_A_FORMAT_YEAR:
        case Itime::CODE_A_FORMAT_MONTH:
        case Itime::CODE_A_FORMAT_MILLI_SECOND:
            fprintf(stderr,
                    "\nInvalid time string(%s) at line %d\n",
                    timeString, lineNo);
            return 0;
            break;
        case Itime::CODE_A_FORMAT_DAY:
            break;
        case Itime::CODE_A_FORMAT_HOUR:
            _commandType = REALTIME_COMMAND;
            _format = USE_TIME_FORMAT;
            break;
        case Itime::CODE_A_FORMAT_MINUTE:
        case Itime::CODE_A_FORMAT_SECOND:
            _commandType = AUTOMATIC_COMMAND;
            _format = USE_TIME_FORMAT;
            break;
    }
    // set the time
    Itime cmd_time;
    cmd_time.CodeAToItime(timeString);
    plannedTpg.time = cmd_time;
    expectedTime = cmd_time;        // current best guess

    // set the command id
    commandId = MnemonicToCmdId(mnemonic);
    if (commandId == EA_CMD_UNKNOWN)
    {
        fprintf(stderr,
                "\nInvalid command mnemonic (%s) at line %d\n",
                mnemonic, lineNo);
        return 0;
    }
    ptr += bytes;

    // set the number of words in param file, if any
    SetNumWordsInParamFile(NumWordsInParamFile(commandId));

    // look for a path and argument of latitutde
    int path;
    float gamma;
    bytes = 0;
    if (sscanf(ptr, " %d%n", &path, &bytes) == 1)
    {
        // catch time format commands + path
        if (_format == USE_TIME_FORMAT)
        {
            fprintf(stderr, "\nLine %d: both time and path are specified\n",
                            lineNo);
            return 0;
        }
        _format = USE_PATH_FORMAT;
        plannedTpg.path = path;
        ptr += bytes;
    }

    bytes = 0;
    if (sscanf(ptr, " %g%n", &gamma, &bytes) == 1)
    {
        _commandType = AUTOMATIC_COMMAND;
        plannedTpg.gamma = gamma;
        ptr += bytes;
    }
    else
    {
        if (_commandType == COMMAND_TYPE_UNKNOWN)
            _commandType = REALTIME_COMMAND;
    }

    if (RealtimeOnly(commandId) && _commandType != REALTIME_COMMAND)
    {
        fprintf(stderr, "\nLine %d: %s: must be a realtime command only\n",
                        lineNo, line);
        return 0;
    }

    // check if this command requires data file
    if ( ! NeedDataFile(commandId))
    {
        // should know the command type and format by now
        if (_commandType != COMMAND_TYPE_UNKNOWN && _format != FORMAT_UNKNOWN)
        {
            // no more junk after this, then it is ok
            if ((unsigned int)(ptr - line) == strlen(line))
                return 1;
            else
            {
                fprintf(stderr, "\nLine %d: extra arguments [%s]\n",
                            lineNo, ptr);
                return 0;
            }
        }
    }

    char filename[MAX_LINE_SIZE];
    if (sscanf(ptr, " %s%n", filename, &bytes) == 1)
    {
        SetDataFilename(filename);
        ptr += bytes;
    }

    // should know the command type and format by now
    if (_commandType != COMMAND_TYPE_UNKNOWN && _format != FORMAT_UNKNOWN)
    {
        // no more junk after this, then it is ok
        if ((unsigned int)(ptr - line) == strlen(line))
            return 1;
        else
        {
            fprintf(stderr, "\nLine %d: extra arguments [%s]\n",
                              lineNo, ptr);
            return 0;
        }
    }

    return 1;

} // Command::ReadReqiCommandString

//-------
// Write 
//-------

Command::StatusE
Command::Write(
    FILE*   ofp)
{
    _WriteMarker(ofp, CMD_START_MARKER);

    _WriteInt(ofp, COMMAND_ID_LABEL, commandId);
    _WriteTpg(ofp, PLANNED_TPG_LABEL, plannedTpg);
    _WriteString(ofp, DATA_FILENAME_LABEL, dataFilename);
    _WriteString(ofp, ORIGINATOR_LABEL, originator);
    _WriteString(ofp, COMMENTS_LABEL, comments);

    _WriteInt(ofp, EFFECT_ID_LABEL, effectId);
    _WriteInt(ofp, STATUS_ID_LABEL, statusId);
    _WriteTime(ofp, EXPECTED_TIME_LABEL, expectedTime);

    _WriteTime(ofp, L1A_TIME_LABEL, l1Time);
    _WriteTime(ofp, HK2_TIME_LABEL, hkdtTime);
    _WriteTime(ofp, L1AP_TIME_LABEL, l1apTime);

    _WriteInt(ofp, L1A_VERIFY_LABEL, l1Verify);
    _WriteInt(ofp, HK2_VERIFY_LABEL, hkdtVerify);
    _WriteInt(ofp, L1AP_VERIFY_LABEL, l1apVerify);

    _WriteMarker(ofp, CMD_END_MARKER);

    return (_status);
}

//----------------
// WriteForHumans 
//----------------

Command::StatusE
Command::WriteForHumans(
FILE*   )
#if 0
FILE*   ofp)
#endif
{
#if 0
    char string[256];

    //---------------------------------------
    // command mnemonic and the command type 
    //---------------------------------------

    CmdTypeE cmd_type = CmdType();
    if (cmd_type == TYP_AUTOMATIC || cmd_type == TYP_REAL_TIME)
    {
        plannedTpg.TpgToString(string);
        fprintf(ofp, "%s (%s - %s)\n", CmdString(), GetCmdTypeString(cmd_type),
            string);
    }
    else
    {
        fprintf(ofp, "%s (%s)\n", CmdString(), GetCmdTypeString(cmd_type));
    }

    //-----------------------
    // data filename or data 
    //-----------------------

    if (dataFilename)
    {
        switch (commandId)
        {
            case CMD_BIN:
                fprintf(ofp, "  BC File: %s\n", dataFilename);
                break;
            case CMD_ANT:
                fprintf(ofp, "  Antenna Sequence: %s\n", dataFilename);
                break;
            case CMD_MON:
                fprintf(ofp, "  RFS Monitor Data: %s\n", dataFilename);
                break;
            default:
                break;
        }
    }

    //-------------------------
    // comments and originator 
    //-------------------------

    if (originator)
        fprintf(ofp, "  Originator: %s\n", originator);
    if (comments)
        fprintf(ofp, "  Comments: %s\n", comments);

    //-------------------
    // effect and status 
    //-------------------

    fprintf(ofp, "  Effect: %s     Status: %s\n", EffectString(),
        CmdStatusString());

    //---------------
    // expected time 
    //---------------

    if (expectedTime != INVALID_TIME)
    {
        expectedTime.ItimeToCodeA(string);
        fprintf(ofp, "  Expected Time: %s\n", string);
    }

    //--------------------
    // verification times 
    //--------------------

    fprintf(ofp, "  L1A: ");
    switch(l1Verify)
    {
        case VER_NO: fprintf(ofp, "Unverified"); break;
        case VER_YES: fprintf(ofp, "Verified"); break;
        case VER_NA: fprintf(ofp, "N/A"); break;
        case VER_OK: fprintf(ofp, "OK"); break;
        default: break;
    }
    l1Time.ItimeToCodeA(string);
    fprintf(ofp, ", %s\n", string);

    fprintf(ofp, "  HK2: ");
    switch(hkdtVerify)
    {
        case VER_NO: fprintf(ofp, "Unverified"); break;
        case VER_YES: fprintf(ofp, "Verified"); break;
        case VER_NA: fprintf(ofp, "N/A"); break;
        case VER_OK: fprintf(ofp, "OK"); break;
        default: break;
    }
    hkdtTime.ItimeToCodeA(string);
    fprintf(ofp, ", %s\n", string);

    fprintf(ofp, "  L1AP: ");
    switch(l1apVerify)
    {
        case VER_NO: fprintf(ofp, "Unverified"); break;
        case VER_YES: fprintf(ofp, "Verified"); break;
        case VER_NA: fprintf(ofp, "N/A"); break;
        case VER_OK: fprintf(ofp, "OK"); break;
        default: break;
    }
    l1apTime.ItimeToCodeA(string);
    fprintf(ofp, ", %s\n", string);
#endif

    return (_status);
}

//-----------------
// SetDataFilename 
//-----------------

int
Command::SetDataFilename(
    const char*     string)
{
    free(dataFilename);     // in case it is non-NULL
    if (string == NULL)
    {
        dataFilename = NULL;
        return(1);
    }
    else
    {
        dataFilename = strdup(string);
        if (dataFilename == NULL)
            return(0);
        else
            return(1);
    }
}

//---------------
// SetOriginator 
//---------------

int
Command::SetOriginator(
    const char*     string)
{
    free(originator);       // in case it is non-NULL
    if (string == NULL)
    {
        originator = NULL;
        return(1);
    }
    else
    {
        originator = strdup(string);
        if (originator == NULL)
            return(0);
        else
            return(1);
    }
}

//-------------
// SetComments 
//-------------

int
Command::SetComments(
    const char*     string)
{
    free(comments);     // in case it is non-NULL
    if (string == NULL)
    {
        comments = NULL;
        return(1);
    }
    else
    {
        comments = strdup(string);
        if (comments == NULL)
            return(0);
        else
            return(1);
    }
}

//=============================================
// ROUTINE: ToReqiString
// PURPOSE: Return a pointer to the string to 
//          be written to the REQI file. 
//=============================================
int
Command::ToReqiString(
char*   string)     // user shall provide space
{
    char tempString[BIG_SIZE];

    //-----------------------------------
    // write the date and cmd mnemonic
    //-----------------------------------
    if (_commandType == AUTOMATIC_COMMAND)
    {
        if (_format == USE_PATH_FORMAT)
        {
            if (plannedTpg.time.ItimeToCodeADate(tempString) == 0)
                return (0);
            (void) sprintf(string, "%s %s %d %6.2f", tempString,
                            CmdString(), plannedTpg.path, plannedTpg.gamma);
        }
        else
        {
            if (plannedTpg.time.ItimeToCodeASecond(tempString) == 0)
                return (0);
            (void) sprintf(string, "%s %s", tempString, CmdString());
        }
    }
    else
    {
        if (_format == USE_PATH_FORMAT)
        {
            if (plannedTpg.time.ItimeToCodeADate(tempString) == 0)
                return (0);
            (void) sprintf(string, "%s %s %d", tempString,
                            CmdString(), plannedTpg.path);
        }
        else
        {
            if (plannedTpg.time.ItimeToCodeAHour(tempString) == 0)
                return (0);
            (void) sprintf(string, "%s %s", tempString, CmdString());
        }
    }

    //-----------------------------------
    // if datafile is specified, write it
    //-----------------------------------
    if (dataFilename != NULL)
    {
       (void)strcat(string, " ");
       (void)strcat(string,dataFilename);
    }
    (void)strcat(string, "\n");
   
     if (_GetOptionalReqiString(tempString))
         (void)strcat(string, tempString);
     else
         return 0;

    return 1;

}//Command::ToReqiString

//-----------
// CmdString 
//-----------

const char*
Command::CmdString()
{
    for (int i = 0; cmdArgsTable[i].mnemonic; i++)
    {
        if (commandId == cmdArgsTable[i].cmdID)
            return (cmdArgsTable[i].mnemonic);
    }
    return (0);
}

//--------------
// EffectString 
//--------------

const char*
Command::EffectString()
{
    return(GetEffectString(effectId));
}

//-----------------
// GetEffectString 
//-----------------

const char*
Command::GetEffectString(
    EffectE     targetEffectId)
{
    for (int i = 0; effect_mnemonics[i].mnemonic; i++)
    {
        if (effect_mnemonics[i].effectId == targetEffectId)
            return (effect_mnemonics[i].mnemonic);
    }
    return (0);
}

//------------------
// GetCmdTypeString 
//------------------

const char*
Command::GetCmdTypeString(
    CmdTypeE    targetCmdType)
{
    for (int i = 0; cmd_type_mnemonics[i].mnemonic; i++)
    {
        if (cmd_type_mnemonics[i].commandType == targetCmdType)
            return (cmd_type_mnemonics[i].mnemonic);
    }
    return (0);
}

//-----------------
// CmdStatusString 
//-----------------

const char*
Command::CmdStatusString()
{
    for (int i = 0; cmd_status_mnemonics[i].mnemonic; i++)
    {
        if (cmd_status_mnemonics[i].cmdStatusId == statusId)
            return (cmd_status_mnemonics[i].mnemonic);
    }
    return (0);
}

//-----------------
// CmdStatusAbbrev 
//-----------------

const char*
Command::CmdStatusAbbrev()
{
    for (int i = 0; cmd_status_mnemonics[i].abbreviation; i++)
    {
        if (cmd_status_mnemonics[i].cmdStatusId == statusId)
            return (cmd_status_mnemonics[i].abbreviation);
    }
    return (0);
}

//--------------
// EarliestTime 
//--------------
// returns the earliest verified time or the expected time
// (if no verified times exist)

Itime
Command::EarliestTime()
const
{
    Itime earliest_time = INVALID_TIME;

    //---------------------------------
    // find the earliest verified time 
    //---------------------------------

    if (l1Time != INVALID_TIME)
    {
        if (earliest_time == INVALID_TIME || l1Time < earliest_time)
            earliest_time = l1Time;
    }
    if (hkdtTime != INVALID_TIME)
    {
        if (earliest_time == INVALID_TIME || hkdtTime < earliest_time)
            earliest_time = hkdtTime;
    }
    if (l1apTime != INVALID_TIME)
    {
        if (earliest_time == INVALID_TIME || l1apTime < earliest_time)
            earliest_time = l1apTime;
    }
    if (earliest_time != INVALID_TIME)
        return(earliest_time);

    //------------------------------------------------
    // no valid verified times, try the expected time 
    //------------------------------------------------

    if (expectedTime != INVALID_TIME)
        return(expectedTime);

    return(INVALID_TIME);
}

//------------
// LatestTime 
//------------
// returns the latest verified time or the expected time
// (if no verified times exist)

Itime
Command::LatestTime()
const
{
    Itime latest_time = INVALID_TIME;

    //-------------------------------
    // find the latest verified time 
    //-------------------------------

    if (l1Time != INVALID_TIME)
    {
        if (latest_time == INVALID_TIME || l1Time > latest_time)
            latest_time = l1Time;
    }
    if (hkdtTime != INVALID_TIME)
    {
        if (latest_time == INVALID_TIME || hkdtTime > latest_time)
            latest_time = hkdtTime;
    }
    if (l1apTime != INVALID_TIME)
    {
        if (latest_time == INVALID_TIME || l1apTime > latest_time)
            latest_time = l1apTime;
    }
    if (latest_time != INVALID_TIME)
        return(latest_time);

    //------------------------------------------------
    // no valid verified times, try the expected time 
    //------------------------------------------------

    if (expectedTime != INVALID_TIME)
        return(expectedTime);

    return(INVALID_TIME);
}

//----------
// BestTime 
//----------
// returns what is believed to be the most accurate time of the command

Itime
Command::BestTime()
const
{
    Itime iv_time = INVALID_TIME;
    if (l1Time != iv_time)
        return(l1Time);
    if (l1apTime != iv_time)
        return(l1apTime);
    if (hkdtTime != iv_time)
        return(hkdtTime);
    if (expectedTime != iv_time)
        return(expectedTime);
    if (plannedTpg.time != iv_time)
        return(plannedTpg.time);
    return(iv_time);
}

//---------
// CmdType 
//---------
// returns the type of command based on the plannedTpg

CmdTypeE
Command::CmdType()
{
    if (plannedTpg.path == INVALID_PATH)
    {
        if (plannedTpg.time == INVALID_TIME)
            return(TYP_UNPLANNED);
        else
            return(TYP_REAL_TIME);
    }
    else    // path is valid
    {
        if (plannedTpg.time == INVALID_TIME)
            return(TYP_UNKNOWN);
        else
            return(TYP_AUTOMATIC);
    }
    return(TYP_UNKNOWN);
}

//-----------
// Matchable 
//-----------
// returns 1 if the passed effect can be matched with the given command

int
Command::Matchable(
EACommandE    ,
EffectE       )
#if 0
    EACommandE    command_id,
    EffectE     effect_id)
#endif
{
#if 0
    //--------------------------------
    // allowable effects for commands 
    //--------------------------------

    static const EffectE k1_k2_k3_set[] = { EFF_UNKNOWN, EFF_ALL_OFF, EFF_RHM,
        EFF_NONE, EFF_MOOT, NUM_EFFECTS };
    static const EffectE k1_k2_k3_reset[] = { EFF_UNKNOWN, EFF_ELECTRONICS_ON,
        EFF_NONE, EFF_MOOT, NUM_EFFECTS };
    static const EffectE k4_k5_k6_set[] = { EFF_UNKNOWN, EFF_ALL_OFF,
        EFF_REPLACEMENT_HEATER_DISABLED, EFF_NONE, EFF_MOOT, NUM_EFFECTS };
    static const EffectE k4_k5_k6_reset[] = { EFF_UNKNOWN, EFF_RHM,
        EFF_REPLACEMENT_HEATER_ENABLED, EFF_NONE, EFF_MOOT, NUM_EFFECTS };
    static const EffectE k7_k8_switch[] = { EFF_UNKNOWN, EFF_DSS_A, EFF_DSS_B,
        EFF_NONE, NUM_EFFECTS };
    static const EffectE k9_k10_switch[] = { EFF_UNKNOWN, EFF_TWTA_1,
        EFF_TWTA_2, EFF_NONE, NUM_EFFECTS };
    static const EffectE k11_k12_switch[] = { EFF_UNKNOWN, EFF_HVPS_ON,
        EFF_HVPS_OFF, EFF_NONE, NUM_EFFECTS };
    static const EffectE k13_k14_switch[] = { EFF_UNKNOWN,
        EFF_SPARE_HEATER_ENABLED, EFF_SPARE_HEATER_DISABLED, EFF_NONE,
        NUM_EFFECTS };
    static const EffectE mode_sbm[] = { EFF_UNKNOWN, EFF_SBM, EFF_HVPS_OFF,
        EFF_NONE, NUM_EFFECTS };
    static const EffectE mode_rom[] = { EFF_UNKNOWN, EFF_ROM, EFF_NONE,
        NUM_EFFECTS };
    static const EffectE mode_ccm[] = { EFF_UNKNOWN, EFF_CCM, EFF_NONE,
        NUM_EFFECTS };
    static const EffectE mode_dbm[] = { EFF_UNKNOWN, EFF_DBM, EFF_NONE,
        NUM_EFFECTS };
    static const EffectE mode_wom[] = { EFF_UNKNOWN, EFF_WOM, EFF_NONE,
        NUM_EFFECTS };
    static const EffectE bin_load[] = { EFF_UNKNOWN,
        EFF_NEW_BINNING_CONSTANTS, NUM_EFFECTS };
    static const EffectE ant_seq[] = { EFF_UNKNOWN, EFF_NEW_ANTENNA_SEQUENCE,
        NUM_EFFECTS };
    static const EffectE wts1[] = { EFF_UNKNOWN, EFF_WTS_1, NUM_EFFECTS };
    static const EffectE wts2[] = { EFF_UNKNOWN, EFF_WTS_2, NUM_EFFECTS };
    static const EffectE mon[] = { EFF_UNKNOWN, EFF_RFS_MONITOR_CHANGE,
        NUM_EFFECTS };
    static const EffectE ovr[] = { EFF_UNKNOWN,
        EFF_TWTA_TRIP_OVERRIDE_ENABLED, NUM_EFFECTS };
#if 0
    static const EffectE caret[] = { EFF_UNKNOWN,
        EFF_HVPS_OFF, NUM_EFFECTS };
#endif
    static const EffectE none[] = { NUM_EFFECTS };
    
    //--------------------------------------
    // unknown commands match with anything 
    //--------------------------------------

    if (command_id == EA_CMD_UNKNOWN)
        return(1);

    //-------------------------------------
    // select the appropriate effect array 
    //-------------------------------------

    // select the array to use
    const EffectE* effect_array;
    switch(command_id)
    {
        //--------------
        // power relays 
        //--------------

        case CMD_K1S: case CMD_K2S: case CMD_K3S:
            effect_array = k1_k2_k3_set;
            break;
        case CMD_K1R: case CMD_K2R: case CMD_K3R:
            effect_array = k1_k2_k3_reset;
            break;

        //---------------------------
        // replacement heater relays 
        //---------------------------

        case CMD_K4S: case CMD_K5S: case CMD_K6S:
            effect_array = k4_k5_k6_set;
            break;
        case CMD_K4R: case CMD_K5R: case CMD_K6R:
            effect_array = k4_k5_k6_reset;
            break;

        //------------
        // dss relays 
        //------------

        case CMD_K7S: case CMD_K7R: case CMD_K8S: case CMD_K8R:
            effect_array = k7_k8_switch;
            break;

        //-------------
        // twta relays 
        //-------------

        case CMD_K9S: case CMD_K9R: case CMD_K10S: case CMD_K10R:
            effect_array = k9_k10_switch;
            break;

        //-------------
        // hvps relays 
        //-------------

        case CMD_K11S: case CMD_K11R: case CMD_K12S: case CMD_K12R:
            effect_array = k11_k12_switch;
            break;

        //---------------------
        // spare heater relays 
        //---------------------

        case CMD_K13S: case CMD_K13R: case CMD_K14S: case CMD_K14R:
            effect_array = k13_k14_switch;
            break;

        //-------
        // modes 
        //-------

        case CMD_SBM:
            effect_array = mode_sbm;
            break;
        case CMD_ROM:
            effect_array = mode_rom;
            break;
        case CMD_CCM:
            effect_array = mode_ccm;
            break;
        case CMD_DBM:
            effect_array = mode_dbm;
            break;
        case CMD_WOM:
            effect_array = mode_wom;
            break;

        //-----------------------------------------
        // binning constants and antenna sequences 
        //-----------------------------------------

        case CMD_BIN:
            effect_array = bin_load;
            break;
        case CMD_ANT:
            effect_array = ant_seq;
            break;

        //-----
        // wts 
        //-----

        case CMD_WTS1:
            effect_array = wts1;
            break;
        case CMD_WTS2:
            effect_array = wts2;
            break;

        //----------
        // monitors 
        //----------

        case CMD_MON:
            effect_array = mon;
            break;
        case CMD_OVR:
            effect_array = ovr;
            break;

        //---------------
        // anything else 
        //---------------

        default:
            effect_array = none;
            break;
    }

    //------------------
    // check the effect 
    //------------------

    for (int i = 0; effect_array[i] != NUM_EFFECTS; i++)
    {
        if (effect_array[i] == effect_id)
            return(1);
    }
#endif
    return(0);

}//Command::Matchable

//------------------
// UpdateWithEffect 
//------------------

int
Command::UpdateWithEffect(
    Command*    effect_cmd)
{
    // make sure the command is matchable
    if (! Matchable(commandId, effect_cmd->effectId))
        return(0);

    // verify that a single time is given for the effect
    int verify_count = 0;
    if (effect_cmd->l1Time != INVALID_TIME)
        verify_count++;
    if (effect_cmd->hkdtTime != INVALID_TIME)
        verify_count++;
    if (effect_cmd->l1apTime != INVALID_TIME)
        verify_count++;
    if (verify_count != 1)
        return(0);

    // don't stomp over any effect other than unknown
    if (effectId != EFF_UNKNOWN && effectId != effect_cmd->effectId)
        return(0);

    if (effect_cmd->l1Time != INVALID_TIME)
    {
        if (l1Time == INVALID_TIME)
        {
            l1Time = effect_cmd->l1Time;
            effectId = effect_cmd->effectId;
            l1Verify = VER_YES;
            return(1);
        }
        else
            return(0);
    }
    if (effect_cmd->hkdtTime != INVALID_TIME)
    {
        if (hkdtTime == INVALID_TIME)
        {
            hkdtTime = effect_cmd->hkdtTime;
            effectId = effect_cmd->effectId;
            hkdtVerify = VER_YES;
            return(1);
        }
        else
            return(0);
    }
    if (effect_cmd->l1apTime != INVALID_TIME)
    {
        if (l1apTime == INVALID_TIME)
        {
            l1apTime = effect_cmd->l1apTime;
            effectId = effect_cmd->effectId;
            l1apVerify = VER_YES;
            return(1);
        }
        else
            return(0);
    }
    return(0);
}

//-----------
// Effective 
//-----------
// returns 1 if the command is effective (actually has or will do something)
// returns 0 otherwise

int
Command::Effective()
{
    if (commandId == EA_CMD_UNKNOWN || commandId == EA_CMD_NONE)
        return(0);

    if (statusId == STA_OK ||
        statusId == STA_UNEXPECTED ||
        statusId == STA_ANOMALOUS)
    {
        return(1);
    }

    return(0);
}

//--------------
// _WriteMarker 
//--------------

void
Command::_WriteMarker(
    FILE*   ofp,
    char*   marker)
{
    fprintf(ofp, "%s\n", marker);
    return;
}

//-----------
// _WriteInt 
//-----------

void
Command::_WriteInt(
    FILE*   ofp,
    char*   label,
    int     value)
{
    fprintf(ofp, "%s", label);
    if (value == INVALID_INT)
        fprintf(ofp, "\n");
    else
        fprintf(ofp, " %d\n", value);
    return;
}

//-----------
// _WriteTpg 
//-----------

void
Command::_WriteTpg(
    FILE*   ofp,
    char*   label,
    Tpg     tpg)
{
    fprintf(ofp, "%s", label);
    char tpg_string[TPG_LEN];
    if (tpg.TpgToString(tpg_string))
        fprintf(ofp, " %s\n", tpg_string);
    else
        fprintf(ofp, "\n");
    return;
}

//--------------
// _WriteString 
//--------------

void
Command::_WriteString(
    FILE*   ofp,
    char*   label,
    char*   string)
{
    fprintf(ofp, "%s", label);
    if (string)
        fprintf(ofp, " %s\n", string);
    else
        fprintf(ofp, "\n");
    return;
}

//------------
// _WriteTime 
//------------

void
Command::_WriteTime(
    FILE*   ofp,
    char*   label,
    Itime   itime)
{
    fprintf(ofp, "%s", label);
    if (itime == INVALID_TIME)
        fprintf(ofp, "\n");
    else
    {
        char string[CODEA_TIME_LEN];
        itime.ItimeToCodeA(string);
        fprintf(ofp, " %s\n", string);
    }
    return;
}

//-------------
// _ReadMarker 
//-------------

int
Command::_ReadMarker(
    FILE*   ifp,
    char*   marker)
{
    char line[256];
    if (fgets(line, 256, ifp) != line)
    {
        if (feof(ifp))
            return (-1);
        else
            return (0);
    }

    char marker_string[256];
    int retval = sscanf(line, " %s", marker_string);
    switch (retval)
    {
    case 1:
        if (strcmp(marker_string, marker) != 0)
            return (0);
        else
            return (1);
        break;
    default:
        return (0);
    }
}

//----------
// _ReadInt 
//----------

int
Command::_ReadInt(
    FILE*   ifp,
    char*   label,
    int*    value)
{
    char line[256];
    if (fgets(line, 256, ifp) != line)
        return (0);

    char label_string[80];
    int retval = sscanf(line, " %s %d", label_string, value);
    switch (retval)
    {
    case 2:
        if (strcmp(label_string, label) != 0)
            return (0);
        else
            return (1);
        break;
    case 1:
        if (strcmp(label_string, label) != 0)
            return (0);
        else
        {
            *value = INVALID_INT;
            return (1);
        }
        break;
    default:
        return (0);
        break;
    }
}

//----------
// _ReadTpg 
//----------

int
Command::_ReadTpg(
    FILE*   ifp,
    char*   label,
    Tpg*    tpg)
{
    char line[256];
    if (fgets(line, 256, ifp) != line)
        return (0);

    char label_string[80], tpg_string[256];
    int retval = sscanf(line, " %s %[^\n]", label_string, tpg_string);
    switch (retval)
    {
    case 2:
        if (strcmp(label_string, label) != 0)
            return (0);
        else
            return (tpg->StringToTpg(tpg_string));
        break;
    case 1:
        if (strcmp(label_string, label) != 0)
            return (0);
        else
        {
            *tpg = INVALID_TPG;
            return (1);
        }
        break;
    default:
        return (0);
        break;
    }
}

//-------------
// _ReadString 
//-------------

int
Command::_ReadString(
    FILE*   ifp,
    char*   label,
    char**  string)
{
    char line[MAX_STRING_SIZE];
    if (fgets(line, MAX_STRING_SIZE, ifp) != line)
        return (0);

    // fget doesn't strip out new-line char, we have to do it
    if (line[strlen(line)-1] == '\n')
        line[strlen(line)-1] = '\0';

    char label_string[80];
    int count;
    int retval = sscanf(line, " %s %n", label_string, &count);

    if (retval != 1)
        return(0);
    if (strcmp(label_string, label) != 0)
        return(0);

    free(*string);
    if (*(line+count) != '\0')
    {
        *string = strdup(line + count);
        if (*string == NULL)
            return(0);
    }
    return(1);
}

//-----------
// _ReadTime 
//-----------

int
Command::_ReadTime(
    FILE*   ifp,
    char*   label,
    Itime*  itime)
{
    char line[256];
    if (fgets(line, 256, ifp) != line)
        return (0);

    char label_string[80], time_string[256];
    int retval = sscanf(line, " %s %s", label_string, time_string);
    switch (retval)
    {
    case 2:
        if (strcmp(label_string, label) != 0)
            return (0);
        else
            return (itime->CodeAToItime(time_string));
        break;
    case 1:
        if (strcmp(label_string, label) != 0)
            return (0);
        else
        {
            *itime = INVALID_TIME;
            return (1);
        }
        break;
    default:
        return (0);
        break;
    }
}

//==============================================
// ROUTINE: MnemonicToCmdId
// PURPOSE: Returns the command id
//          given the mnemonic string. (RER)
//==============================================
// static
EACommandE
Command::MnemonicToCmdId(
    char*   string)
{
    for (int i = 0; i < numCmdArgsEntries; i++)
    {
        if (strcmp(cmdArgsTable[i].mnemonic, string) == 0)
        {
            return(cmdArgsTable[i].cmdID);
        }
    }
    return(EA_CMD_UNKNOWN); 
}

//-----------
// operators 
//-----------

int
operator<(
    const Command& a,
    const Command& b)
{
    return (a.EarliestTime() < b.EarliestTime());
}

int
operator<=(
    const Command& a,
    const Command& b)
{
    Itime atime = a.EarliestTime();
    Itime btime = b.EarliestTime();
    return (atime < btime || atime == btime);
}

int
operator>(
    const Command& a,
    const Command& b)
{
    return (a.EarliestTime() > b.EarliestTime());
}

int
operator>=(
    const Command& a,
    const Command& b)
{
    Itime atime = a.EarliestTime();
    Itime btime = b.EarliestTime();
    return (atime > btime || atime == btime);
}

int
operator==(
    const Command&  a,
    const Command&  b)
{
    // convert to strings
    char* adf = CONVERT_TO_STRING(a.dataFilename);
    char* bdf = CONVERT_TO_STRING(b.dataFilename);
    char* ao = CONVERT_TO_STRING(a.originator);
    char* bo = CONVERT_TO_STRING(b.originator);
    char* ac = CONVERT_TO_STRING(a.comments);
    char* bc = CONVERT_TO_STRING(b.comments);
    return(a.commandId == b.commandId &&
        a.plannedTpg == b.plannedTpg &&
        strcmp(adf, bdf) == 0 &&
        strcmp(ao, bo) == 0 &&
        strcmp(ac, bc) == 0 &&
        a.effectId == b.effectId &&
        a.statusId == b.statusId &&
        a.expectedTime == b.expectedTime &&
        a.l1Time == b.l1Time &&
        a.hkdtTime == b.hkdtTime &&
        a.l1apTime == b.l1apTime &&
        a.l1Verify == b.l1Verify &&
        a.hkdtVerify == b.hkdtVerify &&
        a.l1apVerify == b.l1apVerify &&
        a.binRepetition == b.binRepetition);
}

const char*
Command::_VerifyIdToMnemonic(
const VerifyE       id)
{
    for (int i = 0; cmd_verify_mnemonics[i].mnemonic; i++)
    {
        if (cmd_verify_mnemonics[i].cmdVerifyId == id)
        {
            return(cmd_verify_mnemonics[i].mnemonic);
        }
    }

    return(cmd_verify_mnemonics[0].mnemonic);

} //Command::_VerifyIdToMnemonic

//-----------------------------------------------
// ROUTINE: MnemonicToEffectId
// PURPOSE: Returns the effect id
//          given the mnemonic string.
//-----------------------------------------------
// static
EffectE
Command::MnemonicToEffectId(
char*   string)
{
    for (int i = 0; effect_mnemonics[i].mnemonic; i++)
    {
        if (strcmp(effect_mnemonics[i].mnemonic, string) == 0)
            return(effect_mnemonics[i].effectId);
    }
    return(effect_mnemonics[0].effectId); 

} //Command::MnemonicToEffectId

//-----------------------------------------------
// ROUTINE: MnemonicToStatusId
// PURPOSE: Returns the command status id
//          given the mnemonic string.
//-----------------------------------------------
// static
CmdStatusE
Command::MnemonicToStatusId(
char*   string)
{
    for (int i = 0; cmd_status_mnemonics[i].mnemonic; i++)
    {
        if (strcmp(cmd_status_mnemonics[i].mnemonic, string) == 0)
            return(cmd_status_mnemonics[i].cmdStatusId);
    }
    return(cmd_status_mnemonics[0].cmdStatusId); 

} //Command::MnemonicToStatusId

//-----------------------------------------------
// ROUTINE: MnemonicToVerifyId
// PURPOSE: Returns the command status id
//          given the mnemonic string.
//-----------------------------------------------
// static
VerifyE
Command::MnemonicToVerifyId(
char*   string)
{
    for (int i = 0; cmd_verify_mnemonics[i].mnemonic; i++)
    {
        if (strcmp(cmd_verify_mnemonics[i].mnemonic, string) == 0)
            return(cmd_verify_mnemonics[i].cmdVerifyId);
    }
    return(cmd_verify_mnemonics[0].cmdVerifyId); 

} //Command::MnemonicToVerifyId

//=============================================
// ROUTINE: Get Optional REQI string (repitition, originator and comments)
// PURPOSE: Return a pointer to the string to 
//          be written to the REQI file. 
//=============================================
int
Command::_GetOptionalReqiString(
char*   string)     // user shall provide space
{
    if (string == 0) return 0;

    string[0] = '\0';
    char tempString[BIG_SIZE];

#if 0 // save for SEAWINDS
   //-----------------------------------
   // if is BIN command, write repetition
   //-----------------------------------
   if (commandId == CMD_BIN)
   {
      sprintf(tempString,"REPETITION: %d\n",binRepetition); 
      strcat(string,tempString); 
   }
#endif

   //-----------------------------------
   // if originator is specified, write it
   //-----------------------------------
   if (originator != 0 && originator[0] != 0)
   {
      sprintf(tempString,"ORIGINATOR: %s\n",originator); 
      strcat(string,tempString); 
   }

   //-----------------------------------
   // if comments are specified, write it
   //-----------------------------------
   if (comments != 0 && comments[0] != 0)
   {
      sprintf(tempString,"COMMENTS: %s\n",comments); 
      strcat(string,tempString); 
   }

   return 1;

}//Command::_GetOptionalReqiString

//=============================================
// ROUTINE: Read Optional REQI string (repitition, originator and comments)
//          Return TRUE if ok
//=============================================
int
Command::ReadReqiOptionalString(
int           lineNo,     // line number
const char*   string)     // optional string (with ending '\n')
{
    if (string == 0) return 0;

    char tempString[BIG_SIZE];

#if 0 // save for SEAWINDS
   //-----------------------------------
   // if is BIN command, write repetition
   //-----------------------------------
   if (commandId == CMD_BIN)
   {
      sprintf(tempString,"REPETITION: %d\n",binRepetition); 
      strcat(string,tempString); 
   }
#endif

   //-----------------------------------
   // try to read it in as originator
   //-----------------------------------
   if (originator == 0 &&
               sscanf(string," ORIGINATOR:%[^\n]", tempString) == 1)
   {
       SetOriginator(tempString);
       return 1;
   }
   //-----------------------------------
   // try to read it in as comments
   //-----------------------------------
   else if (comments == 0 &&
                  sscanf(string," COMMENTS:%[^\n]", tempString) == 1)
   {
       SetComments(tempString);
       return 1;
   }
#if 0 // save for SEAWINDS
            else if (sscanf(ptr, " REPETITION:%d", &rep_count) == 1)
            {
                if (commandId != CMD_BIN)
                {
                    fprintf(stderr, "\nRepetition not allowed for %s commands at line %d\n", CmdString(), lineNo);
                    return(_status = ERROR_READING_CMD);
                }
                if (rep_count != 1 && rep_count != 2)
                {
                    fprintf(stderr, "\nRepetition must be 1 or 2 (line %d)\n",
                        lineNo);
                    return(_status = ERROR_READING_CMD);
                }
                binRepetition = (unsigned char)rep_count;
            }
#endif  // save for SEAWINDS

    fprintf(stderr, "\nLine %d: %s: unrecognized syntax\n", lineNo, string);
    return 0;

}//Command::ReadReqiOptionalString

int
Command::NeedDataFile(
EACommandE commandID)
{
    for (int i=0; i < numCmdArgsEntries; i++)
    {
        if (commandID == cmdArgsTable[i].cmdID &&
                               cmdArgsTable[i].numWordsInParamFile != 0)
            return 1;
    }
    return 0;

} // Command::NeedDataFile

int
Command::RealtimeOnly(
EACommandE commandID)
{
    for (int i=0; i < numCmdArgsEntries; i++)
    {
        if (commandID == cmdArgsTable[i].cmdID && cmdArgsTable[i].isRealtime)
            return 1;
    }
    return 0;

} // Command::RealtimeOnly

int
Command::NumWordsInParamFile(
EACommandE commandID)
{
    for (int i=0; i < numCmdArgsEntries; i++)
    {
        if (commandID == cmdArgsTable[i].cmdID && cmdArgsTable[i].isRealtime)
            return cmdArgsTable[i].numWordsInParamFile;
    }
    return 0;

} // Command::NumWordsInParamFile
