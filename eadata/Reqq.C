//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.29   13 Oct 1998 15:34:32   sally
// added L1B file
// 
//    Rev 1.28   08 Oct 1998 16:17:56   sally
// change PBI... to Static...
// 
//    Rev 1.27   01 Sep 1998 16:39:16   sally
// change operator== to Match in order to pass outFP
// 
//    Rev 1.26   01 Sep 1998 10:03:56   sally
//  separate CmdHex from CmdId
// 
//    Rev 1.25   31 Aug 1998 14:41:46   sally
// added REQQ-like PBI commands
// 
//    Rev 1.24   28 Aug 1998 16:30:58   sally
// use REQA's file number to match REQQ file 
// 
//    Rev 1.23   28 Aug 1998 11:20:10   sally
// create ReqqRecordList for matching with REQA
// 
//    Rev 1.22   16 Jul 1998 13:32:02   sally
// added "QS_" in the filename
// .
// 
//    Rev 1.21   13 Jul 1998 14:21:54   sally
// add the datafile format for SCGATEWID
// 
//    Rev 1.20   09 Jul 1998 16:23:02   sally
// catch the output of process_reqi in an output file
// 
//    Rev 1.19   09 Jul 1998 09:29:12   sally
// change format per Lee's memo
// 
//    Rev 1.18   01 Jul 1998 13:13:38   sally
// added embedded commands checking
// 
//    Rev 1.17   08 Jun 1998 13:37:40   sally
// added EraseFile()
// 
//    Rev 1.15   01 Jun 1998 16:44:34   sally
// initialize some obj vars
// 
//    Rev 1.14   29 May 1998 15:26:56   sally
// adapted to the new REQI datafile format, per Lee's memo
// 
//    Rev 1.13   28 May 1998 09:26:56   sally
// update the formats for REQQ, QPF and RTCF
// 
//    Rev 1.12   26 May 1998 10:55:10   sally
// added QPF
// 
//    Rev 1.11   22 May 1998 16:46:58   daffer
// Changed cmd enums (e.g. EA_CMD_MODWOM to EA_CMD_SCMODWOM)
// 
//    Rev 1.10   19 May 1998 14:48:54   daffer
// Changed mnemonics back (again) since I can't check out non-tip revision. 
// Fixed some problems with the return status'
// 
//    Rev 1.9   18 May 1998 15:46:16   daffer
// Changed mnemonics back again.
//
//    Rev 1.8   18 May 1998 15:44:30   daffer
// Changed some mnemonics to conform to 
// and older version of Command.h
// 
//    Rev 1.7   18 May 1998 15:32:10   daffer
// Cleaned up some return status
// 
//    Rev 1.6   30 Apr 1998 14:29:50   daffer
// Added status returns
// 
//    Rev 1.5   13 Apr 1998 14:30:46   sally
// allocate space for _directory and _filename
// 
//    Rev 1.4   10 Apr 1998 14:04:20   daffer
//   Added GetFilename Method to be used in process_reqi.C
// 
//    Rev 1.3   17 Mar 1998 14:41:32   sally
// changed for REQQ
// 
//    Rev 1.2   09 Mar 1998 16:34:26   sally
// 
//    Rev 1.1   26 Feb 1998 10:01:02   sally
// to pacify GNU compiler
// 
//    Rev 1.0   04 Feb 1998 14:17:00   daffer
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

static const char rcs_id_Reqq_C[] =
    "@(#) $Header$";

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "CommonDefs.h"
#include "Reqq.h"
#include "ReqaqList.h"
#include "Command.h"
#include "CmdList.h"
#include "Binning.h"    // for embedded command check
#include "Qpf.h"

#define REQQ_PATH_LEN          6
#define REQQ_GAMMA_LEN         7

const char* ReqqStatusStrings[] =
{
    "REQQ OK",
    "REQQ Error",
    "REQQ Out of Memory",
    "REQQ Automatic Command not allowed",
    "REQQ Command not allowed",
    "REQQ Open Failure",
    "REQQ File Exists",
    "REQQ Error Reading Datafile",
    "REQQ Error Writing Parameter Words",
    "REQQ Empty File",
    "REQQ Unlink Fail Failed",
    "REQQ Chmod Failed",
    "REQQ Error Reading Parameter String"
};

//==============
// Reqq Methods 
//==============

Reqq::Reqq(
    Itime       start_time,
    Itime       end_time,
    const char* reqq_directory,
    int         file_number,
    int         useCurrentNumber,
    int         cmd_id,
    int         last_mode_cmd_id)
