//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "CommonDefs.h"
#include "Reqq.h"
#include "Command.h"
#include "CmdList.h"
#include "Binning.h"    // for embedded command check

#define REQQ_PATH_LEN          6
#define REQQ_GAMMA_LEN         7
#define REQQ_NULL_PATH_STRING  "*****"
#define REQQ_NULL_GAMMA_STRING "******"

//==============
// Reqq Methods 
//==============

Reqq::Reqq(
    Itime       start_time,
    Itime       end_time,
    const char* reqq_directory,
    int         file_number,
    int         cmd_id,
    int         last_mode_cmd_id)
:   _beginDateOfRequest(start_time), _endDateOfRequest(end_time),
    _directory(0), _filename(0), _fileNumber(file_number), _cmdId(cmd_id),
    _lastModeCmdId(last_mode_cmd_id)
{
    if ((_directory = strdup(reqq_directory)) == 0)
    {
        fprintf(stderr, "Reqq: Out of memory\n");
        exit(1);
    }
    char filename[BIG_SIZE];
    (void)sprintf(filename, "%s/REQQ%06d", _directory, _fileNumber);
    if ((_filename = strdup(filename)) == 0)
    {
        fprintf(stderr, "Reqq: Out of memory\n");
        exit(1);
    }
    return;
}

Reqq::~Reqq()
{
    if (_directory) free(_directory);
    if (_filename) free(_filename);
    return;
}

//-------
// Write 
//-------
// pass through a command list and write out REQQ commands

int
Reqq::Write(
    CmdList*    cmd_list)
{
    Command* cmd = cmd_list->GetHead();
    for (int cmd_index = 0; cmd; cmd_index++)
    {
        ReqqCmdTypeE cmd_type = _CmdType(cmd);
        switch(cmd_type)
        {
        case REQQ:
            if (! _ofp)
            {
                _OpenFile();
                _WriteHeader();
            }
            _WriteRecord(cmd);
            break;
        case ERROR:
            fprintf(stderr, "REQQ: command is specified as automatic but is not allowable in an REQQ\n");
            cmd->WriteForHumans(stderr);
            fprintf(stderr, "Program aborted.");
            exit(1);
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
    }

    return(1);
}

//--------------
// ReqqMnemonic 
//--------------

const char*
Reqq::ReqqMnemonic(
    Command*    cmd)
{
    return(cmd->CmdString());

} // Reqq::ReqqMnemonic

//--------------
// _WriteRecord 
//--------------
// writes an REQQ record to the appropriate file
// opens the file if it is not already opened

int
Reqq::_WriteRecord(
    Command*    cmd)
{
    //------------------------
    // determine the mnemonic 
    //------------------------

    const char* mnemonic = ReqqMnemonic(cmd);
    if (! mnemonic)
    {
        fprintf(stderr, "REQQ: %s command not allowable in an REQQ\n",
            cmd->CmdString());
        cmd->WriteForHumans(stderr);
        fprintf(stderr, "Program aborted.");
    }

#if 0
    //--------------------------
    // generate the date string 
    //--------------------------
    char date_string[BDATE_TIME_LEN];
    cmd->plannedTpg.time.ItimeToBDate(date_string);
#endif

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

    //----------------------------------------
    // get antenna command data, if necessary 
    //----------------------------------------
    char ascii_string[32];
    char hex_string[32];
    char* antenna_data;
    if (_CmdType(cmd) == REQQ && cmd->dataFilename != 0)
    {
        ReadAntennaSequence(cmd->dataFilename, ascii_string, hex_string);
        sprintf(cmd->dataFilename, "%s", ascii_string);
        antenna_data = hex_string;
    }
    else
    {
        antenna_data = "********";
    }

    //------------------
    // write the record 
    //------------------
    fprintf(_ofp, "JPL%09d %6s %3s %-24s %5s %6s %8s\n",
        use_cmd_id, REQQ_SENSOR_NAME, mnemonic, timeString,
        pathString, gammaString, antenna_data);

    _recordCount++;

    return(1);
}

//----------
// _CmdType 
//----------

Reqq::ReqqCmdTypeE
Reqq::_CmdType(
    Command*    cmd)
{
    // realtime commands belong in Rtcf
    if (cmd->GetCommandType() == Command::REALTIME_COMMAND)
        return(NOT_REQQ);
    else if (cmd->GetCommandType() == Command::AUTOMATIC_COMMAND)
    {
        int numWordsInParamFile = cmd->GetNumWordsInParamFile();
        // number of words in param file is btwn 0 and 2 is a REQQ
        if (numWordsInParamFile >= 0 && numWordsInParamFile <= 2)
            return(REQQ);
        else
            // long number of words: Qpf
            return(NOT_REQQ);
    }
    else
        return(ERROR);

} // Reqq::_CmdType

//--------------
// _ModeCommand 
//--------------

int
Reqq::_ModeCommand(
    Command*    cmd)
{
    if (cmd->commandId == EA_CMD_MODCAL ||
                 cmd->commandId == EA_CMD_MODWOM ||
                 cmd->commandId == EA_CMD_MODSTB)
    {
        return(1);
    }
    else
        return(0);
}

//-----------
// _OpenFile 
//-----------
// open file with the proper name
// return 1 on success, 0 on failure

int
Reqq::_OpenFile()
{
    _ofp = fopen(_filename, "w");
    if (! _ofp)
    {
        fprintf(stderr, "REQQ: error opening output file\n");
        fprintf(stderr, "  Filename: %s\n", _filename);
        fprintf(stderr, "Program aborting.\n");
        exit(1);
    }
    return(1);
}

//--------------
// _WriteHeader 
//--------------
// write the REQQ header record

int
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
        "REQQ%06d %6s %4s %4s %8s %8s %4d %5d %8s %8s %8s V%02d %39s\n",
        _fileNumber, REQQ_PROJECT_NAME, REQQ_AO_PROVIDER_CODE,
        REQQ_EOC_MMO_CODE, file_creation_date, file_creation_time,
        REQQ_DATA_RECORD_LENGTH, _recordCount, begin_date_of_request,
        end_date_of_request, REQQ_FILE_FORMAT_VERSION_DATE,
        REQQ_FILE_FORMAT_VERSION_NUMBER, REQQ_RESERVED);
    return(1);
}

