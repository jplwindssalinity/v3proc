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
//    Rev 1.20   25 Mar 1999 14:16:58   daffer
// changed EA_CMD_SCTMDONN to
// EA_CMD_SCTMDON
// 
//    Rev 1.19   23 Feb 1999 10:57:28   sally
// change AddSorted to AddSortedWithArgs
// so it won't mask SortedList<Command>::AddSorted
// 
//    Rev 1.18   29 Jan 1999 15:03:34   sally
// added LASP commands
//
//    Rev 1.17   20 Oct 1998 10:52:02   sally
// add static QPF commands (table macro commands)
// 
//    Rev 1.16   14 Oct 1998 16:06:46   sally
// fix type for checksum, should be unsigned short
// 
//    Rev 1.15   28 Aug 1998 11:22:20   sally
// moved ReqqStatusE out of Reqq class
// 
//    Rev 1.14   19 Aug 1998 13:46:12   daffer
// more Cmdlp/effect work
// 
//    Rev 1.13   01 Jul 1998 16:51:14   sally
// added table upload repetition
// 
//    Rev 1.12   05 Jun 1998 09:17:44   daffer
// Worked on 'FindNearestMatchable'
// 
//    Rev 1.11   01 Jun 1998 15:44:48   daffer
// Changed r+ to a+ in Read
// 
//    Rev 1.10   29 May 1998 14:21:00   daffer
// Modified Read method so it'll create the input file
// if none is there.
// 
//    Rev 1.9   22 May 1998 16:29:38   daffer
// Added/modified code for cmdlp/effect processing
// 
//    Rev 1.8   20 Apr 1998 15:17:54   sally
// change List to EAList
// 
//    Rev 1.7   17 Mar 1998 14:40:50   sally
// changed for REQQ
// 
//    Rev 1.6   16 Mar 1998 13:46:38   sally
// fix some parsing errors
// 
//    Rev 1.5   16 Mar 1998 10:51:52   sally
// ReadReqi() are split into two methods
// 
//    Rev 1.4   12 Mar 1998 17:15:26   sally
// adapt to the new QSCAT REQI format
// 
//    Rev 1.3   09 Mar 1998 16:33:48   sally
// adapt to the new REQI format
// 
//    Rev 1.2   26 Feb 1998 09:44:02   sally
// pacify GNU C++
// 
//    Rev 1.1   20 Feb 1998 10:55:50   sally
// L1 to L1A
// 
//    Rev 1.0   04 Feb 1998 14:14:50   daffer
// Initial checking
// Revision 1.3  1998/02/03 00:12:33  sally
// change NRT to L1AP
//
// Revision 1.2  1998/01/30 22:28:54  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================
 
#ifndef CMDLIST_C_INCLUDED
#define CMDLIST_C_INCLUDED

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "Reqq.h"
#include "CmdList.h"
#if 0
#include "LaspProcCmd.h"
#endif

int errno;
 
static const char rcs_cmd_list_id[] =
    "@(#) $Header$";
 
struct LaspProcCmdEntry
{
    unsigned short    cmdHex;
    EACommandE*       implicitCmds;
};

struct LaspCmdWithArgs
{
    EACommandE        cmdId;
    char*             args;
};


//=================
// CmdList methods 
//=================

CmdList::CmdList()
:   _status(OK)
{
    return;
} //CmdList::CmdList

CmdList::CmdList(
const char*     cmdlpFilename,    // cmdlp file
const Itime     start_time,
const Itime     end_time)
:   _status(OK)
{
    Read(cmdlpFilename, start_time, end_time);
    return;
}

CmdList::~CmdList()
{
    // empty, ~List() is sufficient

} //CmdList::~CmdList


//------
// Read 
//------

CmdList::StatusE
CmdList::Read(
    const char*     cmdlpFilename,     // cmdlp filename
    const Itime     start_time,
    const Itime     end_time)
{
    //---------------------
    // open the input file 
    //---------------------

    FILE* ifp = fopen(cmdlpFilename, "r");
    if (ifp == NULL)
    {
        if (errno == ENOENT) {
            // File may not exist, try to create it
            ifp = fopen(cmdlpFilename, "a+");
            if (ifp == NULL) {
                _status = ERROR_OPENING_FILE;
                return (_status);
            }
        }
    }

    //-----------------
    // read input file 
    //-----------------

    int done = 0;
    Command* newCmd;
    do
    {
        newCmd = new Command();
        if (newCmd->GetStatus() != Command::OK)
        {
            _status = CmdList::ERROR_CREATING_CMD;
            done = 1;
            break;
        }
        switch (newCmd->Read(ifp))
        {
            case Command::OK:
            {
                //----------------
                // check the time 
                //----------------

                if ((start_time != INVALID_TIME &&
                    newCmd->LatestTime() < start_time) ||
                    (end_time != INVALID_TIME && 
                    newCmd->EarliestTime() > end_time))
                {
                    delete newCmd;
                }
                else
                {
                    Append(newCmd);
                }
                break;
            }
            case Command::END_OF_FILE:
            {
                done = 1;
                break;
            }
            case Command::ERROR_READING_CMD:
            {
                _status = CmdList::ERROR_READING_CMD;
                done = 1;
                break;
            }
            default:
                break;
        }
    } while (! done);

    //----------------------
    // close the input file 
    //----------------------

    fclose(ifp);

    return (_status);
}