:   _beginDateOfRequest(start_time), _endDateOfRequest(end_time),
    _directory(0), _filename(0), _firstFileNumber(file_number),
    _fileNumber(file_number), _fileVersion(' '), _cmdId(cmd_id),
    _lastModeCmdId(last_mode_cmd_id), _recordCount(0), _status(REQQ_OK),
    _ofp(0), _reqqList(0)
{

    if ((_directory = strdup(reqq_directory)) == 0)
    {
        fprintf(stderr, "Reqq: Out of memory\n");
        _status=REQQ_OUT_OF_MEMORY;
    }
    char filename[BIG_SIZE];
    (void)sprintf(filename, "%s/QS_REQQ%06d", _directory, _fileNumber);

    // if file exists already, and useCurrentNumber flag is on,
    // then generate the next file version
    char newFilename[BIG_SIZE];
    (void)strcpy(newFilename, filename);
    if (access(filename, F_OK) == 0)
    {
        if (useCurrentNumber)
        {
            for (_fileVersion = 'B'; _fileVersion <= 'Z'; _fileVersion++)
            {
                (void)sprintf(newFilename, "%s%c", filename, _fileVersion);
                if (access(newFilename, F_OK) != 0)
                    break;
            }
        }
        else
        {
            _status = REQQ_FILE_EXISTS;
        }
    }

    if ((_filename = strdup(newFilename)) == 0)
    {
        fprintf(stderr, "Reqq: Out of memory\n");
        _status=REQQ_OUT_OF_MEMORY;
    }

    return;
}

Reqq::Reqq(
const char*      filename)  // reqq file path + name
:   _beginDateOfRequest(INVALID_TIME), _endDateOfRequest(INVALID_TIME),
    _directory(0), _filename(0), _firstFileNumber(0),
    _fileNumber(0), _fileVersion(' '), _cmdId(EA_CMD_NONE),
    _lastModeCmdId(EA_CMD_NONE), _recordCount(0), _status(REQQ_OK),
    _ofp(0), _reqqList(0)
{
    _Read(filename);

} // Reqq::Reqq

Reqq::Reqq(
const char*      reqqDirectory,  // reqq directory
int              fileNumber)     // reqq file number
:   _beginDateOfRequest(INVALID_TIME), _endDateOfRequest(INVALID_TIME),
    _directory(0), _filename(0), _firstFileNumber(0),
    _fileNumber(0), _fileVersion(' '), _cmdId(EA_CMD_NONE),
    _lastModeCmdId(EA_CMD_NONE), _recordCount(0), _status(REQQ_OK),
    _ofp(0), _reqqList(0)
{
    char filename[BIG_SIZE];
    (void)sprintf(filename, "%s/QS_REQQ%06d", reqqDirectory, fileNumber);
    _Read(filename);

} // Reqq::Reqq


Reqq::~Reqq()
{
    if (_directory) free(_directory);
    if (_filename) free(_filename);
    if (_reqqList) delete(_reqqList);
    return;
}

//-------
// Write 
//-------
// pass through a command list and write out REQQ commands

ReqqStatusE 
Reqq::Write(
CmdList*    cmd_list,
int         check_for_embedded,
FILE*       output_fp)
{
    FILE* outputFP = (output_fp ? output_fp : stderr);
    Command* cmd = cmd_list->GetHead();
    for (int cmd_index = 0; cmd; cmd_index++)
    {
        switch(Command::GetFileFormat(cmd))
        {
        case Command::FILE_REQQ:
        case Command::FILE_REQQ_STATIC:
            if (! _ofp)
            {
                if (_OpenFile(outputFP) == REQQ_OPEN_FAILURE )
                    return (REQQ_OPEN_FAILURE);
                ReqqStatusE status = _WriteHeader();
                if (status != REQQ_OK )
                    return (status );
                
            }
            if (_WriteRecord(cmd, check_for_embedded, outputFP) != REQQ_OK)
                return (REQQ_ERROR) ;
            break;
        default:
            break;
        }
        cmd = cmd_list->GetNext();
    }

    //--------------------
    // rewrite the header 
    //--------------------

    if (_ofp)
    {
        _WriteHeader();
        fchmod(fileno(_ofp), REQQ_MODE);
        fclose(_ofp);
        _fileNumber++;
        return(_status = REQQ_OK);
    }
    else
        return(_status = REQQ_EMPTY_FILE);

} // Reqq::Write

//--------------
// ReqqMnemonic 
//--------------

