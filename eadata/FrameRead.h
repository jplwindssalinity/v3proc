//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   20 Apr 1998 10:21:22   sally
// change for WindSwatch
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#ifndef Hk2Read_H
#define Hk2Read_H

static const char Hk2Read_h_id[] =
    "@(#) $Header$";

extern int FrameReadBits0_1(char*, int, char*);
extern int FrameReadBits0_3(char*, int, char*);
extern int FrameReadBits4_6(char*, int, char*);
extern int FrameReadBits1_7(char*, int, char*);
extern int FrameReadBits6_7(char*, int, char*);
extern int FrameReadBits4_5(char*, int, char*);
extern int FrameReadBits2_3(char*, int, char*);
extern int FrameReadBits1_2(char*, int, char*);
extern int FrameReadBit7(char*, int, char*);
extern int FrameReadBit6(char*, int, char*);
extern int FrameReadBit5(char*, int, char*);
extern int FrameReadBit4(char*, int, char*);
extern int FrameReadBit3(char*, int, char*);
extern int FrameReadBit2(char*, int, char*);
extern int FrameReadBit1(char*, int, char*);
extern int FrameReadBit0(char*, int, char*);

extern int FrameRead1Byte(char*, int, char*);
extern int FrameRead2Bytes(char*, int, char*);
extern int FrameRead3Bytes(char*, int, char*);
extern int FrameRead4Bytes(char*, int, char*);

#endif // Hk2Read_H