//===============================================
// ROUTINE: ReadReqi
// PURPOSE: Loads all commands from a .reqi file.
//===============================================
CmdList::StatusE
CmdList::ReadReqi(
    const char*     fileName)
{

    //=====> open the input file <=====
    FILE* ifp = fopen(fileName, "r");
    if (ifp == NULL) {
        _status = ERROR_OPENING_FILE;
         return (_status);
    }
 
    //=====> read input file <=====
    char line[MAX_LINE_SIZE];
    Command* newCmd = 0;
    int lineNo = 1;
    while (fgets(line, MAX_LINE_SIZE, ifp) == line)
    {
        // ignore blank or comment lines
        char *firstChar = line;
        if (*firstChar == '#' || *firstChar == '*' ||
                      *firstChar == '/' || *firstChar == '\n')
        {
            lineNo++;
            continue;
        }
        while (*firstChar == ' ')
            firstChar++;

        // command line starts with year (digit)
        if (isdigit(*firstChar))
        {
            // append the previous command
            if (newCmd)
                Append(newCmd);

            newCmd = new Command(lineNo, line);
            if (newCmd->GetStatus() != Command::OK)
            {
                _status = ERROR_READING_CMD;
                break;
            }
        }
        else
        {
            // this should be originator, commands, ...
            if (newCmd)
            {
                if (! newCmd->ReadReqiOptionalString(lineNo, line))
                {
                    _status = ERROR_READING_CMD;
                    break;
                }
            }
            else
            {
                fprintf(stderr, "\nLine %d: missing command line\n", lineNo);
                _status = ERROR_READING_CMD;
                break;
            }
        }

        // If this command doesn't generate a QPF file and it takes
        // parameters, get them and store them in the cmdParams
        // string.
        
        if (newCmd->GetFileFormat(newCmd) != Command::FILE_QPF &&
              newCmd->GetFileFormat(newCmd) != Command::FILE_QPF_STATIC) {
            // If there is a parameter file, read it.
            if (newCmd->dataFilename != 0) {
                unsigned short checksum;
                char paramString[STRING_LEN];
                if ( Reqq::ReadParamWords( newCmd->dataFilename,
                                           newCmd->GetNumWordsInParamFile(),
                                           newCmd->GetDatafileFormat(),
                                           checksum, paramString, 0) == REQQ_OK)
                    (void) newCmd->SetCmdParams(paramString);
                else 
                    _status = ERROR_READING_DATAFILE;
            }
        }
        lineNo++;
    }

    // append the last command
    if (_status == OK)
    {
        if (newCmd)
            Append(newCmd);
    }
 
    fclose(ifp);
    return (_status);

} // CmdList::ReadReqi

//===============================================
// ROUTINE: WriteReqi
// PURPOSE: Writes all commands in the list to 
//          an REQI file.
//===============================================
CmdList::StatusE
CmdList::WriteReqi(const char *fileName)
{
   FILE *fptr;
   Command *cmdPtr;
   char cPtr[1024];
 
   //=====> open the input file <=====
   fptr = fopen(fileName, "w");
   if (fptr == NULL) {
      _status = ERROR_OPENING_FILE;
       return (_status);
   }

   for (cmdPtr=GetHead(); cmdPtr; cmdPtr=GetNext()) {
      if (! cmdPtr->ToReqiString(cPtr))
            break;
      fprintf(fptr,"%s",cPtr); 

      // if this is not the last command, skip a blank line
      if ( ! IsTail(cmdPtr))
         fprintf(fptr, "\n");
   }

   fclose(fptr);  
   return(OK); 
}

//-------
// Write 
//-------

CmdList::StatusE
CmdList::Write(
    const char*     filename)
{
    //----------------------
    // open the output file 
    //----------------------

    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        _status = ERROR_OPENING_FILE;
        return (_status);
    }

    //-------------------
    // write output file 
    //-------------------

    for (Command* cmd = GetHead(); cmd; cmd = GetNext())
    {
        if (cmd->Write(ofp) != Command::OK)
        {
            _status = CmdList::ERROR_WRITING_CMD;
            break;
        }
    }

    //-----------------------
    // close the output file 
    //-----------------------

    fclose(ofp);

    return (_status);
} //CmdList::Write

//----------------
// WriteForHumans 
//----------------

CmdList::StatusE
CmdList::WriteForHumans(
    FILE*   ofp,
    Itime   start_time,
    Itime   end_time)
{
    //-------------------
    // write output file 
    //-------------------

    for (Command* cmd = GetHead(); cmd; cmd = GetNext())
    {
        Itime latest_time = cmd->LatestTime();
        if (start_time != INVALID_TIME && latest_time < start_time)
            continue;
        Itime earliest_time = cmd->EarliestTime();
        if (end_time != INVALID_TIME && earliest_time > end_time)
            continue;
        if (cmd->WriteForHumans(ofp) != Command::OK)
        {
            _status = CmdList::ERROR_WRITING_CMD;
            break;
        }
        fprintf(ofp, "\n");
    }

    return (_status);
} //CmdList::WriteForHumans

//----------
// ExpandBC 
//----------
// converts each binning constants into the bin repetition number of
// commands

CmdList::StatusE
CmdList::ExpandBC()
{
#if 0
    for (Command* cmd = GetHead(); cmd; cmd = GetNext())
    {
        if (cmd->commandId == CMD_BIN && cmd->tableRepetition > 1)
        {
            cmd->tableRepetition = 1;
            Command* new_cmd = new Command;
            *new_cmd = *cmd;
            InsertAfter(new_cmd);
        }
    }
#endif
    return(_status);
}

//---------------
// SetCmdEffects 
//---------------
// pass through the command list and set the effect field of each command

CmdList::StatusE
CmdList::SetCmdEffects()
{
    CmdEffect cmd_effect;
    for (Command* cmd = GetHead(); cmd; cmd = GetNext())
    {
        if (cmd->commandId == EA_CMD_UNKNOWN || cmd->commandId == EA_CMD_NONE)
            continue;
        cmd->effectId = cmd_effect.ApplyCmd(cmd);
    }
    return (_status);
}

//---------------------
// SetCmdExpectedTimes 
//---------------------
// pass through the command list and update the expected times based
// on the eqx list, or the planned time of the command, if the path 
// isn't valid.
//

CmdList::StatusE
CmdList::SetCmdExpectedTimes(
    EqxList*    eqxList)
{
    Itime itime;
    for (Command* cmd = GetHead(); cmd; cmd = GetNext())
    {
        if (cmd->plannedTpg.path != INVALID_PATH &&
            eqxList->TpgToItime(cmd->plannedTpg, &itime)) {
            cmd->expectedTime = itime;
        } 
    }
    return (_status);
}

