//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "Command.h"
#include "CmdList.h"
#include "CmdState.h"
#include "Eqx.h"
#include "Itime.h"
 
static const char rcs_cmd_list_id[] =
    "@(#) $Header$";
 

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
        _status = ERROR_OPENING_FILE;
        return (_status);
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
        if (cmd->commandId == CMD_BIN && cmd->binRepetition > 1)
        {
            cmd->binRepetition = 1;
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
    CmdState cmd_state;
    for (Command* cmd = GetHead(); cmd; cmd = GetNext())
    {
        if (cmd->commandId == EA_CMD_UNKNOWN || cmd->commandId == EA_CMD_NONE)
            continue;
        cmd->effectId = cmd_state.ApplyCmd(cmd);
    }
    return (_status);
}

//---------------------
// SetCmdExpectedTimes 
//---------------------
// pass through the command list and update the expected times based
// on the eqx list (only for commands with a path and gamma)

CmdList::StatusE
CmdList::SetCmdExpectedTimes(
    EqxList*    eqxList)
{
    Itime itime;
    for (Command* cmd = GetHead(); cmd; cmd = GetNext())
    {
        if (cmd->plannedTpg.path != INVALID_PATH &&
            eqxList->TpgToItime(cmd->plannedTpg, &itime))
            cmd->expectedTime = itime;
    }
    return (_status);
}

/*
//--------
// Update 
//--------
// uses the passed cmds and the eqxList to update the command list

CmdList::StatusE
CmdList::Update(
    CmdList*    detectedCmds,
    EqxList*    eqxList)
{
    Sort();             // sort the list
    SetCmdEffects();        // determine command effects for matching
    SetCmdExpectedTimes(eqxList);   // update expected times
    ApplyCmds(detectedCmds);    // add the detected commands
    Sort();             // sort again
    SetCmdEffects();        // determine the effects again
    return (_status);
}

//-----------
// ApplyCmds 
//-----------
// applies the detected commands by matching/updating or inserting

CmdList::StatusE
CmdList::ApplyCmds(
    CmdList*    detectedCmds)
{
    for (Command* cmd = detectedCmds->GetHead(); cmd;
            cmd = detectedCmds->GetNext())
    {
        Command* matchingCmd = _MatchingCmd(cmd);
        if (matchingCmd)
        {
            // set time of original command
            if (cmd->l1Time != INVALID_TIME)
                matchingCmd->l1Time = cmd->l1Time;
            if (cmd->hkdtTime != INVALID_TIME)
                matchingCmd->hkdtTime = cmd->hkdtTime;
            if (cmd->l1apTime != INVALID_TIME)
                matchingCmd->l1apTime = cmd->l1apTime;
        }
        else
        {
            // need to add unmatched command to command list
            AddSorted(cmd);
        }
    }

    return (_status);
}
*/

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
        AddSorted(cmd);

    return (_status);
}

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

    int try_index = 0;
    for (Command* cmd = GetHead(); cmd; cmd = GetNext())
    {
        //-------------------------------
        // determine the time difference 
        //-------------------------------

        Itime effect_time = effect->BestTime();
        Itime time_dif;
        if (cmd->expectedTime > effect_time)
            time_dif = cmd->expectedTime - effect_time;
        else
            time_dif = effect_time - cmd->expectedTime;

        //-----------------------------
        // check for closest matchable 
        //-----------------------------

        EffectE effect_id = effect->effectId;
        if (time_dif < min_time_dif &&
                cmd->Matchable(cmd->commandId, effect_id))
        {
            if ((cmd->l1Time == INVALID_TIME ||
                effect->l1Time == INVALID_TIME) &&
                (cmd->l1apTime == INVALID_TIME ||
                effect->l1apTime == INVALID_TIME) &&
                (cmd->hkdtTime == INVALID_TIME ||
                effect->hkdtTime == INVALID_TIME))
            {
                match_index = try_index;
                match_command = cmd;
                min_time_dif = time_dif;
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
        if (cmd->l1Verify == VER_YES || cmd->l1Verify == VER_NA ||
        cmd->l1Verify == VER_OK)
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
// MissingHkdtVerify 
//-------------------
// returns the last hkdt unverified command prior to the last
// block of verified hkdt commands
// returns 0 is no such hkdt unverified command exists

Command*
CmdList::MissingHkdtVerify(
    int*    index)
{
    int been_verified = 0;
    int try_index = 0;
    for (Command* cmd = GetTail(); cmd; cmd = GetPrev())
    {
        if (cmd->hkdtVerify == VER_YES || cmd->hkdtVerify == VER_NA ||
        cmd->hkdtVerify == VER_OK)
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

#endif
