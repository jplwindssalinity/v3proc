//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_metrics_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include "Metrics.h"
#include "Wind.h"

//=========//
// Metrics //
//=========//

Metrics::Metrics()
:   _crossTrackBins(0), _crossTrackResolution(0.0), _ctd(NULL),
    _lowWindSpeed(DEFAULT_METRICS_LOW_WIND_SPEED),
    _highWindSpeed(DEFAULT_METRICS_HIGH_WIND_SPEED),
    _selectedSumSqrSpdErr(NULL), _selectedSumSqrSpdErrCount(NULL),
    _selectedSumSqrDirErr(NULL), _selectedSumSqrDirErrCount(NULL)
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
    if (cross_track_bins == _crossTrackBins &&
        cross_track_resolution == _crossTrackResolution)
    {
        // already initialized -- everything is OK
        return(1);
    }

    if (_crossTrackBins == 0 && _crossTrackResolution == 0.0)
    {
        // need to allocate
        _Allocate(cross_track_bins);
        for (int cti = 0; cti < cross_track_bins; cti++)
        {
            _ctd[cti] = ((float)cti - ((float)cross_track_bins - 1.0)
                / 2.0) * cross_track_resolution;
        }
        _crossTrackResolution = cross_track_resolution;
        return(1);
    }

    // you're trying to change the number of cross track bins
    // or resolution.
    // shame on you!
    return(0);
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

//---------------//
// Metrics::Read //
//---------------//

