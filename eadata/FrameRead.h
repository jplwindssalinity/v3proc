//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.5   08 Sep 1998 16:24:40   sally
// added HK2 FSW subcoms
// 
//    Rev 1.4   20 Jul 1998 14:14:52   sally
// pass file descriptor to read function, instead of frame buffer
// 
//    Rev 1.3   01 May 1998 14:45:00   sally
// added HK2 file
// 
//    Rev 1.2   28 Apr 1998 15:57:18   sally
// added scatterometer housekeeping (1553) data for HK2
// 
//    Rev 1.1   20 Apr 1998 10:21:22   sally
// change for WindSwatch
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#ifndef FrameRead_H
#define FrameRead_H

static const char Hk2Read_h_id[] =
    "@(#) $Header$";

extern int FrameRead1Byte(int, char*);
extern int FrameRead2Bytes(int, char*);
extern int FrameRead3Bytes(int, char*);
extern int FrameRead4Bytes(int, char*);
extern int FrameRead8Bytes(int, char*);
extern int FrameReadFrameNo(int, char*);

#endif // FrameRead_H
