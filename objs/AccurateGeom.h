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
//		High level instrument geometry functions, which perform
// the precise calculations necessary for calculating Kpr, kfactor, etc.
//======================================================================



int		IntegrateSlice(Spacecraft* spacecraft, Qscat* qscat, Meas* meas,
            int num_look_steps_per_slice, float azimuth_integration_range,
            float azimuth_step_size, int range_gate_clipping, float *X);

float   GetPulseFractionReceived(Qscat* qscat, float range);

int		FindBoxCorners(CoordinateSwitch* antenna_frame_to_gc,
            Spacecraft* spacecraft, Qscat* qscat, float look1, float look2,
            float azi1, float azi2, Outline* box);

int		FindLookAtFreq(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Qscat* qscat, float target_freq,
            float freq_tol, float* look, float azimuth);

int         SpectralResponse(Spacecraft* spacecraft, Qscat* qscat, 
		float freq, float azim, float look, float* response);

int IntegrateFrequencyInterval( Spacecraft* spacecraft, Qscat* qscat,
				float f1, float centroid_look,
				float centroid_azimuth, float bw,
				int num_look_steps_per_slice,
				float azimuth_integration_range,
				float azimuth_step_size, int range_gate_clipping,
				float* X);

int GetPeakSpectralResponse(CoordinateSwitch* antenna_frame_to_gc,
			     Spacecraft* spacecraft, Qscat* qscat, 
			     double* look, double* azim);

struct NegSpecParam{
  Spacecraft* spacecraft;
  Qscat* qscat;
};
double NegativeSpectralResponse(double* x, void* ptr);
#endif





