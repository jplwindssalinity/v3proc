//==============================================================//
// Copyright (C) 1997-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef L2B_H
#define L2B_H

static const char rcs_id_l2b_h[] =
    "@(#) $Id$";

#include "BaseFile.h"
#include "Wind.h"

//======================================================================
// CLASSES
//    L2BHeader, L2BFrame, L2B
//======================================================================

//======================================================================
// CLASS
//    L2BHeader
//
// DESCRIPTION
//    The L2BHeader object contains the contents of a Level 2B header.
//======================================================================

class L2BHeader
{
public:

    //--------------//
    // construction //
    //--------------//

    L2BHeader();
    ~L2BHeader();

    //--------------//
    // input/output //
    //--------------//

    int  Read(FILE* fp);
    int  Write(FILE* fp);
    int  WriteAscii(FILE* fp);

    //-----------//
    // variables //
    //-----------//

    float  crossTrackResolution;
    float  alongTrackResolution;
    int    zeroIndex;
};

//======================================================================
// CLASS
//    L2BFrame
//
// DESCRIPTION
//    The L2BFrame object contains the contents of a Level 2B frame
//    as a structure.
//======================================================================

class L2BFrame
{
public:

    //--------------//
    // construction //
    //--------------//

    L2BFrame();
    ~L2BFrame();

    //-------------------//
    // product variables //
    //-------------------//

    WindSwath  swath;
};

//======================================================================
// CLASS
//    L2B
//
// DESCRIPTION
//    The L2B object allows for the easy writing, reading, and
//    manipulating of Level 2B data.
//======================================================================

class L2B : public BaseFile
{
public:

    //------//
    // enum //
    //------//

    enum StatusE { OK, ERROR_READING_FRAME, ERROR_UNKNOWN };

    //--------------//
    // construction //
    //--------------//

    L2B();
    ~L2B();

    //---------------------//
    // setting and getting //
    //---------------------//

    StatusE  GetStatus() { return(_status); };

    //--------------//
    // input/output //
    //--------------//

    int  ReadHeader() { return(header.Read(_inputFp)); };
    int  WriteHeader() { return(header.Write(_outputFp)); };

    int  ReadDataRec() { return(frame.swath.ReadL2B(_inputFp)); };
    int  ReadHDF(int unnormalize_mle = 1);
    int  ReadHDF(const char* filename, int unnormalize_mle = 1);
    int  ReadHDFDIRTH(const char* filename);
    int  ReadHDF(TlmHdfFile* tlmHdfFile, int unnormalize_mle = 1);
    int  ReadNudgeVectorsFromHdfL2B(const char* filename);
    int  ReadNudgeVectorsFromHdfL2B(TlmHdfFile* tlmHdfFile);

    int  GetArraysForUpdatingDirthHdf(float** spd, float** dir,
             int** num_ambig);

    int  WriteDataRec() { return(frame.swath.WriteL2B(_outputFp)); };

    int  WriteVctr(const char* filename, const int rank);
    int  WriteAscii();

    //-----------//
    // variables //
    //-----------//

    L2BHeader  header;
    L2BFrame   frame;

protected:

    //-----------//
    // variables //
    //-----------//

    StatusE  _status;

    //---------//
    // for HDF //
    //---------//

    int   _OpenHdfDataSets(TlmHdfFile* tlmHdfFile);
    int   _OpenOneHdfDataSet(TlmHdfFile* tlmHdfFile, SourceIdE source,
              ParamIdE param);
    int   _OpenOneHdfDataSetCorrectly(TlmHdfFile* tlmHdfFile, const char* sdsName);
    void  _CloseHdfDataSets();

    int32  _lonSdsId;
    int32  _latSdsId;
    int32  _speedSdsId;
    int32  _dirSdsId;
    int32  _mleSdsId;
    int32  _selectSdsId;
    int32  _numambigSdsId;
    int32  _modelSpeedSdsId;
    int32  _modelDirSdsId;
    int32  _numInForeSdsId;
    int32  _numInAftSdsId;
    int32  _numOutForeSdsId;
    int32  _numOutAftSdsId;
    int32  _mpRainProbSdsId;
    int32  _qualSdsId;
};

#endif
