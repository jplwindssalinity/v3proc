//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef INSTRUMENTGEOM_H
#define INSTRUMENTGEOM_H

static const char rcs_id_instrumentgeom_h[] =
	"@(#) $Id$";

#include "EarthPosition.h"
#include "CoordinateSwitch.h"
#include "Ephemeris.h"
#include "Spacecraft.h"
#include "Instrument.h"
#include "LonLat.h"
#include "Matrix3.h"
#include "Matrix3.h"

//======================================================================
// STRUCTURE
//		Used to package target info
//======================================================================

struct TargetInfoPackage {
	EarthPosition	rTarget;
	float			slantRange;		// km
	float			roundTripTime;	// ms
	float			dopplerFreq;	// Hz
	float			rangeFreq;		// Hz
	float			basebandFreq;	// Hz
};

//======================================================================
// DESCRIPTION
//		Higher level geometry functions.  Typically instrument related.
//======================================================================

CoordinateSwitch	AntennaFrameToGC(OrbitState* orbit_state,
						Attitude* attitude, Antenna* antenna);

int		FindSlice(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, float look,
			float azimuth, float freq_1, float freq_2, float freq_tol,
			Outline* outline, Vector3* look_vector, EarthPosition* centroid);

int		DopplerAndDelay(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, Vector3 vector);

int		TargetInfo(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, Vector3 vector,
			TargetInfoPackage* tip);

int		FindPeakGainAtFreq(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument,
			float target_freq, float freq_tol, float* look, float* azim,
			float* gain);

int		FindFreq(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, float target_freq,
			float freq_tol, float* look, float* azim);

int		FreqGradient(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, float look,
			float look_offset, float azim, float azim_offset, float* df_dlook,
			float* df_dazim);

int		FindPeakGainUsingDeltas(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, float delta_look,
			float delta_azim, float offset, float angle_tol, float* look,
			float* azim, float* gain);

int		FindPeakGainForSlice(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, float look_1,
			float azim_1, float gain_1, float look_2, float azim_2,
			float gain_2, float* peak_gain);

int		FindSliceCorners(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, float look,
			float azim, float target_gain, float corner_look[2],
			float corner_azim[2]);

int		QuadFit(CoordinateSwitch* antenna_frame_to_gc, Spacecraft* spacecraft,
			Instrument* instrument, float look[3], float azim[3], double s[3],
			double c[3]);

int		PeakFit(double c[3], float* peak_gain);

int		PowerGainProduct(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, float look,
			float azim, float* gain);

int		RangeAndRoundTrip(Vector3 vector, CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, TargetInfoPackage* tip);


#endif
