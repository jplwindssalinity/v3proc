//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef L2B_H
#define L2B_H

static const char rcs_id_l2b_h[] =
    "@(#) $Id$";

#include "TlmHdfFile.h"
#include "BaseFile.h"
#include "WindSwath.h"

#define L2B_TYPE  "HDF_L2B"

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
    float  inclination;
    
    int    version_id_major;
    int    version_id_minor;
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

#define HDF_WVC_LAT_SCALE               0.01
#define HDF_WVC_LON_SCALE               0.01
#define HDF_MODEL_SPEED_SCALE           0.01
#define HDF_MODEL_DIR_SCALE             0.01
#define HDF_WIND_SPEED_SCALE            0.01
#define HDF_WIND_DIR_SCALE              0.01
#define HDF_MAX_LIKELIHOOD_EST_SCALE    0.001
#define HDF_WIND_SPEED_SELECTION_SCALE  0.01
#define HDF_WIND_DIR_SELECTION_SCALE    0.01
#define HDF_MP_RAIN_PROBABILITY_SCALE   0.001

#define L2B_HDF_QUAL_FLAG_LAND           0x0080  // 2^7;  bit set => some land in WVC
#define L2B_HDF_QUAL_FLAG_ICE            0x0100  // 2^8;  bit set => some ice  in WVC
#define L2B_HDF_QUAL_FLAG_RAIN_UNUSABLE  0x1000  // 2^12; bit set => rain flag not usable
#define L2B_HDF_QUAL_FLAG_RAIN           0x2000  // 2^13; bit set => rain detected if usable
#define L2B_HDF_QUAL_FLAG_AVAILABLE_DATA 0x4000  // 2^14; bit set => not all 4 looks available

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
    
    int  WriteRETDAT( const char* filename, int write_speed_ridges_flag = 1 );
    int  ReadRETDAT(const char* filename, int read_nudge_vectors_flag = 1);
    
    int  ReadHeader(); // 6/3/2010 AGF moved this method from inline here to L2B.C
    int  WriteHeader() { return(header.Write(_outputFp)); };

    int  ReadDataRec() { return(frame.swath.ReadL2B(_inputFp)); };

    int  SmartRead(const char* filename, int unnormalize_mle_flag = 1);
    int  Read(const char* filename);
    int  ReadPureHdf(const char* filename, int unnormalize_mle_flag = 1);
    int  WriteHdf(const char* filename, int unnormalize_mle_flag = 1);
    int  InsertPureHdf(const char* input_filename,
             const char* output_filename, int unnormalize_mle_flag = 1);

    int  ReadHDF(int unnormalize_mle = 1);
    int  ReadHDF(const char* filename, int unnormalize_mle = 1);
    int  ReadHDFDIRTH(const char* filename);
    int  ReadHDF(TlmHdfFile* tlmHdfFile, int unnormalize_mle = 1);
    int  ReadNudgeVectorsFromHdfL2B(const char* filename, int read_wvc_flags_flag = 0 );
    int  ReadLandIceRainFlagsFromHdfL2B(const char* filename, 
              int read_land_ice_flags, int read_rain_flags );
              
//    int  ReadNudgeVectorsFromHdfL2B(TlmHdfFile* tlmHdfFile);

    int  GetArraysForUpdatingDirthHdf(float** spd, float** dir,
             int** num_ambig);

    int  WriteDataRec() { return(frame.swath.WriteL2B(_outputFp)); };

    int  WriteVctr(const char* filename, const int rank);
    int  WriteAscii();
    int  GetNumCellsSelected();
    int  GetNumCellsWithAmbiguities();

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
    int   _OpenOneHdfDataSetCorrectly(TlmHdfFile* tlmHdfFile,
              const char* sdsName);
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
