//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Qpf.h"
#include "Itime.h"
#include "Command.h"
#include "CmdList.h"
#include "Binning.h"

//=============
// Qpf Methods 
//=============

Qpf::Qpf(
    Itime           start_time,
    Itime           end_time,
    const char*     npf_directory,
    int             file_number)
:   _beginDateOfRequest(start_time), _endDateOfRequest(end_time),
    _directory(0), _filename(0), _fileNumber(file_number)
{
    if ((_directory = strdup(npf_directory)) == 0)
    {
        fprintf(stderr, "Qpf: Out of memory\n");
        exit(1);
    }
    char filename[BIG_SIZE];
    (void)sprintf(filename, "%s/QPF%07d", _directory, _fileNumber);
    if ((_filename = strdup(filename)) == 0)
    {
        fprintf(stderr, "Qpf: Out of memory\n");
        exit(1);
    }
    return;
}

Qpf::~Qpf()
{
    if (_directory) free(_directory);
    if (_filename) free(_filename);
    return;
}

//-------
// Write 
//-------
// pass through a command list and write out QPF commands

int
Qpf::Write(
CmdList*    )
#if 0
CmdList*    cmd_list)
#endif
{
#if 0
    Command* cmd = cmd_list->GetHead();
    for (int cmd_index = 0; cmd; cmd_index++)
    {
        if (cmd->commandId == CMD_BIN)
            _WriteQpfFile(cmd);
        cmd = cmd_list->GetNext();
    }
#endif
    return(1);
}

//---------------
// _WriteQpfFile 
//---------------
// gets bc info, opens, writes, and closes an QPF file

int
Qpf::_WriteQpfFile(
    Command*    cmd)
{
    //-------------------------------
    // read in the binning constants 
    //-------------------------------

    BinningConstants bc;
    bc.ReadNml(cmd->dataFilename);
    unsigned short bc_short_array[BINNING_CONSTANTS_SIZE/2];
    char* bc_char_array = (char *)bc_short_array;
    bc.Array(bc_char_array);

    //------------------------
    // calculate the checksum 
    //------------------------

    int sum = 0x55aa;
    for (int i = 0; i < (BINNING_CONSTANTS_SIZE - 2) / 2; i++)
    {
        sum += bc_short_array[i];
        if (sum & 0x10000)
        {
            sum &= 0xffff;      // remove carry bit
            sum += 1;           // end around
        }
    }
    unsigned short ushort_sum = (unsigned int)sum;
    memcpy(bc_char_array + CHECKSUM_OFFSET, &ushort_sum, 2);

    //-----------------------------
    // check for embedded commands 
    //-----------------------------

    int byte1, bit1, byte2, bit2;
    unsigned short cmd_code;
    char* cmd_name;
    char check_array[BINNING_CONSTANTS_SIZE+2];
    check_array[0] = 0x51;  // the binning constant command
    check_array[1] = 0x2e;
    memcpy(check_array+2, bc_char_array, BINNING_CONSTANTS_SIZE);
    if (CheckForCmds(check_array, BINNING_CONSTANTS_SIZE, &byte1, &bit1,
        &byte2, &bit2, &cmd_code, &cmd_name))
    {
        fprintf(stderr, "\n********** WARNING **********\n");
        fprintf(stderr, "QPF: Embedded command in binning constants\n");
        fprintf(stderr, "  Binning Constants File:\n    %s\n",
            cmd->dataFilename);
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
    // open the output file 
    //----------------------

    FILE* ofp = fopen(_filename, "w");
    if (! ofp)
    {
        fprintf(stderr, "QPF: error opening output file\n");
        fprintf(stderr, "  Filename: %s\n", _filename);
        fprintf(stderr, "Program aborting.\n");
        exit(1);
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

    fprintf(ofp,
        "QPF%07d %6s %4s %4s %8s %8s %4d %5d %8s %8s %8s V%02d %8s %3s %1d %24s\n",
        _fileNumber, QPF_PROJECT_NAME, QPF_AO_PROVIDER_CODE,
        QPF_NASDA_TACC_CODE, file_creation_date, file_creation_time,
        QPF_LENGTH_OF_DATA_RECORD, QPF_NUMBER_OF_DATA_RECORDS,
        begin_date_of_request, end_date_of_request,
        QPF_FILE_FORMAT_VERSION_DATE, QPF_FILE_FORMAT_VERSION_NUMBER,
        date_of_commanding, QPF_DATA_SPECIFICATION, cmd->binRepetition,
        QPF_RESERVED);

    //------------------------
    // write the data records 
    //------------------------

    int index = 0;
    for (int j = 0; j < 11; j++)
    {
        for (int byte = 0; byte < 63; byte++)
        {
            fprintf(ofp, "%04x ", bc_short_array[index]);
            index++;
        }
        fprintf(ofp, "%4s\n", "");
    }
    for (int byte = 0; byte < 59; byte++)
    {
        fprintf(ofp, "%04x ", bc_short_array[index]);
        index++;
    }
    fprintf(ofp, "%24s\n", "");

    //----------------
    // close the file 
    //----------------

    fclose(ofp);
    fchmod(fileno(ofp), QPF_MODE);
    _fileNumber++;

    return(1);
}