const char*
Reqq::ReqqMnemonic(
    Command*    cmd)
{
    return(cmd->mnemonic);

} // Reqq::ReqqMnemonic

//--------------
// _WriteRecord 
//--------------
// writes an REQQ record to the appropriate file
// opens the file if it is not already opened

ReqqStatusE
Reqq::_WriteRecord(
Command*    cmd,
int         check_for_embedded,
FILE*       output_fp)
{
    FILE* outputFP = (output_fp ? output_fp : stderr);
    //------------------------
    // determine the mnemonic 
    //------------------------
    const char* mnemonic = ReqqMnemonic(cmd);
    if (! mnemonic)
    {
        fprintf(outputFP, "REQQ: %s command not allowable in an REQQ\n",
            cmd->mnemonic);
        cmd->WriteForHumans(outputFP);
        _status=REQQ_CMD_NOT_ALLOWED;
        return (_status);

        
    }

    //--------------------------------------
    // determine the reqq command id to use 
    //--------------------------------------

    int use_cmd_id = _cmdId;

    char timeString[CODEA_TIME_LEN];
    char pathString[REQQ_PATH_LEN];
    char gammaString[REQQ_GAMMA_LEN];
    if (cmd->GetFormat() == Command::USE_PATH_FORMAT)
    {
        (void)cmd->plannedTpg.time.ItimeToCodeADate(timeString);
        (void)sprintf(pathString, "%5d", cmd->plannedTpg.path);
        (void)sprintf(gammaString, "%6.2f", cmd->plannedTpg.gamma);
    }
    else
    {
        (void)cmd->plannedTpg.time.ItimeToCodeASecond(timeString);
        (void)strcpy(pathString, REQQ_NULL_PATH_STRING);
        (void)strcpy(gammaString, REQQ_NULL_GAMMA_STRING);
    }
    _cmdId++;

    // get the checksum
    unsigned short checksum = Qpf::Checksum(cmd->cmdHex, QPF_CHECKSUM_SEED);

    //---------------------------------------------------------
    // get static param first two hard-coded param words, if applicable 
    //---------------------------------------------------------
    char paramString[10];
    (void)strcpy(paramString, "**** ****");
    int numStaticParams = cmd->GetNumStaticParams();
    const char* staticParamString = cmd->GetStaticParamString();
    if (numStaticParams > 0)
    {
        char* ptr = paramString;
        int charToCopy = (4 * numStaticParams) + numStaticParams - 1;
        if (charToCopy > 9) charToCopy = 9;
        strncpy(ptr, staticParamString, charToCopy);
        paramString[9] = '\0';
    }
    //----------------------------------------
    // get param words from datafile, if applicable 
    //----------------------------------------
    else if (cmd->dataFilename != 0)
    {
        ReadParamWords(cmd->dataFilename,
                       cmd->GetNumWordsInParamFile(),
                       cmd->GetDatafileFormat(),
                       checksum, paramString, outputFP);
    }

    //------------------
    // write the record 
    //------------------
    char checksumString[5];
    (void)strcpy(checksumString, "****");
    if (numStaticParams >= 3)
    {
        const char* ptr = staticParamString + 10;
        (void)strncpy(checksumString, ptr, 4);
        checksumString[4] = '\0';
    }
    else if (numStaticParams < 0)
        (void)sprintf(checksumString, "%04X", checksum);

    fprintf(_ofp, "JPL%09d %5s  %-12s %-19s %5s %6s %9s %4s\n",
        use_cmd_id, REQQ_SENSOR_NAME, mnemonic, timeString,
        pathString, gammaString, paramString, checksumString);

    _recordCount++;

    //--------------------------------
    // check for embedded commands
    //--------------------------------
    if (check_for_embedded)
    {
        // 2 bytes for command, 4 for arguments, 2 for checksum
        char* embeddedCheckArray = new char [8];

        // first word is the command itself
        unsigned short commandHex = (unsigned short) cmd->cmdHex;
        (void)memcpy(embeddedCheckArray, &commandHex, sizeof(short));
        int embeddedCheckOffset = 2;
        if (paramString[0] != '*')
        {
            unsigned short data;
            if (sscanf(paramString, "%hx", &data) != 1)
                return(_status = REQQ_ERROR_READ_PARAM_STRING);
            (void)memcpy(embeddedCheckArray + embeddedCheckOffset,
                                        &data, sizeof(short));
            embeddedCheckOffset += 2;
            char* secondString = paramString + 5;
            if (*secondString != '*')
            {
                if (sscanf(secondString, "%hx", &data) != 1)
                    return(_status = REQQ_ERROR_READ_PARAM_STRING);
                (void)memcpy(embeddedCheckArray + embeddedCheckOffset,
                                        &data, sizeof(short));
                embeddedCheckOffset += 2;
            }
        }
        (void)memcpy(embeddedCheckArray + embeddedCheckOffset,
                                        &checksum, sizeof(short));
        embeddedCheckOffset += 2;

        EACommandE cmd_code = EA_CMD_NONE;
        const char* cmd_name=0;
        int byte1=0, byte2=0, bit1=0, bit2=0;
        if ( Command::CheckForCommands(embeddedCheckArray,
                         embeddedCheckOffset, byte1, bit1,
                         byte2, bit2, cmd_code, cmd_name, outputFP))
        {
            fprintf(outputFP, "\n********** WARNING **********\n");
            fprintf(outputFP, "REQQ: Embedded command found for %s\n",
                                     cmd->mnemonic);
            if (cmd->dataFilename)
                fprintf(outputFP, "  Data File:\n    %s\n", cmd->dataFilename);
            fprintf(outputFP, "Program Continuing.\n");
            fprintf(outputFP, "*****************************\n");
        }
    }

    return(REQQ_OK);
}

