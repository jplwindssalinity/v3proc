//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.22   20 Oct 1998 10:53:20   sally
// add static QPF commands (table macro commands)
// 
//    Rev 1.21   21 Sep 1998 15:06:10   sally
// added Qpa
// 
//    Rev 1.20   01 Sep 1998 10:03:54   sally
//  separate CmdHex from CmdId
// 
//    Rev 1.19   09 Jul 1998 16:22:58   sally
// catch the output of process_reqi in an output file
// 
//    Rev 1.18   09 Jul 1998 09:29:06   sally
// change format per Lee's memo
// 
//    Rev 1.17   01 Jul 1998 16:51:38   sally
// added table upload repetition
// 
//    Rev 1.16   29 Jun 1998 16:52:06   sally
// added embedded commands checking
// 
//    Rev 1.15   08 Jun 1998 14:56:54   sally
// took out some debug statements
// 
//    Rev 1.14   08 Jun 1998 13:37:22   sally
// added EraseFiles()
// 
//    Rev 1.12   02 Jun 1998 15:03:52   sally
// fixed testQpf
// 
//    Rev 1.11   01 Jun 1998 11:08:40   sally
// fixed some GNU CC compiler warnings
// 
//    Rev 1.10   29 May 1998 15:26:52   sally
// adapted to the new REQI datafile format, per Lee's memo
// 
//    Rev 1.9   28 May 1998 09:26:34   sally
// update the formats for REQQ, QPF and RTCF
// 
//    Rev 1.8   26 May 1998 16:44:38   sally
// create one file for each QPF command
// 
//    Rev 1.7   26 May 1998 10:55:04   sally
// added QPF
// 
//    Rev 1.6   22 May 1998 16:27:04   sally
// add testQpf
// 
//    Rev 1.5   19 May 1998 14:40:50   daffer
// Robustified Write method
// 
//    Rev 1.4   30 Apr 1998 08:37:10   daffer
// Added status returns
// 
//    Rev 1.3   13 Apr 1998 14:30:28   sally
// allocate space for _directory and _filename
// .
// 
//    Rev 1.2   10 Apr 1998 14:04:18   daffer
//   Added GetFilename Method to be used in process_reqi.C
// 
//    Rev 1.1   24 Mar 1998 15:57:22   sally
// de-warn for GNU
// 
//    Rev 1.0   17 Mar 1998 14:42:02   sally
// Initial revision.
// 
//    Rev 1.1   09 Mar 1998 16:34:22   sally
// adapt to the new REQI format
// 
//    Rev 1.0   04 Feb 1998 14:16:30   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:29:18  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

static const char rcs_id_Qpf_C[] =
    "@(#) $Header$";

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "Qpf.h"
#include "Itime.h"
#include "Command.h"
#include "CmdList.h"
#include "Binning.h"

#define QPF_PROJECT_NAME                "QSCAT "
#define QPF_AO_PROVIDER_CODE            "JPL "
#define QPF_NASDA_TACC_CODE             "LASP"
#define QPF_LENGTH_OF_DATA_RECORD       80
#define QPF_FILE_FORMAT_VERSION_DATE    "19980601"
#define QPF_FILE_FORMAT_VERSION_NUMBER  1
#define QPF_RESERVED                    ""

#define QPF_MODE                        0444

#define QPF_WORDS_PER_LINE              16

const char* QpfStatusStrings[] =
{
    "QPF OK",
    "QPF Error",
    "QPF Out of Memory",
    "QPF Open Failsure",
    "QPF Missing Datafile",
    "QPF Num of Parameter Words Mismatched",
    "QPF Read Failsure",
    "QPF Write Failsure",
    "QPF Unlink Fail Failed",
    "QPF Chmod Failed",
    "QPF Empty File",
    "QPF and QPA are not matched (different number)",
    "QPF and QPA have same numbers, but different mnemonics",
    "QPA has bad time string",
    "Macro Table QPF has NULL table"
};

