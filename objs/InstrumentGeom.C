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

	double look_1 = look;
	double azim_1 = azim;
	float gain_1;
	if (! FindPeakGainAtFreq(antenna_frame_to_gc, spacecraft, instrument,
		freq_1, freq_tol, &look_1, &azim_1, &gain_1))
	{
		fprintf(stderr,
			"FindSlice: error finding peak gain for frequency %g\n", freq_1);
		return(0);
	}

	//--------------------------------//
	// find peak gain for frequency 2 //
	//--------------------------------//

	double look_2 = look;
	double azim_2 = azim;
	float gain_2;
	if (! FindPeakGainAtFreq(antenna_frame_to_gc, spacecraft, instrument,
		freq_2, freq_tol, &look_2, &azim_2, &gain_2))
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
		look_1, azim_1, gain_1, look_2, azim_2, gain_2, &peak_gain))
	{
		fprintf(stderr, "FindSlice: error finding peak gain for slice\n");
		return(0);
	}

	//------------------------------//
	// find target gain for corners //
	//------------------------------//

	float target_gain = peak_gain / pow(10.0, 0.3);

	//------------------------------//
	// find corners for frequency 1 //
	//------------------------------//

	double c_look_1[2];
	double c_azim_1[2];
	if (! FindSliceCorners(antenna_frame_to_gc, spacecraft, instrument,
		look_1, azim_1, target_gain, c_look_1, c_azim_1))
	{
		fprintf(stderr,
			"FindSlice: error finding slice corners for frequency 1\n");
		return(0);
	}

	//------------------------------//
	// find corners for frequency 2 //
	//------------------------------//

	double c_look_2[2];
	double c_azim_2[2];
	if (! FindSliceCorners(antenna_frame_to_gc, spacecraft, instrument,
		look_2, azim_2, target_gain, c_look_2, c_azim_2))
	{
		fprintf(stderr,
			"FindSlice: error finding slice corners for frequency 2\n");
		return(0);
	}

	//----------------------//
	// generate the outline //
	//----------------------//

	double c_look[4], c_azim[4];

	c_look[0] = c_look_1[0];
	c_look[1] = c_look_1[1];
	c_look[2] = c_look_2[1];
	c_look[3] = c_look_2[0];

	c_azim[0] = c_azim_1[0];
	c_azim[1] = c_azim_1[1];
	c_azim[2] = c_azim_2[1];
	c_azim[3] = c_azim_2[0];

	EarthPosition sum;
	sum.SetPosition(0.0, 0.0, 0.0);
	for (int i = 0; i < 4; i++)
	{
		Vector3 rlook_antenna;
		rlook_antenna.SphericalSet(1.0, c_look[i], c_azim[i]);
		Vector3 rlook_gc = antenna_frame_to_gc->Forward(rlook_antenna);
		EarthPosition* spot_on_earth = new EarthPosition();
		*spot_on_earth =
			earth_intercept(spacecraft->orbitState.rsat, rlook_gc);
		if (! outline->Append(spot_on_earth))
			return(0);
		sum += *spot_on_earth;
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

	Vector3 vspot(-w_earth * r_target.get(1), w_earth * r_target.get(0), 0);
	Vector3 vrel = sc_orbit_state->vsat - vspot;

	instrument->commandedDoppler = 0.0;
	double xmit_freq, lambda, doppler_freq, new_commanded_doppler, dif;
	do
	{
		xmit_freq = instrument->baseTransmitFreq +
			instrument->commandedDoppler;
		lambda = speed_light / xmit_freq;
		doppler_freq = 2000.0 * (vrel % ulook_gc) / lambda;
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
	Vector3 vspot(-w_earth * rspot->get(1), w_earth * rspot->get(0), 0);
	Vector3 vrel = sc_orbit_state->vsat - vspot;
	double lambda = speed_light / actual_xmit_frequency;
	tip->dopplerFreq = 2.0 * 1000.0 * (vrel % ulook_gc) / lambda;
 
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

#define LOOK_OFFSET			0.01
#define AZIMUTH_OFFSET		0.01
#define ANGLE_OFFSET		0.01
#define ANGLE_TOL			0.001

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
	double				look_1,
	double				azim_1,
	float				gain_1,
	double				look_2,
	double				azim_2,
	float				gain_2,
	float*				peak_gain)
{
	double mid_look = (look_1 + look_2) / 2.0;
	double mid_azim = (azim_1 + azim_2) / 2.0;

	float mid_gain;
	PowerGainProduct(antenna_frame_to_gc, spacecraft, instrument, mid_look,
		mid_azim, &mid_gain);

	if (gain_1 > mid_gain && gain_1 > gain_2)
	{
		*peak_gain = gain_1;
		return(1);
	}
	else if (gain_2 > mid_gain && gain_2 > gain_1)
	{
		*peak_gain = gain_2;
		return(1);
	}
	else if (mid_gain > gain_1 && mid_gain > gain_2)
	{
		double look_array[3], azim_array[3];
		look_array[0] = look_1;
		look_array[1] = mid_look;
		look_array[2] = look_2;
		azim_array[0] = azim_1;
		azim_array[1] = mid_azim;
		azim_array[2] = azim_2;

		double s[3], c[3];
		QuadFit(antenna_frame_to_gc, spacecraft, instrument, look_array,
			azim_array, s, c);
		PeakFit(c, peak_gain);
		return(1);
	}
	return(0);
}

//------------------//
// FindSliceCorners //
//------------------//

#define PEAK_ANGLE_OFFSET		0.00875		// about 0.5 degree
#define LOCAL_ANGLE_OFFSET		0.001		// about 0.06 degree

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

	double look_array[3];
	look_array[0] = look - PEAK_ANGLE_OFFSET * delta_look;
	look_array[1] = look;
	look_array[2] = look + PEAK_ANGLE_OFFSET * delta_look;

	double azim_array[3];
	azim_array[0] = azim - PEAK_ANGLE_OFFSET * delta_azim;
	azim_array[1] = azim;
	azim_array[2] = azim + PEAK_ANGLE_OFFSET * delta_azim;

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

	static const double coef[2] = { -1.0, 1.0 };
	for (int i = 0; i < 2; i++)
	{
		//-----------------------------------//
		// estimate using the peak quadratic //
		//-----------------------------------//

		double target_s = (-c[1] + coef[i] * q) / twoa;
		double local_look = look + target_s * delta_look;
		double local_azim = azim + target_s * delta_azim;

		//-----------------------//
		// fit a local quadratic //
		//-----------------------//

		double local_look_array[3];
		local_look_array[0] = local_look - LOCAL_ANGLE_OFFSET * delta_look;
		local_look_array[1] = local_look;
		local_look_array[2] = local_look + LOCAL_ANGLE_OFFSET * delta_look;

		double local_azim_array[3];
		local_azim_array[0] = local_azim - LOCAL_ANGLE_OFFSET * delta_azim;
		local_azim_array[1] = local_azim;
		local_azim_array[2] = local_azim + LOCAL_ANGLE_OFFSET * delta_azim;

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

		double local_qr = local_c[1] * local_c[1] - 4.0 * local_c[2] *
			(local_c[0] - target_gain);
		if (local_qr < 0.0)
		{
			fprintf(stderr,
				"FindSliceCorners: can't find target gain on local fit\n");
			corner_look[i] = local_look;
			corner_azim[i] = local_azim;
			continue;
		}

		//----------------------//
		// find the target gain //
		//----------------------//

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

/*
static int count = 0;
count++;
if (count >= 1696)
{
dl = (look[2] - look[0]);
da = (azim[2] - azim[0]);
double sr = sqrt(dl*dl + da*da);
dl /= sr;
da /= sr;
for (double xs = -0.03; xs < 0.03; xs += 0.001)
{
double xlook = look[1] + xs * dl;
double xazim = azim[1] + xs * da;
float xgain;
PowerGainProduct(antenna_frame_to_gc, spacecraft, instrument,
	xlook, xazim, &xgain);
printf("%g %g %g\n", xs, xgain, ((c[2] * xs) + c[1]) * xs + c[0]);
}
printf("&\n");
}
*/
 
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

//----------------------------//
// Get2WayElectricalBoresight //
//----------------------------//

//
// This function locates the maximum 2-way gain of a beam in the antenna frame,
// and returns the apparent direction. (look,azimuth (rads)).
// NOTE: This is NOT the direction of the transmitted pulse which determines
// the scattering geometry.
//

int
Get2WayElectricalBoresight(
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
		printf("Error allocating memory in Get2WayElectricalBoresight\n");
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

	double ftol = 1e-3;
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
// Get2WayElectricalBoresight.
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