//---------------
// AddSortedCmds 
//---------------
// removes the cmds in cmds and adds them to the command list (sorted)

CmdList::StatusE
CmdList::AddSortedCmds(
    CmdList*    cmds)
{
    Command* cmd = cmds->GetHead();
    while ((cmd = cmds->RemoveCurrent()))
        AddSortedWithArgs(cmd, 0);

    return (_status);
}

void
CmdList::AddSortedWithArgs(
Command*     cmd,
const char*  args)     // arguments string
{
    //------------------------------------------------------------
    // if this is LASP PROC, then change the command to unverifiable (or N/A).
    // in any case, add this command as a regular command.
    //------------------------------------------------------------
    if (EA_IS_LASP_PROC(cmd->cmdHex))
    {
        cmd->l1aVerify = cmd->hk2Verify = cmd->l1apVerify = VER_NA;
    }

    SortedList<Command>::AddSorted(cmd);

    if ( ! EA_IS_LASP_PROC(cmd->cmdHex))
        return;

    //------------------------------------------------------------
    // for the LASP PROCs, need to tag on the implied inst commands
    //------------------------------------------------------------
    if ( ! Expand(cmd, args))
    {
        fprintf(stderr, "Error: Can't expand the LASP proc commands: %s\n",
                        cmd->mnemonic);
    }
   
    return;

} // CmdList::AddSorted

