//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_instrumentgeom_c[] =
	"@(#) $Id$";

#include "CoordinateSwitch.h"
#include "Ephemeris.h"
#include "Attitude.h"
#include "Antenna.h"
#include "Matrix3.h"
#include "GenericGeom.h"
#include "Spacecraft.h"
#include "Instrument.h"
#include "LonLat.h"
#include "InstrumentGeom.h"
#include "Interpolate.h"
#include "Array.h"
#include "Misc.h"



//------------------//
// AntennaFrameToGC //
//------------------//

CoordinateSwitch
AntennaFrameToGC(
	OrbitState*		sc_orbit_state,
	Attitude*		sc_attitude,
	Antenna*		antenna)
{
	CoordinateSwitch total;

	// geocentric to s/c velocity
	Vector3 sc_xv, sc_yv, sc_zv;
	velocity_frame(sc_orbit_state->rsat, sc_orbit_state->vsat,
		&sc_xv, &sc_yv, &sc_zv);
	CoordinateSwitch gc_to_scv(sc_xv, sc_yv, sc_zv);
	total = gc_to_scv;

	// s/c velocity to s/c body
	CoordinateSwitch scv_to_sc_body(*sc_attitude);
	total.Append(&scv_to_sc_body);

	// s/c body to antenna pedestal
	CoordinateSwitch sc_body_to_ant_ped = antenna->GetScBodyToAntPed();
	total.Append(&sc_body_to_ant_ped);

	// antenna pedestal to antenna frame
	Attitude att;
	att.Set(0.0, 0.0, antenna->azimuthAngle, 1, 2, 3);
	CoordinateSwitch ant_ped_to_ant_frame(att);
	total.Append(&ant_ped_to_ant_frame);

	total = total.ReverseDirection();
	return(total);
}

//--------------//
// LocateSlices //
//--------------//

int
LocateSlices(
	Spacecraft*		spacecraft,
	Instrument*		instrument,
	int				slices_per_spot,
	MeasSpot*		meas_spot)
{
	//-----------//
	// predigest //
	//-----------//

	Antenna* antenna = &(instrument->antenna);
	Beam* beam = antenna->GetCurrentBeam();
	OrbitState* orbit_state = &(spacecraft->orbitState);
	Attitude* attitude = &(spacecraft->attitude);      
	Attitude zero_rpy; // Constructor set rpy to zero


	//------------------//
	// set up meas spot //
	//------------------//

	meas_spot->FreeContents();
	meas_spot->time = instrument->time;
	meas_spot->scOrbitState = *orbit_state;
	meas_spot->scAttitude = *attitude;

	//--------------------------------//
	// generate the coordinate switch //
	//--------------------------------//

	CoordinateSwitch antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
		attitude, antenna);

	CoordinateSwitch zero_rpy_antenna_frame_to_gc = 
	  AntennaFrameToGC(orbit_state, &zero_rpy, antenna);


	//------------------------//
	// determine slicing info //
	//------------------------//

	float total_freq = slices_per_spot * instrument->sliceBandwidth;
	float min_freq = -total_freq / 2.0;

	//-------------------------------------------------//
	// calculate Doppler shift and receiver gate delay //
	//-------------------------------------------------//

	Vector3 vector;
	double look, azimuth;

	if (! beam->GetElectricalBoresight(&look, &azimuth))
		return(0);

	vector.SphericalSet(1.0, look, azimuth);
	TargetInfoPackage tip;
	RangeAndRoundTrip(&zero_rpy_antenna_frame_to_gc, spacecraft, vector, &tip);

	if (! GetTwoWayPeakGain(beam, tip.roundTripTime,
		instrument->antenna.spinRate,&look, &azimuth))
	{
		return(0);
	}

	vector.SphericalSet(1.0, look, azimuth);		// boresight
	DopplerAndDelay(&zero_rpy_antenna_frame_to_gc, spacecraft, instrument, vector);

	//-------------------//
	// for each slice... //
	//-------------------//

	for (int slice_idx = 0; slice_idx < slices_per_spot; slice_idx++)
	{
		//-------------------------//
		// create a new measurment //
		//-------------------------//

		Meas* meas = new Meas();
		meas->pol = beam->polarization;
	
		//----------------------------------------//
		// determine the baseband frequency range //
		//----------------------------------------//

		float f1 = min_freq + slice_idx * instrument->sliceBandwidth;
		float f2 = f1 + instrument->sliceBandwidth;

		//----------------//
		// find the slice //
		//----------------//

		EarthPosition centroid;
		Vector3 look_vector;
		// guess at a reasonable slice frequency tolerance of .1%
		float ftol = fabs(f1 - f2) / 1000.0;
		if (! FindSlice(&antenna_frame_to_gc, spacecraft, instrument,
			look, azimuth, f1, f2, ftol, &(meas->outline), &look_vector,
			&centroid))
		{
			return(0);
		}

		//---------------------------//
		// generate measurement data //
		//---------------------------//

		// get local measurement azimuth
		CoordinateSwitch gc_to_surface =
			centroid.SurfaceCoordinateSystem();
		Vector3 rlook_surface = gc_to_surface.Forward(look_vector);
		double r, theta, phi;
		rlook_surface.SphericalGet(&r, &theta, &phi);
		meas->eastAzimuth = phi;

		// get incidence angle
		meas->incidenceAngle = centroid.IncidenceAngle(look_vector);
		meas->centroid = centroid;

		//-----------------------------//
		// add measurment to meas spot //
		//-----------------------------//

		meas_spot->Append(meas);
	}
	return(1);
}

//------------//
// LocateSpot //
//------------//