Qpf::Qpf(
const char*      filename,
FILE*            outFP)
: _status(QPF_OK), _filename(0), _fileNumber(-1)
{
    if ((_filename = strdup(filename)) == 0)
    {
        fprintf(stderr, "QP: Out of memory\n");
        _status=QPF_OUT_OF_MEMORY;
    }
 
    outFP = (outFP == 0 ? stderr : outFP);
    // open the QPF file
    FILE* qpfFP = fopen(filename, "r");
    if (qpfFP == NULL)
    {
        fprintf(outFP, "Failed to open QP file: %s\n", filename);
        _status = QPF_OPEN_FAILURE;
    }
 
    // read the header line
    _status = _ReadHeader(qpfFP, outFP);
    if (_status != QPF_OK) return;

    return;

} // Qpf::Qpf

Qpf::Qpf(
const char*      qpfDirectory,  // qpf directory
int              fileNumber,    // qpf file number
FILE*            outFP)
: _status(QPF_OK), _filename(0), _fileNumber(-1)
{
    outFP = (outFP == 0 ? stderr : outFP);
    char filename[BIG_SIZE];
    (void)sprintf(filename, "%s/QPF%07d", qpfDirectory, fileNumber);
    // open the QPF file
    FILE* qpfFP = fopen(filename, "r");
    if (qpfFP == NULL)
    {
        fprintf(outFP, "Failed to open QP file: %s\n", filename);
        _status = QPF_OPEN_FAILURE;
    }
 
    // read the header line
    QpfStatusE rc = _ReadHeader(qpfFP, outFP);
    if (rc != QPF_OK) return;

    if ((_filename = strdup(filename)) == 0)
    {
        fprintf(stderr, "QP: Out of memory\n");
        _status=QPF_OUT_OF_MEMORY;
    }
 
    return;

} // Qpf::Qpf


Qpf::~Qpf()
{
    if (_filename)
    {
        free(_filename);
        _filename = 0;
    }

} // Qpf::~Qpf

Qpf::QpfStatusE
Qpf::_ReadHeader(
FILE*       qpfFP,
FILE*       outFP)
{
    rewind(qpfFP);   // rewind to make sure it is a the beginning
 
    char headString[BIG_SIZE];
    char* ptr = fgets(headString, BIG_SIZE - 1, qpfFP);
    if (ptr != headString)
    {
        fprintf(outFP, "%s: Read failed: Empty QP file\n", _filename);
        return(QPF_EMPTY_FILE);
    }

    char qpaString[SHORT_STRING_LEN];
    // read the file number, mnemonic and table size
    int rc = sscanf(headString,
        " %3s%d %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %s %s %s",
               qpaString, &_fileNumber, _cmdDateString,
               _cmdTimeString, _mnemonic);
    if (rc == EOF)
    {
        fprintf(outFP, "%s: Empty QP file\n", _filename);
        return QPF_EMPTY_FILE;
    }
    else if (rc != 5)
    {
        fprintf(outFP, "%s: Can't read header\n", _filename);
        return(QPF_ERROR);
    }
    else
        return(QPF_OK);

} // Qpf::_ReadHeader

const char*
Qpf::GetStatusString(
QpfStatusE     status)
{
    if (status < QPF_OK || status >= QPF_STATUS_LAST)
        return("Unknown");
    else
        return(QpfStatusStrings[(int)status]);

} // Qpf::GetStatusString

unsigned short
Qpf::Checksum(
unsigned short      a,
unsigned short      b)
{
    unsigned short sum = a + b;
    return(sum < a ? ++sum : sum);

} // Qpf::Checksum


//=============
// QpfList Methods 
//=============

QpfList::QpfList(
    Itime           start_time,
    Itime           end_time,
    const char*     npf_directory,
    int             file_number)
:   _beginDateOfRequest(start_time), _endDateOfRequest(end_time),
    _directory(0), _firstFileNumber(file_number), _fileNumber(file_number),
    _status(Qpf::QPF_OK)
{
    if ((_directory = strdup(npf_directory)) == 0)
    {
        fprintf(stderr, "QpfList: Out of memory\n");
        _status = Qpf::QPF_OUT_OF_MEMORY;
    }
    return;
}

QpfList::~QpfList()
{
    if (_directory) free(_directory);
    return;
}

//-------
// Write 
//-------
// pass through a command list and write out QPF commands

