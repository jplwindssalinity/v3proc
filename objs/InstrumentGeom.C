//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_instrumentgeom_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <math.h>
#include "InstrumentGeom.h"
#include "CoordinateSwitch.h"
#include "Ephemeris.h"
#include "Attitude.h"
#include "Antenna.h"
#include "GenericGeom.h"
#include "Constants.h"
#include "LonLat.h"
#include "Beam.h"
#include "Matrix3.h"
#include "Interpolate.h"
#include "Misc.h"

/*
//---------------//
// BeamFrameToGC //
//---------------//

CoordinateSwitch
BeamFrameToGC(
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

	// antenna frame to beam frame
	Beam* beam = antenna->GetCurrentBeam();
	CoordinateSwitch ant_frame_to_beam_frame(beam->GetAntFrameToBeamFrame());
	total.Append(&ant_frame_to_beam_frame);

	total = total.ReverseDirection();
	return(total);
}
*/

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

//-----------//
// FindSlice //
//-----------//

// step size for evaluating gradient
#define FREQ_GRADIENT_ANGLE		0.009		// about 0.5 degree
#define QUADRATIC_ANGLE			0.017		// about 0.5 degree

int
FindSlice(
	CoordinateSwitch*	antenna_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	float				look,
	float				azimuth,
	float				freq_1,
	float				freq_2,
	float				freq_tol,
	Outline*			outline,
	Vector3*			look_vector,
	EarthPosition*		centroid)
{
	float s_peak;
	float look_array[3], azimuth_array[3];
	double s[3];

	//-----------------------//
	// get frequency 1 slice //
	//-----------------------//

	float angle_f1;
	double c_f1[3];
	float look_1 = look;
	float azimuth_1 = azimuth;
	JumpToFreq(antenna_frame_to_gc, spacecraft, instrument, &look_1,
		&azimuth_1, FREQ_GRADIENT_ANGLE, freq_1, freq_tol);
	IsoFreqAngle(antenna_frame_to_gc, spacecraft, instrument, &look_1,
		&azimuth_1, FREQ_GRADIENT_ANGLE, &angle_f1);
	SetPoints(look_1, azimuth_1, QUADRATIC_ANGLE, angle_f1, look_array,
		azimuth_array);
	GainSlice(instrument, look_array, azimuth_array, s, c_f1);
	s_peak = -c_f1[1] / (2.0 * c_f1[2]);
	look_1 = look_array[1] + s_peak * sin(angle_f1);
	azimuth_1 = azimuth_array[1] + s_peak * cos(angle_f1);

	//-----------------------//
	// get frequency 2 slice //
	//-----------------------//

	float angle_f2;
	double c_f2[3];
	float look_2 = look;
	float azimuth_2 = azimuth;
	JumpToFreq(antenna_frame_to_gc, spacecraft, instrument, &look_2,
		&azimuth_2, FREQ_GRADIENT_ANGLE, freq_2, freq_tol);
	IsoFreqAngle(antenna_frame_to_gc, spacecraft, instrument, &look_2,
		&azimuth_2, FREQ_GRADIENT_ANGLE, &angle_f2);
	SetPoints(look_2, azimuth_2, QUADRATIC_ANGLE, angle_f2, look_array,
		azimuth_array);
	GainSlice(instrument, look_array, azimuth_array, s, c_f2);
	s_peak = -c_f2[1] / (2.0 * c_f2[2]);
	look_2 = look_array[1] + s_peak * sin(angle_f2);
	azimuth_2 = azimuth_array[1] + s_peak * cos(angle_f2);

	//----------------------------------------//
	// determine absolute peak gain for slice //
	//----------------------------------------//

	look_array[0] = look_1;
	azimuth_array[0] = azimuth_1;
	look_array[2] = look_2;
	azimuth_array[2] = azimuth_2;
	double c[3];
	GainSlice(instrument, look_array, azimuth_array, s, c);
	s_peak = -c[1] / (2.0 * c[2]);

	float gain;
	if (s_peak < s[0] || s_peak > s[2])
	{
		// peak of pattern is out of slice, choose highest gain in slice
		float gain1 = ((c[2] * s[0]) + c[1]) * s[0] + c[0];
		float gain2 = ((c[2] * s[2]) + c[1]) * s[2] + c[0];
		gain = MAX(gain1, gain2);
	}
	else
	{
		// peak of pattern is in slice, use peak
		gain = ((c[2] * s_peak) + c[1]) * s_peak + c[0];
	}

	float outline_look[2][2];
	float outline_azimuth[2][2];
	double qr;
	float q, twoa, s1, s2;

	//------------------------------------------------------//
	// determine target -3 db gain location for frequency 1 //
	//------------------------------------------------------//

	// just solve the quadratic for the desired gain
	float target_gain = gain / pow(10.0, 0.3);
	qr = c_f1[1]*c_f1[1] - 4.0 * c_f1[2] * (c_f1[0] - target_gain);

	if (qr < 0.0)
	{
		// desired gain not on pattern, use best point
		outline_look[0][0] = look_array[0];
		outline_azimuth[0][0] = azimuth_array[0];
		outline_look[0][1] = look_array[0];
		outline_azimuth[0][1] = azimuth_array[0];
	}
	else
	{
		q = sqrt(qr);
		twoa = 2.0 * c_f1[2];
		s1 = (-c_f1[1] + q) / twoa;
		s2 = (-c_f1[1] - q) / twoa;

		outline_look[0][0] = look_array[0] + s1 * sin(angle_f1);
		outline_azimuth[0][0] = azimuth_array[0] + s1 * cos(angle_f1);
		outline_look[0][1] = look_array[0] + s2 * sin(angle_f1);
		outline_azimuth[0][1] = azimuth_array[0] + s2 * cos(angle_f1);
	}

	//------------------------------------------------------//
	// determine target -3 db gain location for frequency 2 //
	//------------------------------------------------------//

	// just solve the quadratic for the desired gain
	qr = c_f2[1]*c_f2[1] - 4.0 * c_f2[2] * (c_f2[0] - target_gain);

	if (qr < 0.0)
	{
		// desired gain not on pattern, use best point
		outline_look[1][0] = look_array[2];
		outline_azimuth[1][0] = azimuth_array[2];
		outline_look[1][1] = look_array[2];
		outline_azimuth[1][1] = azimuth_array[2];
	}
	else
	{
		q = sqrt(qr);
		twoa = 2.0 * c_f2[2];
		s1 = (-c_f2[1] + q) / twoa;
		s2 = (-c_f2[1] - q) / twoa;

		outline_look[1][0] = look_array[2] + s1 * sin(angle_f2);
		outline_azimuth[1][0] = azimuth_array[2] + s1 * cos(angle_f2);
		outline_look[1][1] = look_array[2] + s2 * sin(angle_f2);
		outline_azimuth[1][1] = azimuth_array[2] + s2 * cos(angle_f2);
	}

	//--------------------//
	// create the outline //
	//--------------------//

	EarthPosition sum;
	sum.SetPosition(0.0, 0.0, 0.0);
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			Vector3 rlook_antenna;
			rlook_antenna.SphericalSet(1.0, outline_look[i][j],
				outline_azimuth[i][j]);
			Vector3 rlook_gc = antenna_frame_to_gc->Forward(rlook_antenna);
			EarthPosition* spot_on_earth = new EarthPosition();
			*spot_on_earth =
				earth_intercept(spacecraft->orbitState.rsat, rlook_gc);
			if (! outline->Append(spot_on_earth))
				return(0);
			sum = sum + *spot_on_earth;
		}
	}

	//-------------------//
	// find the centroid //
	//-------------------//

	EarthPosition earth_center;
	earth_center.SetPosition(0.0, 0.0, 0.0);
	*centroid = earth_intercept(earth_center, sum);

	//--------------------------------------------//
	// determine the look vector for the centroid //
	//--------------------------------------------//

	*look_vector = *centroid - spacecraft->orbitState.rsat;

	return(1);
}

