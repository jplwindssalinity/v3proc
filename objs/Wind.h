//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef WIND_H
#define WIND_H

static const char rcs_id_wind_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include "Matrix.h"
#include "AngleInterval.h"
#include "LonLatWind.h"
#include "WindVector.h"
#include "LonLat.h"
#include "Index.h"
#include "List.h"
/*
#include <mfhdf.h>
#include "Misc.h"
*/

#define NWP_SPEED_CORRECTION  0.84

//======================================================================
// CLASSES
//    WindVectorPlus, WindVectorField, WVC, WindField,
//======================================================================

//======================================================================
// CLASS
//    WindVectorPlus
//
// DESCRIPTION
//    The WindVectorPlus object is subclassed from a WindVector and
//    contains additional information generated by the wind retrieval
//    processing.
//======================================================================

class WindVectorPlus : public WindVector
{
public:

    //--------------//
    // construction //
    //--------------//

    WindVectorPlus();
    ~WindVectorPlus();

    //--------------//
    // input/output //
    //--------------//

    int  WriteL2B(FILE* fp);
    int  WriteAscii(FILE* fp);
    int  ReadL2B(FILE* fp);

    //-----------//
    // variables //
    //-----------//

    float  obj;    // the objective function value
};

//======================================================================
// CLASS
//    WindVectorField
//
// DESCRIPTION
//    The WindVectorField object contains spd, dir, and a position in
//    LonLat.
//======================================================================

class WindVectorField
{
public:

    //--------------//
    // construction //
    //--------------//

    WindVectorField();
    ~WindVectorField();

    //--------------//
    // input/interp //
    //--------------//

    int  ReadVctr(const char* filename);
    int  InterpolateVectorField(LonLat lon_lat, WindVector* nudge_wv,
             int idx);

    //-----------//
    // variables //
    //-----------//

    Vector  dir;
    Vector  spd;
    Vector  lon;
    Vector  lat;
};

//======================================================================
// CLASS
//    WVC
//
// DESCRIPTION
//    The WVC object represents a wind vector cell.  It contains a
//    list of ambiguous solution WindVectorPlus.
//======================================================================

#define RAIN_FLAG_UNUSABLE  0x01    // bit 0 1/0 = not usable/usable
#define RAIN_FLAG_RAIN      0x02    // bit 1 1/0 = rain/no rain
#define RAIN_FLAG_LOCATION  0x04    // bit 2 1/0 = outer/inner

class WVC
{
public:

    //--------------//
    // construction //
    //--------------//

    WVC();
    ~WVC();

    //--------------//
    // input/output //
    //--------------//

    int  WriteL2B(FILE* fp);
    int  ReadL2B(FILE* fp);
    int  WriteVctr(FILE* fp, const int rank);    // 0 = selected
    int  WriteAscii(FILE* fp);
    int  WriteFlower(FILE* fp);

    //--------------//
    // manipulation //
    //--------------//

    int              RemoveDuplicates();
    int              RedistributeObjs();
    float            GetEstimatedSquareError();
    int              SortByObj();
    int              SortByDir();
    WindVectorPlus*  GetNearestToDirection(float dir, int max_rank = 0);
    WindVectorPlus*  GetNearestRangeToDirection(float dir);

    //-------------//
    // GS routines //
    //-------------//

    int  Rank_Wind_Solutions();

    //---------//
    // freeing //
    //---------//

    void  FreeContents();

    //-----------//
    // variables //
    //-----------//

    LonLat                 lonLat;
    WindVectorPlus*        nudgeWV;
    WindVectorPlus*        selected;
    int                    selected_allocated;
    WindVector*            specialVector;    // for DIR or whatever
    List<WindVectorPlus>   ambiguities;
    AngleIntervalListPlus  directionRanges;
    float                  rainProb;
    char                   rainFlagBits;
    // bit 0 1/0 = not usable/usable
    // bit 1 1/0 = rain/no rain
    // bit 2 1/0 = outer/inner

    // these are in the ground system L2B, so we put 'em in too
    // they currently are only used when HDF-ing
    unsigned char          numInFore;
    unsigned char          numInAft;
    unsigned char          numOutFore;
    unsigned char          numOutAft;
};

//======================================================================
// CLASS
//    WindField
//
// DESCRIPTION
//    The WindField object hold a non-ambiguous wind field.
//======================================================================

#define VAP_LON_DIM  360
#define VAP_LAT_DIM  121
#define VAP_TYPE     "VAP"

#define ECMWF_HIRES_LON_DIM  640
#define ECMWF_HIRES_LAT_DIM  321
#define ECMWF_HIRES_TYPE     "ECMWF"

#define ECMWF_LORES_LON_DIM       360
#define ECMWF_LORES_LAT_DIM       181
#define ECMWF_LORES_TYPE          "ONE_DEG"
#define ECMWF_LORES_SCALE_FACTOR  100

#define NCEP1_LON_DIM       360
#define NCEP1_LAT_DIM       181
#define NCEP1_TYPE          "NCEP"
#define NCEP1_SCALE_FACTOR  100

#define NCEP2_LON_DIM       144
#define NCEP2_LAT_DIM       73
#define NCEP2_TYPE          "NCEP2.5"
#define NCEP2_SCALE_FACTOR  100

#define NSCAT_LON_DIM     720
#define NSCAT_LAT_DIM     301
#define NSCAT_TYPE        "NSCAT"
#define NSCAT_LAND_VALUE  -9999.0

class WindField : public LonLatWind
{
public:

    //--------------//
    // construction //
    //--------------//

    WindField();
    ~WindField();

    //--------------//
    // input/output //
    //--------------//

    int  ReadVap(const char* filename);
    int  ReadEcmwfHiRes(const char* filename);
    int  ReadHurricane(const char* filename);
    int  WriteEcmwfHiRes(const char* filename, int extra_time_flag = 0);
    int  ReadEcmwfLoRes(const char* filename);
    int  ReadNSCAT(const char* filename);
    int  ReadNCEP1(const char* filename);
    int  ReadNCEP2(const char* filename);
    int  ReadType(const char* filename, const char* type);
    int  WriteVctr(const char* filename);
    int  NewRes(WindField* windfield, float lon_res, float lat_res);

    //--------//
    // access //
    //--------//

    int  NearestWindVector(LonLat lon_lat, WindVector* wv);
    int  InterpolatedWindVector(LonLat lon_lat, WindVector* wv);

    //----------//
    // tweaking //
    //----------//

    int  FixSpeed(float speed);
    int  SetAllSpeeds(float speed);
    int  ScaleSpeed(float scale);
    int  FakeEcmwfHiRes(float speed);

protected:

    //--------------//
    // construction //
    //--------------//

    int  _Allocate();
    int  _Deallocate();

    //-----------//
    // variables //
    //-----------//

    Index  _lon;
    Index  _lat;

    int    _wrap;             // flag for longitude wrapping
    int    _useFixedSpeed;    // flag for using fixed speed
    float  _fixedSpeed;     // the fixed speed

    WindVector***  _field;
};

//======================================================================
// ENUM  COMPONENT_TYPE
//
// DESCRIPTION
//    Used by the WindSwathClass to determine which set of wind
//    vector component to correlate in the ComponentCovarianceVsCti
//    routine.
//======================================================================

enum COMPONENT_TYPE
{
    UTRUE,    // u component of true wind vector
    VTRUE,    // v component of true wind vector
    UMEAS,    // u component of measured wind vector
    VMEAS     // v component of measured wind vector
};

#endif