int
LocateSpot(
	Spacecraft*		spacecraft,
	Instrument*		instrument,
	MeasSpot*		meas_spot)
{
	//-----------//
	// predigest //
	//-----------//

	Antenna* antenna = &(instrument->antenna);
	Beam* beam = antenna->GetCurrentBeam();
	OrbitState* orbit_state = &(spacecraft->orbitState);
	Attitude* attitude = &(spacecraft->attitude);


	//------------------//
	// set up meas spot //
	//------------------//

	meas_spot->FreeContents();
	meas_spot->time = instrument->time;
	meas_spot->scOrbitState = *orbit_state;
	meas_spot->scAttitude = *attitude;

	//--------------------------------//
	// generate the coordinate switch //
	//--------------------------------//

	CoordinateSwitch antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
		attitude, antenna);

	//---------------------------------------------------//
	// calculate the look vector in the geocentric frame //
	//---------------------------------------------------//

	Vector3 rlook_antenna;

	double look, azimuth;
	if (! beam->GetElectricalBoresight(&look, &azimuth))
	{
		printf("Error determining electrical boresight\n");
		return(0);
	}

	rlook_antenna.SphericalSet(1.0, look, azimuth);
	TargetInfoPackage tip;
	RangeAndRoundTrip(&antenna_frame_to_gc, spacecraft, rlook_antenna, &tip);

	//
	// Using the round trip time from the one way boresight, compute the
	// direction of the 2 way boresight.  Ideally, this should be iterated,
	// but the round trip time difference should be very small.
	//

	if (! GetTwoWayPeakGain(beam,tip.roundTripTime,
		instrument->antenna.spinRate,&look, &azimuth))
	{
		printf("Error determining 2 way electrical boresight\n");
		return(0);
	}

	// Update with new boresight
	rlook_antenna.SphericalSet(1.0, look, azimuth);
	RangeAndRoundTrip(&antenna_frame_to_gc, spacecraft, rlook_antenna, &tip);

	Vector3 rlook_gc = antenna_frame_to_gc.Forward(rlook_antenna);

	//----------------------------//
	// calculate the 3 dB outline //
	//----------------------------//

	Meas* meas = new Meas();	// need the outline to append to

	// Start with the center position.
	EarthPosition *rspot = new EarthPosition;
	*rspot= tip.rTarget;
	if (! meas->outline.Append(rspot))
	{
		printf("Error appending to spot outline\n");
		return(0);
	}

	// get the max gain value.
	float gp_max;
	beam->GetPowerGainProduct(look,azimuth,tip.roundTripTime,
			instrument->antenna.spinRate,&gp_max);

	// Align beam frame z-axis with the electrical boresight.
	Attitude beam_frame;
	beam_frame.Set(0.0,look,azimuth,3,2,1);
	CoordinateSwitch ant_to_beam(beam_frame);
	CoordinateSwitch beam_to_ant = ant_to_beam.ReverseDirection();

	//
	// In the beam frame, for a set of azimuth angles, search for the
	// theta angle that has the required gain product.
	// Convert the results to the geocentric frame and find
	// the earth intercepts.
	//
	
	for (int i=0; i < POINTS_PER_SPOT_OUTLINE + 1; i++)
	{
		double phi = (two_pi * i) / POINTS_PER_SPOT_OUTLINE;

		// Setup for bisection search for the half power product point.

		double theta_max = 0.0;
		double theta_min = 5.0*dtr;
		double theta;
		int NN = (int)(log((theta_min-theta_max)/(0.01*dtr))/log(2)) + 1;
		Vector3 look_mid;
		Vector3 look_mid_ant;
		Vector3 look_mid_gc;

		for (int j = 1; j <= NN; j++)
		{	// Bisection search
			theta = (theta_max + theta_min)/2.0;
			look_mid.SphericalSet(1.0,theta,phi);
			look_mid_ant = beam_to_ant.Forward(look_mid);
			double r,look,azimuth;
			look_mid_ant.SphericalGet(&r,&look,&azimuth);
			float gp;
			beam->GetPowerGainProduct(look,azimuth,tip.roundTripTime,
				instrument->antenna.spinRate,&gp);
			if (gp > 0.5*gp_max)
			{
				theta_max = theta;
			}
			else
			{
				theta_min = theta;
			}
		}

		look_mid_gc = antenna_frame_to_gc.Forward(look_mid_ant);
		EarthPosition *rspot = new EarthPosition;
		*rspot = earth_intercept(orbit_state->rsat,look_mid_gc);
		if (! meas->outline.Append(rspot))
		{
			printf("Error appending to spot outline\n");
			return(0);
		}
	}

	//---------------------------//
	// generate measurement data //
	//---------------------------//

	meas->pol = beam->polarization;

	// get local measurement azimuth
	CoordinateSwitch gc_to_surface = tip.rTarget.SurfaceCoordinateSystem();
	Vector3 rlook_surface = gc_to_surface.Forward(rlook_gc);
	double r, theta, phi;
	rlook_surface.SphericalGet(&r, &theta, &phi);
	meas->eastAzimuth = phi;

	// get incidence angle
	meas->incidenceAngle = tip.rTarget.IncidenceAngle(rlook_gc);
	meas->centroid = tip.rTarget;

	//-----------------------------//
	// add measurment to meas spot //
	//-----------------------------//

	meas_spot->Append(meas);

	return(1);
}

//-----------//
// FindSlice //
//-----------//