int 
CmdList::Expand(
Command*     cmd,      // LASP proc command to be expanded
const char*  args)     // arguments string
{
    assert(cmd != 0);

    if ( ! EA_IS_LASP_PROC(cmd->cmdHex))
        return 0;

    Command* newCmd=0;
    switch(cmd->commandId)
    {
        case EA_CMD_PPS_TURN_ON:
        {
             EACommandE trigCmdIds[] = { EA_CMD_SCK1SET, EA_CMD_SCK2SET,
                                         EA_CMD_SCK3SET, EA_CMD_SCK4RST,
                                         EA_CMD_SCK5RST, EA_CMD_SCK6RST };
             for (unsigned int i=0; i < ElementNumber(trigCmdIds); i++)
             {
                 newCmd = new Command(trigCmdIds[i]);
                 newCmd->expectedTime = cmd->expectedTime;
                 newCmd->SetComments("Triggered by LASP proc PPS_TURN_ON");
                 SortedList<Command>::AddSorted(newCmd);
             }
             break;
        }
        case EA_CMD_CDS_TURN_ON:
        {
            char cdsString[STRING_LEN];
            if (args == 0)
            {
                FILE* ifp = _OpenDatafile(cmd);
                if (ifp == 0) return 0;
                if (fscanf(ifp, "%s", cdsString) != 1)
                {
                    fprintf(stderr, "Error reading datafile: %s\n",
                                            cmd->dataFilename);
                    return 0;
                }
            }
            else
            {
                (void)strncpy(cdsString, args, STRING_LEN - 1);
                cdsString[STRING_LEN - 1] = '\0';
            }
            *cdsString = toupper(*cdsString);
            if (*cdsString == 'A')
            {
                 newCmd = new Command(EA_CMD_SCK7SET);
                 newCmd->expectedTime = cmd->expectedTime;
                 newCmd->SetComments("Triggered by LASP proc CDS_TURN_ON");
                 SortedList<Command>::AddSorted(newCmd);
            }
            else if (*cdsString == 'B')
            {
                 newCmd = new Command(EA_CMD_SCK7RST);
                 newCmd->expectedTime = cmd->expectedTime;
                 newCmd->SetComments("Triggered by LASP proc CDS_TURN_ON");
                 SortedList<Command>::AddSorted(newCmd);
            }
            else
            {
                fprintf(stderr, "Invalid CDS string (%s) for %s\n",
                          cdsString, cmd->mnemonic);
                return 0;
            }
            EACommandE trigCmdIds[] = { EA_CMD_SCK8SET,  EA_CMD_SCK13RST,
                                        EA_CMD_SCK14SET, EA_CMD_SCK17RST,
                                        EA_CMD_SCK18SET, EA_CMD_SCK1RST,
                                        EA_CMD_SCK2RST,  EA_CMD_SCK3RST,
                                        EA_CMD_SCWDTDSCL, EA_CMD_SCWDTEN };
             for (unsigned int i=0; i < ElementNumber(trigCmdIds); i++)
             {
                 newCmd = new Command(trigCmdIds[i]);
                 newCmd->expectedTime = cmd->expectedTime;
                 newCmd->SetComments("Triggered by LASP proc CDS_TURN_ON");
                 SortedList<Command>::AddSorted(newCmd);
             }
             break;
        }
        case EA_CMD_CDS_TURN_OFF:
        {
             EACommandE trigCmdIds[] = { EA_CMD_SCK1SET, EA_CMD_SCK2SET,
                                         EA_CMD_SCK3SET, EA_CMD_SCK4RST,
                                         EA_CMD_SCK5RST, EA_CMD_SCK6RST };
             for (unsigned int i=0; i < ElementNumber(trigCmdIds); i++)
             {
                 newCmd = new Command(trigCmdIds[i]);
                 newCmd->expectedTime = cmd->expectedTime;
                 newCmd->SetComments("Triggered by LASP proc CDS_TURN_OFF");
                 SortedList<Command>::AddSorted(newCmd);
             }
             break;
        }
        case EA_CMD_SAS_TURN_ON:
        {
            char sasString[STRING_LEN], rpmString[STRING_LEN];
            if (args == 0)
            {
                FILE* ifp = _OpenDatafile(cmd);
                if (ifp == 0) return 0;
                if (fscanf(ifp, "%s %s", sasString, rpmString) != 2)
                {
                    fprintf(stderr, "Error reading datafile: %s\n",
                                                cmd->dataFilename);
                    return 0;
                }
            }
            else
            {
                if (sscanf(args, "%s %s", sasString, rpmString) != 2)
                {
                    fprintf(stderr, "Error reading from args (%s)\n", args);
                    return 0;
                }
            }
            *sasString = toupper(*sasString);
            *rpmString = toupper(*rpmString);
            if (*sasString == 'A')
            {
                if (strcmp(rpmString, "R18_0") == 0)
                {
                    newCmd = new Command(EA_CMD_SCSSARST);
                    newCmd->expectedTime = cmd->expectedTime;
                    newCmd->SetComments("Triggered by LASP proc SAS_TURN_ON");
                    SortedList<Command>::AddSorted(newCmd);
                }
                else if (strcmp(rpmString, "R19_8") == 0)
                {
                    newCmd = new Command(EA_CMD_SCSSASET);
                    newCmd->expectedTime = cmd->expectedTime;
                    newCmd->SetComments("Triggered by LASP proc SAS_TURN_ON");
                    SortedList<Command>::AddSorted(newCmd);
                }
                else
                {
                    fprintf(stderr, "Invalid RPM string (%s) for %s\n",
                          rpmString, cmd->mnemonic);
                    return 0;
                }
                EACommandE trigCmdIds[] = { EA_CMD_SCK19SET, EA_CMD_SCK20SET,
                                            EA_CMD_SCK17SET, EA_CMD_SCK18SET };
                for (unsigned int i=0; i < ElementNumber(trigCmdIds); i++)
                {
                    newCmd = new Command(trigCmdIds[i]);
                    newCmd->expectedTime = cmd->expectedTime;
                    newCmd->SetComments("Triggered by LASP proc SAS_TURN_ON");
                    SortedList<Command>::AddSorted(newCmd);
                }
            }
            else if (*sasString == 'B')
            {
                if (strcmp(rpmString, "R18_0") == 0)
                {
                    newCmd = new Command(EA_CMD_SCSSBRST);
                    newCmd->expectedTime = cmd->expectedTime;
                    newCmd->SetComments("Triggered by LASP proc SAS_TURN_ON");
                    SortedList<Command>::AddSorted(newCmd);
                }
                else
                if (strcmp(rpmString, "R19_8") == 0)
                {
                    newCmd = new Command(EA_CMD_SCSSBSET);
                    newCmd->expectedTime = cmd->expectedTime;
                    newCmd->SetComments("Triggered by LASP proc SAS_TURN_ON");
                    SortedList<Command>::AddSorted(newCmd);
                }
                else
                {
                    fprintf(stderr, "Invalid RPM string (%s) for %s\n",
                          rpmString, cmd->mnemonic);
                    return 0;
                }
                EACommandE trigCmdIds[] = { EA_CMD_SCK19RST, EA_CMD_SCK20SET,
                                            EA_CMD_SCK17SET, EA_CMD_SCK18SET };
                for (unsigned int i=0; i < ElementNumber(trigCmdIds); i++)
                {
                    newCmd = new Command(trigCmdIds[i]);
                    newCmd->expectedTime = cmd->expectedTime;
                    newCmd->SetComments("Triggered by LASP proc SAS_TURN_ON");
                    SortedList<Command>::AddSorted(newCmd);
                }
            }
            else
            {
                fprintf(stderr,
                          "Invalid SAS string (%s) for %s\n",
                          sasString, cmd->mnemonic);
                return 0;
            }
            break;
        }
        case EA_CMD_SAS_TURN_OFF:
        {
             EACommandE trigCmdIds[] = { EA_CMD_SCK17RST, EA_CMD_SCK18SET };
             for (unsigned int i=0; i < ElementNumber(trigCmdIds); i++)
             {
                 newCmd = new Command(trigCmdIds[i]);
                 newCmd->expectedTime = cmd->expectedTime;
                 newCmd->SetComments("Triggered by LASP proc SAS_TURN_OFF");
                 SortedList<Command>::AddSorted(newCmd);
             }
             break;
        }
        case EA_CMD_SES_TURN_ON:
        {
            char sesString[STRING_LEN];
            if (args == 0)
            {
                FILE* ifp = _OpenDatafile(cmd);
                if (ifp == 0) return 0;
                if (fscanf(ifp, "%s", sesString) != 1)
                {
                    fprintf(stderr, "Error reading datafile: %s\n",
                                            cmd->dataFilename);
                    return 0;
                }
            }
            else
            {
                (void)strncpy(sesString, args, STRING_LEN - 1);
                sesString[STRING_LEN - 1] = '\0';
            }
            *sesString = toupper(*sesString);
            EACommandE trigCmdIds_1[] = { EA_CMD_SCK9RST, EA_CMD_SCK10SET };
            for (unsigned int i=0; i < ElementNumber(trigCmdIds_1); i++)
            {
                newCmd = new Command(trigCmdIds_1[i]);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc SES_TURN_ON");
                SortedList<Command>::AddSorted(newCmd);
            }
            if (*sesString == 'A')
            {
                newCmd = new Command(EA_CMD_SCK15SET);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc SES_TURN_ON");
                SortedList<Command>::AddSorted(newCmd);
            }
            else if (*sesString == 'B')
            {
                newCmd = new Command(EA_CMD_SCK15RST);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc SES_TURN_ON");
                SortedList<Command>::AddSorted(newCmd);
            }
            else
            {
                fprintf(stderr, "Invalid SES string (%s) for %s\n",
                          sesString, cmd->mnemonic);
                return 0;
            }
            EACommandE trigCmdIds_2[] = { EA_CMD_SCK16SET, EA_CMD_SCK13SET,
                                           EA_CMD_SCK14SET };
            for (int i=0; i < ElementNumber(trigCmdIds_2); i++)
            {
                newCmd = new Command(trigCmdIds_2[i]);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc SES_TURN_ON");
                SortedList<Command>::AddSorted(newCmd);
            }
            break;
        }
        case EA_CMD_TWTA_TURN_ON:
        {
            char twtString[STRING_LEN];
            if (args == 0)
            {
                FILE* ifp = _OpenDatafile(cmd);
                if (ifp == 0) return 0;
                if (fscanf(ifp, "%s", twtString) != 1)
                {
                    fprintf(stderr, "Error reading datafile: %s\n",
                                            cmd->dataFilename);
                    return 0;
                }
            }
            else
            {
                (void)strncpy(twtString, args, STRING_LEN - 1);
                twtString[STRING_LEN - 1] = '\0';
            }
            if (*twtString == '1')
            {
                EACommandE trigCmdIds[] = { EA_CMD_SCK11SET, EA_CMD_SCK12SET,
                                            EA_CMD_SCTWT1SEL, EA_CMD_SCK9SET,
                                            EA_CMD_SCK10SET };
                for (unsigned int i=0; i < ElementNumber(trigCmdIds); i++)
                {
                    newCmd = new Command(trigCmdIds[i]);
                    newCmd->expectedTime = cmd->expectedTime;
                    newCmd->SetComments("Triggered by LASP proc TWTA_TURN_ON");
                    SortedList<Command>::AddSorted(newCmd);
                }
            }
            else if (*twtString == '2')
            {
                EACommandE trigCmdIds[] = { EA_CMD_SCK11RST, EA_CMD_SCK12SET,
                                            EA_CMD_SCTWT2SEL, EA_CMD_SCK9SET,
                                            EA_CMD_SCK10SET };
                for (unsigned int i=0; i < ElementNumber(trigCmdIds); i++)
                {
                    newCmd = new Command(trigCmdIds[i]);
                    newCmd->expectedTime = cmd->expectedTime;
                    newCmd->SetComments("Triggered by LASP proc TWTA_TURN_ON");
                    SortedList<Command>::AddSorted(newCmd);
                }
            }
            else
            {
                fprintf(stderr, "Invalid TWT string (%s) for %s\n",
                          twtString, cmd->mnemonic);
                return 0;
            }
            newCmd = new Command(EA_CMD_TWTA_VFY);
            newCmd->expectedTime = cmd->expectedTime;
            newCmd->SetComments("Triggered by LASP proc TWTA_TURN_ON");
            AddSortedWithArgs(newCmd, 0);

            break;
        }
        case EA_CMD_SES_TURN_OFF:
        {
             EACommandE trigCmdIds[] = { EA_CMD_SCK13RST, EA_CMD_SCK14SET };
             for (unsigned int i=0; i < ElementNumber(trigCmdIds); i++)
             {
                 newCmd = new Command(trigCmdIds[i]);
                 newCmd->expectedTime = cmd->expectedTime;
                 newCmd->SetComments("Triggered by LASP proc SES_TURN_OFF");
                 SortedList<Command>::AddSorted(newCmd);
             }
             break;
        }
        case EA_CMD_TWTA_TURN_OFF:
        {
             EACommandE trigCmdIds[] = { EA_CMD_SCK9RST, EA_CMD_SCK10SET };
             for (unsigned int i=0; i < ElementNumber(trigCmdIds); i++)
             {
                 newCmd = new Command(trigCmdIds[i]);
                 newCmd->expectedTime = cmd->expectedTime;
                 newCmd->SetComments("Triggered by LASP proc TWTA_TURN_OFF");
                 SortedList<Command>::AddSorted(newCmd);
             }
             break;
        }
        case EA_CMD_TWTA_VFY:
        {
             EACommandE trigCmdIds[] = { EA_CMD_SCK21SET, EA_CMD_SCK22SET };
             for (unsigned int i=0; i < ElementNumber(trigCmdIds); i++)
             {
                 newCmd = new Command(trigCmdIds[i]);
                 newCmd->expectedTime = cmd->expectedTime;
                 newCmd->SetComments("Triggered by LASP proc TWTA_VFY");
                 SortedList<Command>::AddSorted(newCmd);
             }
             break;
        }
        case EA_CMD_INST_MODE:
        {
            char modeString[STRING_LEN];
            if (args == 0)
            {
                FILE* ifp = _OpenDatafile(cmd);
                if (ifp == 0) return 0;
                if (fscanf(ifp, "%s", modeString) != 1)
                {
                    fprintf(stderr, "Error reading datafile: %s\n",
                                            cmd->dataFilename);
                    return 0;
                }
            }
            else
            {
                (void)strncpy(modeString, args, STRING_LEN - 1);
                modeString[STRING_LEN - 1] = '\0';
            }
            char* ptr = modeString;
            for (unsigned int i=0; i < strlen(modeString); i++, ptr++)
            {
                *ptr = toupper(*ptr);
            }
            if (strcmp(modeString, "STBY") == 0)
            {
                newCmd = new Command(EA_CMD_SES_RESET_ENBL);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc INST_MODE");
                AddSortedWithArgs(newCmd, 0);

                newCmd = new Command(EA_CMD_SCMODSTB);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc INST_MODE");
                SortedList<Command>::AddSorted(newCmd);
            }
            else if (strcmp(modeString, "RCV") == 0)
            {
                newCmd = new Command(EA_CMD_SES_RESET_ENBL);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc INST_MODE");
                AddSortedWithArgs(newCmd, 0);

                newCmd = new Command(EA_CMD_SCMODRCV);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc INST_MODE");
                SortedList<Command>::AddSorted(newCmd);
            }
            else if (strcmp(modeString, "CAL") == 0)
            {
                newCmd = new Command(EA_CMD_SES_RESET_DSBL);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc INST_MODE");
                AddSortedWithArgs(newCmd, 0);

                newCmd = new Command(EA_CMD_SCMODCAL);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc INST_MODE");
                SortedList<Command>::AddSorted(newCmd);
            }
            else if (strcmp(modeString, "WIND") == 0)
            {
                newCmd = new Command(EA_CMD_SES_RESET_ENBL);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc INST_MODE");
                AddSortedWithArgs(newCmd, 0);

                newCmd = new Command(EA_CMD_SCMODWOM);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc INST_MODE");
                SortedList<Command>::AddSorted(newCmd);
            }
            else
            {
                fprintf(stderr, "Invalid MODE string (%s) for %s\n",
                          modeString, cmd->mnemonic);
                return 0;
            }
            break;
        }
        case EA_CMD_SES_RESET_ENBL:
        {
             EACommandE trigCmdIds[] = { EA_CMD_SCRSRLPR45 };
             for (unsigned int i=0; i < ElementNumber(trigCmdIds); i++)
             {
                 newCmd = new Command(trigCmdIds[i]);
                 newCmd->expectedTime = cmd->expectedTime;
                 newCmd->SetComments("Triggered by LASP proc SES_RESET_ENBL");
                 SortedList<Command>::AddSorted(newCmd);
             }
             break;
        }
        case EA_CMD_SES_RESET_DSBL:
        {
             EACommandE trigCmdIds[] = { EA_CMD_SCRSRLPR };
             for (unsigned int i=0; i < ElementNumber(trigCmdIds); i++)
             {
                 newCmd = new Command(trigCmdIds[i]);
                 newCmd->expectedTime = cmd->expectedTime;
                 newCmd->SetComments("Triggered by LASP proc SES_RESET_DSBL");
                 SortedList<Command>::AddSorted(newCmd);
             }
             break;
        }
        case EA_CMD_INST_QUIK_ON:
        {
            char sideString[STRING_LEN];
            if (args == 0)
            {
                FILE* ifp = _OpenDatafile(cmd);
                if (ifp == 0) return 0;
                if (fscanf(ifp, "%s", sideString) != 1)
                {
                    fprintf(stderr, "Error reading datafile: %s\n",
                                            cmd->dataFilename);
                    return 0;
                }
            }
            else
            {
                (void)strncpy(sideString, args, STRING_LEN - 1);
                sideString[STRING_LEN - 1] = '\0';
            }
            *sideString = toupper(*sideString);
            if (*sideString == 'A')
            {
                 LaspCmdWithArgs trigCmds[] =
                         { { EA_CMD_PLB_TURN_ON, "1" },
                           { EA_CMD_PPS_TURN_ON, "1" },
                           { EA_CMD_CDS_TURN_ON, "A" },
                           { EA_CMD_SAS_TURN_ON, "A R18_0 ABRV" },
                           { EA_CMD_SES_TURN_ON, "A" },
                           { EA_CMD_TWTA_TURN_ON, "1" },
                           { EA_CMD_SAS_SPIN_VFY2, "A R18_0" } };

                                              
                 for (unsigned int i=0; i < ElementNumber(trigCmds); i++)
                 {
                     newCmd = new Command(trigCmds[i].cmdId);
                     newCmd->expectedTime = cmd->expectedTime;
                     newCmd->SetComments("Triggered by LASP proc INST_QUIK_ON");
                     AddSortedWithArgs(newCmd, trigCmds[i].args);
                 }
            }
            else if (*sideString == 'B')
            {
                 LaspCmdWithArgs trigCmds[] =
                         { { EA_CMD_PLB_TURN_ON, "2" },
                           { EA_CMD_PPS_TURN_ON, "2" },
                           { EA_CMD_CDS_TURN_ON, "B" },
                           { EA_CMD_SAS_TURN_ON, "B R18_0 ABRV" },
                           { EA_CMD_SES_TURN_ON, "B" },
                           { EA_CMD_TWTA_TURN_ON, "2" },
                           { EA_CMD_SAS_SPIN_VFY2, "B R18_0" } };

                                              
                 for (unsigned int i=0; i < ElementNumber(trigCmds); i++)
                 {
                     newCmd = new Command(trigCmds[i].cmdId);
                     newCmd->expectedTime = cmd->expectedTime;
                     newCmd->SetComments("Triggered by LASP proc INST_QUIK_ON");
                     AddSortedWithArgs(newCmd, trigCmds[i].args);
                 }
            }
            else
            {
                fprintf(stderr, "Invalid argument (%s) for %s\n",
                          sideString, cmd->mnemonic);
                return 0;
            }
            break;
        }
        case EA_CMD_SES_RESET:
        {
             EACommandE trigCmdIds[] = { EA_CMD_SCSESRST };
             for (unsigned int i=0; i < ElementNumber(trigCmdIds); i++)
             {
                 newCmd = new Command(trigCmdIds[i]);
                 newCmd->expectedTime = cmd->expectedTime;
                 newCmd->SetComments("Triggered by LASP proc SES_RESET");
                 SortedList<Command>::AddSorted(newCmd);
             }
             break;
        }
        case EA_CMD_INST_TURN_OFF:
        {
            EACommandE trigCmdIds[] = { EA_CMD_TWTA_TURN_OFF,
                                        EA_CMD_SES_TURN_OFF,
                                        EA_CMD_SAS_TURN_OFF,
                                        EA_CMD_CDS_TURN_OFF };
            for (unsigned int i=0; i < ElementNumber(trigCmdIds); i++)
            {
                newCmd = new Command(trigCmdIds[i]);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc INST_TURN_OFF");
                AddSortedWithArgs(newCmd, 0);
            }
            break;
        }
        case EA_CMD_RAD_PAR_UPDATE:
        {
            int par;
            if (args == 0)
            {
                FILE* ifp = _OpenDatafile(cmd);
                if (ifp == 0) return 0;
                if (fscanf(ifp, "%d", &par) != 1)
                {
                    fprintf(stderr, "Error reading datafile: %s\n",
                                            cmd->dataFilename);
                    return 0;
                }
            }
            else
            {
                if (sscanf(args, "%d", &par) != 1)
                {
                    fprintf(stderr, "Error reading args: %s\n", args);
                    return 0;
                }
            }
            switch(par)
            {
            case 21:
                newCmd = new Command(EA_CMD_SCGATEWID21);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc RAD_PAR_UPDATE");
                SortedList<Command>::AddSorted(newCmd);
                break;
            case 20:
                newCmd = new Command(EA_CMD_SCGATEWID20);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc RAD_PAR_UPDATE");
                SortedList<Command>::AddSorted(newCmd);
                break;
            case 18:
                newCmd = new Command(EA_CMD_SCGATEWID18);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc RAD_PAR_UPDATE");
                SortedList<Command>::AddSorted(newCmd);
                break;
            case 17:
                newCmd = new Command(EA_CMD_SCGATEWID17);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc RAD_PAR_UPDATE");
                SortedList<Command>::AddSorted(newCmd);
                break;
            case 16:
                newCmd = new Command(EA_CMD_SCGATEWID16);
                newCmd->expectedTime = cmd->expectedTime;
                newCmd->SetComments("Triggered by LASP proc RAD_PAR_UPDATE");
                SortedList<Command>::AddSorted(newCmd);
                break;
            default:
                fprintf(stderr, "Invalid argument (%d) for %s\n",
                              par, cmd->mnemonic);
                return 0;
            }
            newCmd = new Command(EA_CMD_SES_RESET);
            newCmd->expectedTime = cmd->expectedTime;
            newCmd->SetComments("Triggered by LASP proc RAD_PAR_UPDATE");
            AddSortedWithArgs(newCmd, 0);
            break;
        }
        case EA_CMD_SCAT_MOD_ON:
        {
             EACommandE trigCmdIds[] = { EA_CMD_SCTMDON };
             for (unsigned int i=0; i < ElementNumber(trigCmdIds); i++)
             {
                 newCmd = new Command(trigCmdIds[i]);
                 newCmd->expectedTime = cmd->expectedTime;
                 newCmd->SetComments("Triggered by LASP proc SCAT_MOD_ON");
                 SortedList<Command>::AddSorted(newCmd);
             }
             break;
        }
        case EA_CMD_SCAT_MOD_OFF:
        {
             EACommandE trigCmdIds[] = { EA_CMD_SCTMDOFF };
             for (unsigned int i=0; i < ElementNumber(trigCmdIds); i++)
             {
                 newCmd = new Command(trigCmdIds[i]);
                 newCmd->expectedTime = cmd->expectedTime;
                 newCmd->SetComments("Triggered by LASP proc SCAT_MOD_OFF");
                 SortedList<Command>::AddSorted(newCmd);
             }
             break;
        }
        default:
             return 1;
    }
    return 1;

} // CmdList::Expand

