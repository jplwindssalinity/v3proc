//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.19   02 Jun 1999 16:20:18   sally
// add leap second adjustment
// 
//    Rev 1.18   10 May 1999 16:51:12   sally
// add "Append" option
// 
//    Rev 1.17   13 Oct 1998 15:32:50   sally
// 
// added L1B file
// 
//    Rev 1.16   27 Jul 1998 13:58:30   sally
// took out static polynomial table
// .
// 
//    Rev 1.15   29 May 1998 14:20:38   daffer
// Changed GetEALogOrExit to GetEALog.
// 
//    Rev 1.14   04 May 1998 10:52:24   sally
// added HK2 filters
// 
//    Rev 1.13   01 May 1998 14:44:42   sally
// added HK2 file
// 
//    Rev 1.12   21 Apr 1998 16:39:08   sally
// for L2B
// 
//    Rev 1.11   10 Apr 1998 14:04:10   daffer
//   Changed ConfigList to EAConfigList throughout
//   Also had to undo some of the Sally's work re ConfigList
// 
//    Rev 1.10   06 Apr 1998 16:26:26   sally
// merged with SVT
// 
//    Rev 1.9   27 Mar 1998 14:51:04   sally
// fixed some L1A_Derived stuff
// 
//    Rev 1.8   27 Mar 1998 09:57:18   sally
// added L1A Derived data
// 
//    Rev 1.7   24 Mar 1998 09:08:52   sally
// de-warn for GNU compiler
// 
//    Rev 1.6   23 Feb 1998 16:20:36   deliver
// check alertEnable == "0" also
// 
//    Rev 1.5   23 Feb 1998 10:27:42   sally
// add limit checker
// 
//    Rev 1.4   20 Feb 1998 16:42:58   daffer
// Added new EALog method, VWriteMsg, changed some calls to WriteMsg to VWriteMsg
// .
// 
//    Rev 1.3   20 Feb 1998 10:55:46   sally
// L1 to L1A
// 
//    Rev 1.2   17 Feb 1998 14:45:12   sally
// NOPM
// 
//    Rev 1.1   13 Feb 1998 15:10:42   sally
// change type for PolyTable's constructor
// .
// 
//    Rev 1.0   04 Feb 1998 14:14:38   daffer
// Initial checking
// Revision 1.5  1998/02/03 00:12:33  sally
// change NRT to L1AP
//
// Revision 1.4  1998/01/30 22:28:50  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================
 
static const char rcs_id_argsplus_c[] =
    "@(#) $Header$";

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Args.h"
#include "ArgsPlus.h"
#include "ParTab.h"

#include "EAConfigList.h"
#include "EALog.h"
#include "TlmFileList.h"
#include "LimitList.h"
#include "Filter.h"
#include "ToDo.h"
#include "CmdList.h"
#include "Eqx.h"
#include "PolyTable.h"
#include "LeapSecTable.h"

//==================
// ArgsPlus methods 
//==================

ArgsPlus::ArgsPlus(
    int         argc,
    char*       argv[],
    char*       config_filename,
    ArgInfo*    usage_array[])
:   Args(argc, argv, config_filename, usage_array)
{
  // printf( "In ArgsPlus constructor\n");
  // Create the EALog object, log failures to stderr
  if (Log.GetStatus() == EALog::EA_FAILURE) {
    fprintf(stderr, "%s: Error initializing EA Log Object\n",
            _programName);
    //exit(1);
    Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
  }       
  
  // check construction of Args
  switch(_status)
    {
    case OK:
        break;
    case ERROR_ALLOCATING_VALUE_STRING:
      fprintf(stderr, "%s: error allocating for config list\n",
              _programName);
      Log.VWriteMsg("%: error allocating for config list\n",_programName);
      
      //exit(1);
      Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
      break;
    case ERROR_ADDING_PAIR:
      fprintf(stderr, "%s: error adding pair to config list\n",
              _programName);
      Log.VWriteMsg("Error adding pair to config list\n");
      //exit(1);
      Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
      break;
    default:
      fprintf(stderr, "%s: unknown error\n", _programName);
      Log.WriteMsg("unknown error\n");
      //exit(1);
      Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
      break;
    }

    // check for useless arguments
    _BadOptionCheck();

    return;
}

ArgsPlus::~ArgsPlus()
{
    return;
}

//-----------------
// _BadOptionCheck 
//-----------------
// if invalid command line options are present, report and exit

