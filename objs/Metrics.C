//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_metrics_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <string.h>
#include "Metrics.h"
#include "Wind.h"

#define METRICS_HEADER  "mets"

//=========//
// Metrics //
//=========//

Metrics::Metrics()
:   _crossTrackBins(0), _crossTrackResolution(0.0),
    _lowWindSpeed(DEFAULT_METRICS_LOW_WIND_SPEED),
    _highWindSpeed(DEFAULT_METRICS_HIGH_WIND_SPEED),
    _nearSumSqrSpdErr(NULL), _nearSumSqrSpdErrCount(NULL),
    _nearSumSqrDirErr(NULL), _nearSumSqrDirErrCount(NULL),
    _selSumSqrSpdErr(NULL), _selSumSqrSpdErrCount(NULL),
    _selSumSqrDirErr(NULL), _selSumSqrDirErrCount(NULL)
{
    return;
}

Metrics::~Metrics()
{
    _Deallocate();
    return;
}

//---------------------//
// Metrics::Initialize //
//---------------------//

int
Metrics::Initialize(
    int    cross_track_bins,
    float  cross_track_resolution)
{
    if (! _Allocate(cross_track_bins))
        return(0);

    _SetResolution(cross_track_resolution);
    return(1);
}

//----------------------------//
// Metrics::SetWindSpeedRange //
//----------------------------//

int
Metrics::SetWindSpeedRange(
    float  low_speed,
    float  high_speed)
{
    _lowWindSpeed = low_speed;
    _highWindSpeed = high_speed;
    return(1);
}

//----------------//
// Metrics::Clear //
//----------------//

void
Metrics::Clear()
{
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        _nearSumSqrSpdErr[cti] = 0.0;
        _nearSumSqrSpdErrCount[cti] = 0;
        _nearSumSqrDirErr[cti] = 0.0;
        _nearSumSqrDirErrCount[cti] = 0;

        _selSumSqrSpdErr[cti] = 0.0;
        _selSumSqrSpdErrCount[cti] = 0;
        _selSumSqrDirErr[cti] = 0.0;
        _selSumSqrDirErrCount[cti] = 0;
    }
    return;
}

//---------------------//
// Metrics::IndexToCtd //
//---------------------//

float
Metrics::IndexToCtd(
    int  cti)
{
    float ctd = ((float)cti - ((float)_crossTrackBins - 1.0)
            / 2.0) * _crossTrackResolution;
    return(ctd);
}

//---------------//
// Metrics::Read //
//---------------//

int
Metrics::Read(
    const char*  filename)
{
    //---------------//
    // open the file //
    //---------------//

    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL)
        return(0);

    char header[4];
    if (fread((void *)header, sizeof(char), 4, ifp) != 4 ||
        strncmp(header, METRICS_HEADER, 4) != 0)
    {
        fclose(ifp);
        return(0);
    }

    //-------------------//
    // read the "header" //
    //-------------------//

    int cross_track_bins;
    float cross_track_resolution;

    if (fread((void *)&cross_track_bins, sizeof(int), 1, ifp) != 1 ||
        fread((void *)&cross_track_resolution, sizeof(float), 1, ifp) != 1)
    {
        fprintf(stderr, "Metrics::Read: error reading header\n");
        fclose(ifp);
        return(0);
    }

    //------------//
    // initialize //
    //------------//

    if (! Initialize(cross_track_bins, cross_track_resolution))
    {
        fprintf(stderr, "Metrics::Read: error initializing metrics\n");
        fclose(ifp);
        return(0);
    }
    
    //------//
    // read //
    //------//

    if (fread((void *)_nearSumSqrSpdErr, sizeof(double),
            _crossTrackBins, ifp) != (size_t)_crossTrackBins ||
        fread((void *)_nearSumSqrSpdErrCount, sizeof(unsigned long),
            _crossTrackBins, ifp) != (size_t)_crossTrackBins ||
        fread((void *)_nearSumSqrDirErr, sizeof(double),
            _crossTrackBins, ifp) != (size_t)_crossTrackBins ||
        fread((void *)_nearSumSqrDirErrCount, sizeof(unsigned long),
            _crossTrackBins, ifp) != (size_t)_crossTrackBins ||
        fread((void *)_selSumSqrSpdErr, sizeof(double),
            _crossTrackBins, ifp) != (size_t)_crossTrackBins ||
        fread((void *)_selSumSqrSpdErrCount, sizeof(unsigned long),
            _crossTrackBins, ifp) != (size_t)_crossTrackBins ||
        fread((void *)_selSumSqrDirErr, sizeof(double),
            _crossTrackBins, ifp) != (size_t)_crossTrackBins ||
        fread((void *)_selSumSqrDirErrCount, sizeof(unsigned long),
            _crossTrackBins, ifp) != (size_t)_crossTrackBins)
    {
        fprintf(stderr, "Metrics::Read: error reading data\n");
        fclose(ifp);
        return(0);
    }

    fclose(ifp);
    return(1);
}