Qpf::QpfStatusE
QpfList::Write(
CmdList*    cmd_list,
int         check_for_embedded,
FILE*       output_fp)
{
    FILE* outputFP = (output_fp ? output_fp : stderr);
    Command* cmd = cmd_list->GetHead();
    for (int cmd_index = 0; cmd; cmd_index++)   {
#if 0
        if (cmd->GetNumWordsInParamFile() > 2)
#endif
        Command::FileFormatE fileFormat = Command::GetFileFormat(cmd);
        if (fileFormat == Command::FILE_QPF)
        {
            if (_WriteQpfFile(cmd, check_for_embedded, outputFP) 
                                   != Qpf::QPF_OK) {
                return (_status);
            }
        }
        else if (fileFormat == Command::FILE_QPF_STATIC)
        {
            if (_WriteQpfFileFromStaticString(cmd, outputFP)
                                   != Qpf::QPF_OK) {
                return (_status);
            }
        }
        cmd = cmd_list->GetNext();
    }
    return(Qpf::QPF_OK);
}

//---------------
// _WriteQpfFile 
//---------------
// gets bc info, opens, writes, and closes an QPF file

Qpf::QpfStatusE
QpfList::_WriteQpfFile(
Command*    cmd,
int         check_for_embedded,
FILE*       output_fp)
{
    FILE* outputFP = (output_fp ? output_fp : stderr);

    if (cmd->dataFilename == 0)
        return(_status = Qpf::QPF_MISSING_DATAFILE);

    //----------------------
    // open the output file 
    //----------------------

    char filename[BIG_SIZE];
    (void)sprintf(filename, "%s/QPF%07d", _directory, _fileNumber);
    FILE* ofp = fopen(filename, "w");
    if (! ofp)
    {
        fprintf(outputFP, "QPF: error opening output file\n");
        fprintf(outputFP, "  Filename: %s\n", filename);
        return (_status = Qpf::QPF_OPEN_FAILURE);
    }

    //------------------
    // write the header 
    //------------------

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

    // get the date of commanding
    char date_of_commanding[BDATE_TIME_LEN];
    cmd->plannedTpg.time.ItimeToBDate(date_of_commanding);

    // get number of data lines
    int numLines = (int)ceil((double)(cmd->GetNumWordsInParamFile() + 2) /
                       QPF_WORDS_PER_LINE);
    // get command path, if any
    char pathString[4];
    if (cmd->CmdType() == TYP_REAL_TIME)
        (void)strcpy(pathString, "***");
    else
        (void)sprintf(pathString, "%3d", cmd->plannedTpg.path);

    fprintf(ofp,
        "QPF%07d %6s %4s %4s %8s %8s %4d %5d %8s %8s %8s V%02d"
        " %8s %3s %-9s %6d %2d %6s\n",
        _fileNumber, QPF_PROJECT_NAME, QPF_AO_PROVIDER_CODE,
        QPF_NASDA_TACC_CODE, file_creation_date, file_creation_time,
        QPF_LENGTH_OF_DATA_RECORD, numLines,
        begin_date_of_request, end_date_of_request,
        QPF_FILE_FORMAT_VERSION_DATE, QPF_FILE_FORMAT_VERSION_NUMBER,
        date_of_commanding, pathString, cmd->mnemonic,
        cmd->GetNumWordsInParamFile(), cmd->tableRepetition, QPF_RESERVED);

    //--------------------------------------------------------------
    // allocate space for array used for checking embedded commands
    // 2 bytes for command, 2 bytes for checksum
    //--------------------------------------------------------------
    int embeddedCheckArraySize =  2 * cmd->GetNumWordsInParamFile() + 4;
    char* embeddedCheckArray = new char [embeddedCheckArraySize];
    assert(embeddedCheckArray != 0);
    int embeddedCheckOffset=0;

    //------------------------------------
    // write the data records in hex
    //------------------------------------
    FILE* inFP = fopen(cmd->dataFilename, "r");
    if (inFP == NULL)
    {
        (void)fclose(ofp);
        unlink(filename);
        return(_status = Qpf::QPF_OPEN_FAILURE);
    }

    // first write the command
    if (fprintf(ofp, "%04X", cmd->cmdHex) != 4)
    {
        (void)fclose(inFP);
        (void)fclose(ofp);
        unlink(filename);
        return(_status = Qpf::QPF_WRITE_FAIL);
    }
    if ((embeddedCheckOffset + sizeof(short)) > embeddedCheckArraySize)
        return(_status = Qpf::QPF_NUM_PARAM_WORDS_MISMATCHED);
    unsigned short commandHex = (unsigned short) cmd->cmdHex;
    (void)memcpy(embeddedCheckArray, &commandHex, sizeof(short));
    embeddedCheckOffset += sizeof(short);

    unsigned short checksum = Qpf::Checksum(cmd->cmdHex, QPF_CHECKSUM_SEED);
    unsigned short data;
    int wordsInThisLine = 1;
    int wordsInDataFile=0;
    while (fscanf(inFP, "%hx", &data) == 1)
    {
        wordsInDataFile++;
        if ((embeddedCheckOffset + sizeof(short)) > embeddedCheckArraySize)
            return(_status = Qpf::QPF_NUM_PARAM_WORDS_MISMATCHED);
        (void)memcpy(embeddedCheckArray + embeddedCheckOffset,
                                        &data, sizeof(short));
        embeddedCheckOffset += sizeof(short);
        // beginning of a new line
        if (wordsInThisLine == QPF_WORDS_PER_LINE)
        {
            if (fprintf(ofp, "\n") != 1)
            {
                (void)fclose(inFP);
                (void)fclose(ofp);
                unlink(filename);
                return(_status = Qpf::QPF_WRITE_FAIL);
            }
            if (fprintf(ofp, "%04X", data) != 4)
            {
                (void)fclose(inFP);
                (void)fclose(ofp);
                unlink(filename);
                return(_status = Qpf::QPF_WRITE_FAIL);
            }
            checksum = Qpf::Checksum(checksum, data);
            wordsInThisLine = 1;
            continue;
        }

        // words after the first
        if (fprintf(ofp, " %04X", data) != 5)
        {
            (void)fclose(inFP);
            (void)fclose(ofp);
            unlink(filename);
            return(_status = Qpf::QPF_WRITE_FAIL);
        }
        checksum = Qpf::Checksum(checksum, data);
        wordsInThisLine++;
    }

    //-----------------------------------------------------
    // if the number of words in the file doesn't match
    // what it should be, abort
    //-----------------------------------------------------
    if (cmd->GetNumWordsInParamFile() != wordsInDataFile)
    {
        (void)fclose(inFP);
        (void)fclose(ofp);
        unlink(filename);
        return(_status = Qpf::QPF_NUM_PARAM_WORDS_MISMATCHED);
    }

    if (wordsInThisLine == QPF_WORDS_PER_LINE)
    {
        if (fprintf(ofp, "\n") != 1)
        {
            (void)fclose(inFP);
            (void)fclose(ofp);
            unlink(filename);
            return(_status = Qpf::QPF_WRITE_FAIL);
        }
        wordsInThisLine = 0;
    }
    if (fprintf(ofp, " %04X", checksum) != 5)
    {
        (void)fclose(inFP);
        (void)fclose(ofp);
        unlink(filename);
        return(_status = Qpf::QPF_WRITE_FAIL);
    }
    wordsInThisLine++;
    for (int i = wordsInThisLine; i < QPF_WORDS_PER_LINE; i++)
    {
        if (fprintf(ofp, "     ") != 5)
        {
            (void)fclose(inFP);
            (void)fclose(ofp);
            unlink(filename);
            return(_status = Qpf::QPF_WRITE_FAIL);
        }
    }
    if ((embeddedCheckOffset + sizeof(short)) > embeddedCheckArraySize)
        return(_status = Qpf::QPF_NUM_PARAM_WORDS_MISMATCHED);
    (void)memcpy(embeddedCheckArray + embeddedCheckOffset,
                                        &checksum, sizeof(short));
    embeddedCheckOffset += sizeof(short);

    // write the final new line char
    if (fprintf(ofp, "\n") != 1)
    {
        (void)fclose(inFP);
        (void)fclose(ofp);
        unlink(filename);
        return(_status = Qpf::QPF_WRITE_FAIL);
    }

    //----------------
    // close the file 
    //----------------

    (void)fclose(inFP);
    fchmod(fileno(ofp), QPF_MODE);
    (void)fclose(ofp);
    _fileNumber++;

    //--------------------------------
    // check for embedded commands
    //--------------------------------
    if (check_for_embedded)
    {
        EACommandE cmd_code = EA_CMD_NONE;
        const char* cmd_name=0;
        int byte1=0, byte2=0, bit1=0, bit2=0;
        if ( Command::CheckForCommands(embeddedCheckArray,
                         embeddedCheckArraySize, byte1, bit1,
                         byte2, bit2, cmd_code, cmd_name, outputFP))
        {
            fprintf(outputFP, "\n********** WARNING **********\n");
            fprintf(outputFP, "QPF: Embedded command found for %s\n",
                                     cmd->mnemonic);
            fprintf(outputFP, "  Data File:\n    %s\n", cmd->dataFilename);
            fprintf(outputFP, "Program Continuing.\n");
            fprintf(outputFP, "*****************************\n");
        }
    }

    return(Qpf::QPF_OK);

} // QpfList::_WriteQpfFile

