//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.9   08 Jun 1998 13:38:04   sally
// added EraseFile()
// 
//    Rev 1.8   03 Jun 1998 17:20:36   daffer
// Added _last_msg variable, GetLastMsg and error message throughout
// 
//    Rev 1.7   29 May 1998 15:27:04   sally
// adapted to the new REQI datafile format, per Lee's memo
// 
//    Rev 1.6   28 May 1998 09:27:08   sally
// update the formats for REQQ, QPF and RTCF
// 
//    Rev 1.5   19 May 1998 14:52:12   daffer
// Changed RtcfStatusE
// 
//    Rev 1.4   18 May 1998 15:52:22   daffer
// Added GetStatus method
// 
//    Rev 1.3   30 Apr 1998 14:30:24   daffer
// Added status returns
// 
//    Rev 1.2   13 Apr 1998 14:31:00   sally
// allocate space for _directory and _filename
// 
//    Rev 1.1   10 Apr 1998 14:04:22   daffer
//   Added GetFilename Method to be used in process_reqi.C
// 
//    Rev 1.0   04 Feb 1998 14:17:06   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:43  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

// This file outlines the Rtcf object
// The Rtcf object will exit on error

#ifndef RTCF_H
#define RTCF_H

#include "CmdList.h"

static const char rcs_id_rtcf_h[] =
    "@(#) $Header$";

#define RTCF_MODE       0444

//============
// Rtcf class 
//============

class Rtcf
{
 public:
    Rtcf(const char* rtcf_directory, int file_number);
    ~Rtcf();
    
    // match RtcfStatusStrings
    enum RtcfStatusE {
        RTCF_OK,
        RTCF_ERROR,
        RTCF_OUT_OF_MEMORY,
        RTCF_OPEN_FAILURE,
        RTCF_EMPTY_FILE,
        RTCF_CHMOD_FAILED,
        RTCF_UNLINK_FILE_FAILED,
        RTCF_STATUS_LAST       // boundary, not a status
    };
    
    RtcfStatusE     Write(CmdList* cmd_list);
    RtcfStatusE     GetStatus() { return _status; };
    const char*     RtcfMnemonic(Command* cmd);
    int             GetFileNumber() const { return(_fileNumber); };
    const char *    GetFilename() const { return _filename; };

    // erase the previously created REQQ file
    // process REQI creates REQQ, QPF and RTCF, if any fails,
    // all files should be erased
    RtcfStatusE     EraseFile(void);

    static const char*      GetStatusString(RtcfStatusE status);
    
 private:
    RtcfStatusE     _WriteHeader();
    RtcfStatusE     _WriteRecord(Command* cmd);
    RtcfStatusE     _OpenFile();
    
    char*           _directory;
    char*           _filename;
    int             _firstFileNumber;
    int             _fileNumber;
    int             _recordCount;
    RtcfStatusE     _status;
    FILE*           _ofp;
};

//==================
// Helper Functions 
//==================

//Rtcf::RtcfStatusE ReadMonitorData(const char* filename, char* ascii_string,
//        char* hex_string);

#endif
