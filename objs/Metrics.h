//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef METRICS_H
#define METRICS_H

static const char rcs_id_metrics_h[] =
    "@(#) $Id$";

#include "Wind.h"

//======================================================================
// CLASSES
//    Metrics
//======================================================================

//======================================================================
// CLASS
//    Metrics
//
// DESCRIPTION
//    The Metrics object holds wind performance metrics. It has
//    methods for writing metrics in a plottable format and for
//    combining with other metric objects.
//======================================================================

#define DEFAULT_METRICS_LOW_WIND_SPEED   3.0
#define DEFAULT_METRICS_HIGH_WIND_SPEED  30.0

class Metrics
{
public:

    //--------------//
    // construction //
    //--------------//

    Metrics();
    ~Metrics();

    //---------------//
    // configuration //
    //---------------//

    int   Initialize(int cross_track_bins, float cross_track_resolution);
    int   SetWindSpeedRange(float low_speed, float high_speed);
    void  Clear();

    //--------------//
    // input/output //
    //--------------//

    float  IndexToCtd(int cti);
    int    Read(const char* filename);
    int    Write(const char* filename);
    int    WritePlotData(const char* basename);
    FILE*  OpenPlotFile(const char* output_base, const char* extension,
               const char* title, const char* xaxis_label,
               const char* yaxis_label);

    //------------//
    // evaluation //
    //------------//

    int  IsCompatible(const Metrics& m);
    int  Evaluate(WindSwath* swath, float resolution, WindField* truth);

    //-----------//
    // operators //
    //-----------//

    void operator+=(const Metrics& m);

protected:

    int   _Allocate(int cross_track_bins);
    void  _Deallocate();

    void  _SetResolution(float cross_track_resolution)
              { _crossTrackResolution = cross_track_resolution; };

    //-----------//
    // variables //
    //-----------//

    int             _crossTrackBins;
    float           _crossTrackResolution;

    float           _lowWindSpeed;
    float           _highWindSpeed;

    double*         _selectedSumSqrSpdErr;
    unsigned long*  _selectedSumSqrSpdErrCount;

    double*         _selectedSumSqrDirErr;
    unsigned long*  _selectedSumSqrDirErrCount;
};

#endif
