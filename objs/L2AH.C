//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_l2ah_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include "L2AH.h"
#include "Sds.h"
#include "Constants.h"

//======//
// L2AH //
//======//

L2AH::L2AH()
:   _hdfInputFileId(FAIL), _sdId(FAIL), _currentRow(-1)
{
    return;
}

//----------------------//
// L2AH::OpenForReading //
//----------------------//

int
L2AH::OpenForReading(
    const char*  input_file)
{
    //---------------------------//
    // open the SDSs for reading //
    //---------------------------//

    _sdId = SDstart(input_file, DFACC_READ);
    if (_sdId == FAIL)
        return(0);

    _rowNumberSdsId = SDnametoid(_sdId, "row_number");
    _numSigma0SdsId = SDnametoid(_sdId, "num_sigma0");
    _cellLatSdsId = SDnametoid(_sdId, "cell_lat");
    _cellLonSdsId = SDnametoid(_sdId, "cell_lon");
    _cellAzimuthSdsId = SDnametoid(_sdId, "cell_azimuth");
    _cellIncidenceSdsId = SDnametoid(_sdId, "cell_incidence");
    _sigma0SdsId = SDnametoid(_sdId, "sigma0");
    _sigma0AttnMapSdsId = SDnametoid(_sdId, "sigma0_attn_map");
    _kpAlphaSdsId = SDnametoid(_sdId, "kp_alpha");
    _kpBetaSdsId = SDnametoid(_sdId, "kp_beta", &_kpBetaScale);
    _kpGammaSdsId = SDnametoid(_sdId, "kp_gamma");
    _sigma0QualFlagSdsId = SDnametoid(_sdId, "sigma0_qual_flag");
    _sigma0ModeFlagSdsId = SDnametoid(_sdId, "sigma0_mode_flag");
    _surfaceFlagSdsId = SDnametoid(_sdId, "surface_flag");
    _cellIndexSdsId = SDnametoid(_sdId, "cell_index");

    //---------------------------//
    // read the row number array //
    //---------------------------//

    int32 start[1] = { 0 };
    int32 edges[1] = { L2AH_ROWS };

    if (SDreaddata(_rowNumberSdsId, start, NULL, edges,
        (VOIDP)&_rowNumber) == FAIL)
    {
        fprintf(stderr, "Error reading row_number\n");
        return(NULL);
    }

    return (1);
}

//-------------//
// L2AH::Close //
//-------------//

int
L2AH::Close()
{
    //----------------//
    // end SDS access //
    //----------------//

    SDendaccess(_rowNumberSdsId);
    SDendaccess(_numSigma0SdsId);
    SDendaccess(_cellLatSdsId);
    SDendaccess(_cellLonSdsId);
    SDendaccess(_cellAzimuthSdsId);
    SDendaccess(_cellIncidenceSdsId);
    SDendaccess(_sigma0SdsId);
    SDendaccess(_sigma0AttnMapSdsId);
    SDendaccess(_kpAlphaSdsId);
    SDendaccess(_kpBetaSdsId);
    SDendaccess(_kpGammaSdsId);
    SDendaccess(_sigma0QualFlagSdsId);
    SDendaccess(_sigma0ModeFlagSdsId);
    SDendaccess(_surfaceFlagSdsId);
    SDendaccess(_cellIndexSdsId);

    if (Hclose(_hdfInputFileId) != SUCCEED)
        return (0);

    _hdfInputFileId = FAIL;

    return(1);
}

//--------------//
// L2AH::GetWVC //
//--------------//

