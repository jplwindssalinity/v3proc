//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   10 Apr 1998 14:04:18   daffer
//     Changed ConfigList to EAConfigList throughout
// 
//    Rev 1.0   04 Feb 1998 14:16:26   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:31  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

// This file has functions for sending mail

#ifndef MAIL_H
#define MAIL_H

#include "EAConfigList.h"

static const char rcs_id_mail_h[] =
    "@(#) $Header$";

#define SYSTEM_COMMAND_LEN  1024

int SendFile(const char *address, const char* subject, const char* filename);
int SendMsg(const char *address, const char* subject, const char* filename);

#endif
