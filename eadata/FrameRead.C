//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   20 Apr 1998 10:21:20   sally
// change for WindSwatch
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

static const char Hk2Read_C_id[] =
    "@(#) $Header$";

#include <stdio.h>
#include <string.h>

#include "CommonDefs.h"


//---------------------------------------------------------
// bit order: from left to right, i.e. 0 1 2 3 4 5 6 7
//---------------------------------------------------------

int
FrameReadBits0_1(
char*   frame,
int     byteOffset,
char*   data)
{
    *data = GetBits(*(frame + byteOffset), 1, 2);
    return 1;
} // FrameReadBits0_1

int
FrameReadBits0_3(
char*   frame,
int     byteOffset,
char*   data)
{
    *data = GetBits(*(frame + byteOffset), 3, 4);
    return 1;
} // FrameReadBits0_3

int
FrameReadBits4_6(
char*   frame,
int     byteOffset,
char*   data)
{
    *data = GetBits(*(frame + byteOffset), 6, 3);
    return 1;
} // FrameReadBits4_6

int
FrameReadBits1_7(
char*   frame,
int     byteOffset,
char*   data)
{
    *data = GetBits(*(frame + byteOffset), 7, 7);
    return 1;
} // FrameReadBits1_7

int
FrameReadBits6_7(
char*   frame,
int     byteOffset,
char*   data)
{
    *data = GetBits(*(frame + byteOffset), 7, 2);
    return 1;
} // FrameReadBits6_7

int
FrameReadBits4_5(
char*   frame,
int     byteOffset,
char*   data)
{
    *data = GetBits(*(frame + byteOffset), 5, 2);
    return 1;
} // FrameReadBits4_5

int
FrameReadBits2_3(
char*   frame,
int     byteOffset,
char*   data)
{
    *data = GetBits(*(frame + byteOffset), 3, 2);
    return 1;
} // FrameReadBits2_3

int
FrameReadBits1_2(
char*   frame,
int     byteOffset,
char*   data)
{
    *data = GetBits(*(frame + byteOffset), 2, 2);
    return 1;
} // FrameReadBits1_2

int
FrameReadBit7(
char*   frame,
int     byteOffset,
char*   data)
{
    *data = GetBit(*(frame + byteOffset), 7);
    return 1;
} // FrameReadBit7

int
FrameReadBit6(
char*   frame,
int     byteOffset,
char*   data)
{
    *data = GetBit(*(frame + byteOffset), 6);
    return 1;
} // FrameReadBit6

int
FrameReadBit5(
char*   frame,
int     byteOffset,
char*   data)
{
    *data = GetBit(*(frame + byteOffset), 5);
    return 1;
} // FrameReadBit5

int
FrameReadBit4(
char*   frame,
int     byteOffset,
char*   data)
{
    *data = GetBit(*(frame + byteOffset), 4);
    return 1;
} // FrameReadBit4

int
FrameReadBit3(
char*   frame,
int     byteOffset,
char*   data)
{
    *data = GetBit(*(frame + byteOffset), 3);
    return 1;
} // FrameReadBit3

int
FrameReadBit2(
char*   frame,
int     byteOffset,
char*   data)
{
    *data = GetBit(*(frame + byteOffset), 2);
    return 1;
} // FrameReadBit2

int
FrameReadBit1(
char*   frame,
int     byteOffset,
char*   data)
{
    *data = GetBit(*(frame + byteOffset), 1);
    return 1;
} // FrameReadBit1

int
FrameReadBit0(
char*   frame,
int     byteOffset,
char*   data)
{
    *data = GetBit(*(frame + byteOffset), 0);
    return 1;
} // FrameReadBit0

int
FrameRead1Byte(
char*   frame,
int     byteOffset,
char*   data)
{
    (void)memcpy(data, frame + byteOffset, 1);
    return(data != 0 ? 1 : 0);

} // FrameRead1Byte

int
FrameRead2Bytes(
char*   frame,
int     byteOffset,
char*   data)
{
    (void)memcpy(data, frame + byteOffset, 2);
    return(data != 0 ? 1 : 0);

} // FrameRead2Bytes

int
FrameRead3Bytes(
char*   frame,
int     byteOffset,
char*   data)          // a 4-byte data
{
    char* ptr = data;
    ptr++;   // go to the 2nd byte
    (void)memcpy(ptr, frame + byteOffset, 3);
    return(data != 0 ? 1 : 0);

} // FrameRead3Bytes

int
FrameRead4Bytes(
char*   frame,
int     byteOffset,
char*   data)
{
    (void)memcpy(data, frame + byteOffset, 4);
    return(data != 0 ? 1 : 0);

} // FrameRead4Bytes
