//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.18   01 Sep 1998 16:39:18   sally
// change operator== to Match in order to pass outFP
// 
//    Rev 1.17   31 Aug 1998 14:41:50   sally
// added REQQ-like PBI commands
// 
//    Rev 1.16   28 Aug 1998 16:31:00   sally
// use REQA's file number to match REQQ file 
// 
//    Rev 1.15   28 Aug 1998 11:20:42   sally
// create ReqqRecordList for matching with REQA
// 
//    Rev 1.14   09 Jul 1998 16:23:04   sally
// catch the output of process_reqi in an output file
// 
//    Rev 1.13   09 Jul 1998 09:29:12   sally
// change format per Lee's memo
// 
//    Rev 1.12   01 Jul 1998 13:13:22   sally
// added embedded commands checking
// 
//    Rev 1.11   08 Jun 1998 13:37:50   sally
// added EraseFile()
// 
//    Rev 1.9   29 May 1998 15:26:58   sally
// adapted to the new REQI datafile format, per Lee's memo
// 
//    Rev 1.8   28 May 1998 09:27:02   sally
// update the formats for REQQ, QPF and RTCF
// 
//    Rev 1.7   26 May 1998 10:55:14   sally
// added QPF
// 
//    Rev 1.6   19 May 1998 14:48:22   daffer
// Redefined ReqqStatusE
// 
//    Rev 1.5   18 May 1998 15:32:20   daffer
// Cleaned up some return status
// 
//    Rev 1.4   30 Apr 1998 14:29:36   daffer
// Added status returns
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
#define REQQ_FILE_FORMAT_VERSION_DATE      "19980701"
#define REQQ_FILE_FORMAT_VERSION_NUMBER    0
#define REQQ_RESERVED                      ""
#define REQQ_SENSOR_NAME                   "QSCAT"
#define REQQ_NULL_PATH_STRING              "*****"
#define REQQ_NULL_GAMMA_STRING             "******"

#define REQQ_MODE                          0444

enum ReqqStatusE {
    REQQ_OK,
    REQQ_ERROR,
    REQQ_OUT_OF_MEMORY,
    REQQ_AUTO_NOT_ALLOWED,
    REQQ_CMD_NOT_ALLOWED,
    REQQ_OPEN_FAILURE,
    REQQ_FILE_EXISTS,
    REQQ_ERROR_READ_DATAFILE,
    REQQ_ERROR_WRITE_PARAMS,
    REQQ_EMPTY_FILE,
    REQQ_UNLINK_FILE_FAILED,
    REQQ_CHMOD_FAILED,
    REQQ_ERROR_READ_PARAM_STRING,
    REQQ_EOF,
    REQQ_STATUS_LAST     // boundary, not a status
};
//============
// Reqq class 
//============

class ReqqRecordList;
class Reqa;

class Reqq
{
public:

    friend class Reqa;

    // process from REQI file to REQQ files
    Reqq(Itime        start_time,
         Itime        end_time,
         const char*  reqq_directory,
         int          file_number,
         int          useCurrentNumber,
         int          cmd_id,
         int          last_mode_cmd_id);

    // read from REQQ file
    Reqq(const char*  filename);   // reqq file path + name

    // read from REQQ file given REQQ directory and file number
    Reqq(const char*  reqqDirectory,  // reqq directory
         int          fileNumber);    // reqq file number

    virtual ~Reqq();

    ReqqStatusE     Write(CmdList*   cmd_list,
                          int        check_for_embedded,
                          FILE*      outputFP);

    const char*     ReqqMnemonic(Command* cmd);

    int             GetFileNumber() const { return(_fileNumber); };
                    // return ' ', 'B', 'C', ...
    char            GetFileVersion() const { return(_fileVersion); };
    const char*     GetFilename() const { return _filename; };
            
    int             GetCmdId() { return(_cmdId); };
    int             GetLastModeCmdId() { return(_lastModeCmdId); };
    ReqqStatusE     GetStatus() { return (_status);};
    
    static ReqqStatusE  ReadParamWords(
                              const char*      filename,
                              int              numParamWords,
                              EADataFileFormat datafileFormat,
                              unsigned short&  checksum,
                              char*            hexString,   // IN/OUT
                              FILE*            outputFP);

    // erase the previously created REQQ file
    // process REQI creates REQQ, QPF and RTCF, if any fails,
    // all files should be erased
    ReqqStatusE     EraseFile(void);

    static const char*  GetStatusString(ReqqStatusE status);
            
protected:
    ReqqStatusE     _WriteRecord(Command*   cmd,
                                 int        check_for_embedded,
                                 FILE*      outputFP);
    ReqqStatusE     _ModeCommand(Command* cmd);
    ReqqStatusE     _OpenFile(FILE* outputFP);
    ReqqStatusE     _WriteHeader();
    void            _Read(const char* filename);

    Itime           _beginDateOfRequest;
    Itime           _endDateOfRequest;
    char*           _directory;
    char*           _filename;
    int             _firstFileNumber;
    int             _fileNumber;
    char            _fileVersion;
    int             _cmdId;
    int             _lastModeCmdId;
    int             _recordCount;
    ReqqStatusE     _status;
    FILE*           _ofp;

    ReqqRecordList* _reqqList;
};

//==================
// Helper Functions 
//==================

ReqqStatusE ReadAntennaSequence(const char* filename, char* ascii_string,
        char* hex_string);

#endif