int
FindSlice(
	CoordinateSwitch*	antenna_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	double				look,
	double				azim,
	float				freq_1,
	float				freq_2,
	float				freq_tol,
	Outline*			outline,
	Vector3*			look_vector,
	EarthPosition*		centroid)
{
	//--------------------------------//
	// find peak gain for frequency 1 //
	//--------------------------------//

	double f_look[2], f_azim[2];
	float f_gain[2];

	f_look[0] = look;
	f_azim[0] = azim;
	if (! FindPeakGainAtFreq(antenna_frame_to_gc, spacecraft, instrument,
		freq_1, freq_tol, &(f_look[0]), &(f_azim[0]), &(f_gain[0])))
	{
		fprintf(stderr,
			"FindSlice: error finding peak gain for frequency %g\n", freq_1);
		return(0);
	}

	//--------------------------------//
	// find peak gain for frequency 2 //
	//--------------------------------//

	f_look[1] = look;
	f_azim[1] = azim;
	if (! FindPeakGainAtFreq(antenna_frame_to_gc, spacecraft, instrument,
		freq_2, freq_tol, &(f_look[1]), &(f_azim[1]), &(f_gain[1])))
	{
		fprintf(stderr,
			"FindSlice: error finding peak gain for frequency %g\n", freq_2);
		return(0);
	}

	//--------------------------//
	// find peak gain for slice //
	//--------------------------//

	float peak_gain;
	if (! FindPeakGainForSlice(antenna_frame_to_gc, spacecraft, instrument,
		f_look, f_azim, f_gain, &peak_gain))
	{
		fprintf(stderr, "FindSlice: error finding peak gain for slice\n");
		return(0);
	}

	//------------------------------//
	// find target gain for corners //
	//------------------------------//

	float target_gain = peak_gain / pow(10.0, 0.3);

	int corner_idx = 0;
	double c_look[4], c_azim[4];

	//------------------------------//
	// find corners for frequency 1 //
	//------------------------------//

	double two_look[2];
	double two_azim[2];
	if (f_gain[0] > target_gain)
	{
		if (! FindSliceCorners(antenna_frame_to_gc, spacecraft, instrument,
			f_look[0], f_azim[0], target_gain, two_look, two_azim))
		{
			fprintf(stderr,
				"FindSlice: error finding slice corners for frequency 1\n");
			return(0);
		}

		c_look[corner_idx] = two_look[0];
		c_azim[corner_idx] = two_azim[0];
		corner_idx++;

		c_look[corner_idx] = two_look[1];
		c_azim[corner_idx] = two_azim[1];
		corner_idx++;
	}
	else
	{
		// you've got a triangle
		double tc_look, tc_azim;
		if (! FindGainBetween(f_look, f_azim, f_gain, target_gain,
			&tc_look, &tc_azim))
		{
			fprintf(stderr, "FindSlice: error finding triangle corner\n");
			return(0);
		}
		c_look[corner_idx] = tc_look;
		c_azim[corner_idx] = tc_azim;
		corner_idx++;
	}

	//------------------------------//
	// find corners for frequency 2 //
	//------------------------------//

	if (f_gain[1] > target_gain)
	{
		if (! FindSliceCorners(antenna_frame_to_gc, spacecraft, instrument,
			f_look[1], f_azim[1], target_gain, two_look, two_azim))
		{
			fprintf(stderr,
				"FindSlice: error finding slice corners for frequency 2\n");
			return(0);
		}

		c_look[corner_idx] = two_look[1];
		c_azim[corner_idx] = two_azim[1];
		corner_idx++;

		c_look[corner_idx] = two_look[0];
		c_azim[corner_idx] = two_azim[0];
		corner_idx++;
	}
	else
	{
		// you've got a triangle
		double tc_look, tc_azim;
		if (! FindGainBetween(f_look, f_azim, f_gain, target_gain,
			&tc_look, &tc_azim))
		{
			fprintf(stderr, "FindSlice: error finding triangle corner\n");
			return(0);
		}
		c_look[corner_idx] = tc_look;
		c_azim[corner_idx] = tc_azim;
		corner_idx++;
	}

	//----------------------//
	// generate the outline //
	//----------------------//

	EarthPosition sum;
	float gain;
	sum.SetPosition(0.0, 0.0, 0.0);
	for (int i = 0; i < corner_idx; i++)
	{
		Vector3 rlook_antenna;
		rlook_antenna.SphericalSet(1.0, c_look[i], c_azim[i]);
		Vector3 rlook_gc = antenna_frame_to_gc->Forward(rlook_antenna);
		EarthPosition* spot_on_earth = new EarthPosition();
		*spot_on_earth =
			earth_intercept(spacecraft->orbitState.rsat, rlook_gc);

		if (! outline->Append(spot_on_earth))
			return(0);

		if (! PowerGainProduct(antenna_frame_to_gc, spacecraft, instrument,
			c_look[i], c_azim[i], &gain))
		{
			return(0);
		}
		sum += (*spot_on_earth * gain);
	}

	//------------------------//
	// determine the centroid //
	//------------------------//

	EarthPosition earth_center;
	earth_center.SetPosition(0.0, 0.0, 0.0);
	*centroid = earth_intercept(earth_center, sum);

	//---------------------------//
	// determine the look vector //
	//---------------------------//

	*look_vector = *centroid - spacecraft->orbitState.rsat;

	return(1);
}

//-----------------//
// DopplerAndDelay //
//-----------------//
// Estimate the ideal doppler frequency and receiver gate delay.
// Later, this should be replaced with the Doppler and range tracking stuff.

#define DOPPLER_ACCURACY	1.0		// 1 Hz

