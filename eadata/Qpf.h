//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.14   20 Oct 1998 10:53:16   sally
// add static QPF commands (table macro commands)
// 
//    Rev 1.13   21 Sep 1998 15:06:12   sally
// added Qpa
// 
//    Rev 1.12   09 Jul 1998 16:23:00   sally
// catch the output of process_reqi in an output file
// 
//    Rev 1.11   29 Jun 1998 16:52:08   sally
// added embedded commands checking
// 
//    Rev 1.10   08 Jun 1998 13:37:34   sally
// added EraseFiles()
// 
//    Rev 1.8   28 May 1998 09:26:36   sally
// update the formats for REQQ, QPF and RTCF
// 
//    Rev 1.7   26 May 1998 16:45:16   sally
// create one file for each QPF command
// 
//    Rev 1.6   26 May 1998 10:55:08   sally
// added QPF
// 
//    Rev 1.5   22 May 1998 16:27:18   sally
// add testQpf
// 
//    Rev 1.4   19 May 1998 14:40:48   daffer
// Robustified Write method
// 
//    Rev 1.3   30 Apr 1998 14:28:54   daffer
// Added status returns
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

#include "CommonDefs.h"
#include "Itime.h"
#include "CmdList.h"

static const char rcs_id_npf_h[] =
    "@(#) $Header$";


#define QPF_CHECKSUM_SEED               0x55aa

//===========
// Qpf class 
//===========

class Qpf
{
public:

    enum QpfStatusE {
        QPF_OK,
        QPF_ERROR,
        QPF_OUT_OF_MEMORY,
        QPF_OPEN_FAILURE,
        QPF_MISSING_DATAFILE,
        QPF_NUM_PARAM_WORDS_MISMATCHED,
        QPF_READ_FAIL,
        QPF_WRITE_FAIL,
        QPF_UNLINK_FILE_FAILED,
        QPF_CHMOD_FAILED,
        QPF_EMPTY_FILE,
        QPF_QPA_NOT_MATCHED,
        QPF_QPA_MATCHED_WITH_ERRORS,
        QPA_BAD_TIME_STRING,
        QPF_NULL_STATIC_TABLE,
        QPF_STATUS_LAST       // boundary, not a status
    };

    // read from QPF file
    Qpf(const char* qpfFilename,
        FILE*       outFP);

    // read from QPF file given QPF directory and file number
    Qpf(const char*  qpfDirectory,  // qpf directory
        int          fileNumber,    // qpf file number
        FILE*        outFP);


    virtual ~Qpf();

    QpfStatusE  GetStatus(void) const { return(_status); }
    const char* GetFilename(void) const { return(_filename); }
    int         GetFileNumber(void) const { return(_fileNumber); }
    const char* GetMnemonic(void) const { return(_mnemonic); }
    const char* GetDateString(void) const { return(_cmdDateString); }
    const char* GetTimeString(void) const { return(_cmdTimeString); }

    static const char*      GetStatusString(Qpf::QpfStatusE status);
    static unsigned short   Checksum(unsigned short a,
                                     unsigned short b);

protected:
    virtual QpfStatusE  _ReadHeader(FILE* qpFP, FILE* outFP);

    QpfStatusE  _status;
    char*       _filename;
    int         _fileNumber;
    char        _mnemonic[SHORT_STRING_LEN];
    char        _cmdDateString[SHORT_STRING_LEN];
    char        _cmdTimeString[SHORT_STRING_LEN];

};

class QpfList
{
public:
    QpfList(Itime start_time, Itime end_time, const char* npf_directory,
        int file_number);
    ~QpfList();

    Qpf::QpfStatusE  Write(CmdList* cmd_list,
                      int      check_for_embedded,
                      FILE*    outputFP);
    int         GetFileNumber() const { return(_fileNumber); };
    Qpf::QpfStatusE  GetStatus() { return _status; };

    // NOTE: call this method only when Write() is done
    // NOTE: need to call FreeFilenames() to free the space
    Qpf::QpfStatusE  GetFilenames(char**&    filenames,  // IN/OUT: ptr to str
                             int&       numFiles);  // IN/OUT: number of files

    void        FreeFilenames(char**     filenames,  // IN: ptr to strings
                              int        numFiles);  // IN: number of files

    // erase the previously created REQQ file
    // process REQI creates REQQ, QPF and RTCF, if any fails,
    // all files should be erased
    Qpf::QpfStatusE      EraseFiles(void);

private:

    Qpf::QpfStatusE  _WriteQpfFile(Command* cmd,
                              int      check_for_embedded,
                              FILE*    outputFP);
    Qpf::QpfStatusE  _WriteQpfFileFromStaticString(Command* cmd,
                              FILE*    outputFP);

    Itime       _beginDateOfRequest;
    Itime       _endDateOfRequest;
    char*       _directory;
    int         _firstFileNumber;
    int         _fileNumber;
    Qpf::QpfStatusE  _status;

};

#endif