//---------------
// _WriteQpfFileFromStaticString 
//---------------
Qpf::QpfStatusE
QpfList::_WriteQpfFileFromStaticString(
Command*    cmd,
FILE*       output_fp)
{
    FILE* outputFP = (output_fp ? output_fp : stderr);

    const char* staticParamString = cmd->GetStaticParamString();
    if (staticParamString == 0 || *staticParamString == '\0')
        return (_status = Qpf::QPF_NULL_STATIC_TABLE);

    //----------------------
    // open the output file 
    //----------------------
    char filename[BIG_SIZE];
    (void)sprintf(filename, "%s/QPF%07d", _directory, _fileNumber);
    FILE* ofp = fopen(filename, "w");
    if (! ofp)
    {
        fprintf(outputFP, "QPF: error opening output file\n");
        fprintf(outputFP, "  Filename: %s\n", filename);
        return (_status = Qpf::QPF_OPEN_FAILURE);
    }

    //------------------
    // write the header 
    //------------------

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

    // get the date of commanding
    char date_of_commanding[BDATE_TIME_LEN];
    cmd->plannedTpg.time.ItimeToBDate(date_of_commanding);

    // get number of data lines
    int numLines = (int)ceil((double)(cmd->GetNumWordsInParamFile() + 2) /
                       QPF_WORDS_PER_LINE);
    // get command path, if any
    char pathString[4];
    if (cmd->CmdType() == TYP_REAL_TIME)
        (void)strcpy(pathString, "***");
    else
        (void)sprintf(pathString, "%3d", cmd->plannedTpg.path);

    fprintf(ofp,
        "QPF%07d %6s %4s %4s %8s %8s %4d %5d %8s %8s %8s V%02d"
        " %8s %3s %-9s %6d %2d %6s\n",
        _fileNumber, QPF_PROJECT_NAME, QPF_AO_PROVIDER_CODE,
        QPF_NASDA_TACC_CODE, file_creation_date, file_creation_time,
        QPF_LENGTH_OF_DATA_RECORD, numLines,
        begin_date_of_request, end_date_of_request,
        QPF_FILE_FORMAT_VERSION_DATE, QPF_FILE_FORMAT_VERSION_NUMBER,
        date_of_commanding, pathString, cmd->mnemonic,
        cmd->GetNumWordsInParamFile(), cmd->tableRepetition, QPF_RESERVED);

    // first write the command
    if (fprintf(ofp, "%04X", cmd->cmdHex) != 4)
    {
        (void)fclose(ofp);
        unlink(filename);
        return(_status = Qpf::QPF_WRITE_FAIL);
    }

printf("staticParamString = %s\n", staticParamString);
    // then write the static table string
    if (fprintf(ofp, "%s", staticParamString) != strlen(staticParamString))
    {
        (void)fclose(ofp);
        unlink(filename);
        return(_status = Qpf::QPF_WRITE_FAIL);
    }

    // write the final new line char
    if (fprintf(ofp, "\n") != 1)
    {
        (void)fclose(ofp);
        unlink(filename);
        return(_status = Qpf::QPF_WRITE_FAIL);
    }

    //----------------
    // close the file 
    //----------------

    fchmod(fileno(ofp), QPF_MODE);
    (void)fclose(ofp);
    _fileNumber++;

    return(Qpf::QPF_OK);

} // QpfList::_WriteQpfFileFromStaticString

