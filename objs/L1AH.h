//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef L1AH_H
#define L1AH_H

static const char rcs_id_l1ah_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include "L1A.h"
#include "hdf.h"

//======================================================================
// CLASSES
//    L1AH
//======================================================================

#define FRAME_TIME_NAME  "frame_time"

//======================================================================
// CLASS
//    L1AH
//
// DESCRIPTION
//    The L1AH class is a subclass of L1A that allows for the writing
//    of L1A HDF files.
//======================================================================

class L1AH : public L1A
{
public:

    L1AH();

    int NextRecord();

    int OpenHdfForWriting();
    int OpenHdfForReading();
    int CreateVdatas();
    int WriteVdatas();
    int EndVdataOutput();

    int OpenSDSForWriting();
    int CreateSDSs();
    int WriteSDSs();
    int EndSDSOutput();

    int WriteHDFFrame();

    int WriteHDFHeader();

    int CloseHdfInputFile();
    int CloseHdfOutputFile();

protected:

    int32  _hdfInputFileId;
    int32  _hdfOutputFileId;

    int32  _sdsInputFileId;
    int32  _sdsOutputFileId;

    int    _currentRecordIdx;
};

#endif
