//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.7   20 Oct 1998 10:52:42   sally
// add static QPF commands (table macro commands)
// 
//    Rev 1.6   08 Sep 1998 16:24:24   sally
// added HK2 FSW subcoms
// 
//    Rev 1.5   22 May 1998 16:21:48   sally
// added cds error message tables
// 
//    Rev 1.4   19 May 1998 15:48:44   sally
// rename error message map
// 
//    Rev 1.3   04 May 1998 17:20:18   sally
// added setup HK2 Limits
// 
//    Rev 1.2   01 Apr 1998 13:35:50   sally
// for L1A Derived table
// 
//    Rev 1.1   27 Mar 1998 09:58:36   sally
// added L1A Derived data
// 
//    Rev 1.0   04 Feb 1998 14:15:04   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:13  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef COMMONDEFS_H
#define COMMONDEFS_H

static const char rcs_id_common_defs_h[] = "@(#) $Header$";

/* Do not change the values of TRUE and FALSE */
#ifndef TRUE
#define TRUE 1
#endif 

#ifndef FALSE
#define FALSE 0
#endif 

#ifndef STRING_LEN
#define STRING_LEN  80
#endif

#ifndef BIG_SIZE
#define BIG_SIZE            1024
#endif

#ifndef BIG_4K_SIZE
#define BIG_4K_SIZE         4096
#endif

#ifndef SHORT_STRING_LEN
#define SHORT_STRING_LEN    40
#endif

#ifndef MAX_FILENAME_LEN
#define MAX_FILENAME_LEN    1024
#endif

#define PAGE_BREAK_CHAR     ''
#define RING_BELL           system("echo  ")
#define YELLOW_BACKGROUND   system("xsetroot -solid yellow")
#define RED_BACKGROUND      system("xsetroot -solid red")

#define ElementNumber(arr) ((unsigned int) (sizeof(arr) / sizeof(arr[0])))

#define EA_IS_ODD(x)  (x % 2 == 0 ? 0 : 1)
#define EA_IS_EVEN(x) (x % 2 == 0 ? 1 : 0)

#ifndef MAX_OF_TWO
#define MAX_OF_TWO(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN_OF_TWO
#define MIN_OF_TWO(x,y) ((x) < (y) ? (x) : (y))
#endif

#ifndef SAVE_COPY
#define SAVE_COPY(to,from,len) \
                { (void)strncpy(to,from,len-1); to[len-1]='\0'; }
#endif

//---------------------------
// Level 0 Quality Flag Maps 
//---------------------------
 
extern const char *crc_correct_map[];
extern const char *sc_time_err_map[];
extern const char *att_correct_map[];

//---------------------------
// Level 1 Quality Flag Maps 
//---------------------------

extern const char *miss_ephem_map[];
extern const char *pred_ephem_used_map[];
extern const char *analog_cur_map[];

//------------------------
// Instrument Status Maps 
//------------------------

extern const char *twta_map[];
extern const char *twt_map[];
extern const char *dss_map[];
extern const char *twta_trip_override_map[];
extern const char *cmf_map[];
extern const char *mode_map[];
extern const char *ext_mode_map[];
extern const char *unk_mode_map[];
extern const char *hvps_shut_en_map[];
extern const char *twt_mon_en_map[];
extern const char *bc_dump_map[];
extern const char *cur_beam_map[];
extern const char *rx_pro_map[];

//------------------------
// Error Engineering Maps 
//------------------------

extern const char *type1_error_msg_map[];
extern const int Type1ErrMsgMapSize;

extern const char *type2_error_msg_map[];
extern const int Type2ErrMsgMapSize;

extern const char *cds_fsw_crit_var_obj_id[];
extern const int CdsFswCritVarObjIdSize;

extern const char *cds_fsw_obj_id[];
extern const int CdsFswObjIdSize;

extern const char *lack_start_reqs_map[];
extern const char *err_queue_full_map[];
extern const char *bin_param_err_map[];
extern const char *def_bin_const_map[];
extern const char *twta_trip_map[];
extern const char *lock_map[];
extern const char *hvps_backup_off_map[];

//-------------------------
// Antenna Deployment Maps 
//-------------------------

extern const char *ant_deploy_map[]; 
extern const char *relay_map[];
extern const char *wts_1_map[];
extern const char *wts_2_map[];

//--------------------
// Relay derived maps 
//--------------------

extern const char *heater_map[];

//-----------------------------
// Hk2 Data Block Header Maps 
//-----------------------------

extern const char *data_type_map[];
extern const char *dwell_mode_map[];

//------------
// Relay Maps 
//------------

extern const unsigned char k1_k2_k3_map[2][2][2];
extern const unsigned char k4_k5_k6_map[2][2][2];
extern const unsigned char k7_k8_map[2][2];
extern const unsigned char k9_k10_map[2][2];
extern const unsigned char k11_k12_map[2][2];
extern const unsigned char k13_k14_map[2][2];

extern const unsigned char wts1_wts2_map[2][2];

//==================================================================

typedef char            IotBoolean;
typedef unsigned char   UChar;
typedef char            BYTE6[6];

// gets bit pos from byte
inline
char
GetBit(
char    byte,   // the byte to be extracted from
int     pos)    // bit position (0-7)
{
    return ((byte>>pos) & 0x01);
}//GetBit

//-------------------------------------------------------------
// bit order: from right to left , i.e. 7 6 5 4 3 2 1 0
//-------------------------------------------------------------
inline
char
GetBits(
char    byte,   // the byte to be extracted from
int     pos,    // leftmost bit position (0-7)
int     numBits)// number of bits to the right
{
    // gets numBits from position(left) pos from byte
    // move the desired field to the right.
    // ~0 is all 1-bits;
    // shifting it left numBits positions with ~0<<numBits places zeros
    // in the rightmost numBits; complementing that with ~ makes a mask
    // with 1's in the rightmost numBits
    return ( (byte>>(pos+1-numBits)) & ~(~0<<numBits) );
}//GetBits

#ifdef TESTBITS

#include <stdio.h>
#include "CommonDefs.h"
 
main(
int,
char*[])
{
    unsigned char num = 0x6a;  // 01101010
    for (int i=1; i < 7; i++)
    {
        printf("%d: Get 2 Bits starting from bit #%d = %d\n",
                       num, i, GetBits(num, i, 2));
    }
} // main

#endif // TESTBITS

#endif // COMMONDEFS_H