//---------------------
// ReadAntennaSequence 
//---------------------

int
ReadAntennaSequence(
    const char*     filename,
    char*           ascii_string,
    char*           hex_string)
{
    //---------------
    // open the file 
    //---------------

    FILE* ifp = fopen(filename, "r");
    if (! ifp)
    {
        fprintf(stderr, "REQQ: error opening antenna sequence file\n");
        fprintf(stderr, "  Antenna Sequence Filename: %s\n", filename);
        fprintf(stderr, "Program aborting.\n");
        exit(1);
    }

    //----------------------
    // read in the sequence 
    //----------------------

    char beam[8][3];
    int i=0;
    for (i = 0; i < 8; i++)
    {
        if (fscanf(ifp, " %2s", (char*)&(beam[i])) != 1)
        {
            fclose(ifp);
            fprintf(stderr, "REQQ: error reading antenna sequence file\n");
            fprintf(stderr, "  Antenna Sequence Filename: %s\n", filename);
            fprintf(stderr, "Program aborting.\n");
            exit(1);
        }
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

    for (i = 0; i < 8; i++)
    {
        int match = 0;
        cmd_word.int_cmd <<= 3;     // shift for next antenna beam number
        for (int j = 0; j < 8; j++)
        {
            if (strcasecmp(beam[7-i], cur_beam_map[j]) == 0)
            {
                match = 1;
                cmd_word.int_cmd |= j;
                break;
            }
        }
        if (! match)
        {
            fprintf(stderr, "REQQ: unknown antenna beam in antenna sequence file\n");
            fprintf(stderr, "  Antenna Sequence Filename: %s\n", filename);
            fprintf(stderr, "  Beam: %s\n", beam[i]);
            fprintf(stderr, "Program aborting.\n");
            exit(1);
        }
    }

    //------------------------
    // calculate the checksum 
    //------------------------

    unsigned long sum = 0xaa;
    for (i = 1; i < 4; i++)
    {
        sum += cmd_word.char_cmd[i];
        if (sum & 0x100)
        {
            // do the end around carry thing
            sum &= 0xff;    // take out the carry bit
            sum += 1;       // add it at the end
        }
    }

    //------------------
    // shuffle the data 
    //------------------

    // index zero never had anything
    cmd_word.char_cmd[0] = cmd_word.char_cmd[2];
    unsigned char tmp_char = cmd_word.char_cmd[1];
    cmd_word.char_cmd[1] = cmd_word.char_cmd[3];
    cmd_word.char_cmd[2] = (unsigned char)sum;
    cmd_word.char_cmd[3] = tmp_char;

    //-----------------------------
    // check for embedded commands 
    //-----------------------------

    char check_array[6];
    check_array[0] = 0xd0;  // the antenna sequence command
    check_array[1] = 0xaf;
    memcpy(check_array + 2, cmd_word.char_cmd, 4);

    int byte1, bit1, byte2, bit2;
    unsigned short cmd_code;
    char* cmd_name;
    if (CheckForCmds(check_array, 6, &byte1, &bit1, &byte2, &bit2,
        &cmd_code, &cmd_name))
    {
        fprintf(stderr, "\n********** WARNING **********\n");
        fprintf(stderr, "NPF: Embedded command in antenna sequence\n");
        fprintf(stderr, "  Antenna Sequence File:\n    %s\n",
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

    for (i = 0; i < 8; i++)
    {
        // convert character to upper case
        beam[i][1] = (char)toupper((int)beam[i][1]);
    }
    sprintf(ascii_string, "%2s %2s %2s %2s %2s %2s %2s %2s", beam[0], beam[1],
        beam[2], beam[3], beam[4], beam[5], beam[6], beam[7]);

    //--------------------
    // set the hex string 
    //--------------------
    sprintf(hex_string, "%08.8X", cmd_word.int_cmd);

    return(1);
}
