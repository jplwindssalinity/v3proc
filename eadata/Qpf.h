//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.2   13 Apr 1998 14:30:56   sally
// allocate space for _directory and _filename
// 
//    Rev 1.1   10 Apr 1998 14:04:18   daffer
//   Added GetFilename Method to be used in process_reqi.C
// 
//    Rev 1.0   17 Mar 1998 14:42:04   sally
// Initial revision.
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

// This file outlines the Qpf object
// The Qpf object will exit on error

#ifndef QPF_H
#define QPF_H

#include "Itime.h"
#include "CmdList.h"

static const char rcs_id_npf_h[] =
    "@(#) $Header$";

#define QPF_PROJECT_NAME                "QSCAT "
#define QPF_AO_PROVIDER_CODE            "JPL "
#define QPF_NASDA_TACC_CODE             "LASP"
#define QPF_LENGTH_OF_DATA_RECORD       320
#define QPF_NUMBER_OF_DATA_RECORDS      12
#define QPF_FILE_FORMAT_VERSION_DATE    "19950601"
#define QPF_FILE_FORMAT_VERSION_NUMBER  1
#define QPF_DATA_SPECIFICATION          "BIN"
#define QPF_RESERVED                    ""

#define QPF_MODE                        0444

//===========
// Qpf class 
//===========

class Qpf
{
public:
    Qpf(Itime start_time, Itime end_time, const char* npf_directory,
        int file_number);
    ~Qpf();

    int         Write(CmdList* cmd_list);
    int         GetFileNumber() const { return(_fileNumber); };
    const char* GetFilename() const { return _filename; }

private:
    int         _WriteQpfFile(Command* cmd);

    Itime       _beginDateOfRequest;
    Itime       _endDateOfRequest;
    char*       _directory;
    char*       _filename;
    int         _fileNumber;

};

/*

class Qpf
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
