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
:   _crossTrackBins(0), _ctd(NULL), _selectedSumSqrSpdErr(NULL),
    _selectedSumSqrSpdErrCount(NULL)
{
    return;
}

Metrics::~Metrics()
{
    _Deallocate();
    return;
}

//---------------//
// Metrics::Read //
//---------------//

int
Metrics::Read(
    const char*  filename)
{
    FILE* ifp = fopen(filename, "w");
    if (ifp == NULL)
        return(0);

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

    ofp = OpenPlotFile(basename, "selrms", "Selected RMS Speed Error",
        NULL, "Cross Track Distance (km)", "RMS Speed Error (m/s)");
    if (ofp == NULL)
        return(0);
    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        if (_selectedSumSqrSpdErrCount[cti] == 0)
            continue;
        double ctd = _ctd[cti];
        double value = sqrt(_selectedSumSqrSpdErr[cti]
            / _selectedSumSqrSpdErrCount[cti]);
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
    const char*  subtitle,
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

//---------------------//
// Metrics::Initialize //
//---------------------//

int
Metrics::Initialize(
    WindSwath*  swath,
    float       resolution)
{
    int cross_track_bins = swath->GetCrossTrackBins();
    _alongTrackBins = swath->GetAlongTrackBins();

    if (cross_track_bins == _crossTrackBins)
        return(1);    // already been set, no problem here

    if (_crossTrackBins == 0)
    {
        // need to allocate
        _Allocate(cross_track_bins);
        if (! swath->CtdArray(resolution, _ctd))
            return(0);
        return(1);
    }

    // you're trying to change the number of cross track bins
    // shame on you!
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
    if (! Initialize(swath, resolution))
        return(0);

    for (int cti = 0; cti < _crossTrackBins; cti++)
    {
        for (int ati = 0; ati < _alongTrackBins; ati++)
        {
            WVC* wvc = swath->GetWVC(cti, ati);
            if (wvc == NULL || wvc->selected == NULL)
                continue;

            WindVector true_wv;
            if (! truth->InterpolatedWindVector(wvc->lonLat, &true_wv))
                continue;

            double dif = wvc->selected->spd - true_wv.spd;
            double dif_sqr = dif * dif;

            _selectedSumSqrSpdErr[cti] += dif_sqr;
            _selectedSumSqrSpdErrCount[cti]++;
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
    _crossTrackBins = cross_track_bins;

    _ctd = new float[_crossTrackBins];
    _selectedSumSqrSpdErr = new double[_crossTrackBins];
    _selectedSumSqrSpdErrCount = new double[_crossTrackBins];

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
