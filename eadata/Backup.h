//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   04 Feb 1998 14:14:44   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:07  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef BACKUP_H
#define BACKUP_H

#include <sys/types.h>

static const char rcs_backup_h[]="@(#) $Header$";

int Backup(const char* filename, const char* type, const int count);
int Copy(const char* from_file, const char* to_file, const mode_t mode);

#endif
