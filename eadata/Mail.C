//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   04 Feb 1998 14:16:24   daffer
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

static const char rcs_id_Mail_C[] =
    "@(#) $Header$";

#include <stdlib.h>
#include "Mail.h"

//---------
// SendMsg 
//---------
// sends the specified message to the specified address with the
// specified subject line.  if subject is 0, the subject field is
// left blank.  returns 1 on success, 0 on error
 
int
SendMsg(
    const char*     address,
    const char*     subject,
    const char*     message)
{
    char system_command[SYSTEM_COMMAND_LEN];
 
    if (! address || ! message)
        return(0);
 
    if (subject)
        sprintf(system_command, "echo \"%s\" | mailx -s \"%s\" %s",
            message, subject, address);
    else
        sprintf(system_command, "echo \"%s\" | mailx %s",
            message, address);
 
    if (system(system_command) == -1)
        return (0);
    else
        return (1);
}

//----------
// SendFile 
//----------
// sends the contents of a file to the specified address with the
// specified subject line.  if subject is 0, the subject field is
// left blank.  returns 1 on success, 0 on error
 
int
SendFile(
    const char*     address,
    const char*     subject,
    const char*     filename)
{
    char system_command[SYSTEM_COMMAND_LEN];
 
    if (! address)
        return(0);
 
    if (! filename)
        return(0);
 
    if (subject)
        sprintf(system_command, "mailx -s \"%s\" %s < %s",
            subject, address, filename);
    else
        sprintf(system_command, "mailx %s < %s", address, filename);
 
    if (system(system_command) == -1)
        return (0);
    else
        return (1);
}
