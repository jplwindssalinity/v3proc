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
#include "Meas.h"

#define POINTS_PER_SPOT_OUTLINE		18

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

int		LocateSlices(Spacecraft* spacecraft,
			Instrument* instrument, int slices_per_spot, MeasSpot* meas_spot);

int		LocateSpot(Spacecraft* spacecraft,
			Instrument* instrument, MeasSpot* meas_spot);

int		FindSlice(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, double look,
			double azimuth, float freq_1, float freq_2, float freq_tol,
			Outline* outline, Vector3* look_vector, EarthPosition* centroid);

int		DopplerAndDelay(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, Vector3 vector);

int		TargetInfo(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, Vector3 vector,
			TargetInfoPackage* tip);

int		FindPeakGainAtFreq(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument,
			float target_freq, float freq_tol, double* look, double* azim,
			float* gain);

int		FindFreq(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, float target_freq,
			float freq_tol, double* look, double* azim);

int		FreqGradient(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, double look,
			double look_offset, double azim, double azim_offset,
			double* df_dlook, double* df_dazim);

int		FindPeakGainUsingDeltas(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, double delta_look,
			double delta_azim, double offset, double angle_tol, double* look,
			double* azim, float* gain);

int		FindPeakGainForSlice(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, double look[2],
			double azim[2], float gain[2], float* peak_gain);

int		FindGainBetween(double f_look[2], double f_azim[2], float f_gain[2],
			float target_gain, double* tc_look, double* tc_azim);

int		FindSliceCorners(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, double look,
			double azim, float target_gain, double corner_look[2],
			double corner_azim[2]);

int		QuadFit(CoordinateSwitch* antenna_frame_to_gc, Spacecraft* spacecraft,
			Instrument* instrument, double look[3], double azim[3],
			double s[3], double c[3]);

int		PeakFit(double c[3], float* peak_gain);

int		PowerGainProduct(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Instrument* instrument, double look,
			double azim, float* gain);

int		RangeAndRoundTrip(CoordinateSwitch* antenna_frame_to_gc,
			Spacecraft* spacecraft, Vector3 vector, TargetInfoPackage* tip);

int		Get2WayElectricalBoresight(Beam* beam, double round_trip_time,
            double azimuth_rate, double* look, double* azimuth);
double	ReciprocalPowerGainProduct(double*,void*);

#endif
