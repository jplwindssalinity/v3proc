//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   26 Feb 1998 09:56:34   sally
// to pacify GNU compiler
// 
//    Rev 1.0   04 Feb 1998 14:15:30   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:29:05  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Headers.h"

static const char rcs_id_Headers_C[] =
    "@(#) $Header$";

//----------------------
// PrintNameTypeAndTime 
//----------------------
// print a name, telemetry type, start time, and end time in a nice format

void
PrintNameTypeAndTimes(
    FILE*           ofp,
    const char*     name,
    const SourceIdE tlm_type,
    const Itime     start_time,
    const Itime     end_time)
{
    fprintf(ofp, "%s\n", name);
    for (unsigned int i = 0; i < strlen(name); i++)
        fprintf(ofp, "-");
    fprintf(ofp, "\n");
    fprintf(ofp, "%s %s\n", TLM_TYPE_LABEL, source_id_map[tlm_type]);

    char start_time_string[CODEA_TIME_LEN];
    if (start_time == INVALID_TIME)
        fprintf(ofp, "%s %s\n", START_TIME_LABEL, INVALID_TIME_LABEL);
    else
    {
        start_time.ItimeToCodeA(start_time_string);
        fprintf(ofp, "%s %s\n", START_TIME_LABEL, start_time_string);
    }

    char end_time_string[CODEA_TIME_LEN];
    if (end_time == INVALID_TIME)
        fprintf(ofp, "%s %s\n", END_TIME_LABEL, INVALID_TIME_LABEL);
    else
    {
        end_time.ItimeToCodeA(end_time_string);
        fprintf(ofp, "%s %s\n", END_TIME_LABEL, end_time_string);
    }

    fflush(ofp);
}

//-------------------
// PrintNameAndTimes 
//-------------------
// print the name, start time, and end time in a nice format

void
PrintNameAndTimes(
    FILE*           ofp,
    const char*     name,
    const Itime     start_time,
    const Itime     end_time)
{
    fprintf(ofp, "%s\n", name);
    for (unsigned int i = 0; i < strlen(name); i++)
        fprintf(ofp, "-");
    fprintf(ofp, "\n");

    char start_time_string[CODEA_TIME_LEN];
    if (start_time == INVALID_TIME)
        fprintf(ofp, "%s %s\n", START_TIME_LABEL, INVALID_TIME_LABEL);
    else
    {
        start_time.ItimeToCodeA(start_time_string);
        fprintf(ofp, "%s %s\n", START_TIME_LABEL, start_time_string);
    }

    char end_time_string[CODEA_TIME_LEN];
    if (end_time == INVALID_TIME)
        fprintf(ofp, "%s %s\n", END_TIME_LABEL, INVALID_TIME_LABEL);
    else
    {
        end_time.ItimeToCodeA(end_time_string);
        fprintf(ofp, "%s %s\n", END_TIME_LABEL, end_time_string);
    }

    fflush(ofp);
}

//---------------
// LogAndExecute 
//---------------

void
LogAndExecute(
    const char*     command)
{
    char cmd_string[CMD_STRING_SIZE];
    char program_name[PROGRAM_STRING_SIZE];
    sscanf(command, "%s ", program_name);
    sprintf(cmd_string, "(%s; echo; echo \"Program Completed: %s\") &", command,
        program_name);
    printf("\n%s\n", command);
    (void) system(cmd_string);
    return;
}
