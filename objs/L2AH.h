//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef L2AH_H
#define L2AH_H

static const char rcs_id_l1ah_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include "L2A.h"
#include "ETime.h"
#include "hdf.h"

//======================================================================
// CLASSES
//    L2AH
//======================================================================

#define AT_WIDTH   1624
#define CT_WIDTH   76

#define L2AH_ROWS          1702
#define L2AH_MEASUREMENTS  3240

#define UNUSABLE         0x0001
#define NEGATIVE         0x0004

#define CELL_LON_SCALE         0.01
#define CELL_LAT_SCALE         0.01
#define CELL_AZIMUTH_SCALE     0.01
#define CELL_INCIDENCE_SCALE   0.01
#define SIGMA0_SCALE           0.01
#define SIGMA0_ATTN_MAP_SCALE  0.01
#define KP_ALPHA_SCALE         0.001

//======================================================================
// CLASS
//    L2AH
//
// DESCRIPTION
//    The L2AH class allows for the easy reading in of WVC measurement
//    lists from L2A HDF files.
//======================================================================

class L2AH
{
public:

    enum LandFlagE { ALL = 0, OCEAN_ONLY };

    L2AH();

    int  OpenForReading(const char* filename);
    int  Close();

    MeasList*  GetWVC(int cti, int ati, LandFlagE land_flag);

protected:

    //------------//
    // file stuff //
    //------------//

    int32   _hdfInputFileId;

    int32   _sdId;

    //----------//
    // SDS ID's //
    //----------//

    int32  _rowNumberSdsId;
    int32  _numSigma0SdsId;
    int32  _cellLatSdsId;
    int32  _cellLonSdsId;
    int32  _cellAzimuthSdsId;
    int32  _cellIncidenceSdsId;
    int32  _sigma0SdsId;
    int32  _sigma0AttnMapSdsId;
    int32  _kpAlphaSdsId;
    int32  _kpBetaSdsId;
    int32  _kpGammaSdsId;
    int32  _sigma0QualFlagSdsId;
    int32  _sigma0ModeFlagSdsId;
    int32  _surfaceFlagSdsId;
    int32  _cellIndexSdsId;

    //---------------//
    // scale factors //
    //---------------//

    float64  _kpBetaScale;

    //-------------//
    // row storage //
    //-------------//

    int16    _numSigma0;
    int16    _rowNumber[L2AH_ROWS];
    int16    _cellLat[L2AH_MEASUREMENTS];
    uint16   _cellLon[L2AH_MEASUREMENTS];
    uint16   _cellAzimuth[L2AH_MEASUREMENTS];
    int16    _cellIncidence[L2AH_MEASUREMENTS];
    int16    _sigma0[L2AH_MEASUREMENTS];
    int16    _sigma0AttnMap[L2AH_MEASUREMENTS];
    int16    _kpAlpha[L2AH_MEASUREMENTS];
    uint16   _kpBeta[L2AH_MEASUREMENTS];
    float32  _kpGamma[L2AH_MEASUREMENTS];
    uint16   _sigma0QualFlag[L2AH_MEASUREMENTS];
    uint16   _sigma0ModeFlag[L2AH_MEASUREMENTS];
    uint16   _surfaceFlag[L2AH_MEASUREMENTS];
    uint8    _cellIndex[L2AH_MEASUREMENTS];

    int16    _currentRow;
};

#endif
