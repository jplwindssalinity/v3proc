//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef WINDQ_H
#define WINDQ_H

static const char rcs_id_windq_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include <mfhdf.h>
#include "Wind.h"

//======================================================================
// CLASSES
//    QWVC
//======================================================================

//======================================================================
// CLASS
//    QWVC
//
// DESCRIPTION
//    The QWVC object represents a wind vector cell for QuikSCAT.  It
//    is based on a WVC object, but contains QuikSCAT specific
//    elements such as the quality flag from the L2B file.
//    Right now, it doesn't do much by itself, but is used by the L2B
//    object to read in a QuikSCAT L2B file.
//======================================================================

#define RAIN_FLAG_UNUSABLE  0x1000    // bit 12
#define RAIN_FLAG_RAIN      0x2000    // bit 13

class QWVC : public WVC
{
public:

    //--------------//
    // construction //
    //--------------//

    QWVC();
    ~QWVC();

    //-----------//
    // variables //
    //-----------//

    int  qualityFlag;
};

#endif
