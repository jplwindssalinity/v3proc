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
#include "GenericGeom.h"
#include "Constants.h"
#include "LonLat.h"
/*
#include "Beam.h"
#include "Matrix3.h"
*/

//---------------//
// BeamFrameToGC //
//---------------//

CoordinateSwitch
BeamFrameToGC(
	OrbitState*		sc_orbit_state,
	Attitude*		sc_attitude,
	Antenna*		antenna,
	Beam*			beam)
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

	// antenna frame to beam frame
	CoordinateSwitch ant_frame_to_beam_frame(beam->GetAntFrameToBeamFrame());
	total.Append(&ant_frame_to_beam_frame);

	total = total.ReverseDirection();
	return(total);
}

//----------------//
// PositionParams //
//----------------//

//
// Compute some useful numbers for the point on the earth's surface
// intercepted by a particular direction in a beam frame.
//
// Inputs:
//	ulook_beam = unit vector in beam frame pointing in direction of interest.
//	sc_orbit_state = pointer to object specifying the current s/c state.
//	beam_frame_to_gc = pointer to currently valid coordinate switch object.
//	chirp_rate = transmitted frequency sweep rate (Hz/sec).
//	center_frequency = transmit center frequency (Hz).
//	pulse_width = length of transmit pulse (sec).
//	grid_delay = time between falling edge of the PRI clock and grid on (sec).
//	receive_gate_delay = time from falling PRI to middle of receive gate (sec).
//	system_delay = 2-way delay through SES electronics (sec).
//
// Outputs (by address):
//	rspot = earth intercept point.
//	fdop = doppler frequency shift (Hz).
//	frange = baseband frequency shift due to range (Hz).
//

int
PositionParams(
	Vector3 ulook_beam,
	OrbitState*	sc_orbit_state,
	CoordinateSwitch* beam_frame_to_gc,
	double chirp_rate,
	double center_frequency,
	double pulse_width,
	double grid_delay,
	double receive_gate_delay,
	double system_delay,
	EarthPosition* rspot,
	double* fdop,
	double* frange) 	

{

	// Compute earth intercept point and range.
    Vector3 ulook_gc = beam_frame_to_gc->Forward(ulook_beam);
    *rspot = earth_intercept(sc_orbit_state->rsat,ulook_gc);
	double range = (sc_orbit_state->rsat - *rspot).Magnitude();

	// Compute doppler shift for the earth intercept point.
	Vector3 vspot(-w_earth*rspot->get(1),w_earth*rspot->get(0),0);
	Vector3 vrel = sc_orbit_state->vsat - vspot;
	double lambda = speed_light / center_frequency;
	*fdop = -2.0*(vrel % ulook_gc) / lambda;

	// Compute baseband frequency shift due to range.
	double transmit_center = pulse_width/2.0 + grid_delay;
	double flight_time = 2*range/speed_light;
	double echo_center = transmit_center + flight_time + system_delay;
	*frange = chirp_rate*(receive_gate_delay - echo_center);

}