void
ArgsPlus::_BadOptionCheck()
{
    for (EAStringPair* pair = _cmdLineConfigList.GetHead();
            pair != NULL;
            pair = _cmdLineConfigList.GetNext())
    {
      if (_IsBadOption(pair->keyword))
        {
          fprintf(stderr, "%s: invalid option %s\n", 
                  _programName,
                  pair->keyword);
          Usage();
          Log.VWriteMsg(": Invalid option %s",pair->keyword);
          Log.WriteMsg(pair->keyword);
          //exit(1);
          Log.WriteMsg(ArgsPlus::_UsageMsg);
          Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
        }
    }
    return;
}

//-------------------
// GetSourceIdOrExit 
//-------------------

SourceIdE
ArgsPlus::GetSourceIdOrExit(
    char*       tlm_type_string)
{
        if (! tlm_type_string)
        {
                fprintf(stderr, "%s: telemetry type <null> error\n", 
                        _programName);
                Log.WriteMsg(" telemetry type <null> error\n");
                //exit(1);
                Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
        }
        if (! tlm_type_string)
                return (SOURCE_UNKNOWN);

        SourceIdE tlm_source = ParTabAccess::GetSourceId(tlm_type_string);
        if (tlm_source == SOURCE_UNKNOWN)
        {
                fprintf(stderr, "%s: unknown telemetry type %s\n", 
                        _programName,
                        tlm_type_string);
                Log.VWriteMsg(": unknown telemetry type %s\n",tlm_type_string);
                Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
        }
        return(tlm_source);
}

//--------------------
// GetStartTimeOrExit 
//--------------------

#define TIME_STRING "    Time must be specified as YYYY-MM-DDTHH:MM:SS.mmmZ"

Itime
ArgsPlus::GetStartTimeOrExit(
    char*       start_time_string)
{
        if (! start_time_string)
        {
                fprintf(stderr, "%s: start time <null> error\n", 
                        _programName);
                Log.WriteMsg("start time <null> error\n");
                Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");  
        }

        Itime start_time;
        if (! start_time.CodeAToItime(start_time_string))
        {
                fprintf(stderr, "%s: invalid start time %s\n %s\n",
                        _programName, start_time_string,TIME_STRING);
                Log.VWriteMsg(": invalid start time %s\n %s\n ",start_time_string,TIME_STRING );
                //exit(1);
                Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
        }
        return(start_time);
}

//--------------
// GetStartTime 
//--------------

Itime
ArgsPlus::GetStartTime(
    char*   start_time_string)
{
    if (! start_time_string)
        return(INVALID_TIME);

        Itime start_time;
        if (! start_time.CodeAToItime(start_time_string))
        {
                fprintf(stderr, "%s: invalid start time %s\n %s\n",
                        _programName, start_time_string,TIME_STRING);
                Log.VWriteMsg(" invalid start time %s\n %s\n ",
                             start_time_string, TIME_STRING);
                //exit(1);
                Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
        }
        return(start_time);
}//ArgsPlus::GetStartTime

//------------------
// GetEndTimeOrExit 
//------------------

Itime
ArgsPlus::GetEndTimeOrExit(
    char*       end_time_string)
{
        if (! end_time_string)
        {
                fprintf(stderr, "%s: end time <null> error\n", _programName);
                Log.WriteMsg(" end time <null> error\n");
                //exit(1);
                Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
        }

        Itime end_time;
        if (! end_time.CodeAToItime(end_time_string))
        {
                fprintf(stderr, "%s: invalid end time %s\n %s\n",
                        _programName, end_time_string, TIME_STRING);
                Log.VWriteMsg(": invalid end time %s\n %s\n", 
                             end_time_string, TIME_STRING);
                Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
        }
        return(end_time);
}

//------------
// GetEndTime 
//------------

Itime
ArgsPlus::GetEndTime(
    char*   end_time_string)
{
    if (! end_time_string)
        return(INVALID_TIME);

        Itime end_time;
        if (! end_time.CodeAToItime(end_time_string))
        {
                fprintf(stderr, "%s: invalid end time %s\n %s\n",
                        _programName, end_time_string, TIME_STRING);
                Log.VWriteMsg(" invalid end time %s\n %s\n",
                             end_time_string, TIME_STRING);
                Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
        }
        return(end_time);
}//ArgsPlus::GetEndTime

//-------------------
// GetTlmFilesOrExit 
//-------------------

