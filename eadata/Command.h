//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
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

#include "CommonDefs.h"
#include "Tpg.h"

//-----------
// CONSTANTS 
//-----------

#define CMD_START_MARKER    "----START----"
#define COMMAND_ID_LABEL    "Cmd_ID:"
#define PLANNED_TPG_LABEL   "Planned_TPG:"
#define DATA_FILENAME_LABEL "Data_Filename:"
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
#define CMD_END_MARKER      "-----END-----"

#define INVALID_INT         -1
#define MAX_STRING_SIZE     1024
#define MAX_LINE_SIZE       1024

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
    EA_CMD_NONE = -1,
    EA_CMD_UNKNOWN = 0,
    EA_CMD_RGAWID = 0X0F03,
    EA_CMD_RGBWID = 0X0F05,
    EA_CMD_TRPWID = 0X0F06,
    EA_CMD_PRFCLK = 0X0F07,
    EA_CMD_RCVGAN = 0X0F08,
    EA_CMD_GRDNOR = 0X0F0A,
    EA_CMD_GRDDIS = 0X0F0B,
    EA_CMD_RCVPON = 0X0F0C,
    EA_CMD_RCVPNR = 0X0F0D,
    EA_CMD_TOVRON = 0X0F0E,
    EA_CMD_TOVROF = 0X0F0F,
    EA_CMD_TMDONN = 0X0F10,
    EA_CMD_TMDOFF = 0X0F11,
    EA_CMD_TWTTRS = 0X0F12,
    EA_CMD_SESRES = 0X0F13,
    EA_CMD_SMLDIS = 0X0F3A,
    EA_CMD_HTRDIS = 0X0F3B,
    EA_CMD_TWTMND = 0X0F3D,
    EA_CMD_CALPER = 0X0F40,
    EA_CMD_RSRLPR = 0X0F41,
    EA_CMD_PATENB = 0X0F50,
    EA_CMD_PATDIS = 0X0F51,
    EA_CMD_RGTPWD = 0X0F52,
    EA_CMD_ORBTIC = 0X0FA0,
    EA_CMD_ITMSYN = 0X0FA1,
    EA_CMD_SSTSWH = 0X0FBA,
    EA_CMD_SCLSWH = 0X0FBB,
    EA_CMD_SWOSWH = 0X0FBD,
    EA_CMD_SCMSWH = 0X0FBE,
    EA_CMD_SRCSWH = 0X0FBF,
    EA_CMD_MROSTR = 0X0FC0,
    EA_CMD_TBLSTR = 0X0FC1,
    EA_CMD_SEQUPP = 0X0FC2,
    EA_CMD_SEQLOW = 0X0FC3,
    EA_CMD_BMOFST = 0X0FD0,
    EA_CMD_SASOFS = 0X0FD1,
    EA_CMD_SSLENB = 0X0FD2,
    EA_CMD_SSLDIS = 0X0FD3,
    EA_CMD_BRASWH = 0X0FD6,
    EA_CMD_BRBSWH = 0X0FD7,
    EA_CMD_BDASWH = 0X0FE2,
    EA_CMD_BDBSWH = 0X0FE3,
    EA_CMD_ENGSWH = 0X0FE8,
    EA_CMD_STASWH = 0X0FE9,
    EA_CMD_PSTSWH = 0X0FEA,
    EA_CMD_PCLSWH = 0X0FEB,
    EA_CMD_PRCSWH = 0X0FEC,
    EA_CMD_PWOSWH = 0X0FED,
    EA_CMD_NO__OP = 0X0FF7,
    EA_CMD_SMLENB = 0X0FFA,
    EA_CMD_HTRENB = 0X0FFB,
    EA_CMD_TWTMNE = 0X0FFD,
    EA_CMD_SSTTBL = 0X55CA,
    EA_CMD_SCLTBL = 0X55CB,
    EA_CMD_SRCTBL = 0X55CC,
    EA_CMD_SWOTBL = 0X55CD,
    EA_CMD_SCMTBL = 0X55CE,
    EA_CMD_BRATBL = 0X55E6,
    EA_CMD_BRBTBL = 0X55E7,
    EA_CMD_BDATBL = 0X55F2,
    EA_CMD_BDBTBL = 0X55F3,
    EA_CMD_ENGTBL = 0X55F8,
    EA_CMD_STATBL = 0X55F9,
    EA_CMD_PSTTBL = 0X55FA,
    EA_CMD_PCLTBL = 0X55FB,
    EA_CMD_PRCTBL = 0X55FC,
    EA_CMD_PWOTBL = 0X55FD,
    EA_CMD_K9RESS = 0XAA01,
    EA_CMD_K10RES = 0XAA02,
    EA_CMD_K11RES = 0XAA04,
    EA_CMD_K19RES = 0XAA07,
    EA_CMD_K12RES = 0XAA08,
    EA_CMD_K25RES = 0XAA0B,
    EA_CMD_SSBRES = 0XAA0D,
    EA_CMD_K20RES = 0XAA0E,
    EA_CMD_K15RES = 0XAA10,
    EA_CMD_K26RES = 0XAA13,
    EA_CMD_K21RES = 0XAA19,
    EA_CMD_K23RES = 0XAA1C,
    EA_CMD_K16RES = 0XAA20,
    EA_CMD_SSARES = 0XAA23,
    EA_CMD_K22RES = 0XAA31,
    EA_CMD_K24RES = 0XAA38,
    EA_CMD_K9SETT = 0XAAC1,
    EA_CMD_K10SET = 0XAAC2,
    EA_CMD_K11SET = 0XAAC4,
    EA_CMD_K19SET = 0XAAC7,
    EA_CMD_K12SET = 0XAAC8,
    EA_CMD_K25SET = 0XAACB,
    EA_CMD_SSBSET = 0XAACD,
    EA_CMD_K20SET = 0XAACE,
    EA_CMD_K15SET = 0XAAD0,
    EA_CMD_K26SET = 0XAAD3,
    EA_CMD_K21SET = 0XAAD9,
    EA_CMD_K23SET = 0XAADC,
    EA_CMD_K16SET = 0XAAE0,
    EA_CMD_SSASET = 0XAAE3,
    EA_CMD_K22SET = 0XAAF1,
    EA_CMD_K24SET = 0XAAF7,
    EA_CMD_SWPTCH = 0XE3E3,
    EA_CMD_MODCAL = 0XF007,
    EA_CMD_MODWOM = 0XF00E,
    EA_CMD_MODSTB = 0XF070,
    EA_CMD_CDSRES = 0XF093,
    EA_CMD_MODRCV = 0XF0E0
};