//--------------
// _ModeCommand 
//--------------

ReqqStatusE
Reqq::_ModeCommand(
    Command*    cmd)
{
    if (cmd->commandId == EA_CMD_SCMODCAL ||
                 cmd->commandId == EA_CMD_SCMODWOM ||
                 cmd->commandId == EA_CMD_SCMODSTB)
    {
        return(REQQ_OK);
    }
    else
        return(REQQ_ERROR);
}

//-----------
// _OpenFile 
//-----------
// open file with the proper name
// return OK on success,OpenFailure on failure

ReqqStatusE
Reqq::_OpenFile(
FILE*     output_fp)
{
    FILE* outputFP = (output_fp ? output_fp : stderr);
    _ofp = fopen(_filename, "w");
    if (! _ofp)
    {
        fprintf(outputFP, "REQQ: error opening output file\n");
        fprintf(outputFP, "  Filename: %s\n", _filename);
        fprintf(outputFP, "Program aborting.\n");
        _status=REQQ_OPEN_FAILURE;
    }
    return(_status);
}

//--------------
// _WriteHeader 
//--------------
// write the REQQ header record

ReqqStatusE
Reqq::_WriteHeader()
{
    rewind(_ofp);   // rewind for insertion

    // get the current time
    Itime current_time((time_t)time(NULL));
    char file_creation_date[BDATE_TIME_LEN];
    current_time.ItimeToBDate(file_creation_date);
    char file_creation_time[HMS_TIME_LEN];
    current_time.ItimeToHMS(file_creation_time);

    // get the request dates
    char begin_date_of_request[BDATE_TIME_LEN];
    _beginDateOfRequest.ItimeToBDate(begin_date_of_request);
    char end_date_of_request[BDATE_TIME_LEN];
    _endDateOfRequest.ItimeToBDate(end_date_of_request);

    fprintf(_ofp,
        "QS_REQQ%06d%c %6s %4s %4s %8s %8s %4d %5d %8s %8s %8s V%02d %35s\n",
        _fileNumber, _fileVersion, REQQ_PROJECT_NAME, REQQ_AO_PROVIDER_CODE,
        REQQ_EOC_MMO_CODE, file_creation_date, file_creation_time,
        REQQ_DATA_RECORD_LENGTH, _recordCount, begin_date_of_request,
        end_date_of_request, REQQ_FILE_FORMAT_VERSION_DATE,
        REQQ_FILE_FORMAT_VERSION_NUMBER, REQQ_RESERVED);
    return(REQQ_OK);
}