//------------//
// JumpToFreq //
//------------//

int
JumpToFreq(
	CoordinateSwitch*	antenna_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	float*				look,
	float*				azimuth,
	float				grad_angle,
	float				target_freq,
	float				freq_tol)
{
	Vector3 vector;
	do	
	{
		//-------------------------//
		// evalulate the frequency //
		//-------------------------//

		vector.SphericalSet(1.0, *look, *azimuth);
		TargetInfoPackage tip;
		if (! TargetInfo(vector, antenna_frame_to_gc, spacecraft, instrument,
			&tip))
		{
			return(0);
		}

		//---------------//
		// check if done //
		//---------------//

		if (fabs(target_freq - tip.basebandFreq) < freq_tol)
			break;

		//----------------------------------//
		// calculate the frequency gradient //
		//----------------------------------//

		float df_dlook, df_dazim;
		if (! FreqGradient(antenna_frame_to_gc, spacecraft, instrument,
			*look, *azimuth, grad_angle, &df_dlook, &df_dazim))
		{
			return(0);
		}

		//-----------------------------------------------------//
		// calculate look and azimuth for the target frequency //
		//-----------------------------------------------------//

/* not needed I think
		if (! TargetInfo(vector, antenna_frame_to_gc, spacecraft, instrument,
			&tip))
		{
			return(0);
		}
*/
		float delta_look, delta_azim;
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
		*azimuth += delta_azim;

	} while (1);

	return(1);
}