//----------------//
// Metrics::Write //
//----------------//

int
Metrics::Write(
    const char*  filename)
{
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    char* metrics_string = METRICS_HEADER;

    if (fwrite(metrics_string, sizeof(char), 4, ofp) != 4 ||
        fwrite((void *)&_crossTrackBins, sizeof(int), 1, ofp) != 1 ||
        fwrite((void *)&_crossTrackResolution, sizeof(float), 1, ofp) != 1 ||
        fwrite((void *)_nearSumSqrSpdErr, sizeof(double),
            _crossTrackBins, ofp) != (size_t)_crossTrackBins ||
        fwrite((void *)_nearSumSqrSpdErrCount, sizeof(unsigned long),
            _crossTrackBins, ofp) != (size_t)_crossTrackBins ||
        fwrite((void *)_nearSumSqrDirErr, sizeof(double),
            _crossTrackBins, ofp) != (size_t)_crossTrackBins ||
        fwrite((void *)_nearSumSqrDirErrCount, sizeof(unsigned long),
            _crossTrackBins, ofp) != (size_t)_crossTrackBins ||
        fwrite((void *)_selSumSqrSpdErr, sizeof(double),
            _crossTrackBins, ofp) != (size_t)_crossTrackBins ||
        fwrite((void *)_selSumSqrSpdErrCount, sizeof(unsigned long),
            _crossTrackBins, ofp) != (size_t)_crossTrackBins ||
        fwrite((void *)_selSumSqrDirErr, sizeof(double),
            _crossTrackBins, ofp) != (size_t)_crossTrackBins ||
        fwrite((void *)_selSumSqrDirErrCount, sizeof(unsigned long),
            _crossTrackBins, ofp) != (size_t)_crossTrackBins)
    {
        fclose(ofp);
        return(0);
    }

    fclose(ofp);

    return(1);
}

//------------------------//
// Metrics::WritePlotData //
//------------------------//

int
Metrics::WritePlotData(
    const char*  basename)
{
    FILE* ofp = NULL;

    //-------------------------//
    // nearest rms speed error //
    //-------------------------//

    ofp = OpenPlotFile(basename, "near_rms_spd_err", "Nearest RMS Speed Error",
        "Cross Track Distance (km)", "RMS Speed Error (m/s)");
    if (ofp == NULL)
        return(0);
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        if (_nearSumSqrSpdErrCount[cti] == 0)
            continue;
        double ctd = IndexToCtd(cti);
        double value = sqrt(_nearSumSqrSpdErr[cti]
            / (double)_nearSumSqrSpdErrCount[cti]);
        fprintf(ofp, "%g %g\n", ctd, value);
    }
    fclose(ofp);

    //-----------------------------//
    // nearest rms direction error //
    //-----------------------------//

    ofp = OpenPlotFile(basename, "near_rms_dir_err",
        "Nearest RMS Direction Error", "Cross Track Distance (km)",
        "RMS Direction Error (deg)");
    if (ofp == NULL)
        return(0);
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        if (_nearSumSqrDirErrCount[cti] == 0)
            continue;
        double ctd = IndexToCtd(cti);
        double value = rtd * sqrt(_nearSumSqrDirErr[cti]
            / (double)_nearSumSqrDirErrCount[cti]);
        fprintf(ofp, "%g %g\n", ctd, value);
    }
    fclose(ofp);

    //--------------------------//
    // selected rms speed error //
    //--------------------------//

    ofp = OpenPlotFile(basename, "sel_rms_spd_err", "Selected RMS Speed Error",
        "Cross Track Distance (km)", "RMS Speed Error (m/s)");
    if (ofp == NULL)
        return(0);
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        if (_selSumSqrSpdErrCount[cti] == 0)
            continue;
        double ctd = IndexToCtd(cti);
        double value = sqrt(_selSumSqrSpdErr[cti]
            / (double)_selSumSqrSpdErrCount[cti]);
        fprintf(ofp, "%g %g\n", ctd, value);
    }
    fclose(ofp);

    //------------------------------//
    // selected rms direction error //
    //------------------------------//

    ofp = OpenPlotFile(basename, "sel_rms_dir_err",
        "Selected RMS Direction Error", "Cross Track Distance (km)",
        "RMS Direction Error (deg)");
    if (ofp == NULL)
        return(0);
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        if (_selSumSqrDirErrCount[cti] == 0)
            continue;
        double ctd = IndexToCtd(cti);
        double value = rtd * sqrt(_selSumSqrDirErr[cti]
            / (double)_selSumSqrDirErrCount[cti]);
        fprintf(ofp, "%g %g\n", ctd, value);
    }
    fclose(ofp);

    return(1);
}