MeasList*
L2AH::GetWVC(
    int        cti,
    int        ati,
    LandFlagE  land_flag)
{
    int32 start[2] = { 0, 0 };
    int32 edges[2] = { 1, 3240 };

    //------------------------------------------------//
    // only read the row if you don't have it already //
    //------------------------------------------------//

    MeasList* ml = new MeasList();

    if (_currentRow != ati)
    {
        //----------------------//
        // find the desired row //
        //----------------------//

        int start_idx = -1;
        for (int i = 0; i < L2AH_ROWS; i++)
        {
            if (_rowNumber[i] == ati + 1)
            {
                // found a match, only take it if it is closer to the middle
                if (abs(i - L2AH_ROWS / 2) < abs(start_idx - L2AH_ROWS / 2))
                    start_idx = i;
            }
        }
        if (start_idx == -1)
            return(NULL);

        //--------------//
        // read the row //
        //--------------//

        start[0] = start_idx;

        // number of sigma-0's
        if (SDreaddata(_numSigma0SdsId, start, NULL, edges,
            (VOIDP)&_numSigma0) == FAIL)
        {
            fprintf(stderr, "Error reading num_sigma0\n");
            return(NULL);
        }

        if (_numSigma0 == 0)
            return(NULL);

        edges[1] = _numSigma0;

        // cell lat
        if (SDreaddata(_cellLatSdsId, start, NULL, edges,
            (VOIDP)_cellLat) == FAIL)
        {
            fprintf(stderr, "Error reading cell_lat\n");
            return(NULL);
        }

        // cell lon
        if (SDreaddata(_cellLonSdsId, start, NULL, edges,
            (VOIDP)_cellLon) == FAIL)
        {
            fprintf(stderr, "Error reading cell_lon\n");
            return(NULL);
        }

        // cell azimuth
        if (SDreaddata(_cellAzimuthSdsId, start, NULL, edges,
            (VOIDP)_cellAzimuth) == FAIL)
        {
            fprintf(stderr, "Error reading cell_azimuth\n");
            return(NULL);
        }

        // cell incidence
        if (SDreaddata(_cellIncidenceSdsId, start, NULL, edges,
            (VOIDP)_cellIncidence) == FAIL)
        {
            fprintf(stderr, "Error reading cell_incidence\n");
            return(NULL);
        }

        // sigma-0
        if (SDreaddata(_sigma0SdsId, start, NULL, edges,
            (VOIDP)_sigma0) == FAIL)
        {
            fprintf(stderr, "Error reading sigma0\n");
            return(NULL);
        }

        // sigma0 attn map
        if (SDreaddata(_sigma0AttnMapSdsId, start, NULL, edges,
            (VOIDP)_sigma0AttnMap) == FAIL)
        {
            fprintf(stderr, "Error reading sigma0_attn_map\n");
            return(NULL);
        }

        // alpha
        if (SDreaddata(_kpAlphaSdsId, start, NULL, edges,
            (VOIDP)_kpAlpha) == FAIL)
        {
            fprintf(stderr, "Error reading kp_alpha\n");
            return(NULL);
        }

        // beta
        if (SDreaddata(_kpBetaSdsId, start, NULL, edges,
            (VOIDP)_kpBeta) == FAIL)
        {
            fprintf(stderr, "Error reading kp_beta\n");
            return(NULL);
        }

        // gamma
        if (SDreaddata(_kpGammaSdsId, start, NULL, edges,
            (VOIDP)_kpGamma) == FAIL)
        {
            fprintf(stderr, "Error reading kp_gamma\n");
            return(NULL);
        }

        // sigma-0 quality flag
        if (SDreaddata(_sigma0QualFlagSdsId, start, NULL, edges,
            (VOIDP)_sigma0QualFlag) == FAIL)
        {
            fprintf(stderr, "Error reading sigma0_qual_flag\n");
            return(NULL);
        }

        // sigma-0 mode flag
        if (SDreaddata(_sigma0ModeFlagSdsId, start, NULL, edges,
            (VOIDP)_sigma0ModeFlag) == FAIL)
        {
            fprintf(stderr, "Error reading sigma0_mode_flag\n");
            return(NULL);
        }

        // surface flag
        if (SDreaddata(_surfaceFlagSdsId, start, NULL, edges,
            (VOIDP)_surfaceFlag) == FAIL)
        {
            fprintf(stderr, "Error reading surface_flag\n");
            return(NULL);
        }

        // cell index
        if (SDreaddata(_cellIndexSdsId, start, NULL, edges,
            (VOIDP)_cellIndex) == FAIL)
        {
            fprintf(stderr, "Error reading cell_index\n");
            return(NULL);
        }

        _currentRow = ati;
    }

    //---------------------------//
    // assemble the measurements //
    //---------------------------//

    int target_cell_index = cti + 1;
    for (int i = 0; i < _numSigma0; i++)
    {
        // ignore wrong cross track indices
        if (_cellIndex[i] != target_cell_index)
            continue;

        // ignore bad sigma0's
        if (_sigma0QualFlag[i] & UNUSABLE)
            continue;

        // ignore land sigma0's, maybe?
        if (land_flag == OCEAN_ONLY && _surfaceFlag[i]) // & 0x00000001)
            continue;

        // create a measurement
        Meas* new_meas = new Meas();

        // set the land flag
        new_meas->landFlag = (int)(_surfaceFlag[i] & 0x00000001);

        // set the lat and lon
        new_meas->centroid.SetAltLonGDLat(0.0,
            _cellLon[i] * CELL_LON_SCALE * dtr,
            _cellLat[i] * CELL_LAT_SCALE * dtr);

        // set the azimuth
        new_meas->eastAzimuth = (450.0 - _cellAzimuth[i] * CELL_AZIMUTH_SCALE)
            * dtr;
        if (new_meas->eastAzimuth >= two_pi)
            new_meas->eastAzimuth -= two_pi;

        // set the incidence
        new_meas->incidenceAngle =
            _cellIncidence[i] * CELL_INCIDENCE_SCALE * dtr;

        // set sigma0
        new_meas->value = _sigma0[i] * SIGMA0_SCALE
            + _sigma0AttnMap[i] * SIGMA0_ATTN_MAP_SCALE /
            cos(new_meas->incidenceAngle);
        new_meas->value = pow(10.0, 0.1 * new_meas->value);
        if (_sigma0QualFlag[i] & NEGATIVE)
            new_meas->value = -(new_meas->value);

        // set the beam index and measurement type
        new_meas->beamIdx = (int)(_sigma0ModeFlag[i] & 0x0004);
        new_meas->beamIdx >>= 2;
        if (new_meas->beamIdx == 0)
            new_meas->measType = Meas::HH_MEAS_TYPE;
        else
            new_meas->measType = Meas::VV_MEAS_TYPE;

        // number of slices
        new_meas->numSlices = -1;

        // kp alpha, beta, and gamma
        new_meas->A = _kpAlpha[i] * KP_ALPHA_SCALE;
        new_meas->B = _kpBeta[i] * _kpBetaScale;
        new_meas->C = _kpGamma[i];

        // hack fore/aft info into scanAngle
        if (_sigma0ModeFlag[i] & 0x0008)
            new_meas->scanAngle = 0.0;    // fore
        else
            new_meas->scanAngle = 1.0;    // aft

        //------------------------//
        // append the measurement //
        //------------------------//

        if (! ml->Append(new_meas))
        {
            fprintf(stderr, "Error appending measurement\n");
            return(NULL);
        }
    }
    return(ml);
}
