//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_hdf_help_c[] =
    "@(#) $Id$";

#include <hdf.h>
#include <mfhdf.h>
#include "HdfHelp.h"

//--------------------//
// HdfHelp::IsHdfFile //
//--------------------//

int
HdfHelp::IsHdfFile(
    const char*  filename)
{
    int32 file_id = Hopen(filename, DFACC_READ, 0);
    if (file_id == FAIL) {
        return(0);
    }
    Hclose(file_id);
    return(1);
}
