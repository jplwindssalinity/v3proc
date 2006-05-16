//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef L2A_H
#define L2A_H

static const char rcs_id_l2a_h[] =
    "@(#) $Id$";

#include "BaseFile.h"
#include "Meas.h"


//======================================================================
// CLASSES
//    L2AHeader, L2AFrame, L2A
//======================================================================

//======================================================================
// CLASS
//    L2AHeader
//
// DESCRIPTION
//    The L2AHeader object contains the contents of a Level 2A header.
//======================================================================

class L2AHeader
{
public:

    //--------------//
    // construction //
    //--------------//

    L2AHeader();
    ~L2AHeader();

    //--------------//
    // input/output //
    //--------------//

    int  Read(FILE* fp);
    int  Write(FILE* fp);
    int  WriteAscii(FILE* fp);

    //-----------//
    // variables //
    //-----------//

    float   crossTrackResolution;    // km
    float   alongTrackResolution;    // km
    int     crossTrackBins;
    int     alongTrackBins;
    int     zeroIndex;    // cti of bin for 0 km cross track distance
    double  startTime;    // zero point of along track axis
};

//======================================================================
// CLASS
//    L2AFrame
//
// DESCRIPTION
//    The L2AFrame object contains the contents of a Level 2A frame
//    as a structure.
//======================================================================

class L2AFrame
{
public:

    //--------------//
    // construction //
    //--------------//

    L2AFrame();
    ~L2AFrame();

    //--------------//
    // input/output //
    //--------------//

    int  Read(FILE* fp);
    int  Write(FILE* fp);
    int  WriteAscii(FILE* fp);

    int  ReadGS(FILE* fp);
    int  CombineFrames(L2AFrame* frameGroup25, L2AFrame* frameGroup50);
    int  CopyFrame(L2AFrame* nframe0, L2AFrame* nframe1);

    //-----------//
    // variables //
    //-----------//

    unsigned int   rev;
    int            ati;
    int            cti;
    MeasList       measList;    // a list of measurements from a single frame
};

//======================================================================
// CLASS
//    L2A
//
// DESCRIPTION
//    The L2A object allows for the easy writing, reading, and
//    manipulating of Level 2A data.
//    Level 2A data consists of spatially co-located measurements as
//    opposed to the time ordered measurements in Level 1B data.
//======================================================================

class L2A : public BaseFile
{
public:

    //------//
    // enum //
    //------//

    enum StatusE { OK, ERROR_READING_HEADER, ERROR_READING_FRAME,
        ERROR_UNKNOWN };

    //--------------//
    // construction //
    //--------------//

    L2A();
    ~L2A();

    //--------------//
    // input/output //
    //--------------//

    int  WriteHeader();
    int  WriteHeaderAscii();
    int  ReadHeader();
    int  WriteDataRec();
    int  WriteDataRecAscii();
    int  ReadDataRec();
    int  ReadGroupRec(int reset, L2AFrame* frameGroup25,
             L2AFrame* frameGroup50);
    int  ReadGSDataRec();

    //---------------------//
    // setting and getting //
    //---------------------//

    StatusE  GetStatus() { return(_status); };

    //-----------//
    // variables //
    //-----------//

    L2AHeader  header;
    L2AFrame   frame;

protected:

    //-----------//
    // variables //
    //-----------//

    StatusE  _status;
    int      _headerRead;
    int      _headerWritten;
    int      idx;
};

#endif