ReqqStatusE
Reqq::ReadParamWords(
const char*         filename,
int                 numParamWords,
EADataFileFormat    datafileFormat,
unsigned short&     checksum,
char*               paramString,    // IN/OUT
FILE*               output_fp)
{
    assert(datafileFormat != EA_DATAFILE_NONE &&
            (numParamWords == 1 || numParamWords == 2));

    FILE* outputFP = (output_fp ? output_fp : stderr);
    //---------------
    // open the file 
    //---------------
    FILE* ifp = fopen(filename, "r");
    if (! ifp)
    {
        fprintf(outputFP, "REQQ: error opening REQQ data file\n");
        fprintf(outputFP, "  REQQ Data Filename: %s\n", filename);
        return (REQQ_OPEN_FAILURE);
    }

    //---------------------------------------------------
    // read in the string depending on the type,
    // calculate the checksum, then return the string
    //---------------------------------------------------
    switch (datafileFormat)
    {
        case EA_DATAFILE_UDEC2_ASCII:
        {
            unsigned short data=0;
            if (fscanf(ifp, "%hu", &data) != 1)
                return(REQQ_ERROR_READ_DATAFILE);
            if (sprintf(paramString, "%04X ****", data) != 9)
                return(REQQ_ERROR_WRITE_PARAMS);
            checksum = Qpf::Checksum(checksum, data);
            break;
        }
        case EA_DATAFILE_2UDEC2_ASCII:
        {
            unsigned short data1=0, data2=0;
            if (fscanf(ifp, "%hu %hu", &data1, &data2) != 2)
                return(REQQ_ERROR_READ_DATAFILE);
            if (sprintf(paramString, "%04X %04X", data1, data2) != 9)
                return(REQQ_ERROR_WRITE_PARAMS);
            checksum = Qpf::Checksum(checksum, data1);
            checksum = Qpf::Checksum(checksum, data2);
            break;
        }
        case EA_DATAFILE_HEX2_ASCII:
        {
            unsigned short data=0;
            if (fscanf(ifp, "%hx", &data) != 1)
                return(REQQ_ERROR_READ_DATAFILE);
            if (sprintf(paramString, "%04X ****", data) != 9)
                return(REQQ_ERROR_WRITE_PARAMS);
            checksum = Qpf::Checksum(checksum, data);
            break;
        }
        case EA_DATAFILE_HEX4_TO_2HEX2:
        {
            unsigned int data=0;
            if (fscanf(ifp, "%x", &data) != 1)
                return(REQQ_ERROR_READ_DATAFILE);
            unsigned short *first2Bytes, *last2Byte;
            first2Bytes = (unsigned short*) &data;
            last2Byte = first2Bytes + 1;
            if (sprintf(paramString, "%04X %04X",
                                *first2Bytes, *last2Byte) != 9)
                return(REQQ_ERROR_WRITE_PARAMS);
            checksum = Qpf::Checksum(checksum, *first2Bytes);
            checksum = Qpf::Checksum(checksum, *last2Byte);
            break;
        }
        case EA_DATAFILE_UDEC4_TO_2HEX2:
        {
            unsigned int data=0;
            if (fscanf(ifp, "%u", &data) != 1)
                return(REQQ_ERROR_READ_DATAFILE);
            unsigned short *first2Bytes, *last2Byte;
            first2Bytes = (unsigned short*) &data;
            last2Byte = first2Bytes + 1;
            if (sprintf(paramString, "%04X %04X",
                                *first2Bytes, *last2Byte) != 9)
                return(REQQ_ERROR_WRITE_PARAMS);
            checksum = Qpf::Checksum(checksum, *first2Bytes);
            checksum = Qpf::Checksum(checksum, *last2Byte);
            break;
        }
        default:  // shouldn't happen, but guard it anyway
            (void) fclose(ifp);
            return(REQQ_ERROR_READ_DATAFILE);
    }

    (void) fclose(ifp);
    return REQQ_OK;

} //Reqq::ReadParamWords

ReqqStatusE
Reqq::EraseFile(void)
{
    // erase the previously created REQQ file
    char filename[BIG_SIZE];
    (void)sprintf(filename, "%s/REQQ%06d", _directory, _firstFileNumber);
    // no file created, done
    if (access(filename, F_OK) != 0)
        return(REQQ_OK);

    if (chmod(filename, 0600) == 0)
    {
        if (unlink(filename) == 0)
            return(REQQ_OK);
        else
            return(REQQ_UNLINK_FILE_FAILED);
    }
    else
        return(REQQ_CHMOD_FAILED);

} // Reqq::EraseFile

const char*
Reqq::GetStatusString(
ReqqStatusE     status)
{
    if (status < REQQ_OK || status >= REQQ_STATUS_LAST)
        return("Unknown");
    else
        return(ReqqStatusStrings[(int)status]);
 
} // Reqq::GetStatusString

void
Reqq::_Read(
const char*    filename)
{
    _reqqList = new ReqqRecordList;
    _status = _reqqList->Read(filename, stderr);
    if ((_filename = strdup(filename)) == 0)
    {
        fprintf(stderr, "Reqq: Out of memory\n");
        _status=REQQ_OUT_OF_MEMORY;
    }
} // Reqq::_Read
