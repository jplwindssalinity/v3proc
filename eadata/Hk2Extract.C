//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.3   07 Jul 1999 13:23:34   sally
// add a few more extraction functions
// 
//    Rev 1.2   04 Nov 1998 10:09:50   sally
// add "packet sequence count" parameter
// 
//    Rev 1.1   09 Sep 1998 15:06:14   sally
// add minor_frame_count as the first SDS ID
// 
//    Rev 1.0   08 Sep 1998 16:25:52   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#include <assert.h>
#include <time.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>

#include "CommonDefs.h"
#include "L1AExtract.h"
#include "Parameter.h"
#include "PolyTable.h"
#include "Itime.h"

static const char rcs_id_HK2Extract_C[] = "@(#) $Header$";


int
Hk2ExtractFrameNo(
TlmHdfFile*     hk2File,
int32*          sdsIDs,
int32           start,
unsigned char&  frameNo)
{
    if (Extract16Bit0_3(hk2File, sdsIDs, start, 1, 1, &frameNo))
        return 1;
    else return(-1);

} // Hk2ExtractFrameNo

int
ExtractData1D_Odd(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_EVEN(frameNo)) return 0;

    return (hk2File->GetDatasetData1D(sdsIDs[1], start,
                  stride, length, buffer) == HDF_SUCCEED ?  1 : -1);

} // ExtractData1D_Odd

int
ExtractData1D_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    return (hk2File->GetDatasetData1D(sdsIDs[1], start,
                  stride, length, buffer) == HDF_SUCCEED ?  1 : -1);

} // ExtractData1D_Even

int
ExtractData1D_int2_float_Odd(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_EVEN(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractData1D_int2_float(hk2File, newSdsIDs, start,
                  stride, length, buffer, 0));

} // ExtractData1D_int2_float_Odd

int
ExtractData1D_int2_float_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractData1D_int2_float(hk2File, newSdsIDs, start,
                  stride, length, buffer, 0));

} // ExtractData1D_int2_float_Even

int
ExtractData1D_int1_float_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractData1D_int1_float(hk2File, newSdsIDs, start,
                  stride, length, buffer, 0));

} // ExtractData1D_int1_float_Even

int
ExtractData1D_uint1_float_Odd(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_EVEN(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractData1D_uint1_float(hk2File, newSdsIDs, start,
                  stride, length, buffer, 0));

} // ExtractData1D_uint1_float_Odd

int
ExtractData1D_uint1_float_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractData1D_uint1_float(hk2File, newSdsIDs, start,
                  stride, length, buffer, 0));

} // ExtractData1D_uint1_float_Even

int
ExtractData1D_uint2_float_Odd(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_EVEN(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractData1D_uint2_float(hk2File, newSdsIDs, start,
                  stride, length, buffer, 0));

} // ExtractData1D_uint2_float_Odd

int
ExtractData1D_uint2_float_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractData1D_uint2_float(hk2File, newSdsIDs, start,
                  stride, length, buffer, 0));

} // ExtractData1D_uint2_float_Even

int
Extract8Bit0_Odd(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_EVEN(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(Extract1of8Bits(hk2File,newSdsIDs,
                               start,stride,length, 0, buffer, 0));
 
}//Extract8Bit0_Odd

int
Extract8Bit0_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an even frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(Extract1of8Bits(hk2File,newSdsIDs,
                               start,stride,length, 0, buffer, 0));
 
}//Extract8Bit0_Even

int
Extract8Bit1_Odd(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_EVEN(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(Extract1of8Bits(hk2File,newSdsIDs,
                               start,stride,length, 1, buffer, 0));
 
}//Extract8Bit1_Odd

int
Extract8Bit1_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an even frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(Extract1of8Bits(hk2File,newSdsIDs,
                               start,stride,length, 1, buffer, 0));
 
}//Extract8Bit1_Even

int
Extract8Bit2_Odd(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_EVEN(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(Extract1of8Bits(hk2File,newSdsIDs,
                               start,stride,length, 2, buffer, 0));
 
}//Extract8Bit2_Odd

int
Extract8Bit2_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an even frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(Extract1of8Bits(hk2File,newSdsIDs,
                               start,stride,length, 2, buffer, 0));
 
}//Extract8Bit2_Even

int
Extract8Bit3_Odd(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_EVEN(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(Extract1of8Bits(hk2File,newSdsIDs,
                               start,stride,length, 3, buffer, 0));
 
}//Extract8Bit3_Odd

int
Extract8Bit3_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an even frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(Extract1of8Bits(hk2File,newSdsIDs,
                               start,stride,length, 3, buffer, 0));
 
}//Extract8Bit3_Even