int
Metrics::Read(
    const char*  filename)
{
    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL)
        return(0);

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

    double* selected_sum_sqr_spd_err = new double[_crossTrackBins];
    unsigned long* selected_sum_sqr_spd_err_count =
        new unsigned long[_crossTrackBins];
    double* selected_sum_sqr_dir_err = new double[_crossTrackBins];
    unsigned long* selected_sum_sqr_dir_err_count =
        new unsigned long[_crossTrackBins];
    
    if (fread((void *)selected_sum_sqr_spd_err, sizeof(double),
            _crossTrackBins, ifp) != (size_t)_crossTrackBins ||
        fread((void *)selected_sum_sqr_spd_err_count, sizeof(unsigned long),
            _crossTrackBins, ifp) != (size_t)_crossTrackBins ||
        fread((void *)selected_sum_sqr_dir_err, sizeof(double),
            _crossTrackBins, ifp) != (size_t)_crossTrackBins ||
        fread((void *)selected_sum_sqr_dir_err_count, sizeof(unsigned long),
            _crossTrackBins, ifp) != (size_t)_crossTrackBins)
    {
        fprintf(stderr, "Metrics::Read: error reading data\n");
        fclose(ifp);
        return(0);
    }

    //---------//
    // combine //
    //---------//

    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        _selectedSumSqrSpdErr[cti] += selected_sum_sqr_spd_err[cti];
        _selectedSumSqrSpdErrCount[cti] += selected_sum_sqr_spd_err_count[cti];
        _selectedSumSqrDirErr[cti] += selected_sum_sqr_dir_err[cti];
        _selectedSumSqrDirErrCount[cti] += selected_sum_sqr_dir_err_count[cti];
    }

    //------//
    // free //
    //------//

    delete [] selected_sum_sqr_spd_err;
    delete [] selected_sum_sqr_spd_err_count;
    delete [] selected_sum_sqr_dir_err;
    delete [] selected_sum_sqr_dir_err_count;

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

    if (fwrite((void *)&_crossTrackBins, sizeof(int), 1, ofp) != 1 ||
        fwrite((void *)&_crossTrackResolution, sizeof(float), 1, ofp) != 1 ||
        fwrite((void *)_selectedSumSqrSpdErr, sizeof(double),
            _crossTrackBins, ofp) != (size_t)_crossTrackBins ||
        fwrite((void *)_selectedSumSqrSpdErrCount, sizeof(unsigned long),
            _crossTrackBins, ofp) != (size_t)_crossTrackBins ||
        fwrite((void *)_selectedSumSqrDirErr, sizeof(double),
            _crossTrackBins, ofp) != (size_t)_crossTrackBins ||
        fwrite((void *)_selectedSumSqrDirErrCount, sizeof(unsigned long),
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

    //--------------------------//
    // selected rms speed error //
    //--------------------------//

    ofp = OpenPlotFile(basename, "sel_rms_spd_err", "Selected RMS Speed Error",
        "Cross Track Distance (km)", "RMS Speed Error (m/s)");
    if (ofp == NULL)
        return(0);
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        if (_selectedSumSqrSpdErrCount[cti] == 0)
            continue;
        double ctd = _ctd[cti];
        double value = sqrt(_selectedSumSqrSpdErr[cti]
            / (double)_selectedSumSqrSpdErrCount[cti]);
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
        if (_selectedSumSqrDirErrCount[cti] == 0)
            continue;
        double ctd = _ctd[cti];
        double value = rtd * sqrt(_selectedSumSqrDirErr[cti]
            / (double)_selectedSumSqrDirErrCount[cti]);
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

//-------------------//
// Metrics::Evaluate //
//-------------------//

int
Metrics::Evaluate(
    WindSwath*  swath,
    float       resolution,
    WindField*  truth)
{
    int cross_track_bins = swath->GetCrossTrackBins();
    if (! Initialize(cross_track_bins, resolution))
        return(0);

    int along_track_bins = swath->GetAlongTrackBins();

    for (int cti = 0; cti < cross_track_bins; cti++)
    {
        for (int ati = 0; ati < along_track_bins; ati++)
        {
            WVC* wvc = swath->GetWVC(cti, ati);
            if (wvc == NULL || wvc->selected == NULL)
                continue;

            WindVector true_wv;
            if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
                continue;

            //-------------------//
            // check speed range //
            //-------------------//

            if (true_wv.spd < _lowWindSpeed || true_wv.spd >= _highWindSpeed)
                continue;

            //--------------------------//
            // selected RMS speed error //
            //--------------------------//

            double spd_dif = wvc->selected->spd - true_wv.spd;
            double spd_dif_sqr = spd_dif * spd_dif;

            _selectedSumSqrSpdErr[cti] += spd_dif_sqr;
            _selectedSumSqrSpdErrCount[cti]++;

            //------------------------------//
            // selected RMS direction error //
            //------------------------------//

            double dir_dif = ANGDIF(wvc->selected->dir, true_wv.dir);
            double dir_dif_sqr = dir_dif * dir_dif;

            _selectedSumSqrDirErr[cti] += dir_dif_sqr;
            _selectedSumSqrDirErrCount[cti]++;
        }
    }
    return(1);
}

//--------------------//
// Metrics::_Allocate //
//--------------------//

void
Metrics::_Allocate(
    int  cross_track_bins)
{
    //-------------//
    // allocate... //
    //-------------//

    _crossTrackBins = cross_track_bins;

    _ctd = new float[_crossTrackBins];
    _selectedSumSqrSpdErr = new double[_crossTrackBins];
    _selectedSumSqrSpdErrCount = new unsigned long[_crossTrackBins];
    _selectedSumSqrDirErr = new double[_crossTrackBins];
    _selectedSumSqrDirErrCount = new unsigned long[_crossTrackBins];

    //-------------------//
    // ...and initialize //
    //-------------------//

    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        _ctd[cti] = 0.0;
        _selectedSumSqrSpdErr[cti] = 0.0;
        _selectedSumSqrSpdErrCount[cti] = 0;
        _selectedSumSqrDirErr[cti] = 0.0;
        _selectedSumSqrDirErrCount[cti] = 0;
    }

    return;
}

//----------------------//
// Metrics::_Deallocate //
//----------------------//

void
Metrics::_Deallocate()
{
    if (_selectedSumSqrSpdErr != NULL)
        delete [] _selectedSumSqrSpdErr;

    if (_selectedSumSqrSpdErrCount != NULL)
        delete [] _selectedSumSqrSpdErrCount;

    return;
}
