//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.11   23 Feb 1999 11:13:30   sally
// L2A array size chaned from 810 to 3240
// 
//    Rev 1.10   07 Dec 1998 15:45:16   sally
// add pr_12float4_6()
// 
//    Rev 1.9   20 Nov 1998 16:03:42   sally
// change some data types and limit check arrays
// 
//    Rev 1.8   13 Oct 1998 15:34:30   sally
// added L1B file
// 
//    Rev 1.7   03 Jun 1998 10:10:44   sally
// change parameter names and types due to LP's changes
// 
//    Rev 1.6   20 Apr 1998 10:22:54   sally
// change for WindSwatch
// 
//    Rev 1.5   17 Apr 1998 16:51:18   sally
// add L2A and L2B file formats
// 
//    Rev 1.4   30 Mar 1998 15:14:10   sally
// added L2A parameter table
// 
//    Rev 1.3   23 Mar 1998 15:40:06   sally
// adapt to derived science data
// 
//    Rev 1.2   26 Feb 1998 10:00:18   sally
// to pacify GNU compiler
// 
//    Rev 1.1   12 Feb 1998 16:47:46   sally
// add start and end time
// Revision 1.6  1998/02/02 17:40:46  sally
// add a new print
//
// Revision 1.5  1998/01/31 00:36:39  sally
// add "0x" in front of hex printing
//
// Revision 1.4  1998/01/30 22:28:41  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef PRINT_H
#define PRINT_H

static const char rcs_id_print_h[] = "@(#) $Header$";

#include <stdio.h>

#define TLM_TYPE_LABEL      "Telemetry Type:"
#define START_TIME_LABEL    "    Start Time:"
#define END_TIME_LABEL      "      End Time:"
#define INVALID_TIME_LABEL  "*"

// see below for pr_char1x
extern void pr_char2x(FILE* ofp, char* dataP);
extern void pr_char2x_4(FILE* ofp, char* dataP);
extern void pr_char2x_5(FILE* ofp, char* dataP);
extern void pr_char2x_2_8(FILE* ofp, char* dataP);
extern void pr_char2x_2_8_linesep(FILE* ofp, char* dataP);
extern void pr_char3x(FILE* ofp, char* dataP);
extern void pr_char4x(FILE* ofp, char* dataP);
extern void pr_char4x_4(FILE* ofp, char* dataP);
extern void pr_char13x(FILE* ofp, char* dataP);
extern void pr_char28x(FILE* ofp, char* dataP);
extern void pr_char32x(FILE* ofp, char* dataP);

extern void pr_uint1_76(FILE* ofp, char* dataP);
extern void pr_uint1_49(FILE* ofp, char* dataP);
extern void pr_uint1_49_linesep(FILE* ofp, char* dataP);
extern void pr_int1_76(FILE* ofp, char* dataP);
extern void pr_int1_3240(FILE* ofp, char* dataP);

extern void pr_uint2_4(FILE* ofp, char* dataP);
extern void pr_uint2_5(FILE* ofp, char* dataP);
extern void pr_uint2_12(FILE* ofp, char* dataP);
extern void pr_uint2_25(FILE* ofp, char* dataP);
extern void pr_uint2_76(FILE* ofp, char* dataP);
extern void pr_uint2_100(FILE* ofp, char* dataP);
extern void pr_uint2_3240(FILE* ofp, char* dataP);
extern void pr_uint2_100_linesep(FILE* ofp, char* dataP);
extern void pr_uint2_100_12(FILE* ofp, char* dataP);
extern void pr_int2_100(FILE* ofp, char* dataP);

extern void pr_uint3(FILE* ofp, char* dataP);
extern void pr_uint4_4(FILE* ofp, char* dataP);
extern void pr_uint4_12(FILE* ofp, char* dataP);
extern void pr_uint4_25(FILE* ofp, char* dataP);

extern void pr_uint3(FILE* ofp, char* dataP);
extern void pr_uint4_4(FILE* ofp, char* dataP);
extern void pr_uint4_25(FILE* ofp, char* dataP);
extern void pr_uint4_100(FILE* ofp, char* dataP);
extern void pr_uint4_100_12(FILE* ofp, char* dataP);

extern void pr_float4_6_25(FILE* ofp, char* dataP);
extern void pr_float4_6_100(FILE* ofp, char* dataP);
extern void pr_float4_6_100_linesep(FILE* ofp, char* dataP);

extern void pr_12float4_6(FILE* ofp, char* dataP);
extern void pr_76float4_6(FILE* ofp, char* dataP);
extern void pr_3240float4_6(FILE* ofp, char* dataP);
extern void pr_76_4_float4_6(FILE* ofp, char* dataP);
extern void pr_100_8_float4_6(FILE* ofp, char* dataP);

extern void pr_bit(FILE* ofp, char* dataP);

extern void pr_binchar(FILE* ofp, char* dataP);
extern void pr_binchar2(FILE* ofp, char* dataP);
extern void pr_binchar4(FILE* ofp, char* dataP);
extern void pr_extract_map(FILE* ofp, char* dataP);

//-*******************************************************************
//      All of the printing functions require a file pointer to an
//      open file and a char pointer to the data to be printed.
//      All of the printing functions assume proper alignment.
//-*******************************************************************

inline void pr_char1x( FILE *ofp, char *dataP)
{
    fprintf(ofp, "0x%02x", *(unsigned char *)(dataP));
    return;
}

inline void pr_int1(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%4d", *(dataP));
    return;
}

// left justified
inline void pr_int1_lj(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%d", *(dataP));
    return;
}

inline void pr_uint1(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%4u", *((unsigned char *)dataP));
    return;
}

// left justified
inline void pr_uint1_lj(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%u", *((unsigned char *)dataP));
    return;
}

inline void pr_int2(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%6d", *((short *)dataP));
    return;
}

// left justified
inline void pr_int2_lj(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%d", *((short *)dataP));
    return;
}

inline void pr_uint2(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%6u", *((unsigned short *)dataP));
    return;
}

// left justified
inline void pr_uint2_lj(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%u", *((unsigned short *)dataP));
    return;
}

inline void pr_int4(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%11ld", *((long *)dataP));
    return;
}

// left justified
inline void pr_int4_lj(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%ld", *((long *)dataP));
    return;
}

inline void pr_uint4(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%11ld", *((unsigned long *)dataP));
    return;
}

// left justified
inline void pr_uint4_lj(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%ld", *((unsigned long *)dataP));
    return;
}

inline void pr_uint4_11(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%11ld", *((unsigned long *)dataP));
}

inline void pr_float4_8(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%.8g", *((float *)dataP));
    return;
}

inline void pr_float4_6(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%.6g", *((float *)dataP));
    return;
}

inline void pr_float4_4(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%.4g", *((float *)dataP));
    return;
}

inline void pr_float4_3(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%.3g", *((float *)dataP));
    return;
}

inline void pr_float8_10(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%.10g", *((double *)dataP));
    return;
}

inline void pr_string(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%s", dataP);
    return;
}

#endif //PRINT_H
