//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef ACCURATEGEOM_H
#define ACCURATEGEOM_H

static const char rcs_id_accurategeom_h[] =
    "@(#) $Id$";

#include "EarthPosition.h"
#include "CoordinateSwitch.h"
#include "Ephemeris.h"
#include "Spacecraft.h"
#include "InstrumentGeom.h"
#include "LonLat.h"
#include "Matrix3.h"

//======================================================================
// DESCRIPTION
// High level instrument geometry functions, which perform
// the precise calculations necessary for calculating Kpr, kfactor, etc.
//======================================================================

int    IntegrateSlice(Spacecraft* spacecraft, Qscat* qscat, Meas* meas,
           int num_look_steps_per_slice, double azimuth_integration_range,
           double azimuth_step_size, int range_gate_clipping, float *X);

double  GetPulseFractionReceived(Qscat* qscat, double range);

int  FindBoxCorners(CoordinateSwitch* antenna_frame_to_gc,
         Spacecraft* spacecraft, Qscat* qscat, double look1, double look2,
         double azi1, double azi2, Outline* box);

int  FindLookAtFreq(CoordinateSwitch* antenna_frame_to_gc,
         Spacecraft* spacecraft, Qscat* qscat, double target_freq,
         double freq_tol, double* look, double azimuth);

int  SpectralResponse(Spacecraft* spacecraft, Qscat* qscat, double freq,
         double azim, double look, double* response);

int  IntegrateFrequencyInterval(Spacecraft* spacecraft, Qscat* qscat,
         double f1, double centroid_look, double centroid_azimuth, double bw,
         int num_look_steps_per_slice, double azimuth_integration_range,
         double azimuth_step_size, int range_gate_clipping, double ftol,
         double* X);

int  GetPeakSpectralResponse(CoordinateSwitch* antenna_frame_to_gc,
         Spacecraft* spacecraft, Qscat* qscat, double* look, double* azim);

struct NegSpecParam
{
    Spacecraft*  spacecraft;
    Qscat*       qscat;
};

double NegativeSpectralResponse(double* x, void* ptr);

#endif
