//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   04 Feb 1998 14:16:32   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:33  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

// This file outlines the Npf object
// The Npf object will exit on error

#ifndef NPF_H
#define NPF_H

#include "Itime.h"
#include "CmdList.h"

static const char rcs_id_npf_h[] =
    "@(#) $Header$";

#define NPF_PROJECT_NAME                "ADEOS "
#define NPF_AO_PROVIDER_CODE            "JPL "
#define NPF_NASDA_TACC_CODE             "TACC"
#define NPF_LENGTH_OF_DATA_RECORD       320
#define NPF_NUMBER_OF_DATA_RECORDS      12
#define NPF_FILE_FORMAT_VERSION_DATE    "19950601"
#define NPF_FILE_FORMAT_VERSION_NUMBER  1
#define NPF_DATA_SPECIFICATION          "BIN"
#define NPF_RESERVED                    ""

#define NPF_MODE                        0444

//===========
// Npf class 
//===========

class Npf
{
public:
    Npf(Itime start_time, Itime end_time, const char* npf_directory,
        int file_number);
    ~Npf();

    int         Write(CmdList* cmd_list);
    int         GetFileNumber() { return(_fileNumber); };

private:
    int         _WriteNpfFile(Command* cmd);

    Itime       _beginDateOfRequest;
    Itime       _endDateOfRequest;
    const char* _directory;
    int         _fileNumber;
};

/*

class Npf
{
public:
    Reqq(Itime start_time, Itime end_time, int file_number, int cmd_id,
        int last_mode_cmd_id);
    ~Reqq();

    int         Write(CmdList* cmd_list);
    const char* ReqqMnemonic(Command* cmd);

    int         GetFileNumber() { return(_fileNumber); };
    int         GetCmdId() { return(_cmdId); };
    int         GetLastModeCmdId() { return(_lastModeCmdId); };

private:
    int     _WriteRecord(Command* cmd);
    int     _ModeCommand(Command* cmd);
    int     _CanBeInReqq(Command* cmd);
    int     _OpenFile();
    int     _WriteHeader();

    int     _cmdId;
    int     _lastModeCmdId;
    int     _recordCount;
    FILE*   _ofp;
};

//==================
// Helper Functions 
//==================

int ReadAntennaSequence(const char* filename, char* ascii_string,
        char* hex_string);
*/

#endif