char*
ArgsPlus::GetTlmFilesOrExit(
    char*       tlm_files_string,
    SourceIdE   tlm_type,
    char*       hk2_files_string,
    char*       l1a_files_string,
    char*       l1ap_files_string,
    char*       l1adrv_files_string,
    char*       l1b_files_string)
{
    char* tlm_files=0;

    if (tlm_files_string)
        tlm_files = tlm_files_string;
    else
    {
        switch(tlm_type)
        {
        case SOURCE_UNKNOWN:
            fprintf(stderr, "%s: unknown Telemetry Type\n", _programName);
            Log.WriteMsg(" unknown Telemetry Type\n");
            //exit(1);
            Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
            break;

        case SOURCE_L1A:
            if (l1a_files_string)
                tlm_files = l1a_files_string;
            else
            {
                fprintf(stderr, "%s: unspecified L1A Telemetry Files\n",
                      _programName);
                Log.WriteMsg(" unspecified L1A Telemetry Files\n");
                Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
            }
            break;

        case SOURCE_HK2:
            if (hk2_files_string)
                tlm_files = hk2_files_string;
            else
            {
                fprintf(stderr, 
                      "%s: unspecified HK Telemetry Files\n",
                      _programName);
                Log.WriteMsg(" unspecified HK Telemetry Files\n");
                //exit(1);
                Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
            }
          break;

        case SOURCE_L1AP:
            if (l1ap_files_string)
                tlm_files = l1ap_files_string;
            else
            {
                fprintf(stderr, 
                      "%s: unspecified L1AP Telemetry Files\n",
                      _programName);
                Log.WriteMsg(" unspecified L1AP Telemetry Files\n");
                //exit(1);
                Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
            }
            break;

        case SOURCE_L1A_DERIVED:
            if (l1adrv_files_string)
                tlm_files = l1adrv_files_string;
            else
            {
                fprintf(stderr, "%s: unspecified L1A Derived Telemetry Files\n",
                                        _programName);
                Log.WriteMsg(" unspecified L1A Derived Telemetry Files\n");
                Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
            }
            break;

        case SOURCE_L1B:
            if (l1b_files_string)
                tlm_files = l1b_files_string;
            else
            {
                fprintf(stderr, "%s: unspecified L1B Telemetry Files\n",
                      _programName);
                Log.WriteMsg(" unspecified L1B Telemetry Files\n");
                Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
            }
            break;
        default:
            Log.WriteMsg(" unknown telemetry type\n");
            Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
            break;
        }
    }
    return(tlm_files);
}

//-------------------
// TlmFileListOrExit 
//-------------------

TlmFileList*
ArgsPlus::TlmFileListOrExit(
    SourceIdE       tlm_type,
    char*           tlm_files,
    Itime           start_time,
    Itime           end_time)
{
    if (! tlm_files)
    {
      fprintf(stderr,
              "%s: telemetry files <null> error\n", _programName);
      Log.WriteMsg(" telemetry files <null> error\n");
      //exit(1);
      Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
    }

    TlmFileList::StatusE fileListStatus;
    TlmFileList* tlm_file_list = new TlmFileList(tlm_type,
                                     (const char*)tlm_files,
                                     fileListStatus, start_time, end_time);
    switch (fileListStatus)
    {
    case TlmFileList::OK:
        Log.GetTlmFileList(tlm_file_list);  
        return (tlm_file_list);
        break;

    case TlmFileList::INVALID_SOURCE_ID:
        fprintf(stderr, "%s: unknown Telemetry Type\n", _programName);
        Log.WriteMsg(" unknown Telemetry Type\n");
        Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
        break;

    case TlmFileList::ERROR_CREATING_TLM_FILE:
        fprintf(stderr, "%s: error accessing Telemetry Files %s\n",
              _programName, tlm_files);
        Log.VWriteMsg(" error accessing Telemetry Files %s\n", tlm_files);
        Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
        break;

    case TlmFileList::NO_MORE_FILES:
        fprintf(stderr, "%s: no Telemetry Files\n", _programName);
        Log.VWriteMsg(" no Telemetry Files\n Telemetry Type: %s\n",
                   source_id_map[tlm_type]);
        fprintf(stderr, "Telemetry Type: %s\n", source_id_map[tlm_type]);
        char time_string[CODEA_TIME_LEN];
        if (start_time != INVALID_TIME)
        {
            start_time.ItimeToCodeA(time_string);
            fprintf(stderr, "    Start Time: %s\n", time_string);
            Log.VWriteMsg(   "    Start Time: %s\n", time_string);
        }
        if (end_time != INVALID_TIME)
        {
            end_time.ItimeToCodeA(time_string);
            fprintf(stderr, "      End Time: %s\n", time_string);
            Log.VWriteMsg(   "      End Time: %s\n", time_string);
        }
        Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
        break;

     default:
        break;
    }
    fprintf(stderr, 
          "%s: unknown error accessing Telemetry Files\n", _programName);
    Log.WriteMsg(" unknown error accessing Telemetry Files\n");
    Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
    return 0;

} //ArgsPlus::TlmFileListOrExit

