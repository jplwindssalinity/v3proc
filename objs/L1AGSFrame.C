//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_l1a_gs_frame_c[] =
    "@(#) $Id$";

#include "L1AGSFrame.h"

L1AGSFrame::L1AGSFrame()
{
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
