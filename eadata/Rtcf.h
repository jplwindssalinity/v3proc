//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
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

    int             Write(CmdList* cmd_list);
    const char*     RtcfMnemonic(Command* cmd);
    int             GetFileNumber() const { return(_fileNumber); };
    const char *    GetFilename() const { return _filename; };
    
private:
    int             _WriteHeader();
    int             _WriteRecord(Command* cmd);
    int             _OpenFile();

    char*           _directory;
    char*           _filename;
    int             _fileNumber;
    int             _recordCount;
    FILE*           _ofp;
};

//==================
// Helper Functions 
//==================

int ReadMonitorData(const char* filename, char* ascii_string,
        char* hex_string);

#endif
