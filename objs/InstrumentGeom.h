//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef INSTRUMENTGEOM_H
#define INSTRUMENTGEOM_H

static const char rcs_id_instrumentgeom_h[] =
	"@(#) $Id$";

#include "Ephemeris.h"
#include "CoordinateSwitch.h"
#include "Attitude.h"
#include "Antenna.h"
#include "Beam.h"
#include "LonLat.h"
#include "Matrix3.h"
#include "Spacecraft.h"
#include "Instrument.h"

//======================================================================
// STRUCTURE
//		Used to package target info
//======================================================================

struct TargetInfoPackage {
	EarthPosition	rTarget;
	float			slantRange;		// km
	float			roundTripTime;	// ms
	float			dopplerFreq;	// Hz
	float			rangeFreq;
	float			basebandFreq;
};

//======================================================================
// DESCRIPTION
//		Higher level geometry functions.  Typically instrument related.
//======================================================================

//CoordinateSwitch	BeamFrameToGC(OrbitState* orbit_state, Attitude* attitude,
//						Antenna* antenna);

CoordinateSwitch	AntennaFrameToGC(OrbitState* orbit_state,
						Attitude* attitude, Antenna* antenna);

int		FindSlice(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, float look,
			float azimuth, float freq_1, float freq_2, float freq_tol,
			Outline* outline, Vector3* look_vector, EarthPosition* centroid);

int		JumpToFreq(CoordinateSwitch* beam_frame_to_gc, Spacecraft* spacecraft,
			Instrument* instrument, float* az, float* el,
			float gradient_angle, float target_freq, float freq_tol);

int		FreqGradient(CoordinateSwitch* beam_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, float az, float el,
				float grad_angle, float* df_daz, float* df_del);

int		DopplerAndDelay(Vector3 vector, CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument);

int		TargetInfo(Vector3 vector, CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument,
			TargetInfoPackage* tip);

int		IsoFreqAngle(CoordinateSwitch* beam_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, float* az,
			float* el, float grad_angle, float* angle);

int		SetPoints(float az_0, float el_0, float distance, float angle,
			float az[3], float el[3]);

int		GainSlice(Instrument* instrument, float az[3], float el[3],
			double s[3], double c[3]);

int		DetailedGainSlice(Instrument* instrument, float az[3], float el[3],
			double s[3], double c[3]);

#endif