Qpf::QpfStatusE
QpfList::GetFilenames(
char**&    filenames,  // IN/OUT: ptr to strings
int&       numFiles)   // IN/OUT: number of files
{
    if (_fileNumber == _firstFileNumber) return Qpf::QPF_ERROR;

    numFiles = _fileNumber - _firstFileNumber;
    filenames = new char*[numFiles];
    if (filenames == 0) return Qpf::QPF_ERROR;

    for (int i = _firstFileNumber, k=0; i < _fileNumber; i++, k++)
    {
        filenames[k] = new char[BIG_SIZE];
        if (filenames[k] == 0)
        {
            delete [] filenames[k];;
            return Qpf::QPF_ERROR;
        }
        (void)sprintf(filenames[k], "%s/QPF%07d", _directory, i);
    }
    return(Qpf::QPF_OK);

} // QpfList::GetFilenames

Qpf::QpfStatusE
QpfList::EraseFiles(void)
{
    char filename[BIG_SIZE];
    for (int i = _firstFileNumber; i <= _fileNumber; i++)
    {
        (void)sprintf(filename, "%s/QPF%07d", _directory, i);

        // no file created, done
        if (access(filename, F_OK) != 0)
            continue;

        if (chmod(filename, 0600) == 0)
        {
            if (unlink(filename) != 0)
                return(Qpf::QPF_UNLINK_FILE_FAILED);
        }
        else
            return(Qpf::QPF_CHMOD_FAILED);
    }
    return(Qpf::QPF_OK);

} // QpfList::EraseFiles