//----------------------
// FindNearestMatchable 
//----------------------

Command*
CmdList::FindNearestMatchable(
    Command*    effect,
    int*        index)
{
    int match_index = 0;
    Command* match_command = NULL;
    Itime min_time_dif = MAX_TIME;

    Itime effect_time = effect->BestTime();
    EffectE effect_id = effect->effectId;

    Itime time_dif;

    int try_index = 0;

    for (Command* cmd = GetHead(); cmd; cmd = GetNext()) {
        //-------------------------------
        // determine the time difference 
        //-------------------------------

        if (cmd->expectedTime > effect_time)
            time_dif = cmd->expectedTime - effect_time;
        else
            time_dif = effect_time - cmd->expectedTime;
        if (cmd->effectId == EFF_COMMAND_HISTORY_CHANGE) {
            if (time_dif < min_time_dif && 
                effect->commandId == (EACommandE) cmd->effect_value) {
                if ((cmd->l1aTime == INVALID_TIME ||
                     effect->l1aTime == INVALID_TIME) &&
                    (cmd->l1apTime == INVALID_TIME ||
                     effect->l1apTime == INVALID_TIME) &&
                    (cmd->hk2Time == INVALID_TIME ||
                     effect->hk2Time == INVALID_TIME)) {
                    match_index = try_index;
                    match_command = cmd;
                    min_time_dif = time_dif;
                }
            }
        } else if (effect->effectId == EFF_COMMAND_HISTORY_CHANGE) {
            if (time_dif < min_time_dif && 
                cmd->commandId == (EACommandE) effect->effect_value) {
                if ((cmd->l1aTime == INVALID_TIME ||
                     effect->l1aTime == INVALID_TIME) &&
                    (cmd->l1apTime == INVALID_TIME ||
                     effect->l1apTime == INVALID_TIME) &&
                    (cmd->hk2Time == INVALID_TIME ||
                     effect->hk2Time == INVALID_TIME)) {
                    match_index = try_index;
                    match_command = cmd;
                    min_time_dif = time_dif;
                }
            }
        } else {

          //-----------------------------
          // check for closest matchable 
          //-----------------------------

          if (time_dif < min_time_dif &&
                  cmd->Matchable(cmd->commandId, effect_id)) {
              if ((cmd->l1aTime == INVALID_TIME ||
                   effect->l1aTime == INVALID_TIME) &&
                  (cmd->l1apTime == INVALID_TIME ||
                   effect->l1apTime == INVALID_TIME) &&
                  (cmd->hk2Time == INVALID_TIME ||
                   effect->hk2Time == INVALID_TIME)) {
                  match_index = try_index;
                  match_command = cmd;
                  min_time_dif = time_dif;
              }
          }
        }
        try_index++;
    }

    if (match_command != NULL)
    {
        *index = match_index;
        return(match_command);
    }
    return(NULL);
}