//-----------------
// FilterSetOrNull 
//-----------------

FilterSet*
ArgsPlus::FilterSetOrNull(
    SourceIdE       tlm_type,
    char*           filters_string)
{
    if (! filters_string)
        return(NULL);

    const FilTabEntry* filter_table=0;
    switch(tlm_type)
    {
    case SOURCE_L1A:
    case SOURCE_L1AP:
    case SOURCE_L1A_DERIVED:
        filter_table = L1AFilTab;
        break;
    case SOURCE_HK2:
        filter_table = HK2FilTab;
        break;
    default:
        Log.VWriteMsg(" Filter table unavailable for this filetype\n");
        Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
        break;
    }

    char bad_string[FILTER_NAME_LENGTH] = "";
    FilterSet* filter_set = new FilterSet(filters_string,
                                        filter_table, bad_string);
    if (! filter_set)
    {
      fprintf(stderr, 
              "%s: error creating Filter from %s\n",
              _programName, filters_string);
      Log.VWriteMsg(" error creating Filter from %s\n", filters_string);
      Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
    }

    if (*bad_string != '\0')
    {
      fprintf(stderr, 
              "%s: unknown %s Filter abbreviation \"%s\"\n",
              _programName, source_id_map[tlm_type], bad_string);
      Log.VWriteMsg(" unknown %s Filter abbreviation\"%s\"\n",
                   source_id_map[tlm_type], bad_string);
      Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
    }

    return(filter_set);
}

//--------------------
// OutputFileOrStdout 
//--------------------

FILE*
ArgsPlus::OutputFileOrStdout(
    char*   output_filename,
    int     append)
{
    if (! output_filename)
        return (stdout);

    FILE* ofp;
    if (append)
        ofp = fopen(output_filename, "a");
    else
        ofp = fopen(output_filename, "w");
    if (! ofp)
    {
      fprintf(stderr, 
              "%s: error opening Output File %s\n", 
              _programName,
              output_filename);
      Log.VWriteMsg(" error opening Output File %s\n",output_filename);
      Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
    }
    return (ofp);
}

int
ArgsPlus::OutputFdOrStdout(
    char*   output_filename)
{
    // if filename is not given, ==> stdout
    if (! output_filename)
        return (1);

    int ofd = open(output_filename, O_CREAT| O_TRUNC | O_WRONLY, 0664);
    if (ofd == -1)
    {
      fprintf(stderr, 
              "%s: error opening Output File %s\n", 
              _programName,
              output_filename);
      Log.VWriteMsg(" error opening Output %s\n",output_filename);
      Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
    }
    return (ofd);

}//ArgsPlus::OutputFdOrStdout

//----------------
// ToDoListOrExit 
//----------------
ToDoList*
ArgsPlus::ToDoListOrExit(
const char* filename)
{
    //----------------------------------------------
    // if filename is NULL, exit
    //----------------------------------------------
    if (! filename)
    {
      fprintf(stderr, 
              "%s: ToDo filename not specified\n",
              _programName);
      Log.WriteMsg(" ToDo filename not specified\n");
      Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
    }

    //----------------------------------------------
    // if ToDoList creation failed, exit
    //----------------------------------------------
    ToDoList* todoList = new ToDoList(filename);
    if (! todoList)
    {
      fprintf(stderr, 
              "%s: error creating ToDoList from %s\n", 
              _programName,
              filename);
      Log.VWriteMsg(" error creating ToDoList from %s\n", filename);
      Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
    }
    if (todoList->GetStatus() == ToDoList::OK)
        return(todoList);
    else
    {
      fprintf(stderr, 
              "%s: error creating ToDoList from %s\n", 
              _programName,
              filename);
      Log.VWriteMsg(" error creating ToDoList from %s\n", filename);
      Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
    }
    return 0;

}//ArgsPlus::ToDoListOrExit

//--------------------
// GetLimitFileOrExit 
//--------------------

