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

    int  Initialize(int cross_track_bins, float cross_track_resolution);

    //--------------//
    // input/output //
    //--------------//

    int    Read(const char* filename);
    int    Write(const char* filename);
    int    WritePlotData(const char* basename);
    FILE*  OpenPlotFile(const char* output_base, const char* extension,
               const char* title, const char* subtitle,
               const char* xaxis_label, const char* yaxis_label);

    //------------//
    // evaluation //
    //------------//

    int  Evaluate(WindSwath* swath, float resolution, WindField* truth);

protected:

    void  _Allocate(int cross_track_bins);
    void  _Deallocate();

    //-----------//
    // variables //
    //-----------//

    int             _crossTrackBins;
    float           _crossTrackResolution;
    float*          _ctd;

    double*         _selectedSumSqrSpdErr;
    unsigned long*  _selectedSumSqrSpdErrCount;

    double*         _selectedSumSqrDirErr;
    unsigned long*  _selectedSumSqrDirErrCount;
};

#endif
