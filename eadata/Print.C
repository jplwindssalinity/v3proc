//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.4   20 Apr 1998 10:22:52   sally
// change for WindSwatch
// 
//    Rev 1.3   17 Apr 1998 16:51:16   sally
// add L2A and L2B file formats
// 
//    Rev 1.2   30 Mar 1998 15:14:08   sally
// added L2A parameter table
// 
//    Rev 1.1   23 Mar 1998 15:40:02   sally
// adapt to derived science data
// 
//    Rev 1.0   04 Feb 1998 14:16:54   daffer
// Initial checking
// Revision 1.7  1998/02/02 17:40:46  sally
// add a new print
//
// Revision 1.6  1998/02/01 20:43:49  sally
// took out printf()
//
// Revision 1.5  1998/01/31 00:36:39  sally
// add "0x" in front of hex printing
//
// Revision 1.4  1998/01/30 22:29:22  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#include <stdio.h>
#include <string.h>
#include "Print.h"

static const char rcs_id_Print_C[] = "@(#) $Header$";

//-*******************************************************************
//      All of the printing functions require a file pointer to an
//      open file and a char pointer to the data to be printed.
//      All of the printing functions assume proper alignment.
//-*******************************************************************

void pr_char2x(FILE *ofp, char *dataP)
{
    unsigned short *ptr = (unsigned short *)dataP;
    fprintf(ofp, "0x%04x ", *(ptr));
    return;
}

void pr_char3x(FILE *ofp, char *dataP)
{
    fprintf(ofp, "0x");
    for (int i = 0; i < 3; i++) {
        fprintf(ofp, "%02x", *(unsigned char *)(dataP + i));
    }
    return;
}

void pr_char4x(FILE *ofp, char *dataP)
{
    unsigned int *ptr = (unsigned int *)dataP;
    fprintf(ofp, "0x%08x ", *(ptr));
    return;
}

void pr_char13x(FILE *ofp, char *dataP)
{
    fprintf(ofp, "0x");
    for (int i = 0; i < 13; i++) {
        fprintf(ofp, "%02x", *(unsigned char *)(dataP + i));
    }
    return;
}

void pr_char2x_4(FILE *ofp, char *dataP)
{
    unsigned short *ptr = (unsigned short *)dataP;
    for (int i = 0; i < 4; i++)
        fprintf(ofp, "0x%04x ", *(ptr + i));
    return;
}

void pr_char2x_5(FILE *ofp, char *dataP)
{
    unsigned short *ptr = (unsigned short *)dataP;
    for (int i = 0; i < 5; i++)
        fprintf(ofp, "0x%04x ", *(ptr + i));
    return;
}

void pr_char2x_2_8(FILE *ofp, char *dataP)
{
    unsigned short *ptr = (unsigned short *)dataP;
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 8; j++, ptr++)
            fprintf(ofp, "0x%04x ", *ptr);
    return;
}

// line separated
void pr_char2x_2_8_linesep(FILE *ofp, char *dataP)
{
    unsigned short *ptr = (unsigned short *)dataP;
    for (int i = 0; i < 2; i++)
    {
        fprintf(ofp, "          ");
        for (int j = 0; j < 8; j++, ptr++)
            fprintf(ofp, "0x%04x ", *ptr);
        fprintf(ofp, "\n");
    }
    return;
}

void pr_char4x_4(FILE *ofp, char *dataP)
{
    unsigned int *ptr = (unsigned int *)dataP;
    for (int i = 0; i < 4; i++)
        fprintf(ofp, "0x%08x ", *(ptr + i));
    return;
}

void pr_char28x(FILE *ofp, char *dataP)
{
    int i;
    for (i = 0; i < 28; i++) {
        fprintf(ofp, "%02x", *(unsigned char *)(dataP + i));
    }
    return;
}

void pr_uint1_76(FILE *ofp, char *dataP)
{
    unsigned char *ptr = (unsigned char *)dataP;
    for (int j = 0; j < 76; j++, ptr++)
        fprintf(ofp, "%6u ", *ptr);
    return;
}

void pr_uint1_49(FILE *ofp, char *dataP)
{
    unsigned char *ptr = (unsigned char *)dataP;
    for (int j = 0; j < 49; j++, ptr++)
        fprintf(ofp, "%4u ", *ptr);
    return;
}

void pr_uint1_49_linesep(FILE *ofp, char *dataP)
{
    unsigned char *ptr = (unsigned char *)dataP;
    for (int i = 0; i < 7; i++)
    {
        fprintf(ofp, "          ");
        for (int j = 0; j < 7; j++, ptr++)
            fprintf(ofp, "%4u ", *ptr);
        fprintf(ofp, "\n");
    }
    return;
}

void pr_int1_76(FILE *ofp, char *dataP)
{
    char *ptr = (char *)dataP;
    for (int j = 0; j < 76; j++, ptr++)
        fprintf(ofp, "%6d ", *ptr);
    return;
}

void pr_int1_810(FILE *ofp, char *dataP)
{
    char *ptr = (char *)dataP;
    for (int j = 0; j < 810; j++, ptr++)
        fprintf(ofp, "%6d ", *ptr);
    return;
}

void pr_uint2_4(FILE *ofp, char *dataP)
{
    unsigned short *ptr = (unsigned short *)dataP;
    for (int i = 0; i < 4; i++)
        fprintf(ofp, "%6u ", *(ptr + i));
    return;
}

void pr_uint2_5(FILE *ofp, char *dataP)
{
    unsigned short *ptr = (unsigned short *)dataP;
    for (int i = 0; i < 5; i++)
        fprintf(ofp, "%6u ", *(ptr + i));
    return;
}