char*
ArgsPlus::GetLimitFileOrExit(
char*       tlm_limit_file_string,
SourceIdE   tlm_type,
char*       hk2_limit_file_string,
char*       l1a_limit_file_string,
char*       l1ap_limit_file_string,
char*       l1adrv_limit_file_string)
{
    char* limit_file=0;

    if (tlm_limit_file_string)
        limit_file = tlm_limit_file_string;
    else
    {
        switch(tlm_type)
        {
        case SOURCE_UNKNOWN:
            fprintf(stderr, "%s: unknown Telemetry Type\n", _programName);
            Log.WriteMsg(" unknown Telemetry Type\n");
            Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
            break;

        case SOURCE_L1A:
            if (l1a_limit_file_string)
                limit_file = l1a_limit_file_string;
            else
            {
                fprintf(stderr, "%s: unspecified L1A Limit File\n",
                                                      _programName);
                Log.WriteMsg(" unspecified L1A Limit File\n");
                Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
            }
            break;

        case SOURCE_HK2:
            if (hk2_limit_file_string)
                limit_file = hk2_limit_file_string;
            else
            {
                fprintf(stderr, "%s: unspecified HK Limit File\n",
                                                            _programName);
                Log.WriteMsg(" unspecified HK Limit File\n");
                Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
            }
            break;

        case SOURCE_L1AP:
            if (l1ap_limit_file_string)
                limit_file = l1ap_limit_file_string;
            else
            {
                fprintf(stderr, "%s: unspecified L1AP Limit File\n",
                      _programName);
                Log.WriteMsg(" unspecified L1AP Limit File\n");
                Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
            }
            break;

        case SOURCE_L1A_DERIVED:
            if (l1adrv_limit_file_string)
                limit_file = l1adrv_limit_file_string;
            else
            {
                fprintf(stderr, "%s: unspecified L1A Derived Limit File\n",
                      _programName);
                Log.WriteMsg(" unspecified L1A Derived Limit File\n");
                Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
            }
            break;

        default:
            Log.WriteMsg("No limit file available\n");
            Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
            break;
        }
    }
    return(limit_file);

}//ArgsPlus::GetLimitFileOrExit

//----------------------------
// LimitListForCheckingOrExit 
//----------------------------
// open the limit list file for reading only, discard disabled limit checkers

LimitList*
ArgsPlus::LimitListForCheckingOrExit(
    SourceIdE       tlm_type,
    const char*     filename,
    FILE*           output_file_pointer)
{
    return(_LimitListOrExit(tlm_type, filename, output_file_pointer, 0));
}

//----------------------------
// LimitListForUpdatingOrExit 
//----------------------------
// open the limit list file for reading and writing, read the entire list

LimitList*
ArgsPlus::LimitListForUpdatingOrExit(
    SourceIdE       tlm_type,
    const char*     filename,
    FILE*           output_file_pointer)
{
    return(_LimitListOrExit(tlm_type, filename, output_file_pointer, 1));
}

//------------------
// _LimitListOrExit 
//------------------

LimitList*
ArgsPlus::_LimitListOrExit(
    SourceIdE       tlm_type,
    const char*     filename,
    FILE*           output_file_pointer,
    const int       keep_disabled)
{
    LimitList* limitList = new LimitList(tlm_type, filename,
        output_file_pointer, keep_disabled);
    switch (limitList->GetStatus())
    {
    case LimitList::GOOD:
        return (limitList);
        break;
    default:
      fprintf(stderr, "%s: error opening limit file %s\n",
              _programName, filename);
      Log.VWriteMsg(" error opening limit file %s\n", filename);
      Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
      break;
    }
    return (0);
}

FILE*
ArgsPlus::AlertFileOrNull(
const char* alert_filename)
{
    if (! alert_filename)
        return(0);
    FILE* ofp;
    ofp = fopen(alert_filename, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening Alert File %s\n", _programName,
            alert_filename);
        Log.VWriteMsg(" error opening Alert File %s\n",alert_filename);
        Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
    }
    return (ofp);
}//ArgsPlus::AlertFileOrNull

//-------------
// CmdlpOrExit 
//-------------

CmdList*
ArgsPlus::CmdlpOrExit(
    const char*     cmdlp_filename)
{
    CmdList* cmdlp = new CmdList(cmdlp_filename);
    if (cmdlp->GetStatus() != CmdList::OK)
    {
      fprintf(stderr, "%s: error reading command list file %s\n",
              _programName, cmdlp_filename);
      Log.VWriteMsg(" error reading command list file %s\n",cmdlp_filename);
      Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
      
    }
    return(cmdlp);
}