//-----------------------//
// Metrics::OpenPlotFile //
//-----------------------//

#define QUOTE  '"'

FILE*
Metrics::OpenPlotFile(
    const char*  output_base,
    const char*  extension,
    const char*  title,
    const char*  xaxis_label,
    const char*  yaxis_label)
{
    char filename[1024];
    sprintf(filename, "%s.%s", output_base, extension);
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        return(NULL);
    }

    char subtitle[1024];
    sprintf(subtitle, "%g - %g m/s", _lowWindSpeed, _highWindSpeed);

    if (title != NULL)
        fprintf(ofp, "@ title %c%s%c\n", QUOTE, title, QUOTE);
    if (subtitle != NULL)
        fprintf(ofp, "@ subtitle %c%s%c\n", QUOTE, subtitle, QUOTE);
    if (xaxis_label != NULL)
        fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE, xaxis_label, QUOTE);
    if (yaxis_label != NULL)
        fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, yaxis_label, QUOTE);

    return(ofp);
}

//-----------------------//
// Metrics::IsCompatible //
//-----------------------//

int
Metrics::IsCompatible(
    const Metrics&  m)
{
    if (_crossTrackBins == m._crossTrackBins &&
        _crossTrackResolution == m._crossTrackResolution &&
        _lowWindSpeed == m._lowWindSpeed &&
        _highWindSpeed == m._highWindSpeed)
    {
        return(1);
    }
    return(0);
}

//-------------------//
// Metrics::Evaluate //
//-------------------//

int
Metrics::Evaluate(
    WindSwath*  swath,
    float       resolution,
    WindField*  truth)
{
    Clear();    // clear the metrics first
    int cross_track_bins = swath->GetCrossTrackBins();
    if (! Initialize(cross_track_bins, resolution))
        return(0);

    int along_track_bins = swath->GetAlongTrackBins();

    for (int cti = 0; cti < cross_track_bins; cti++)
    {
        for (int ati = 0; ati < along_track_bins; ati++)
        {
            WVC* wvc = swath->GetWVC(cti, ati);
            if (wvc == NULL)
                continue;

            WindVector true_wv;
            if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
                continue;

            //-------------------//
            // check speed range //
            //-------------------//

            if (true_wv.spd < _lowWindSpeed || true_wv.spd >= _highWindSpeed)
                continue;

            //-------------------------//
            // nearest RMS speed error //
            //-------------------------//

            WindVectorPlus* nearest = wvc->GetNearestToDirection(true_wv.dir);
            if (nearest == NULL)
                continue;

            double spd_dif = nearest->spd - true_wv.spd;
            double spd_dif_sqr = spd_dif * spd_dif;

            _nearSumSqrSpdErr[cti] += spd_dif_sqr;
            _nearSumSqrSpdErrCount[cti]++;

            //-----------------------------//
            // nearest RMS direction error //
            //-----------------------------//

            double dir_dif = ANGDIF(nearest->dir, true_wv.dir);
            double dir_dif_sqr = dir_dif * dir_dif;

            _nearSumSqrDirErr[cti] += dir_dif_sqr;
            _nearSumSqrDirErrCount[cti]++;

            //--------------------------//
            // selected RMS speed error //
            //--------------------------//

            if (wvc->selected == NULL)
                continue;

            spd_dif = wvc->selected->spd - true_wv.spd;
            spd_dif_sqr = spd_dif * spd_dif;

            _selSumSqrSpdErr[cti] += spd_dif_sqr;
            _selSumSqrSpdErrCount[cti]++;

            //------------------------------//
            // selected RMS direction error //
            //------------------------------//

            dir_dif = ANGDIF(wvc->selected->dir, true_wv.dir);
            dir_dif_sqr = dir_dif * dir_dif;

            _selSumSqrDirErr[cti] += dir_dif_sqr;
            _selSumSqrDirErrCount[cti]++;
        }
    }
    return(1);
}

