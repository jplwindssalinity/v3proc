//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef HDF_HELP_H
#define HDF_HELP_H

static const char rcs_id_hdf_help_h[] =
    "@(#) $Id$";

//======================================================================
// CLASS
//    HdfHelp
//
// DESCRIPTION
//    The HdfHelp class simply holds utility functions for Hdf files.
//======================================================================

class HdfHelp
{
public:
    static int IsHdfFile(const char* filename);
};

#endif