int
DopplerAndDelay(
	CoordinateSwitch*	antenna_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	Vector3				vector)
{
	//-----------------------------------------------------//
	// calculate receiver gate delay to put echo in center //
	//-----------------------------------------------------//

	int current_beam_idx = instrument->antenna.currentBeamIdx;
	double pulse_width = instrument->antenna.beam[current_beam_idx].pulseWidth;

	OrbitState* sc_orbit_state = &(spacecraft->orbitState);
	Vector3 ulook_gc = antenna_frame_to_gc->Forward(vector);
	EarthPosition r_target = earth_intercept(sc_orbit_state->rsat, ulook_gc);
	double slant_range = (sc_orbit_state->rsat - r_target).Magnitude();
	double round_trip_time = 2.0 * slant_range / speed_light_kps;
	instrument->receiverGateDelay = pulse_width / 2.0 + round_trip_time +
		instrument->systemDelay;

	//--------------------------------------------------//
	// calculate baseband frequency w/o Doppler command //
	//--------------------------------------------------//

	double chirp_start = instrument->chirpStartM * pulse_width +
		instrument->chirpStartB;
	double transmit_center = -chirp_start / instrument->chirpRate;
	double echo_center = transmit_center + round_trip_time +
		instrument->systemDelay;
	double range_freq = instrument->chirpRate *
		(instrument->receiverGateDelay - echo_center);

	Vector3 vspot(-w_earth * r_target.Get(1), w_earth * r_target.Get(0), 0);
	Vector3 vrel = sc_orbit_state->vsat - vspot;

	instrument->commandedDoppler = 0.0;
	double xmit_freq, lambda, doppler_freq, new_commanded_doppler, dif;
	do
	{
		xmit_freq = instrument->baseTransmitFreq +
			instrument->commandedDoppler;
		lambda = speed_light_kps / xmit_freq;
		doppler_freq = 2.0 * (vrel % ulook_gc) / lambda;
		new_commanded_doppler = range_freq - doppler_freq;
		dif = fabs(instrument->commandedDoppler - new_commanded_doppler);
		instrument->commandedDoppler = new_commanded_doppler;
	} while (dif > DOPPLER_ACCURACY);

	return(1);
}

//------------//
// TargetInfo //
//------------//
// Compute some useful numbers for the target on the earth's surface
// intercepted by a particular direciton in the antenna frame.
// The vector is a directional vector specified in the antenna frame.
 
int
TargetInfo(
	CoordinateSwitch*	antenna_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	Vector3				vector,
	TargetInfoPackage*	tip)
{
	// dereference
	OrbitState* sc_orbit_state = &(spacecraft->orbitState);

	// Compute earth intercept point and range
	Vector3 ulook_gc = antenna_frame_to_gc->Forward(vector);
	tip->rTarget = earth_intercept(sc_orbit_state->rsat, ulook_gc);
	EarthPosition* rspot = &(tip->rTarget);
	tip->slantRange = (sc_orbit_state->rsat - *rspot).Magnitude();
 
	// Compute doppler shift for the earth intercept point.
	double actual_xmit_frequency = instrument->baseTransmitFreq +
		instrument->commandedDoppler;
	Vector3 vspot(-w_earth * rspot->Get(1), w_earth * rspot->Get(0), 0);
	Vector3 vrel = sc_orbit_state->vsat - vspot;
	double lambda = speed_light_kps / actual_xmit_frequency;
	tip->dopplerFreq = 2.0 * (vrel % ulook_gc) / lambda;
 
	// Compute baseband frequency shift due to range
	int current_beam_idx = instrument->antenna.currentBeamIdx;
	double pulse_width = instrument->antenna.beam[current_beam_idx].pulseWidth;
	double chirp_start = instrument->chirpStartM * pulse_width +
		instrument->chirpStartB;
	double transmit_center = -chirp_start / instrument->chirpRate;
	tip->roundTripTime = 2.0 * tip->slantRange / speed_light_kps;
	double echo_center = transmit_center + tip->roundTripTime +
		instrument->systemDelay;
	tip->rangeFreq = instrument->chirpRate *
		(instrument->receiverGateDelay - echo_center);
	tip->basebandFreq = tip->rangeFreq - (tip->dopplerFreq +
		instrument->commandedDoppler);
 
return(1);
}

//--------------------//
// FindPeakGainAtFreq //
//--------------------//
// Finds the look, azimuth, and gain value for the peak gain at
// a given baseband frequency

#define LOOK_OFFSET			0.005
#define AZIMUTH_OFFSET		0.005
#define ANGLE_OFFSET		0.01		// start delta for golden section
#define ANGLE_TOL			0.00001		// within this of peak gain

int
FindPeakGainAtFreq(
	CoordinateSwitch*	antenna_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	float				target_freq,
	float				freq_tol,
	double*				look,
	double*				azim,
	float*				gain)
{
	float dif;
	Vector3 vector;
	TargetInfoPackage tip;
	do
	{
		//--------------------//
		// find the frequency //
		//--------------------//

		if (! FindFreq(antenna_frame_to_gc, spacecraft, instrument,
			target_freq, freq_tol, look, azim))
		{
			return(0);
		}

		//------------------------------------//
		// determine the iso-frequency deltas //
		//------------------------------------//

		double df_dlook, df_dazim;
		if (! FreqGradient(antenna_frame_to_gc, spacecraft, instrument, *look,
			LOOK_OFFSET, *azim, AZIMUTH_OFFSET, &df_dlook, &df_dazim))
		{
			return(0);
		}
		// rotate gradient 90 degrees for iso-frequency deltas
		double delta_look = df_dazim;
		double delta_azim = -df_dlook;

		//----------------------//
		// locate the peak gain //
		//----------------------//

		if (! FindPeakGainUsingDeltas(antenna_frame_to_gc, spacecraft,
			instrument, delta_look, delta_azim, ANGLE_OFFSET, ANGLE_TOL,
			look, azim, gain))
		{
			return(0);
		}

		//--------------------------------------//
		// check if still on iso-frequency line //
		//--------------------------------------//

		vector.SphericalSet(1.0, *look, *azim);
		if (! TargetInfo(antenna_frame_to_gc, spacecraft, instrument, vector,
			&tip))
		{
			return(0);
		}
		dif = fabs(tip.basebandFreq - target_freq);
	} while (dif > freq_tol);

	return(1);
}