//--------------//
// FreqGradient //
//--------------//

int
FreqGradient(
	CoordinateSwitch*	antenna_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	float				look,
	float				azimuth,
	float				grad_angle,
	float*				df_dlook,
	float*				df_dazim)
{
	TargetInfoPackage tip1, tip2;

	// calculate angle deltas
	float half_grad_angle = grad_angle / 2.0;

	// calculate look angles
	float look1 = look - half_grad_angle;
	float look2 = look + half_grad_angle;

	// convert to vectors
	Vector3 vector_look1;
	vector_look1.SphericalSet(1.0, look1, azimuth);

	Vector3 vector_look2;
	vector_look2.SphericalSet(1.0, look2, azimuth);

	// calculate baseband frequency
	if (! TargetInfo(vector_look1, antenna_frame_to_gc, spacecraft, instrument,
			&tip1) ||
		! TargetInfo(vector_look2, antenna_frame_to_gc, spacecraft, instrument,
			&tip2))
	{
		return(0);
	}

	// calculate look gradient
	*df_dlook = (tip2.basebandFreq - tip1.basebandFreq) / (look2 - look1);

	// calculate azimuth angles
	float azim1 = azimuth - half_grad_angle;
	float azim2 = azimuth + half_grad_angle;

	// convert to look angles
	Vector3 vector_azim1;
	vector_azim1.SphericalSet(1.0, look, azim1);

	Vector3 vector_azim2;
	vector_azim2.SphericalSet(1.0, look, azim2);

	// calculate baseband frequency
	if (! TargetInfo(vector_azim1, antenna_frame_to_gc, spacecraft, instrument,
			&tip1) ||
		! TargetInfo(vector_azim2, antenna_frame_to_gc, spacecraft, instrument,
			&tip2))
	{
		return(0);
	}

	// calculate azimuth gradient
	*df_dazim = (tip2.basebandFreq - tip1.basebandFreq) / (azim2 - azim1);

	return(1);
}

//------------//
// TargetInfo //
//------------//
// Compute some useful numbers for the target on the earth's surface
// intercepted by a particular direciton in the antenna frame.
//
// Inputs:

int
TargetInfo(
	Vector3				vector,
	CoordinateSwitch*	antenna_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	TargetInfoPackage*	tip)
{
	// dereference
	OrbitState* sc_orbit_state = &(spacecraft->orbitState);

	// Compute earth intercept point and range.
	Vector3 ulook_gc = antenna_frame_to_gc->Forward(vector);
	tip->rTarget = earth_intercept(sc_orbit_state->rsat, ulook_gc);
	EarthPosition* rspot = &(tip->rTarget);
	tip->slantRange = (sc_orbit_state->rsat - *rspot).Magnitude();

	// Compute doppler shift for the earth intercept point.
	double actual_xmit_frequency = instrument->baseTransmitFreq +
		instrument->commandedDoppler;
	Vector3 vspot(-w_earth * rspot->get(1), w_earth * rspot->get(0), 0);
	Vector3 vrel = sc_orbit_state->vsat - vspot;
	double lambda = speed_light / actual_xmit_frequency;
	tip->dopplerFreq = -2.0 * 1000.0 * (vrel % ulook_gc) / lambda;

	// Compute baseband frequency shift due to range
	int current_beam_idx = instrument->antenna.currentBeamIdx;
	float pulse_width = instrument->antenna.beam[current_beam_idx].pulseWidth;
	double chirp_start = instrument->chirpStartM * pulse_width +
		instrument->chirpStartB;
	double transmit_center = -chirp_start / instrument->chirpRate;
	double flight_time = 2 * tip->slantRange / speed_light_kps;
	double echo_center = transmit_center + flight_time +
		instrument->systemDelay;
	double rx_gate_center = instrument->receiverGateDelay +
		instrument->receiverGateWidth / 2.0;
	tip->rangeFreq = instrument->chirpRate * (rx_gate_center - echo_center);
	tip->basebandFreq = tip->rangeFreq - tip->dopplerFreq +
		instrument->commandedDoppler;

	return(1);
}

//--------------//
// IsoFreqAngle //
//--------------//

