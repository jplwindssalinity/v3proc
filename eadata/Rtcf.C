//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// Revision 1.9  1999/09/30 23:01:39  sally
// update 9/30/99
//
// 
//    Rev 1.15   29 Jan 1999 15:04:42   sally
// added LASP proc commands
// 
//    Rev 1.14   08 Jan 1999 16:37:30   sally
// add LASP proc commands
// 
//    Rev 1.13   01 Sep 1998 10:03:58   sally
//  separate CmdHex from CmdId
// 
//    Rev 1.12   16 Jul 1998 13:32:18   sally
// added "QS_" in the filename
// 
//    Rev 1.11   08 Jun 1998 13:37:58   sally
// added EraseFile()
// 
//    Rev 1.10   03 Jun 1998 17:20:36   daffer
// Added _last_msg variable, GetLastMsg and error message throughout
// 
//    Rev 1.9   01 Jun 1998 11:08:58   sally
// fixed some GNU CC compiler warnings
// 
//    Rev 1.8   29 May 1998 15:27:00   sally
// adapted to the new REQI datafile format, per Lee's memo
// 
//    Rev 1.7   28 May 1998 09:27:06   sally
// update the formats for REQQ, QPF and RTCF
// 
//    Rev 1.6   19 May 1998 14:52:26   daffer
// Robustified write methods, changed some return status'
// 
//    Rev 1.5   30 Apr 1998 14:30:12   daffer
// Added status returns
// 
//    Rev 1.4   13 Apr 1998 14:30:52   sally
// allocate space for _directory and _filename
// 
//    Rev 1.3   10 Apr 1998 14:04:22   daffer
//   Added GetFilename Method to be used in process_reqi.C
// 
//    Rev 1.2   24 Mar 1998 15:57:24   sally
// de-warn for GNU
// 
//    Rev 1.1   17 Mar 1998 14:41:40   sally
// changed for REQQ
// 
//    Rev 1.0   04 Feb 1998 14:17:04   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:29:23  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

static const char rcs_id_Rtcf_C[] =
    "@(#) $Header$";

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "CommonDefs.h"
#include "Command.h"
#include "Rtcf.h"
#include "Reqq.h"   // for reading antenna sequence
#include "Binning.h"    // for embedded command check
#include "Qpf.h"

#define STEP_NUMBER_WIDTH   11
#define STEP_NUMBER_LABEL   "Step Number"
#define DATE_WIDTH          -11
#define DATE_LABEL          "Date"
#define RTCF_COMMAND_WIDTH  -15
#define RTCF_COMMAND_LABEL  "Command"
#define RTCF_ARGS_WIDTH     -17
#define RTCF_ARGS_LABEL     "Arg_1 Arg_2 Arg_3"
#define RTCF_ONE_ARG_WIDTH  -5

const char* RtcfStatusStrings[] =
{
    "RTCF OK",
    "RTCF Error",
    "RTCF Out of Memory",
    "RTCF Open Failsure",
    "RTCF Empty File",
    "RTCF Chmod Failed",
    "RTCF Unlink File Failed"
};

//==============
// Rtcf Methods 
//==============

Rtcf::Rtcf(
    const char* rtcf_directory,
    int         file_number)
:   _directory(0), _filename(0),
    _firstFileNumber(file_number), _fileNumber(file_number),
    _recordCount(0), _status(RTCF_OK), _ofp(0)
{
    if ((_directory = strdup(rtcf_directory)) == 0)
    {
        fprintf(stderr, "Rtcf: Out of memory\n");
        _status=RTCF_OUT_OF_MEMORY;
    }
    char filename[BIG_SIZE];
    (void)sprintf(filename, "%s/QS_RTCF%06d", _directory, _fileNumber);
    if ((_filename = strdup(filename)) == 0)
    {
        fprintf(stderr, "Rtcf: Out of memory\n");
        _status=RTCF_OUT_OF_MEMORY;
    }
    return;
}

Rtcf::~Rtcf()
{
    if (_directory) free(_directory);
    if (_filename) free(_filename);
    return;
}

//-------
// Write 
//-------
// pass through a command list and write out RTCF commands

Rtcf::RtcfStatusE
Rtcf::Write(
CmdList*    cmd_list)
{
    Command* cmd = cmd_list->GetHead();
    for (int cmd_index = 0; cmd; cmd_index++)
    {
        if (Command::GetFileFormat(cmd) == Command::FILE_RTCF)
        {
            if (! _ofp)
            {
                if (_OpenFile() != RTCF_OK) 
                    return (RTCF_OPEN_FAILURE);
                if (_WriteHeader() != RTCF_OK)
                    return( RTCF_ERROR );
                _fileNumber++;
            }
            if (_WriteRecord(cmd) != RTCF_OK )
                return (RTCF_ERROR);
        }
        cmd = cmd_list->GetNext();
    }

    if (_ofp)
    {
        fchmod(fileno(_ofp), RTCF_MODE);
        fclose(_ofp);
        return(_status = RTCF_OK);
    }
    else {
        return(_status = RTCF_EMPTY_FILE);
    }

} // Rtcf::Write

//--------------
// RtcfMnemonic 
//--------------

const char*
Rtcf::RtcfMnemonic(
    Command*    cmd)
{
    return(cmd->mnemonic);
}

