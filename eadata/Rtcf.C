//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
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
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "CommonDefs.h"
#include "Command.h"
#include "Rtcf.h"
#include "Reqq.h"   // for reading antenna sequence
#include "Binning.h"    // for embedded command check

#define STEP_NUMBER_WIDTH   11
#define STEP_NUMBER_LABEL   "Step Number"
#define DATE_WIDTH          11
#define DATE_LABEL          "Date"
#define RTCF_COMMAND_WIDTH  9
#define RTCF_COMMAND_LABEL  "Command"


//==============
// Rtcf Methods 
//==============

Rtcf::Rtcf(
    const char* rtcf_directory,
    int         file_number)
:   _directory(0), _filename(0),
    _fileNumber(file_number), _recordCount(0),
    _ofp(0)
{
    if ((_directory = strdup(rtcf_directory)) == 0)
    {
        fprintf(stderr, "Rtcf: Out of memory\n");
        exit(1);
    }
    char filename[BIG_SIZE];
    (void)sprintf(filename, "%s/RTCF%06d", _directory, _fileNumber);
    if ((_filename = strdup(filename)) == 0)
    {
        fprintf(stderr, "Rtcf: Out of memory\n");
        exit(1);
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

int
Rtcf::Write(
    CmdList*    cmd_list)
{
    Command* cmd = cmd_list->GetHead();
    for (int cmd_index = 0; cmd; cmd_index++)
    {
        int numWordsInParamFile = cmd->GetNumWordsInParamFile();
        if (cmd->GetCommandType() == Command::REALTIME_COMMAND &&
                    numWordsInParamFile >= 0 && numWordsInParamFile <= 2)
        {
            if (! _ofp)
            {
                _OpenFile();
                _WriteHeader();
                _fileNumber++;
            }
            _WriteRecord(cmd);
        }
        cmd = cmd_list->GetNext();
    }

    if (_ofp)
    {
        fchmod(fileno(_ofp), RTCF_MODE);
        fclose(_ofp);
    }

    return(1);
}

//--------------
// RtcfMnemonic 
//--------------

const char*
Rtcf::RtcfMnemonic(
    Command*    cmd)
{
    return(cmd->CmdString());
}

//--------------
// _WriteHeader 
//--------------
// write the RTCF header record

int
Rtcf::_WriteHeader()
{
    fprintf(_ofp, "%*s %*s %*s\n", STEP_NUMBER_WIDTH, STEP_NUMBER_LABEL,
        DATE_WIDTH, DATE_LABEL, RTCF_COMMAND_WIDTH, RTCF_COMMAND_LABEL);
    return(1);
}

//--------------
// _WriteRecord 
//--------------
// writes an RTCF record to the appropriate file
// opens the file if it is not already opened

int
Rtcf::_WriteRecord(
Command*    )
#if 0
Command*    cmd)
#endif
{
#if 0
    //------------------------
    // determine the mnemonic 
    //------------------------

    const char* mnemonic = RtcfMnemonic(cmd);

    //--------------------------
    // generate the date string 
    //--------------------------

    char date_string[CODEA_TIME_LEN];
    cmd->plannedTpg.time.ItimeToCodeADate(date_string);

    //----------------------
    // write out the record 
    //----------------------

    _recordCount++;
    char ascii_string[32];
    char hex_string[32];
    if (cmd->commandId == CMD_ANT)
    {
        ReadAntennaSequence(cmd->dataFilename, ascii_string, hex_string);
        sprintf(cmd->dataFilename, "%s", ascii_string);
        fprintf(_ofp, "%*d %+*s %+*s (%s, %s)\n", STEP_NUMBER_WIDTH,
            _recordCount, DATE_WIDTH, date_string, RTCF_COMMAND_WIDTH,
            mnemonic, hex_string, ascii_string);
    }
    else if (cmd->commandId == CMD_MON)
    {
        ReadMonitorData(cmd->dataFilename, ascii_string, hex_string);
        sprintf(cmd->dataFilename, "%s", ascii_string);
        fprintf(_ofp, "%*d %+*s %+*s (%s, %s)\n", STEP_NUMBER_WIDTH,
            _recordCount, DATE_WIDTH, date_string, RTCF_COMMAND_WIDTH,
            mnemonic, hex_string, ascii_string);
    }
    else
        fprintf(_ofp, "%*d %+*s %+*s\n", STEP_NUMBER_WIDTH, _recordCount,
            DATE_WIDTH, date_string, RTCF_COMMAND_WIDTH, mnemonic);
#endif
    return(1);
}

//-----------
// _OpenFile 
//-----------
// open file with the proper name
// return 1 on success, 0 on failure

int
Rtcf::_OpenFile()
{
    _ofp = fopen(_filename, "w");
    if (! _ofp)
    {
        fprintf(stderr, "RTCF: error opening output file\n");
        fprintf(stderr, "  Filename: %s\n", _filename);
        fprintf(stderr, "Program aborting.\n");
        exit(1);
    }
    return(1);
}

//-----------------
// ReadMonitorData 
//-----------------
 
int
ReadMonitorData(
    const char* filename,
    char*       ascii_string,
    char*       hex_string)
{
    //---------------
    // open the file 
    //---------------
 
    FILE* ifp = fopen(filename, "r");
    if (! ifp)
    {
        fprintf(stderr, "Error opening trip monitor data file\n");
        fprintf(stderr, "  Trip Monitor Data Filename:\n    %s\n", filename);
        fprintf(stderr, "Program aborting.\n");
        exit(1);
    }
 
    //------------------
    // read in the data 
    //------------------
 
    int monitor_enable, shutdown_enable, trip_duration;
    if (fscanf(ifp, "%d %d %d", &monitor_enable, &shutdown_enable,
        &trip_duration) != 3)
    {
        fclose(ifp);
        fprintf(stderr, "Error reading trip monitor data file\n");
        fprintf(stderr, "  Trip Monitor Data Filename:\n    %s\n", filename);
        fprintf(stderr, "Program aborting.\n");
        exit(1);
    }
    fclose(ifp);
 
    //----------------------------
    // determine the command word 
    //----------------------------

    union
    {
        unsigned int    int_cmd;
        unsigned char   char_cmd[4];
    } cmd_word;
    cmd_word.int_cmd = 0;

    switch (monitor_enable)
    {
    case 1:
        cmd_word.char_cmd[0] |= 0x80;
        break;
    case 0:
        break;
    default:
        fprintf(stderr, "Error interpreting monitor enable flag: %d\n",
            monitor_enable);
        fprintf(stderr, "  Trip Monitor Data Filename:\n    %s\n", filename);
        fprintf(stderr, "Program aborting.\n");
        exit(1);
    }

    switch (shutdown_enable)
    {
    case 1:
        cmd_word.char_cmd[0] |= 0x40;
        break;
    case 0:
        break;
    default:
        fprintf(stderr, "Error interpreting shutdown enable flag: %d\n",
            shutdown_enable);
        fprintf(stderr, "  Trip Monitor Data Filename:\n    %s\n", filename);
        fprintf(stderr, "Program aborting.\n");
    }

    if (trip_duration < 0 || trip_duration > 4095)
    {
        fprintf(stderr, "Trip duration not between 0 and 4095: %d\n",
            trip_duration);
        fprintf(stderr, "  Trip Monitor Data Filename:\n    %s\n", filename);
        fprintf(stderr, "Program aborting.\n");
    }
    cmd_word.int_cmd += (trip_duration << 16);
 
    //------------------------
    // calculate the checksum 
    //------------------------
 
    unsigned long sum = 0xaa;
    for (int i = 1; i < 2; i++)
    {
        sum += cmd_word.char_cmd[i];
        if (sum & 0x100)
        {
            // do the end around carry thing
            sum &= 0xff;    // take out the carry bit
            sum += 1;      // add it at the end
        }
    }
    cmd_word.char_cmd[3] = sum;
 
    //-----------------------------
    // check for embedded commands 
    //-----------------------------
 
    char check_array[6];
    check_array[0] = 0x6e;  // the rfs trip monitor command
    check_array[1] = 0x11;
    memcpy(check_array + 2, cmd_word.char_cmd, 4);
 
    int byte1, bit1, byte2, bit2;
    unsigned short cmd_code;
    char* cmd_name;
    if (CheckForCmds(check_array, 6, &byte1, &bit1, &byte2, &bit2,
        &cmd_code, &cmd_name))
    {
        fprintf(stderr, "\n********** WARNING **********\n");
        fprintf(stderr, "NPF: Embedded command in RFS Trip Monitor Command\n");
        fprintf(stderr, "  RFS Trip Monitor Data File:\n    %s\n",
            filename);
        fprintf(stderr, "  Embedded Command: %s (%hx)\n", cmd_name, cmd_code);
        fprintf(stderr,
            "  Command Range: byte %d, bit %d to byte %d, bit %d\n",
            byte1, bit1, byte2, bit2);
        fprintf(stderr,
            "  (Bytes start at 0 and include the command word.)\n");
        fprintf(stderr, "Program Continuing.\n");
        fprintf(stderr, "*****************************\n");
    }
 
    //----------------------
    // set the ascii string 
    //----------------------
 
    sprintf(ascii_string, "%d %d %d", monitor_enable, shutdown_enable,
        trip_duration);
 
    //--------------------
    // set the hex string 
    //--------------------

    sprintf(hex_string, "%08.8X", cmd_word.int_cmd);
 
    return(1);
}
