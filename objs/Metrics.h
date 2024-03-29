//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef METRICS_H
#define METRICS_H

static const char rcs_id_metrics_h[] =
    "@(#) $Id$";

#include "Wind.h"
#include "WindSwath.h"
#include "LonLatWind.h"

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
#define DEFAULT_MAX_DIRECTION_ERROR      200.0*dtr

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

    int   Initialize(int cross_track_bins, float cross_track_resolution,
		     int speed_bins, float speed_resolution);
    int   SetWindSpeedRange(float low_speed, float high_speed);
    void  SetMaxDirectionError(float val) { _maxDirectionError = val; };
    void  Clear();

    //--------------//
    // input/output //
    //--------------//

    float  IndexToCtd(int cti);
    float  IndexToSpeed(int ispd);
    int    SpeedToIndex(float speed);
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
    int  Evaluate(WindSwath* swath, float resolution,
		  int speed_bins, float speed_resolution, LonLatWind* truth);
    int  Evaluate(WindSwath* swath, float resolution,
		  int speed_bins, float speed_resolution, float* tspd, float* tdir);

    //-----------//
    // operators //
    //-----------//

    void operator+=(const Metrics& m);

protected:

    int   _Allocate(int cross_track_bins, int speed_bins);
    void  _Deallocate();

    void  _SetResolution(float cross_track_resolution, float speed_resolution)
              { _crossTrackResolution = cross_track_resolution;
              _speedResolution = speed_resolution; };

    //-----------//
    // variables //
    //-----------//

    int             _crossTrackBins;
    float           _crossTrackResolution;

    int             _speedBins;
    float           _speedResolution;

    float           _lowWindSpeed;
    float           _highWindSpeed;
    float           _maxDirectionError;

    double*         _nearSumSqrSpdErr;
    unsigned long*  _nearSumSqrSpdErrCount;

    double*         _nearSumSqrDirErr;
    unsigned long*  _nearSumSqrDirErrCount;

    double*         _selSumSqrSpdErr;
    unsigned long*  _selSumSqrSpdErrCount;

    double*         _selSumSqrDirErr;
    unsigned long*  _selSumSqrDirErrCount;

    double*         _nearSumSqrSpdErrVsSpd;
    unsigned long*  _nearSumSqrSpdErrVsSpdCount;

    double*         _nearSumSqrDirErrVsSpd;
    unsigned long*  _nearSumSqrDirErrVsSpdCount;

    double*         _selSumSqrSpdErrVsSpd;
    unsigned long*  _selSumSqrSpdErrVsSpdCount;

    double*         _selSumSqrDirErrVsSpd;
    unsigned long*  _selSumSqrDirErrVsSpdCount;
};

#endif