//----------//
// FindFreq //
//----------//
// Finds the look and azimuth of a point with the given frequency

int
FindFreq(
	CoordinateSwitch*	antenna_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	float				target_freq,
	float				freq_tol,
	double*				look,
	double*				azim)
{
	Vector3 vector;
	TargetInfoPackage tip;
	do
	{
		//-------------------------------------//
		// get the starting baseband frequency //
		//-------------------------------------//

		vector.SphericalSet(1.0, *look, *azim);
		TargetInfoPackage tip;
		if (! TargetInfo(antenna_frame_to_gc, spacecraft, instrument, vector,
			&tip))
		{
			return(0);
		}

		//---------------//
		// check if done //
		//---------------//

		if (fabs(target_freq - tip.basebandFreq) < freq_tol)
			break;

		//------------------------//
		// calculate the gradient //
		//------------------------//

		double df_dlook, df_dazim;
		if (! FreqGradient(antenna_frame_to_gc, spacecraft, instrument, *look,
			LOOK_OFFSET, *azim, AZIMUTH_OFFSET, &df_dlook, &df_dazim))
		{
			return(0);
		}

		//---------------------------------------//
		// calculate the target look and azimuth //
		//---------------------------------------//

		double delta_look, delta_azim;
		if (df_dazim == 0.0)
			delta_azim = 0.0;
		else
		{
			delta_azim = (target_freq - tip.basebandFreq) /
				(df_dlook * df_dlook / df_dazim + df_dazim);
		}
 
		if (df_dlook == 0.0)
			delta_look = 0.0;
		else
		{
			delta_look = (target_freq - tip.basebandFreq) /
				(df_dazim * df_dazim / df_dlook + df_dlook);
		}
 
		//------------------------------//
		// jump to the look and azimuth //
		//------------------------------//
 
		*look += delta_look;
		*azim += delta_azim;

	} while (1);

	return(1);
}

//--------------//
// FreqGradient //
//--------------//
// Calculate the baseband frequency gradient

int
FreqGradient(
	CoordinateSwitch*	antenna_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	double				look,
	double				look_offset,
	double				azim,
	double				azim_offset,
	double*				df_dlook,
	double*				df_dazim)
{
	TargetInfoPackage tip_1, tip_2;

	//------------------------//
	// calculate look vectors //
	//------------------------//

	double look_1 = look - look_offset;
	double look_2 = look + look_offset;
	Vector3 vector_look_1, vector_look_2;
	vector_look_1.SphericalSet(1.0, look_1, azim);
	vector_look_2.SphericalSet(1.0, look_2, azim);

	//------------------------------//
	// calculate baseband frequency //
	//------------------------------//

	if (! TargetInfo(antenna_frame_to_gc, spacecraft, instrument,
		vector_look_1, &tip_1) ||
		! TargetInfo(antenna_frame_to_gc, spacecraft, instrument,
		vector_look_2, &tip_2))
	{
		return(0);
	}

	//-------------------------//
	// calculate look gradient //
	//-------------------------//

	*df_dlook = (tip_2.basebandFreq - tip_1.basebandFreq) / (look_2 - look_1);

	//---------------------------//
	// calculate azimuth vectors //
	//---------------------------//

	double azim_1 = azim - azim_offset;
	double azim_2 = azim + azim_offset;
	Vector3 vector_azim_1, vector_azim_2;
	vector_azim_1.SphericalSet(1.0, look, azim_1);
	vector_azim_2.SphericalSet(1.0, look, azim_2);

	//------------------------------//
	// calculate baseband frequency //
	//------------------------------//

	if (! TargetInfo(antenna_frame_to_gc, spacecraft, instrument,
		vector_azim_1, &tip_1) ||
		! TargetInfo(antenna_frame_to_gc, spacecraft, instrument,
		vector_azim_2, &tip_2))
	{
		return(0);
	}

	//----------------------------//
	// calculate azimuth gradient //
	//----------------------------//

	*df_dazim = (tip_2.basebandFreq - tip_1.basebandFreq) / (azim_2 - azim_1);

	return(1);
}

//-------------------------//
// FindPeakGainUsingDeltas //
//-------------------------//

#define R	0.61803399
#define C	(1.0-R)

