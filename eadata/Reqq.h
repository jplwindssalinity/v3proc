//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.3   13 Apr 1998 14:30:58   sally
// allocate space for _directory and _filename
// 
//    Rev 1.2   10 Apr 1998 14:04:20   daffer
//   Added GetFilename Method to be used in process_reqi.C
// 
//    Rev 1.1   17 Mar 1998 14:41:38   sally
// changed for REQQ
// 
//    Rev 1.0   04 Feb 1998 14:17:02   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:42  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

// This file outlines the Reqq object
// The Reqq object will exit on error

#ifndef REQQ_H
#define REQQ_H

#include <stdio.h>
#include "Command.h"
#include "CmdList.h"

static const char rcs_id_reqq_h[] =
    "@(#) $Header$";

// ADEOS constants
#define REQQ_PROJECT_NAME                  "QSCAT "
#define REQQ_AO_PROVIDER_CODE              "JPL "
#define REQQ_EOC_MMO_CODE                  "LASP"
#define REQQ_DATA_RECORD_LENGTH            256
#define REQQ_FILE_FORMAT_VERSION_DATE      "19950601"
#define REQQ_FILE_FORMAT_VERSION_NUMBER    1
#define REQQ_RESERVED                      ""
#define REQQ_SENSOR_NAME                   "QSCAT "

#define REQQ_MODE                          0444

//============
// Reqq class 
//============

class Reqq
{
public:
    enum ReqqCmdTypeE
    {
        REQQ, NOT_REQQ, ERROR
    };

    Reqq(Itime        start_time,
         Itime        end_time,
         const char*  reqq_directory,
         int          file_number,
         int          cmd_id,
         int          last_mode_cmd_id);

    virtual ~Reqq();

    int             Write(CmdList* cmd_list);
    const char*     ReqqMnemonic(Command* cmd);

    int             GetFileNumber() const { return(_fileNumber); };
    const char*     GetFilename() const { return _filename; };
            
    int             GetCmdId() { return(_cmdId); };
    int             GetLastModeCmdId() { return(_lastModeCmdId); };

private:
    int             _WriteRecord(Command* cmd);
    int             _ModeCommand(Command* cmd);
    int             _OpenFile();
    int             _WriteHeader();
    ReqqCmdTypeE    _CmdType(Command* cmd);

    Itime           _beginDateOfRequest;
    Itime           _endDateOfRequest;
    char*           _directory;
    char*           _filename;
    int             _fileNumber;
    int             _cmdId;
    int             _lastModeCmdId;
    int             _recordCount;
    FILE*           _ofp;
};

//==================
// Helper Functions 
//==================

int ReadAntennaSequence(const char* filename, char* ascii_string,
        char* hex_string);

#endif