void pr_uint2_25(FILE *ofp, char *dataP)
{
    unsigned short *ptr = (unsigned short *)dataP;
    for (int i = 0; i < 25; i++)
        fprintf(ofp, "%6u ", *(ptr + i));
    return;
}

void pr_uint2_76(FILE *ofp, char *dataP)
{
    unsigned short *ptr = (unsigned short *)dataP;
    for (int i = 0; i < 76; i++)
        fprintf(ofp, "%6u ", *(ptr + i));
    return;
}

void pr_uint2_810(FILE *ofp, char *dataP)
{
    unsigned short *ptr = (unsigned short *)dataP;
    for (int i = 0; i < 810; i++)
        fprintf(ofp, "%6u ", *(ptr + i));
    return;
}

void pr_uint2_100_linesep(FILE *ofp, char *dataP)
{
    unsigned short *ptr = (unsigned short *)dataP;
    for (int i = 0; i < 10; i++)
    {
        fprintf(ofp, "          ");
        for (int j = 0; j < 10; j++, ptr++)
            fprintf(ofp, "%6u ", *ptr);
        fprintf(ofp, "\n");
    }
    return;
}

void pr_uint2_100_12(FILE *ofp, char *dataP)
{
    unsigned short *ptr = (unsigned short *)dataP;
    for (int i = 0; i < 100; i++, ptr += 12)
    {
        unsigned short * secondPtr = ptr;
        for (int j = 0; j < 12; j++)
            fprintf(ofp, "%6u ", secondPtr[j]);
    }
    return;
}

void pr_uint4_4(FILE *ofp, char *dataP)
{
    unsigned int *ptr = (unsigned int *)dataP;
    for (int i = 0; i < 4; i++)
        fprintf(ofp, "%11u ", *(ptr + i));
    return;
}

void pr_uint4_25(FILE *ofp, char *dataP)
{
    unsigned int *ptr = (unsigned int *)dataP;
    for (int i = 0; i < 25; i++)
        fprintf(ofp, "%11u ", *(ptr + i));
    return;
}

void pr_uint4_100(FILE *ofp, char *dataP)
{
    unsigned int *ptr = (unsigned int *)dataP;
    for (int i = 0; i < 100; i++)
        fprintf(ofp, "%11u ", *(ptr + i));
    return;
}

void pr_float4_6_25(FILE *ofp, char *dataP)
{
    float *ptr = (float *)dataP;
    for (int i = 0; i < 25; i++, ptr++)
    {
        fprintf(ofp, "%.6g ", *ptr);
    }
    return;
}

void pr_float4_6_100(FILE *ofp, char *dataP)
{
    float *ptr = (float *)dataP;
    for (int i = 0; i < 100; i++, ptr++)
    {
        fprintf(ofp, "%.6g ", *ptr);
    }
    return;
}

void pr_float4_6_100_linesep(FILE *ofp, char *dataP)
{
    float *ptr = (float *)dataP;
    for (int i = 0; i < 10; i++)
    {
        fprintf(ofp, "          ");
        for (int j = 0; j < 10; j++, ptr++)
            fprintf(ofp, "%.6g ", *ptr);
        fprintf(ofp, "\n");
    }
    return;
}

void pr_76float4_6(FILE *ofp, char *dataP)
{
    float *ptr = (float *)dataP;
    for (int i = 0; i < 76; i++, ptr++)
    {
        fprintf(ofp, "%.6g ", *ptr);
    }
    return;
}

void pr_810float4_6(FILE *ofp, char *dataP)
{
    float *ptr = (float *)dataP;
    for (int i = 0; i < 810; i++, ptr++)
    {
        fprintf(ofp, "%.6g ", *ptr);
    }
    return;
}

void pr_76_4_float4_6(FILE *ofp, char *dataP)
{
    float* ptr = (float *)dataP;
    for (int i = 0; i < 76; i++)
        for (int j = 0; j < 4; j++, ptr++)
            fprintf(ofp, "%.6g ", *ptr);
    return;
}
void pr_bit(FILE *ofp, char *dataP)
{
    fprintf(ofp, "%u", *((unsigned char *)dataP));
    return;
}

void pr_binchar(FILE *ofp, char *dataP)
{
    char charValue = *dataP;
    for (int i = 0; i < 8; i++)
    {
        int shift = 7-i;
        char bitchar = '0' + ((charValue >> shift) & 0x01);
        fprintf(ofp, "%c", bitchar);
    }

    return;
}

void pr_binchar2(FILE *ofp, char *dataP)
{
        pr_binchar(ofp, dataP);
        pr_binchar(ofp, dataP + 1);
} //pr_binchar2

void pr_binchar4(FILE *ofp, char *dataP)
{
        pr_binchar(ofp, dataP);
        pr_binchar(ofp, dataP + 1);
        pr_binchar(ofp, dataP + 2);
        pr_binchar(ofp, dataP + 3);
} //pr_binchar2

#define PER_LINE 4  // 8-bit maps per line
void pr_extract_map(FILE *ofp, char *dataP)
{
    for (int i = 0; i < 32; i++) {
        int modval = i % PER_LINE;
        if (modval == 0) {
            fprintf(ofp, "  Words %3d - %3d:  ", i*8, (i+PER_LINE)*8 - 1);
        }
        pr_binchar(ofp, dataP + i);
        if (i%4 == 3) {
            fprintf(ofp, "\n");
        }
    }
    return;
}