//---------------------//
// Metrics::operator+= //
//---------------------//
// This operator will initialize this object if necessary.

void
Metrics::operator+=(
    const Metrics&  m)
{
    if (_crossTrackBins == 0)
    {
        if (! Initialize(m._crossTrackBins, m._crossTrackResolution))
        {
            fprintf(stderr,
                "Metrics::operator+=: error initializing Metrics\n");
            exit(1);
        }
        _lowWindSpeed = m._lowWindSpeed;
        _highWindSpeed = m._highWindSpeed;
    }
    if (! IsCompatible(m))
    {
        fprintf(stderr,
            "Metrics::operator+=: attempting to add incompatible Metrics\n");
        exit(1);
    }

    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        _nearSumSqrSpdErr[cti] += m._nearSumSqrSpdErr[cti];
        _nearSumSqrSpdErrCount[cti] += m._nearSumSqrSpdErrCount[cti];
        _nearSumSqrDirErr[cti] += m._nearSumSqrDirErr[cti];
        _nearSumSqrDirErrCount[cti] += m._nearSumSqrDirErrCount[cti];

        _selSumSqrSpdErr[cti] += m._selSumSqrSpdErr[cti];
        _selSumSqrSpdErrCount[cti] += m._selSumSqrSpdErrCount[cti];
        _selSumSqrDirErr[cti] += m._selSumSqrDirErr[cti];
        _selSumSqrDirErrCount[cti] += m._selSumSqrDirErrCount[cti];
    }

    return;
}

//--------------------//
// Metrics::_Allocate //
//--------------------//
// Reallocates and clears.

int
Metrics::_Allocate(
    int  cross_track_bins)
{
    //---------------//
    // reallocate... //
    //---------------//

    _crossTrackBins = cross_track_bins;

    //---------//
    // nearest //
    //---------//

    _nearSumSqrSpdErr = (double *)realloc(_nearSumSqrSpdErr,
        sizeof(double) * _crossTrackBins);
    if (_nearSumSqrSpdErr == NULL) return(0);

    _nearSumSqrSpdErrCount
        = (unsigned long*)realloc(_nearSumSqrSpdErrCount,
            sizeof(unsigned long) * _crossTrackBins);
    if (_nearSumSqrSpdErrCount == NULL) return(0);

    _nearSumSqrDirErr = (double *)realloc(_nearSumSqrDirErr,
        sizeof(double) * _crossTrackBins);
    if (_nearSumSqrDirErr == NULL) return(0);

    _nearSumSqrDirErrCount
        = (unsigned long*)realloc(_nearSumSqrDirErrCount,
            sizeof(unsigned long) * _crossTrackBins);
    if (_nearSumSqrDirErrCount == NULL) return(0);

    //----------//
    // selected //
    //----------//

    _selSumSqrSpdErr = (double *)realloc(_selSumSqrSpdErr,
        sizeof(double) * _crossTrackBins);
    if (_selSumSqrSpdErr == NULL) return(0);

    _selSumSqrSpdErrCount
        = (unsigned long*)realloc(_selSumSqrSpdErrCount,
            sizeof(unsigned long) * _crossTrackBins);
    if (_selSumSqrSpdErrCount == NULL) return(0);

    _selSumSqrDirErr = (double *)realloc(_selSumSqrDirErr,
        sizeof(double) * _crossTrackBins);
    if (_selSumSqrDirErr == NULL) return(0);

    _selSumSqrDirErrCount
        = (unsigned long*)realloc(_selSumSqrDirErrCount,
            sizeof(unsigned long) * _crossTrackBins);
    if (_selSumSqrDirErrCount == NULL) return(0);

    //--------------//
    // ...and clear //
    //--------------//

    Clear();

    return(1);
}

//----------------------//
// Metrics::_Deallocate //
//----------------------//

void
Metrics::_Deallocate()
{
    free(_nearSumSqrSpdErr);
    free(_nearSumSqrSpdErrCount);
    free(_nearSumSqrDirErr);
    free(_nearSumSqrDirErrCount);

    free(_selSumSqrSpdErr);
    free(_selSumSqrSpdErrCount);
    free(_selSumSqrDirErr);
    free(_selSumSqrDirErrCount);

    return;
}
