//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef INSTRUMENTGEOM_H
#define INSTRUMENTGEOM_H

static const char rcs_id_instrumentgeom_h[] =
	"@(#) $Id$";

#include "EarthPosition.h"
#include "CoordinateSwitch.h"
#include "Ephemeris.h"
#include "Spacecraft.h"
#include "LonLat.h"
#include "Matrix3.h"
#include "Matrix3.h"
#include "Meas.h"
#include "Qscat.h"

#define POINTS_PER_SPOT_OUTLINE		18
#define DEFAULT_CONTOUR_LEVEL		0.5

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

CoordinateSwitch  AntennaFrameToGC(OrbitState* orbit_state,
                      Attitude* attitude, Antenna* antenna);

int  LocateSlices(Spacecraft* spacecraft, Qscat* qscat, MeasSpot* meas_spot);

int  LocateSliceCentroids(Spacecraft* spacecraft, Qscat* qscat,
         MeasSpot* meas_spot, float* Esn = NULL, float gain_threshold = 0.0,
         int max_slices = 0);

int  LocateSpot(Spacecraft* spacecraft, Qscat* qscat, MeasSpot* meas_spot,
         float Esn = 0.0, float contour_level = DEFAULT_CONTOUR_LEVEL);

int  FindSlice(CoordinateSwitch* antenna_frame_to_gc, Spacecraft* spacecraft,
         Qscat* qscat, double look, double azimuth, float freq_1, float freq_2,
         float freq_tol, Outline* outline, Vector3* look_vector,
         EarthPosition* centroid);

double	IdealRtt(Spacecraft* spacecraft, Qscat* qscat);

int  RttToIdealRxDelay(Qscat* qscat, double rtt);

int  IdealCommandedDoppler(Spacecraft* spacecraft, Qscat* qscat);

int  IdealCommandedDopplerForRange(Spacecraft* spacecraft, float offset);

int  TargetInfo(CoordinateSwitch* antenna_frame_to_gc, Spacecraft* spacecraft,
         Qscat* qscat, Vector3 vector, TargetInfoPackage* tip);

int  FindPeakGainAtFreq(CoordinateSwitch* antenna_frame_to_gc,
         Spacecraft* spacecraft, Qscat* qscat, float target_freq,
         float freq_tol, double* look, double* azim, float* gain);

int  FindFreq(CoordinateSwitch* antenna_frame_to_gc, Spacecraft* spacecraft,
         Qscat* qscat, float target_freq, float freq_tol, double* look,
         double* azim);

int  FreqGradient(CoordinateSwitch* antenna_frame_to_gc,
         Spacecraft* spacecraft, Qscat* qscat, double look,
         double look_offset, double azim, double azim_offset,
         double* df_dlook, double* df_dazim);

int  FindPeakGainUsingDeltas(CoordinateSwitch* antenna_frame_to_gc,
         Spacecraft* spacecraft, Qscat* qscat, double delta_look,
         double delta_azim, double offset, double angle_tol, double* look,
         double* azim, float* gain);

int  FindPeakGainForSlice(CoordinateSwitch* antenna_frame_to_gc,
         Spacecraft* spacecraft, Qscat* qscat, double look[2], double azim[2],
         float gain[2], float* peak_gain);

int  FindGainBetween(double f_look[2], double f_azim[2], float f_gain[2],
         float target_gain, double* tc_look, double* tc_azim);

int  FindSliceCorners(CoordinateSwitch* antenna_frame_to_gc,
         Spacecraft* spacecraft, Qscat* qscat, double look, double azim,
         float target_gain, double corner_look[2], double corner_azim[2]);

int  QuadFit(CoordinateSwitch* antenna_frame_to_gc, Spacecraft* spacecraft,
         Qscat* qscat, double look[3], double azim[3], double s[3],
         double c[3]);

int  PeakFit(double c[3], float* peak_gain);

int  PowerGainProduct(CoordinateSwitch* antenna_frame_to_gc,
         Spacecraft* spacecraft, Qscat* qscat, double look, double azim,
         float* gain);

int  RangeAndRoundTrip(CoordinateSwitch* antenna_frame_to_gc,
         Spacecraft* spacecraft, Vector3 vector, TargetInfoPackage* tip);

int  GetTwoWayPeakGain(Beam* beam, double round_trip_time,
         double azimuth_rate, double* look, double* azimuth,
         CoordinateSwitch* antenna_frame_to_gc=NULL,
         Spacecraft* spacecraft=NULL);

int  GetTwoWayPeakGain2(CoordinateSwitch* antenna_frame_to_gc,
         Spacecraft* spacecraft, Beam* beam, double azimuth_rate,
         double* look, double* azimuth);

double	NegativePowerGainProduct(double*, void*);

#endif