int
FindPeakGainUsingDeltas(
	CoordinateSwitch*	antenna_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	double				delta_look,
	double				delta_azim,
	double				angle_offset,
	double				angle_tol,
	double*				look,
	double*				azim,
	float*				gain)
{
	//------------------------//
	// scale to radian deltas //
	//------------------------//

	double scale = sqrt(delta_look * delta_look + delta_azim * delta_azim);
	delta_look /= scale;
	delta_azim /= scale;

	//--------------------------------//
	// golden section search for peak //
	//--------------------------------//

	double ax = -angle_offset;
	double cx = angle_offset;
	double bx = ax + (cx - ax) * R;

	//--------------------//
	// widen if necessary //
	//--------------------//

	float again, bgain, cgain;

	PowerGainProduct(antenna_frame_to_gc, spacecraft, instrument,
		*look + ax * delta_look, *azim + ax * delta_azim, &again);
	PowerGainProduct(antenna_frame_to_gc, spacecraft, instrument,
		*look + bx * delta_look, *azim + bx * delta_azim, &bgain);
	PowerGainProduct(antenna_frame_to_gc, spacecraft, instrument,
		*look + cx * delta_look, *azim + cx * delta_azim, &cgain);

	do
	{
		if (again > bgain)
		{
			ax -= angle_offset;
			PowerGainProduct(antenna_frame_to_gc, spacecraft, instrument,
				*look + ax * delta_look, *azim + ax * delta_azim, &again);
			continue;
		}

		if (cgain > bgain)
		{
			cx += angle_offset;
			PowerGainProduct(antenna_frame_to_gc, spacecraft, instrument,
				*look + cx * delta_look, *azim + cx * delta_azim, &cgain);
			continue;
		}

		break;
	} while (1);

	//---------------//
	// do the search //
	//---------------//

	double x0, x1, x2, x3;
	x0 = ax;
	x3 = cx;
	if (cx - bx > bx - ax)
	{
		x1 = bx;
		x2 = bx + C * (cx - bx);
	}
	else
	{
		x2 = bx;
		x1 = bx - C * (bx - ax);
	}

	float f1, f2;
	PowerGainProduct(antenna_frame_to_gc, spacecraft, instrument,
		*look + x1 * delta_look, *azim + x1 * delta_azim, &f1);
	PowerGainProduct(antenna_frame_to_gc, spacecraft, instrument,
		*look + x2 * delta_look, *azim + x2 * delta_azim, &f2);

	while (x3 - x0 > angle_tol)
	{
		if (f2 > f1)
		{
			x0 = x1;
			x1 = x2;
			x2 = x2 + C * (x3 - x2);
			f1 = f2;
			PowerGainProduct(antenna_frame_to_gc, spacecraft, instrument,
				*look + x2 * delta_look, *azim + x2 * delta_azim, &f2);
		}
		else
		{
			x3 = x2;
			x2 = x1;
			x1 = x1 - C * (x1 - x0);
			f2 = f1;
			PowerGainProduct(antenna_frame_to_gc, spacecraft, instrument,
				*look + x1 * delta_look, *azim + x1 * delta_azim, &f1);
		}
	}

	if (f1 > f2)
	{
		*look += x1 * delta_look;
		*azim += x1 * delta_azim;
		*gain = f1;
	}
	else
	{
		*look += x2 * delta_look;
		*azim += x2 * delta_azim;
		*gain = f2;
	}

	return(1);
}

//----------------------//
// FindPeakGainForSlice //
//----------------------//
// Finds the peak gain in a slice

int
FindPeakGainForSlice(
	CoordinateSwitch*	antenna_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	double				look[2],
	double				azim[2],
	float				gain[2],
	float*				peak_gain)
{
	double mid_look = (look[0] + look[1]) / 2.0;
	double mid_azim = (azim[0] + azim[1]) / 2.0;

	float mid_gain;
	PowerGainProduct(antenna_frame_to_gc, spacecraft, instrument, mid_look,
		mid_azim, &mid_gain);

	if (gain[0] >= mid_gain && gain[0] >= gain[1])
	{
		*peak_gain = gain[0];
		return(1);
	}
	else if (gain[1] >= mid_gain && gain[1] >= gain[0])
	{
		*peak_gain = gain[1];
		return(1);
	}
	else if (mid_gain >= gain[0] && mid_gain >= gain[1])
	{
		double look_array[3], azim_array[3];
		look_array[0] = look[0];
		look_array[1] = mid_look;
		look_array[2] = look[1];
		azim_array[0] = azim[0];
		azim_array[1] = mid_azim;
		azim_array[2] = azim[1];

		double s[3], c[3];
		if (! QuadFit(antenna_frame_to_gc, spacecraft, instrument, look_array,
			azim_array, s, c))
		{
			return(0);
		}
		PeakFit(c, peak_gain);
		return(1);
	}
	return(0);
}

//-----------------//
// FindGainBetween //
//-----------------//
// Finds the target gain along the line connecting the provided look and
// azimuth points

int
FindGainBetween(
	double				f_look[2],
	double				f_azim[2],
	float				f_gain[2],
	float				target_gain,
	double*				tc_look,
	double*				tc_azim)
{
	double delta_look = f_look[1] - f_look[0];
	double delta_azim = f_azim[1] - f_azim[0];
	float delta_gain = f_gain[1] - f_gain[0];

	float gain_frac = (target_gain - f_gain[0]) / delta_gain;
	*tc_look = f_look[0] + gain_frac * delta_look;
	*tc_azim = f_azim[0] + gain_frac * delta_azim;
	return(1);
}

//------------------//
// FindSliceCorners //
//------------------//

#define PEAK_ANGLE_OFFSET		0.00875		// about 0.50 degree
#define LOCAL_ANGLE_OFFSET		0.000875	// about 0.050 degree