enum EffectE
{
    EFF_UNKNOWN = 0,
    EFF_ALL_OFF, EFF_ELECTRONICS_ON, EFF_RHM,
    EFF_REPLACEMENT_HEATER_ENABLED, EFF_REPLACEMENT_HEATER_DISABLED,
    EFF_DSS_A, EFF_DSS_B,
    EFF_SPARE_HEATER_ENABLED, EFF_SPARE_HEATER_DISABLED,
    EFF_SBM, EFF_ROM, EFF_CCM, EFF_DBM, EFF_WOM,
    EFF_TWTA_1, EFF_TWTA_2,
    EFF_HVPS_ON, EFF_HVPS_OFF,
    EFF_NEW_BINNING_CONSTANTS, EFF_NEW_ANTENNA_SEQUENCE,
    EFF_WTS_1, EFF_WTS_2, EFF_TWTA_TRIP_OVERRIDE_ENABLED,
    EFF_RFS_MONITOR_CHANGE,
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

struct CmdMnemonics
{
    EACommandE  commandId;
    char*       mnemonic;
};

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

struct EACommandArgsEntry
{
    EACommandE   cmdID;          // command ID
    char*        mnemonic;
    char*        description;
    int          numWordsInParamFile; // 0=no datafile, -1=variable length
    char         isRealtime;     // real time command takes no arg of latitude
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

