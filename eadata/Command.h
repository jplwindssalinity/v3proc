//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.28   09 Apr 1999 13:58:38   sally
// read number of words from files if the table length is variable
// 
//    Rev 1.27   25 Mar 1999 14:04:20   daffer
// Incorporated Uploadable table support
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
#include "Command_1.h"
#include "UpldTbl.h"
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
#define L1AP_VERIFY_LABEL   "L1AP_Verify:"
#define EFFECT_VALUE_LABEL  "Effect_Value:"
#define TABLE_TYPE_LABEL      "Table_Type:"
#define TABLE_ID_LABEL      "Table_ID:"
#define QPX_FILENAME_LABEL  "Qpx_Filename:"
#define UQPX_LABEL          "UQpx_Filename:"
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

// enum EACommandE is now in Command_1.h



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

    void              SetNumWordsInParamFile(int numWords);
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
    char*           qpx_filename; //
    char*           uqpx_filename; //SES tables can change without 
                                   //table commanding.

    EffectE         effectId;
    unsigned short int effect_value; // For Command History Queue, stores 
                                     // cmdHex value of command in history queue.

    CmdStatusE  statusId;
    EAUpldTbl_TypeE  table_type;
    EAUpldTblE       tableid;
    EAUpldTbl_Table_StatusE table_status;
    //    short int * table; // For table uploads.
    //    char      * filled; // uploaded tables; 1=filled, 0=not.

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

    UpldTbl         *tbl;

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
