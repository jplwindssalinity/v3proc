//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   04 Feb 1998 14:15:32   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:20  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef HEADERS_H
#define HEADERS_H

static const char rcs_id_headers_h[] =
    "@(#) $Header$";

#include <stdio.h>
#include "Parameter.h"
#include "Itime.h"

#define TLM_TYPE_LABEL      "Telemetry Type:"
#define START_TIME_LABEL    "    Start Time:"
#define END_TIME_LABEL      "      End Time:"
#define INVALID_TIME_LABEL  "*"

#define CMD_STRING_SIZE     1024
#define PROGRAM_STRING_SIZE 64

void        PrintNameTypeAndTimes(FILE* ofp, const char* name,
                const SourceIdE tlm_type, const Itime start_time,
                const Itime end_time);
void        PrintNameAndTimes(FILE* ofp, const char* name,
                const Itime start_time, const Itime end_time);
void        LogAndExecute(const char* command);

#endif //HEADERS_H
