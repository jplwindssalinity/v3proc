//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   04 Feb 1998 14:14:42   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:51  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================
 
static const char rcs_id_Backup_C[] = "@(#) $Header$";

#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "Backup.h"

#ifdef __SCAT_GNUCPP__
#include <stdio.h>
#endif

int errno;

//--------
// Backup 
//--------
// maintains backups with the name filename.type.# where # is
// between 1 and count.
// returns 1 on success, 0 on failure

int
Backup(
    const char*     filename,
    const char*     type,
    const int       count)
{
    //--------------------------------------
    // determine the number of count digits 
    //--------------------------------------

    int digits = (int)log10((double)count) + 1;

    //--------------------------------
    // shift the files, if they exist 
    //--------------------------------

    char from_filename[1024];
    char to_filename[1024];
    int i;
    for (i = count; i > 1; i--)
    {
        if (sprintf(from_filename, "%s.%s.%*d", filename,
            type, digits, i-1) < 0)
        {
            return(0);
        }
        if (sprintf(to_filename, "%s.%s.%*d", filename,
            type, digits, i) < 0)
        {
            return(0);
        }
        if (rename(from_filename, to_filename) && errno != ENOENT)
            return(0);
    }
    if (count > 0)
    {
        sprintf(to_filename, "%s.%s.%*d", filename, type, digits, i);
        if (! Copy(filename, to_filename, 0644))
            return(0);
    }
    return(1);
}

//------
// Copy 
//------
// does a slow but sure copy from one file to another
// returns 1 on success, 0 on failure

int
Copy(
    const char*     from_file,
    const char*     to_file,
    const mode_t    mode)
{
    //----------------
    // open the files 
    //----------------

    int ifd = open(from_file, O_RDONLY);
    if (ifd == -1)
        return(0);
    int ofd = creat(to_file, mode);
    if (ofd == -1)
        return(0);

    //---------------
    // copy the file 
    //---------------

    char c;
    while (read(ifd, (void *)&c, (size_t) 1) == 1)
    {
        if (write(ofd, (void *)&c, (size_t) 1) != 1)
            return(0);
    }

    //-----------------
    // close the files 
    //-----------------

    close(ifd);
    close(ofd);

    return(1);
}