//-----------------
// MissingL1AVerify 
//-----------------
// returns the last l1 unverified command prior to the last
// block of verified l1 commands
// returns 0 is no such l1 unverified command exists

Command*
CmdList::MissingL1AVerify(
    int*    index)
{
    int been_verified = 0;
    int try_index = 0;
    for (Command* cmd = GetTail(); cmd; cmd = GetPrev())
    {
        if (cmd->l1aVerify == VER_YES || cmd->l1aVerify == VER_NA ||
        cmd->l1aVerify == VER_OK)
            been_verified = 1;
        else if (been_verified)
        {
            *index = try_index;
            return(cmd);
        }
        try_index++;
    }
    return(0);
}

//-------------------
// MissingHk2Verify 
//-------------------
// returns the last hkdt unverified command prior to the last
// block of verified hkdt commands
// returns 0 is no such hkdt unverified command exists

Command*
CmdList::MissingHk2Verify(
    int*    index)
{
    int been_verified = 0;
    int try_index = 0;
    for (Command* cmd = GetTail(); cmd; cmd = GetPrev())
    {
        if (cmd->hk2Verify == VER_YES || cmd->hk2Verify == VER_NA ||
        cmd->hk2Verify == VER_OK)
            been_verified = 1;
        else if (been_verified)
        {
            *index = try_index;
            return(cmd);
        }
        try_index++;
    }
    return(0);
}

