//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_l1a_gs_frame_c[] =
    "@(#) $Id$";

#include <string.h>

#include "L1AGSFrame.h"

L1AGSFrame::L1AGSFrame()
{
    (void)memset(&in_pcd, 0, sizeof(in_pcd));
    (void)memset(&status, 0, sizeof(status));
    (void)memset(&engdata, 0, sizeof(engdata));
    (void)memset(pad1, 0, 2);
    (void)memset(SESerrflags, 0, 13);
    (void)memset(&pad2, 0, 1);
    (void)memset(memro, 0, 26);
    (void)memset(pcd, 0, 32);
    (void)memset(&in_eu, 0, sizeof(in_eu));
    (void)memset(&in_science, 0, sizeof(in_science));
    (void)memset(&la1_frame_inst_status, 0, sizeof(la1_frame_inst_status));
    (void)memset(&l1a_frame_err_status, 0, sizeof(l1a_frame_err_status));
    (void)memset(&l1a_frame_qual_flag, 0, sizeof(l1a_frame_qual_flag));
    (void)memset(&l1a_pulse_qual_flag, 0, 13);
    (void)memset(&pad3, 0, 1);

}

#ifdef test_L1AGSFrame

int
main(
int     ,
char**  )
{
    L1AGSFrame  gsFrame;
printf("total = %d\n", sizeof(L1AGSFrame));    // 5976
    return(0);
}

#endif