void
QpfList::FreeFilenames(
char**     filenames,  // IN: ptr to strings
int        numFiles)   // IN: number of files
{
    for (int i = 0; i < numFiles; i++)
    {
        delete [] filenames[i];
    }
    delete [] filenames;

} // QpfList::GetFilenames

#ifdef TESTQPF

main(
int      argc,
char**   argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s input_table_file output_qpf_file type\n",
                                   argv[0]);
        fprintf(stderr, " where: type = 1(rga), 2(rgb), 3(dta), 4(dtb)\n");
        exit(1);
    }
    int type= atoi(argv[3]);
    if (type > 4 || type < 0)
    {
        fprintf(stderr, "Type (%d) is not supported\n", type);
        exit(1);
    }
    
    FILE* inFP = fopen(argv[1], "r");
    if (inFP == NULL)
    {
        fprintf(stderr, "%s: Openning (%s) failed\n", argv[0], argv[1]);
        exit(2);
    }
    FILE* outFP = fopen(argv[2], "w");
    if (outFP == NULL)
    {
        fprintf(stderr, "%s: Openning (%s) failed\n", argv[0], argv[2]);
        exit(2);
    }

    EACommandE commandID = EA_CMD_NONE;
    switch (type)
    {
        case 1: commandID = EA_CMD_SCBRATBL; break;
        case 2: commandID = EA_CMD_SCBRBTBL; break;
        case 3: commandID = EA_CMD_SCBDATBL; break;
        case 4: commandID = EA_CMD_SCBDBTBL; break;
        default:
            fprintf(stderr, "%d: unsupported command\n", type);
            break;
    }

    if (fprintf(outFP, "%04X\n", commandID) != 5)
    {
        fprintf(stderr, "%s: write command failed\n", argv[0]);
        exit(2);
    }

    unsigned short checksum = Qpf::Checksum(commandID, QPF_CHECKSUM_SEED);
    unsigned short data;
    while (fscanf(inFP, "%hx", &data) == 1)
    {
        if (fprintf(outFP, "%04X\n", data) != 5)
        {
            fprintf(stderr, "%s: write data (%04X) failed\n", argv[0], data);
            exit(2);
        }
        checksum = Qpf::Checksum(checksum, data);
    }

    if (fprintf(outFP, "%04X\n", checksum) != 5)
    {
        fprintf(stderr, "%s: write command failed\n", argv[0]);
        exit(2);
    }
    (void) fclose(inFP);
    (void) fclose(outFP);

    exit(0);

} // main

#endif // TESTQPF