//--------------
// _WriteHeader 
//--------------
// write the RTCF header record

Rtcf::RtcfStatusE
Rtcf::_WriteHeader()
{
    fprintf(_ofp, "%*s %*s %*s %*s\n",
                  STEP_NUMBER_WIDTH, STEP_NUMBER_LABEL,
                  DATE_WIDTH, DATE_LABEL,
                  RTCF_COMMAND_WIDTH, RTCF_COMMAND_LABEL,
                  RTCF_ARGS_WIDTH, RTCF_ARGS_LABEL);
    return(RTCF_OK);
}

//--------------
// _WriteRecord 
//--------------
// writes an RTCF record to the appropriate file
// opens the file if it is not already opened

Rtcf::RtcfStatusE
Rtcf::_WriteRecord(
Command*    cmd)
{
    //------------------------
    // determine the mnemonic 
    //------------------------
    const char* mnemonic = RtcfMnemonic(cmd);

    //--------------------------
    // generate the date string 
    //--------------------------
    char date_string[CODEA_TIME_LEN];
    cmd->plannedTpg.time.ItimeToCodeADate(date_string);

    //---------------------------------------------
    // get param words from datafile, if applicable
    //---------------------------------------------
    char args_string[SHORT_STRING_LEN];
    char rtcf_args[BIG_SIZE];
    unsigned short checksum=Qpf::Checksum(cmd->cmdHex, QPF_CHECKSUM_SEED);
    unsigned short oldChecksum = checksum;
    if (cmd->dataFilename != 0)
    {
        if (Reqq::ReadParamWords(cmd->dataFilename,
                             cmd->GetNumWordsInParamFile(),
                             cmd->GetDatafileFormat(),
                             checksum, args_string, 0) != REQQ_OK)
        {
            fprintf(stderr, "Error reading datafile %s\n", cmd->dataFilename);
            return(RTCF_ERROR);
        }

        //--------------------------------------------------------------
        // returned args_string is words separated by a space,
        // formated it to the RTCF argument's format.  No fill.
        //--------------------------------------------------------------
        rtcf_args[0] = '\0';
        char* oneString=0;
        char oneRtcfArg[BIG_SIZE];
        for (oneString = (char*)strtok(args_string, " "); oneString;
                    oneString = (char*)strtok(0, " "))
        {
            if (rtcf_args[0] != '\0') strcat(rtcf_args, " ");
            (void)sprintf(oneRtcfArg, "%*s", RTCF_ONE_ARG_WIDTH, oneString);
            strcat(rtcf_args, oneRtcfArg);
        }

        //--------------------------------------------------------------
        // if checksum is changed, then this is a non LASP PROC command,
        // need to write checksum too
        //--------------------------------------------------------------
        if (checksum != oldChecksum &&
             strlen(rtcf_args) <= (size_t)((abs(RTCF_ONE_ARG_WIDTH) * 2) + 1))
        {
            (void)sprintf(oneRtcfArg, "%04X", checksum);
            (void)strcat(rtcf_args, " ");
            (void)strcat(rtcf_args, oneRtcfArg);
        }
    }

    //----------------------
    // write out the record 
    //----------------------

    _recordCount++;
    if (cmd->dataFilename == 0)
    {
        fprintf(_ofp, "%*d %*s %*s\n",
                      STEP_NUMBER_WIDTH, _recordCount,
                      DATE_WIDTH, date_string,
                      RTCF_COMMAND_WIDTH, mnemonic);
    }
    else
    {
        // convert all chars to upper case
        char* ptr = rtcf_args;
        for (unsigned int i=0; i < strlen(rtcf_args); i++, ptr++)
        {
            *ptr = toupper(*ptr);
        }

        fprintf(_ofp, "%*d %*s %*s %*s\n",
                      STEP_NUMBER_WIDTH, _recordCount,
                      DATE_WIDTH, date_string,
                      RTCF_COMMAND_WIDTH, mnemonic,
                      RTCF_ARGS_WIDTH, rtcf_args);
    }
    return(RTCF_OK);
}

//-----------
// _OpenFile 
//-----------
// open file with the proper name
// return RTCF_OK on success, RTCF_OPEN_FAILURE on failure

Rtcf::RtcfStatusE
Rtcf::_OpenFile()
{
    _ofp = fopen(_filename, "w");
    if (! _ofp)
    {
        return (RTCF_OPEN_FAILURE);
    }
    _status=Rtcf::RTCF_OK;
    return(_status);
}


Rtcf::RtcfStatusE
Rtcf::EraseFile(void)
{
    char filename[BIG_SIZE];
    (void)sprintf(filename, "%s/RTCF%06d", _directory, _firstFileNumber);
    // no file created, done
    if (access(filename, F_OK) != 0)
        return(RTCF_OK);

    if (chmod(filename, 0600) == 0)
    {
        if (unlink(filename) == 0)
            return(RTCF_OK);
        else
            return(RTCF_UNLINK_FILE_FAILED);
    }
    else
        return(RTCF_CHMOD_FAILED);

} // Rtcf::EraseFile

const char*
Rtcf::GetStatusString(
RtcfStatusE     status)
{
    if (status < RTCF_OK || status >= RTCF_STATUS_LAST)
        return("Unknown");
    else
        return(RtcfStatusStrings[(int)status]);
 
} // Rtcf::GetStatusString