/*
//------------------//
// FindSliceOutline //
//------------------//

int
FindSliceOutline(
	float		freq_1,
	float		freq_2,
	float		freq_tol,
	Outline*	outline)
{
	//------------------------------------------------//
	// find a point for each of the given frequencies //
	//------------------------------------------------//

	float theta_1, phi_1, theta_2, phi_2;
	if (! FindFreq(&theta_1, &phi_1, freq_1, freq_tol) ||
		! FindFreq(&theta_2, &phi_2, freq_2, freq_tol))
	{
		return(0);
	}

	//---------------------------------------------//
	// find the peak gain point for each frequency //
	//---------------------------------------------//

	if (! PeakGainForFreq(&theta_1, &phi_1, freq_1, freq_tol) ||
		! PeakGainForFreq(&theta_2, &phi_2, freq_2, freq_tol))
	{
		return(0);
	}

	//------------------------------------------------//
	// find the peak gain in the cell //
	// it is assumed to be between the two peak gains //
	//------------------------------------------------//

	float peak_theta, peak_phi, peak_gain;
	if (! PeakBetween(theta_1, phi_1, theta_2, phi_2, &peak_theta, &peak_phi,
		&peak_gain))
	{
		return(0);
	}

	//------------------------------------------//
	// find the -3 dB points for each frequency //
	//------------------------------------------//

	float theta[4], phi[4];
	float target_gain = peak_gain - 3.0;
	if (! TargetGainForFreq(theta_1, phi_1, freq_1, freq_tol, target_gain,
			-1, &(theta[0]), &(phi[0])) ||
		! TargetGainForFreq(theta_1, phi_1, freq_1, freq_tol, target_gain,
			1, &(theta[1]), &(phi[1])) ||
		! TargetGainForFreq(theta_2, phi_2, freq_2, freq_tol, target_gain,
			1, &(theta[2]), &(phi[2])) ||
		! TargetGainForFreq(theta_2, phi_2, freq_2, freq_tol, target_gain,
			-1, &(theta[3]), &(phi[3])))
	{
		return(0);
	}

	//--------------------//
	// create the outline //
	//--------------------//

	for (int i = 0; i < 4; i++)
	{
		if (! PositionParams(theta[0], phi[0]))
		{
			return(0);
		}
		LonLat new_lonlat = new LonLat();
		if (new_lonlat == NULL)
			return(0);
		new_lonlat.longitude = xxx;
		new_lonlat.latitude = xxx;
		if (! outline->Append(new_lonlat))
		{
			delete new_lonlat;
			return(0);
		}
	}

	return(1);
}

/*
//--------------------//
// FreqGradients //
//--------------------//

int
FreqGradients(
	float	theta,
	float	phi,
	float	grad_angle,
	float*	df_dtheta,
	float*	df_dphi)
{
	float half_grad_angle = grad_angle / 2.0;

	float t1 = theta - half_grad_angle;
	float t2 = theta + half_grad_angle;
	float ft1 = BasebandFreq(t1, phi);
	float ft2 = BasebandFreq(t2, phi);
	*df_dtheta = (ft2 - ft1) / (t2 - t1);

	float p1 = phi - half_grad_angle;
	float p2 = phi + half_grad_angle;
	float fp1 = BasebandFreq(theta, p1);
	float fp2 = BasebandFreq(theta, p2);
	*df_dphi = (fp2 - fp1) / (p2 - p1);

	return(1);
}



int
FindPatternPeak(
	float*	theta,
	float*	phi,
	float	target_freq,
	float	freq_tol)
{
	
}

int
TargetGainForFreq()
{
	//--------------------------------------------//
	// search along frequency for the target gain //
	//--------------------------------------------//
}

//----------------------//
// PeakGainForFreq //
//----------------------//

int
PeakGainForFreq()
{
	//-----------------------------------------------//
	// find the nearest point at the given frequency //
	//-----------------------------------------------//

	if (! FindFreq(freq_1) ||
		! FindFreq(freq_2))
	{
		return(0);
	}

	//-------------------------------------------------//
	// calculate the theta and phi frequency gradients //
	//-------------------------------------------------//

	if ! FreqGradients(*theta, *phi, grad_angle, &df_dtheta, &df_dphi))
	{
		return(0);
	}

	//------------------------------------//
	// find delta ratio for zero gradient //
	//------------------------------------//
	// may need to invert factor

	float d_phi_factor = -df_dtheta / df_dphi;

	//-----------------------------------------------//
	// search for gain peak in sold angle increments //
	//-----------------------------------------------//

	dphi
}

//------------------//
// GradientFreqJump //
//------------------//
// Given the following:
//		A BeamFrameToGC coordinate switch
//		A theta/phi pair
//		A gradient angle
//		A target freq
// Replace the theta/phi pair with a theta/phi pair that
// is closer to the target freq

int
GradientFreqJump(
	float*				theta,
	float*				phi,
	float				grad_angle,
	float				target_freq)
{
	//-------------------------------------------------//
	// calculate the theta and phi frequency gradients //
	//-------------------------------------------------//

	if ! FreqGradients(*theta, *phi, grad_angle, &df_dtheta, &df_dphi))
	{
		return(0);
	}

	//-----------------------------------------------------//
	// estimate the theta and phi for the target frequency //
	//-----------------------------------------------------//

	float f = BasebandFreq(*theta, *phi);
	float dt, dp;
	if (df_dp == 0.0)
		dp = 0.0;
	else
		dp = (target_freq - f) / (df_dt * df_dt / df_dp + df_dp);

	if (df_dt == 0.0)
		dt = 0.0;
	else
		dt = (target_freq - f) / (df_dp * df_dp / df_dt + df_dt);

	//---------------------------//
	// jump to the theta and phi //
	//---------------------------//

	float new_theta = *theta + dt;
	float new_phi = *phi + dp;

	//----------------------------------------------//
	// verify that the new theta and phi are better //
	//----------------------------------------------//

	float new_f = BasebandFreq(new_theta, new_phi);
	if (fabs(new_f - target_freq) > fabs(f - target_freq))
		return(0);

	*theta = new_theta;
	*phi = new_phi;

	return(1);
}


//----------//
// FindFreq //
//----------//

#define GRADIENT_ANGLE		0.02
#define MIN_GRADIENT_ANGLE	0.0025

int
FindFreq(
	float*	theta,
	float*	phi,
	float	target_freq,
	float	freq_tol)
{
	float new_theta = *theta;
	float new_phi = *phi;

	float grad_angle = GRADIENT_ANGLE;

	for(;;)
	{
		if (! PositionParams(new_theta, new_phi))
			return(0);

		float freq = xxx;
		if (fabs(freq - target_freq) < freq_tol)
		{
			*theta = new_theta;
			*phi = new_phi;
			return(1);
		}

		if (! GradientFreqJump(&new_theta, &new_phi, grad_angle, target_freq))
		{
			// try decreasing the gradient angle
			if (grad_angle < MIN_GRADIENT_ANGLE)
				return(0);
			grad_angle /= 2.0;
		}
	}
	return(0);
}
*/