//---------------
// EqxListOrExit 
//---------------

EqxList*
ArgsPlus::EqxListOrExit(
    const char*     eqx_filename)
{
    EqxList* eqx_list = new EqxList(eqx_filename);
    if (eqx_list->GetStatus() != EqxList::OK)
    {
      fprintf(stderr, "%s: error reading EQX File %s\n",
              _programName, eqx_filename);
      Log.VWriteMsg(" error reading EQX File %s\n",eqx_filename);
      Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
    }
    return(eqx_list);
}

//----------------
// MailAlertCheck 
//----------------
// complains if the user want to mail the alert file but has not
// specified an alert file or mail address

void
ArgsPlus::MailAlertCheck(
    const char*     mail_alert,
    const char*     alert_filename,
    const char*     mail_address)
{
    if (mail_alert == 0 || strcmp(mail_alert, "0") == 0)
        return;

    if (! alert_filename)
    {
      fprintf(stderr,
              "%s: an Alert File must be specified in order to Mail Alert\n",
              _programName);
      Log.WriteMsg(
                   ": an Alert File must be specified in order to Mail Alert\n");
      Usage();
      //exit(1);
      Log.WriteMsg(ArgsPlus::_UsageMsg);
      Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
    }

    if (! mail_address)
    {
      fprintf(stderr,
              "%s: a Mail Address must be specified in order to Mail Alert\n",
              _programName);
      Log.WriteMsg(
                   ": a Mail Address must be specified in order to Mail Alert\n"
                   );
      Usage();
      //exit(1);
      Log.WriteMsg(ArgsPlus::_UsageMsg);
      Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
    }
}

#if 0
//-----------
// AlertFile 
//-----------

FILE*
ArgsPlus::AlertFile(
    char*       alert_filename)
{
    FILE* fp = fopen(alert_filename, "w");
    if (fp == 0)
    {
      fprintf(stderr, "%s: error opening Alert File %s\n",
              _programName, alert_filename);
      Log.VWriteMsg(": error opening Alert File %s\n",alert_filename);
      Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
    }
    return(fp);
}

//---------
// LogFile 
//---------

// FILE*
// ArgsPlus::LogFile(
//      char*           log_filename)
// {
//   FILE* fp = fopen(log_filename, "a+");
//   if (fp == 0)
//     {
//       fprintf(stderr, "%s: error opening Log File %s\n",
//            _programName, log_filename);
////       exit(1);
//         Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
//     }
//   return(fp);
// }

//------------
// OutputFile 
//------------

FILE*
ArgsPlus::OutputFile(
    char*       output_filename)
{
    FILE* fp = fopen(output_filename, "w");
    if (fp == 0)
    {
        fprintf(stderr, "%s: error opening Output File %s\n",
            _programName, output_filename);
        Log.VWriteMsg( "error opening Output File %s\n",output_filename );
        Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
    }
    return(fp);
}
#endif

PolynomialTable*
ArgsPlus::PolynomialTableOrNull(
    char*   polynomialFilename)
{
    // if polynomial filename is not specified or is a null string, return 0
    if (polynomialFilename == 0 || polynomialFilename[0] == '\0')
        return 0;

    EA_PolynomialErrorNo polyStatus = EA_POLY_OK;
    PolynomialTable* polyTableP = new PolynomialTable(
                                      polynomialFilename, polyStatus);
    if ( polyStatus != EA_POLY_OK)
    {
        fprintf(stderr, "Creating PolynomialTable failed\n");
        Log.WriteMsg( "Creating PolynomialTable failed\n");
        Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
    }

    return(polyTableP);

}//ArgsPlus:PolynomialTableOrNull

void
ArgsPlus::LeapSecTableOrExit(
const char*   leapSecTableFilename)
{
    if (Itime::CreateLeapSecTable(leapSecTableFilename) == 0)
    {
        fprintf(stderr, "%s: error creating Leap Second Table %s\n",
                                   _programName, leapSecTableFilename);
        Log.VWriteMsg( "error creating Leap Second Table %s\n",
                                   leapSecTableFilename );
        Log.SetWriteAndExit( EALog::EA_FAILURE," -- Aborting --\n");
    }
    return;

} // ArgsPlus::LeapSecTableOrExit

void 
ArgsPlus::Usage() {
  Args::Usage();
  Log.SetAndWrite(EALog::EA_FAILURE,ArgsPlus::_UsageMsg);
}