    //---------
    // methods 
    //---------

    Command(int lineNo, const char*  reqiCommandString);
    Command();
    virtual ~Command();

    StatusE     GetStatus() { return(_status); };
    StatusE     Read(FILE* ifp);
    StatusE     Write(FILE* ofp);
    StatusE     WriteForHumans(FILE* ofp);

    int         ReadReqiCommandString(int lineNo, const char* string);
    int         ReadReqiOptionalString(int lineNo, const char* string);
    int         SetDataFilename(const char* string);
    int         SetOriginator(const char* string);
    int         SetComments(const char* string);

    void          SetCommandType(CommandTypeE commandType)
                            { _commandType = commandType; }
    CommandTypeE  GetCommandType(void) const {return _commandType; }
    void          SetFormat(FormatE format) { _format = format; }
    FormatE       GetFormat(void) const {return _format; }
    void          SetNumWordsInParamFile(int numWords)
                      { _numWordsInParamFile = numWords; }
    int           GetNumWordsInParamFile(void) const
                      {return _numWordsInParamFile; }

    int           ToReqiString(char* string);

    const char* CmdString();
    const char* EffectString(); 
    const char* GetEffectString(EffectE targetEffectId);
    const char* GetCmdTypeString(CmdTypeE targetCmdType);
    const char* CmdStatusString(); 
    const char* CmdStatusAbbrev(); 

    const char* L1AVerifyString(){return(_VerifyIdToMnemonic(l1Verify));}; 
    const char* HkdtVerifyString(){return(_VerifyIdToMnemonic(hkdtVerify));}; 
    const char* L1ApVerifyString(){return(_VerifyIdToMnemonic(l1apVerify));}; 

    static EACommandE MnemonicToCmdId(char* string); // mnemonic -> commandId
    static EffectE    MnemonicToEffectId(char* string); // mnemonic -> effectId
    static CmdStatusE MnemonicToStatusId(char* string); // mnemonic -> statusId
    static VerifyE    MnemonicToVerifyId(char* string); // mnemonic -> verifyId
    static int        Matchable(EACommandE command_id, EffectE effect_id);


    Itime       EarliestTime() const;
    Itime       LatestTime() const;
    Itime       BestTime() const;

    CmdTypeE    CmdType();

    int         UpdateWithEffect(Command* effect_cmd);
    int         Effective();

    static int  NeedDataFile(EACommandE commandID);
    static int  RealtimeOnly(EACommandE commandID);
    static int  NumWordsInParamFile(EACommandE commandID);

    //-----------
    // variables 
    //-----------

    EACommandE  commandId;
    Tpg         plannedTpg;
    char*       dataFilename;
    char*       originator;
    char*       comments;

    EffectE     effectId;
    CmdStatusE  statusId;
    Itime       expectedTime;

    Itime       l1Time;
    Itime       hkdtTime;
    Itime       l1apTime;

    VerifyE     l1Verify;
    VerifyE     hkdtVerify;
    VerifyE     l1apVerify;

    unsigned char   binRepetition;      // only used for reqi processing

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
    int         _ReadTpg(FILE* ifp, char* label, Tpg* tpg);
    int         _ReadString(FILE* ifp, char* label, char** string);
    int         _ReadTime(FILE* ifp, char* label, Itime* itime);

                //Get Optional REQI string (repitition, originator and comments)
    int         _GetOptionalReqiString(char* string);

    static const char*  _VerifyIdToMnemonic(const VerifyE id);

    //-----------
    // variables 
    //-----------

    StatusE        _status;
    CommandTypeE   _commandType;
    FormatE        _format;
    int            _numWordsInParamFile; // 0=no datafile, -1=variable length
};

int operator<(const Command&, const Command&);
int operator<=(const Command&, const Command&);
int operator>(const Command&, const Command&);
int operator>=(const Command&, const Command&);
int operator==(const Command&, const Command&);

#endif