int
IsoFreqAngle(
	CoordinateSwitch*	antenna_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	float*				look,
	float*				azimuth,
	float				grad_angle,
	float*				angle)
{
	//------------------------//
	// calculate the gradient //
	//------------------------//

	float df_dlook, df_dazim;
	if (! FreqGradient(antenna_frame_to_gc, spacecraft, instrument, *look,
		*azimuth, grad_angle, &df_dlook, &df_dazim))
	{
		return(0);
	}

	//--------------------------------------------//
	// determine the iso-frequency gradient angle //
	//--------------------------------------------//

	*angle = atan2(df_dlook, df_dazim);		// max grad angle
	*angle += pi / 2.0;						// iso freq angle

	return(1);
}

//-----------//
// SetPoints //
//-----------//

int
SetPoints(
	float		look_0,
	float		azim_0,
	float		distance,
	float		angle,
	float		look[3],
	float		azim[3])
{
	//----------------------------------------//
	// calculate two outer points on the line //
	//----------------------------------------//

	float delta_look = distance * sin(angle);
	look[0] = look_0 - delta_look;
	look[2] = look_0 + delta_look;

	float delta_azim = distance * cos(angle);
	azim[0] = azim_0 - delta_azim;
	azim[2] = azim_0 + delta_azim;

	return(1);
}

//-----------//
// GainSlice //
//-----------//

int
GainSlice(
	Instrument*			instrument,
	float				look[3],
	float				azim[3],
	double				s[3],
	double				c[3])
{
	//------------------------//
	// calculate center point //
	//------------------------//

	look[1] = (look[0] + look[2]) / 2.0;
	azim[1] = (azim[0] + azim[2]) / 2.0;

	//-------------------------------//
	// calculate projected distances //
	//-------------------------------//

	s[1] = 0.0;
	float dl, da;
	dl = (look[1] - look[0]);
	da = (azim[1] - azim[0]);
	s[0] = -sqrt(dl*dl + da*da);
	dl = (look[2] - look[1]);
	da = (azim[2] - azim[1]);
	s[2] = sqrt(dl*dl + da*da);

	//---------------------------------//
	// get gain for those three points //
	//---------------------------------//

	double gain[3];
	int idx = instrument->antenna.currentBeamIdx;
	if (! instrument->antenna.beam[idx].GetPowerGain(look[0], azim[0],
			&gain[0]) ||
		! instrument->antenna.beam[idx].GetPowerGain(look[1], azim[1],
			&gain[1]) ||
		! instrument->antenna.beam[idx].GetPowerGain(look[2], azim[2],
			&gain[2]))
	{
		return(0);
	}

	//-----------------//
	// fit a quadratic //
	//-----------------//

	if (! polcoe(s, gain, 2, c))
		return(0);

	return(1);
}

//-------------------//
// DetailedGainSlice //
//-------------------//

int
DetailedGainSlice(
	Instrument*		instrument,
	float			look[3],
	float			azim[3],
	double			s[3],
	double			c[3])
{
	//------------------------//
	// calculate center point //
	//------------------------//

	look[1] = (look[0] + look[2]) / 2.0;
	azim[1] = (azim[0] + azim[2]) / 2.0;

	//-------------------------------//
	// calculate projected distances //
	//-------------------------------//

	s[1] = 0.0;
	float dl, da;
	dl = (look[1] - look[0]);
	da = (azim[1] - azim[0]);
	s[0] = -sqrt(dl*dl + da*da);
	dl = (look[2] - look[1]);
	da = (azim[2] - azim[1]);
	s[2] = sqrt(dl*dl + da*da);

	int idx = instrument->antenna.currentBeamIdx;
	double angle = atan2(dl, da);
	angle += pi / 2.0;						// iso freq angle
	double xgain;
	for (float xs = -0.1; xs < 0.1; xs += 0.001)
	{
		float xlook = xs * cos(angle) + look[1];
		float xazim = xs * sin(angle) + azim[1];
		if (instrument->antenna.beam[idx].GetPowerGain(xlook, xazim, &xgain))
			printf("%g %g\n", xs, xgain);
	}

	//---------------------------------//
	// get gain for those three points //
	//---------------------------------//

	double gain[3];
	if (! instrument->antenna.beam[idx].GetPowerGain(look[0], azim[0],
			&gain[0]) ||
		! instrument->antenna.beam[idx].GetPowerGain(look[1], azim[1],
			&gain[1]) ||
		! instrument->antenna.beam[idx].GetPowerGain(look[2], azim[2],
			&gain[2]))
	{
		return(0);
	}

	//-----------------//
	// fit a quadratic //
	//-----------------//

	if (! polcoe(s, gain, 2, c))
		return(0);

	return(1);
}
