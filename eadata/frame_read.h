/*=========================================================
** Copyright  (C)1996, California Institute of Technology. 
** U.S. Government sponsorship under 
** NASA Contract NAS7-1260 is acknowledged
**
**
** CM Log
** $Log$
// 
//    Rev 1.0   18 Aug 1998 11:04:08   sally
// Initial revision.
** 
** $Date$
** $Revision$
** $Author$
**
=========================================================*/

#ifndef FrameRead_H
#define FrameRead_H

static const char Hk2Read_h_id[] =
    "@(#) $Header$";

extern int FrameRead1Byte(int, char*);
extern int FrameRead2Bytes(int, char*);
extern int FrameRead3Bytes(int, char*);
extern int FrameRead4Bytes(int, char*);
extern int FrameRead8Bytes(int, char*);

#endif /* FrameRead_H */