int
FindSliceCorners(
	CoordinateSwitch*	antenna_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	double				look,
	double				azim,
	float				target_gain,
	double				corner_look[2],
	double				corner_azim[2])
{
	//------------------------------------//
	// determine the iso-frequency deltas //
	//------------------------------------//
 
	double df_dlook, df_dazim;
	if (! FreqGradient(antenna_frame_to_gc, spacecraft, instrument, look,
		LOOK_OFFSET, azim, AZIMUTH_OFFSET, &df_dlook, &df_dazim))
	{
		return(0);
	}
	// rotate gradient 90 degrees for iso-frequency deltas
	double delta_look = df_dazim;
	double delta_azim = -df_dlook;

	//------------------------//
	// scale to radian deltas //
	//------------------------//
 
	double scale = sqrt(delta_look * delta_look + delta_azim * delta_azim);
	delta_look /= scale;
	delta_azim /= scale;

	//-----------------//
	// fit a quadratic //
	//-----------------//

	double peak_look_step = PEAK_ANGLE_OFFSET * delta_look;
	double peak_azim_step = PEAK_ANGLE_OFFSET * delta_azim;

	double look_array[3];
	look_array[0] = look - peak_look_step;
	look_array[1] = look;
	look_array[2] = look + peak_look_step;

	double azim_array[3];
	azim_array[0] = azim - peak_azim_step;
	azim_array[1] = azim;
	azim_array[2] = azim + peak_azim_step;

	double s[3], c[3];
	if (! QuadFit(antenna_frame_to_gc, spacecraft, instrument, look_array,
		azim_array, s, c))
	{
		fprintf(stderr, "FindSliceCorners: error fitting main quadratic\n");
		return(0);
	}

	//-------------------------//
	// check out the quadratic //
	//-------------------------//

	double qr = c[1] * c[1] - 4.0 * c[2] * (c[0] - target_gain);
	if (qr < 0.0)
	{
		corner_look[0] = look;
		corner_azim[0] = azim;
		corner_look[1] = look;
		corner_azim[1] = azim;
		return(1);
	}

	//-------------------//
	// for each point... //
	//-------------------//

	double q = sqrt(qr);
	double twoa = 2.0 * c[2];

	double local_look_step = LOCAL_ANGLE_OFFSET * delta_look;
	double local_azim_step = LOCAL_ANGLE_OFFSET * delta_azim;

	static const double coef[2] = { -1.0, 1.0 };
	for (int i = 0; i < 2; i++)
	{
		//-----------------------------------//
		// estimate using the peak quadratic //
		//-----------------------------------//

		double target_s = (-c[1] + coef[i] * q) / twoa;
		double local_look = look + target_s * delta_look;
		double local_azim = azim + target_s * delta_azim;

		//-------------------//
		// initialize points //
		//-------------------//

		double local_look_array[3];
		local_look_array[0] = local_look - local_look_step;
		local_look_array[1] = local_look;
		local_look_array[2] = local_look + local_look_step;

		double local_azim_array[3];
		local_azim_array[0] = local_azim - local_azim_step;
		local_azim_array[1] = local_azim;
		local_azim_array[2] = local_azim + local_azim_step;

		float local_gain_array[3];
		for (int j = 0; j < 3; j++)
		{
			if (! PowerGainProduct(antenna_frame_to_gc, spacecraft,
				instrument, local_look_array[j], local_azim_array[j],
				&(local_gain_array[j])))
			{
				return(0);
			}
		}

		//-------------//
		// step search //
		//-------------//

		while (target_gain > local_gain_array[0] &&
				local_gain_array[0] > local_gain_array[1] &&
				local_gain_array[1] > local_gain_array[2])
		{
			for (int j = 2; j > 0; j--)
			{
				local_look_array[j] = local_look_array[j-1];
				local_azim_array[j] = local_azim_array[j-1];
				local_gain_array[j] = local_gain_array[j-1];
			}
			local_look_array[0] -= local_look_step;
			local_azim_array[0] -= local_azim_step;
			if (! PowerGainProduct(antenna_frame_to_gc, spacecraft,
				instrument, local_look_array[0], local_azim_array[0],
				&(local_gain_array[0])))
			{
				return(0);
			}
		}

		while (target_gain > local_gain_array[2] &&
				local_gain_array[2] > local_gain_array[1] &&
				local_gain_array[1] > local_gain_array[0])
		{
			for (int j = 0; j < 2; j++)
			{
				local_look_array[j] = local_look_array[j+1];
				local_azim_array[j] = local_azim_array[j+1];
				local_gain_array[j] = local_gain_array[j+1];
			}
			local_look_array[2] += local_look_step;
			local_azim_array[2] += local_azim_step;
			if (! PowerGainProduct(antenna_frame_to_gc, spacecraft,
				instrument, local_look_array[2], local_azim_array[2],
				&(local_gain_array[2])))
			{
				return(0);
			}
		}

		//-----------------//
		// fit a quadratic //
		//-----------------//

		double local_s[3], local_c[3];
		if (! QuadFit(antenna_frame_to_gc, spacecraft, instrument,
				local_look_array, local_azim_array, local_s, local_c))
		{
			fprintf(stderr,
				"FindSliceCorners: error fitting local quadratic\n");
			return(0);
		}

		//-------------------------//
		// check out the quadratic //
		//-------------------------//

		if (local_c[2] == 0.0)
		{
			// linear fit
			target_s = (target_gain - local_c[0]) / local_c[1];
		}
		else
		{
			// quadratic fit

			double local_qr = local_c[1] * local_c[1] - 4.0 * local_c[2] *
				(local_c[0] - target_gain);
			if (local_qr < 0.0)
			{
				corner_look[i] = local_look;
				corner_azim[i] = local_azim;
				continue;
			}

			double local_q = sqrt(local_qr);
			double local_twoa = 2.0 * local_c[2];
			double target_s_1 = (-local_c[1] + local_q) / local_twoa;
			double target_s_2 = (-local_c[1] - local_q) / local_twoa;
			double abs_1 = fabs(target_s_1);
			double abs_2 = fabs(target_s_2);
			if (abs_1 < abs_2)
				target_s = target_s_1;
			else
				target_s = target_s_2;
		}

		corner_look[i] = local_look + target_s * delta_look;
		corner_azim[i] = local_azim + target_s * delta_azim;
	}

	return(1);
}

//---------//
// QuadFit //
//---------//
// Generates coefficients for a quadratic fit of a beam pattern