int
Extract8Bit4_Odd(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_EVEN(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(Extract1of8Bits(hk2File,newSdsIDs,
                               start,stride,length, 4, buffer, 0));
 
}//Extract8Bit4_Odd

int
Extract8Bit4_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an even frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(Extract1of8Bits(hk2File,newSdsIDs,
                               start,stride,length, 4, buffer, 0));
 
}//Extract8Bit4_Even

int
Extract8Bit5_Odd(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_EVEN(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(Extract1of8Bits(hk2File,newSdsIDs,
                               start,stride,length, 5, buffer, 0));
 
}//Extract8Bit5_Odd

int
Extract8Bit5_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an even frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(Extract1of8Bits(hk2File,newSdsIDs,
                               start,stride,length, 5, buffer, 0));
 
}//Extract8Bit5_Even

int
Extract8Bit6_Odd(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_EVEN(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(Extract1of8Bits(hk2File,newSdsIDs,
                               start,stride,length, 6, buffer, 0));
 
}//Extract8Bit6_Odd

int
Extract8Bit6_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an even frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(Extract1of8Bits(hk2File,newSdsIDs,
                               start,stride,length, 6, buffer, 0));
 
}//Extract8Bit6_Even

int
Extract8Bit7_Odd(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_EVEN(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(Extract1of8Bits(hk2File,newSdsIDs,
                               start,stride,length, 7, buffer, 0));
 
}//Extract8Bit7_Odd

int
Extract8Bit7_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an even frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(Extract1of8Bits(hk2File,newSdsIDs,
                               start,stride,length, 7, buffer, 0));
 
}//Extract8Bit7_Even

int
Extract8Bit5_6_Odd(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_EVEN(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractSomeOf8Bits(hk2File,newSdsIDs,start,stride,length,
                                     6,2,buffer, 0));
 
}//Extract8Bit7_Odd

int
Extract8Bit0_1_Odd(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_EVEN(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractSomeOf8Bits(hk2File,newSdsIDs,start,stride,length,
                                     1,2,buffer, 0));
 
}//Extract8Bit0_1_Odd

int
Extract8Bit2_3_Odd(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_EVEN(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractSomeOf8Bits(hk2File,newSdsIDs,start,stride,length,
                                     3, 2, buffer, 0));
 
}//Extract8Bit2_3_Odd

int
Extract8Bit6_7_Odd(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_EVEN(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractSomeOf8Bits(hk2File,newSdsIDs,start,stride,length,
                                     7,2,buffer, 0));
 
}//Extract8Bit6_7_Odd

int
Extract8Bit4_5_Odd(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an odd frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_EVEN(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractSomeOf8Bits(hk2File,newSdsIDs,start,stride,length,
                                     5,2,buffer, 0));
 
}//Extract8Bit4_5_Odd

int
Extract8Bit6_7_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an even frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractSomeOf8Bits(hk2File,newSdsIDs,start,stride,length,
                                     7,2,buffer, 0));
 
}//Extract8Bit6_7_Even

int
Extract8Bit5_7_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an even frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractSomeOf8Bits(hk2File,newSdsIDs,start,stride,length,
                                     7,3,buffer, 0));
 
}//Extract8Bit5_7_Even

int
Extract8Bit4_5_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an even frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractSomeOf8Bits(hk2File,newSdsIDs,start,stride,length,
                                     5,2,buffer, 0));
 
}//Extract8Bit4_5_Even

int
Extract8Bit3_7_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an even frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractSomeOf8Bits(hk2File,newSdsIDs,start,stride,length,
                                     7,5,buffer, 0));
 
}//Extract8Bit3_7_Even

int
Extract8Bit0_3_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an even frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractSomeOf8Bits(hk2File,newSdsIDs,start,stride,length,
                                     3,4,buffer, 0));
 
}//Extract8Bit0_3_Even

int
Extract8Bit1_3_Even(
TlmHdfFile* hk2File,
int32*      sdsIDs,
int32       start,
int32       stride,
int32       length,
VOIDP       buffer,
PolynomialTable*)     // unused
{
    // make sure this is an even frame
    unsigned char frameNo=0;
    if (Hk2ExtractFrameNo(hk2File, sdsIDs, start, frameNo) < 0)
        return -1;
    if (EA_IS_ODD(frameNo)) return 0;

    int32 newSdsIDs[1];
    newSdsIDs[0] = sdsIDs[1];
    return(ExtractSomeOf8Bits(hk2File,newSdsIDs,start,stride,length,
                                     3, 3, buffer, 0));
 
}//Extract8Bit1_3_Even