//------------------
// MissingL1ApVerify 
//------------------
// returns the last l1ap unverified command prior to the last
// block of verified l1ap commands
// returns 0 is no such l1ap unverified command exists

Command*
CmdList::MissingL1ApVerify(
    int*    index)
{
    int been_verified = 0;
    int try_index = 0;
    for (Command* cmd = GetTail(); cmd; cmd = GetPrev())
    {
        if (cmd->l1apVerify == VER_YES || cmd->l1apVerify == VER_NA ||
        cmd->l1apVerify == VER_OK)
            been_verified = 1;
        else if (been_verified)
        {
            *index = try_index;
            return(cmd);
        }
        try_index++;
    }
    return(0);
}
    
/*
//-----------------
// ReportAnomalies 
//-----------------

CmdList::StatusE
CmdList::ReportAnomalies(
    FILE*   fp,
    Itime   start_time,
    Itime   end_time,
    int*    anomaly_count)
{
    *anomaly_count = 0;
    for (Command* cmd = GetHead(); cmd; cmd = GetNext())
    {
        // reject commands outside of specified time
        Itime detected_time = cmd->DetectedTime();
        if (start_time != INVALID_TIME)
        {
            if (cmd->expectedTime == INVALID_TIME)
            {
                if (cmd->plannedTpg.time + Itime(SEC_PER_DAY) < start_time &&
                    detected_time < start_time)
                {
                    continue;
                }
            }
            else
            {
                if (cmd->expectedTime < start_time &&
                    detected_time < start_time)
                {
                    continue;
                }
            }
        }
        if (end_time != INVALID_TIME)
        {
            if (cmd->expectedTime == INVALID_TIME)
            {
                if (cmd->plannedTpg.time > end_time &&
                    detected_time > start_time)
                {
                    continue;
                }
            }
            else
            {
                if (cmd->expectedTime > end_time &&
                    detected_time > end_time)
                {
                    continue;
                }
            }
        }

        // check for unexpected command
        if (cmd->IsUnexpectedCmd())
        {
            (*anomaly_count)++;
            fprintf(fp, "Unexpected command:\n");
            cmd->WriteForHumans(fp);
        }

        if (detected_time == INVALID_TIME)
        {
            (*anomaly_count)++;
            fprintf(fp, "Unverified command:\n");
            cmd->WriteForHumans(fp);
        }
    }
    return (_status);
}


//--------------
// _MatchingCmd 
//--------------
// returns the first command that "matches" the target command

Command*
CmdList::_MatchingCmd(
    Command*    targetCmd)
{
    Command* real_time_match = 0;

    //---------------------------------------------------------
    // if target command has a command ID, try to find a match 
    //---------------------------------------------------------

    if (targetCmd->commandId)
    {
        for (Command* cmd = GetHead(); cmd; cmd = GetNext())
        {
            if (! targetCmd->CmdIdMatch(cmd))
                continue;   // id's don't match
            if (targetCmd->HasTimeConflictWith(cmd))
                continue;   // time conflict
            Itime target_time = targetCmd->DetectedTime();
            if (cmd->expectedTime.AbsDif(target_time) < THRESHOLD_APPROX_TIME)
            {
                return (cmd);
            }
            if (! cmd->IsRealTimeCmd())
                continue;   // not a real time command
            if (cmd->plannedTpg.time.StartOfDay() != target_time.StartOfDay())
                continue;   // not on same day
            if (real_time_match)
                continue;   // already a real time command
            real_time_match = cmd;  // remember first matching RTC
        }
    }

    //------------------------------------------------------
    // if target command has an effect, try to find a match 
    //------------------------------------------------------

    if (targetCmd->effect)
    {
        for (Command* cmd = GetHead(); cmd; cmd = GetNext())
        {
            if (! targetCmd->EffectMatch(cmd))
                continue;   // effects don't match
            if (targetCmd->HasTimeConflictWith(cmd))
                continue;   // time conflict
            Itime target_time = targetCmd->DetectedTime();
            if (cmd->expectedTime.AbsDif(target_time) < THRESHOLD_APPROX_TIME)
            {
                return (cmd);
            }
            if (! cmd->IsRealTimeCmd())
                continue;   // not a real time command
            if (cmd->plannedTpg.time.StartOfDay() != target_time.StartOfDay())
                continue;   // not on same day
            if (real_time_match)
                continue;   // already a real time command
            real_time_match = cmd;  // remember first matching RTC
        }
    }

    //-------------------------------------------
    // return nearest real time command if found 
    //-------------------------------------------

    if (real_time_match)
        return (real_time_match);

    return (0);
}
*/

FILE*
CmdList::_OpenDatafile(
Command*       cmd)
{
    if (cmd->dataFilename == 0)
    {
        fprintf(stderr, "cmd->mnemonic: datafile is not specified\n");
        return 0;
    }
    FILE* ifp = fopen(cmd->dataFilename, "r");
    if (ifp == 0)
    {
        fprintf(stderr, "Error opening datafile: %s\n", cmd->dataFilename);
        return 0;
    }
    return(ifp);

} //CmdList::_OpenDatafile

#endif
