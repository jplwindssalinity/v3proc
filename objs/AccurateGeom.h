//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef ACCURATEGEOM_H
#define ACCURATEGEOM_H

static const char rcs_id_accurategeom_h[] =
	"@(#) $Id$";

#include "EarthPosition.h"
#include "CoordinateSwitch.h"
#include "Ephemeris.h"
#include "Spacecraft.h"
#include "Instrument.h"
#include "InstrumentGeom.h"
#include "LonLat.h"
#include "Matrix3.h"

//======================================================================
// DESCRIPTION
//		High level instrument geometry functions, which perform
// the precise calculations necessary for calculating Kpr, kfactor, etc.
//======================================================================


int		IntegrateSlices(Spacecraft* spacecraft, Instrument* instrument,
			MeasSpot* meas_spot, int num_look_steps_per_slice,
			float azimuth_integration_range, float azimuth_step_size);

int		FindBoxCorners(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, float look1,
			float look2, float azi1, float azi2, Outline* box);

int		FindLookAtFreq(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument,
			float target_freq, float freq_tol, float* look, float azimuth);

#endif
