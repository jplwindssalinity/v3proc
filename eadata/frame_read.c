/*=========================================================
** Copyright  (C)1996, California Institute of Technology. 
** U.S. Government sponsorship under 
** NASA Contract NAS7-1260 is acknowledged
**
**
** CM Log
** $Log$
// 
//    Rev 1.0   18 Aug 1998 11:04:06   sally
// Initial revision.
** 
**    Rev 1.5   20 Jul 1998 14:13:56   sally
** pass file descriptor to read function, instead of frame buffer
** 
**    Rev 1.4   21 May 1998 10:12:18   sally
** fixed comments
** 
**    Rev 1.3   01 May 1998 14:44:54   sally
** added HK2 file
** 
**    Rev 1.2   28 Apr 1998 15:56:42   sally
** added scatterometer housekeeping (1553) data for HK2
** 
**    Rev 1.1   20 Apr 1998 10:21:20   sally
** change for WindSwatch
** 
** $Date$
** $Revision$
** $Author$
**
=========================================================*/

static const char Hk2Read_C_id[] =
    "@(#) $Header$";

#include <stdio.h>
#include <unistd.h>
#include <string.h>

int
FrameRead1Byte(
int     infd,
char*   data)
{
    return(read(infd, data, 1) == 1 ? 1 : 0);

} /* FrameRead1Byte */

int
FrameRead2Bytes(
int     infd,
char*   data)
{
    return(read(infd, data, 2) == 2 ? 1 : 0);

} /* FrameRead2Bytes */

int
FrameRead3Bytes(
int     infd,
char*   data)          /* a 4-byte data */
{
    char buf[3];
    char* ptr=0;
    if (read(infd, buf, 3) != 3)
        return 0;
    ptr = (char*)data;
    ptr++;
    (void)memcpy(ptr, buf, 3);
    return 1;

} /* FrameRead3Bytes */

int
FrameRead4Bytes(
int     infd,
char*   data)
{
    return(read(infd, data, 4) == 4 ? 1 : 0);

} /* FrameRead4Bytes */

int
FrameRead8Bytes(
int     infd,
char*   data)
{
    return(read(infd, data, 8) == 8 ? 1 : 0);

} /* FrameRead8Bytes */