int
QuadFit(
	CoordinateSwitch*	antenna_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	double				look[3],
	double				azim[3],
	double				s[3],
	double				c[3])
{
	//-------------------------------//
	// calculate projected distances //
	//-------------------------------//
 
	s[1] = 0.0;
	double dl, da;
	dl = (look[1] - look[0]);
	da = (azim[1] - azim[0]);
	s[0] = -sqrt(dl*dl + da*da);
	dl = (look[2] - look[1]);
	da = (azim[2] - azim[1]);
	s[2] = sqrt(dl*dl + da*da);

	//-------------------//
	// for each point... //
	//-------------------//
 
	TargetInfoPackage tip;
	Vector3 vector;
	float gain[3];
	double dgain[3];
	for (int i = 0; i < 3; i++)
	{
		if (! PowerGainProduct(antenna_frame_to_gc, spacecraft, instrument,
			look[i], azim[i], &(gain[i])))
		{
			return(0);
		}
		dgain[i] = gain[i];		// transfer to double for quad fit
	}
 
	//-----------------//
	// fit a quadratic //
	//-----------------//
 
	if (! polcoe(s, dgain, 2, c))
		return(0);

	return(1);
}

//---------//
// PeakFit //
//---------//
// Finds the peak gain for a given gain slice fit

int
PeakFit(
	double		c[3],
	float*		peak_gain)
{
	if (c[2] == 0.0)
		return(0);

//	double dlook_ds = (look_array[2] - look_array[0]) / (s[2] - s[0]);
//	double dazim_ds = (azim_array[2] - azim_array[0]) / (s[2] - s[0]);

	double s_peak = -c[1] / (2.0 * c[2]);
//	*peak_look = look_array[1] + s_peak * dlook_ds;
//	*peak_azim = azim_array[1] + s_peak * dazim_ds;
	*peak_gain = ((c[2] * s_peak) + c[1]) * s_peak + c[0];

	return(1);
}

//------------------//
// PowerGainProduct //
//------------------//
 
int
PowerGainProduct(
	CoordinateSwitch*	antenna_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	double				look,
	double				azim,
	float*				gain)
{
	Vector3 vector;
	vector.SphericalSet(1.0, look, azim);
	TargetInfoPackage tip;
	RangeAndRoundTrip(antenna_frame_to_gc, spacecraft, vector, &tip);
	int idx = instrument->antenna.currentBeamIdx;
	if (! instrument->antenna.beam[idx].GetPowerGainProduct(look, azim,
		tip.roundTripTime, instrument->antenna.spinRate, gain))
	{
		return(0);
	}
	return(1);
}

//-------------------//
// RangeAndRoundTrip //
//-------------------//
// Calculates the range and round trip time
 
int
RangeAndRoundTrip(
	CoordinateSwitch*	antenna_frame_to_gc,
	Spacecraft*			spacecraft,
	Vector3				vector,
	TargetInfoPackage*	tip)
{
	// dereference
	OrbitState* sc_orbit_state = &(spacecraft->orbitState);
 
	// Compute earth intercept point and range.
	Vector3 ulook_gc = antenna_frame_to_gc->Forward(vector);
	tip->rTarget = earth_intercept(sc_orbit_state->rsat, ulook_gc);
	tip->slantRange = (sc_orbit_state->rsat - tip->rTarget).Magnitude();
	tip->roundTripTime = 2.0 * tip->slantRange / speed_light_kps;
 
	return(1);
}

//-------------------//
// GetTwoWayPeakGain //
//-------------------//

//
// This function locates the maximum 2-way gain of a beam in the antenna frame,
// and returns the apparent direction. (look,azimuth (rads)).
// NOTE: This is NOT the direction of the transmitted pulse which determines
// the scattering geometry.
//

#define TWO_WAY_PEAK_GAIN_ANGLE_TOLERANCE  1e-5

int
GetTwoWayPeakGain(
	Beam*	beam,
	double	round_trip_time,
	double	azimuth_rate,
	double*	look,
	double*	azimuth)
{
	// Start with the one-way electrical boresight.
	beam->GetElectricalBoresight(look,azimuth);

	int ndim = 2;
	double** p = (double**)make_array(sizeof(double),2,3,4);
	if (p == NULL)
	{
		printf("Error allocating memory in GetTwoWayPeakGain\n");
		return(0);
	}

	p[0][0] = *look;
	p[0][1] = *azimuth;
	p[1][0] = *look + 1.0*dtr;
	p[1][1] = *azimuth;
	p[2][0] = *look;
	p[2][1] = *azimuth + 1.0*dtr;

	for (int i=0; i < ndim+1; i++)
	{
		p[i][2] = round_trip_time;
		p[i][3] = azimuth_rate;
	}

	double ftol = TWO_WAY_PEAK_GAIN_ANGLE_TOLERANCE;
	downhill_simplex((double**)p,ndim,ndim+2,ftol,
		ReciprocalPowerGainProduct,beam);

	*look = p[0][0];
	*azimuth = p[0][1];

	free_array(p,2,3,4);

	return(1);
}

//----------------------------//
// ReciprocalPowerGainProduct //
//----------------------------//

//
// ReciprocalPowerGainProduct is the function to be minimized by
// GetTwoWayPeakGain.
// It computes the reciprocal of the PowerGainProduct for the inputs given
// in the input vector.  The elements of the input vector are:
//
// x[0] = look angle (rad)
// x[1] = azimuth angle (rad)
// x[2] = round trip time (sec)
// x[3] = spin rate (rad/sec)
// beam = pointer to a beam object containing the pattern to use.
//

double ReciprocalPowerGainProduct(double* x, void* beam)

{

	double gp;
	((Beam*)beam)->GetPowerGainProduct(x[0],x[1],x[2],x[3],&gp);
	if (gp == 0.0)
	{
		printf("Error: ReciprocalPowerGainProduct received a 0 gain value\n");
		exit(-1);
	}
	return(1.0 / gp);

}
