//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_instrumentgeom_c[] =
	"@(#) $Id$";

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

//-----------//
// FindSlice //
//-----------//

// step size for evaluating gradient
#define FREQ_GRADIENT_ANGLE		0.008		// about 0.5 degree

int
FindSlice(
	CoordinateSwitch*	beam_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	float				freq_1,
	float				freq_2,
	float				freq_tol,
	Outline*			outline,
	Vector3*			centroid_beam_look)
{
	float s_peak;
	float az[3], el[3];
	double s[3];

	//-----------------------//
	// get frequency 1 slice //
	//-----------------------//

	float angle_f1;
	double c_f1[3];
	float az_1 = 0.0;
	float el_1 = 0.0;
	JumpToFreq(beam_frame_to_gc, spacecraft, instrument, &az_1, &el_1,
		FREQ_GRADIENT_ANGLE, freq_1, freq_tol);
	IsoFreqAngle(beam_frame_to_gc, spacecraft, instrument, &az_1, &el_1,
		FREQ_GRADIENT_ANGLE, &angle_f1);
	SetPoints(az_1, el_1, FREQ_GRADIENT_ANGLE, angle_f1, az, el);
	GainSlice(instrument, az, el, s, c_f1);
	s_peak = -c_f1[1] / 2.0 * c_f1[2];
	az_1 = az[0] + s_peak * cos(angle_f1);
	el_1 = el[0] + s_peak * sin(angle_f1);

	//-----------------------//
	// get frequency 2 slice //
	//-----------------------//

	float angle_f2;
	double c_f2[3];
	float az_2 = 0.0;
	float el_2 = 0.0;
	JumpToFreq(beam_frame_to_gc, spacecraft, instrument, &az_2, &el_2,
		FREQ_GRADIENT_ANGLE, freq_2, freq_tol);
	IsoFreqAngle(beam_frame_to_gc, spacecraft, instrument, &az_2, &el_2,
		FREQ_GRADIENT_ANGLE, &angle_f2);
	SetPoints(az_2, el_2, FREQ_GRADIENT_ANGLE, angle_f2, az, el);
	GainSlice(instrument, az, el, s, c_f2);
	s_peak = -c_f2[1] / 2.0 * c_f2[2];
	az_2 = az[0] + s_peak * cos(angle_f2);
	el_2 = el[0] + s_peak * sin(angle_f2);

	//----------------------------------------//
	// determine absolute peak gain for slice //
	//----------------------------------------//

	az[0] = az_1;
	el[0] = el_1;
	az[2] = az_2;
	el[2] = el_2;
	double angle = atan2(el_2 - el_1, az_2 - az_1);
	double c[3];
	GainSlice(instrument, az, el, s, c);
	s_peak = -c[1] / 2.0 * c[2];
	float peak_az = az[0] + s_peak * cos(angle);
	float peak_el = el[0] + s_peak * sin(angle);
	float gain = ((c[2] * s_peak) + c[1]) * s_peak + c[0];

	float outline_az[2][2];
	float outline_el[2][2];

	//------------------------------------------------------//
	// determine target -3 db gain location for frequency 1 //
	//------------------------------------------------------//

	// just solve the quadratic for the desired gain
	float target_gain = gain - 3.0;
	float q = sqrt(c_f1[1]*c_f1[1] - 4.0 * c_f1[2] * (c_f1[0] - target_gain));
	float twoa = 2.0 * c_f1[2];
	float s1 = (-c_f1[1] + q) / twoa;
	float s2 = (-c_f1[1] - q) / twoa;

	outline_az[0][0] = az[0] + s1 * cos(angle_f1);
	outline_el[0][0] = el[0] + s1 * sin(angle_f1);
	outline_az[0][1] = az[0] + s2 * cos(angle_f1);
	outline_el[0][1] = el[0] + s2 * sin(angle_f1);

	//------------------------------------------------------//
	// determine target -3 db gain location for frequency 2 //
	//------------------------------------------------------//

	// just solve the quadratic for the desired gain
	q = sqrt(c_f2[1]*c_f2[1] - 4.0 * c_f2[2] * (c_f2[0] - target_gain));
	twoa = 2.0 * c_f2[2];
	s1 = (-c_f2[1] + q) / twoa;
	s2 = (-c_f2[1] - q) / twoa;

	outline_az[1][0] = az[2] + s1 * cos(angle_f2);
	outline_el[1][0] = el[2] + s1 * sin(angle_f2);
	outline_az[1][1] = az[2] + s2 * cos(angle_f2);
	outline_el[1][1] = el[2] + s2 * sin(angle_f2);

	//--------------------//
	// create the outline //
	//--------------------//

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			Vector3 rlook_beam;
			rlook_beam.AzimuthElevationSet(1.0, outline_az[i][j],
				outline_el[i][j]);
			Vector3 rlook_gc = beam_frame_to_gc->Forward(rlook_beam);
			EarthPosition spot_on_earth =
				earth_intercept(spacecraft->orbitState.rsat, rlook_gc);
			double alt, lat, lon;
			if (spot_on_earth.GetAltLatLon(EarthPosition::GEODETIC,
				&alt, &lat, &lon) == 0)
			{
				printf("Error: FindSlice can't convert spot_on_earth\n");
				return(0);
			}
			LonLat* ll = new LonLat;
			ll->longitude = lon;
			ll->latitude = lat;
			outline->Append(ll);
		}
	}

	return(1);
}

//------------//
// JumpToFreq //
//------------//

int
JumpToFreq(
	CoordinateSwitch*	beam_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	float*				az,
	float*				el,
	float				grad_angle,
	float				target_freq,
	float				freq_tol)
{
	Vector3 ulook_beam;
	do	
	{
		//-------------------------//
		// evalulate the frequency //
		//-------------------------//

		ulook_beam.AzimuthElevationSet(1.0, *az, *el);
		TargetInfoPackage tip;
		if (! TargetInfo(ulook_beam, beam_frame_to_gc, spacecraft, instrument,
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

		float df_daz, df_del;
		if (! FreqGradient(beam_frame_to_gc, spacecraft, instrument, *az, *el,
			grad_angle, &df_daz, &df_del))
		{
			return(0);
		}

		//----------------------------------------------//
		// calculate az and el for the target frequency //
		//----------------------------------------------//

		if (! TargetInfo(ulook_beam, beam_frame_to_gc, spacecraft, instrument,
			&tip))
		{
			return(0);
		}
		float delta_az, delta_el;
		if (df_del == 0.0)
			delta_el = 0.0;
		else
		{
			delta_el = (target_freq - tip.basebandFreq) /
				(df_daz * df_daz / df_del + df_del);
		}

		if (df_daz == 0.0)
			delta_az = 0.0;
		else
		{
			delta_az = (target_freq - tip.basebandFreq) /
				(df_del * df_del / df_daz + df_daz);
		}

		//-----------------------//
		// jump to the az and el //
		//-----------------------//

		*az += delta_az;
		*el += delta_el;

	} while (1);

	return(1);
}

//--------------//
// FreqGradient //
//--------------//

int
FreqGradient(
	CoordinateSwitch*	beam_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	float				az,
	float				el,
	float				grad_angle,
	float*				df_daz,
	float*				df_del)
{
	TargetInfoPackage tip1, tip2;

	// calculate angle deltas
	float half_grad_angle = grad_angle / 2.0;

	// calculate azimuth angles
	float az1 = az - half_grad_angle;
	float az2 = az + half_grad_angle;

	// convert to look angles
	Vector3 ulook_beam_az1;
	ulook_beam_az1.AzimuthElevationSet(1.0, az1, el);

	Vector3 ulook_beam_az2;
	ulook_beam_az2.AzimuthElevationSet(1.0, az2, el);

	// calculate baseband frequency
	if (! TargetInfo(ulook_beam_az1, beam_frame_to_gc, spacecraft, instrument,
			&tip1) ||
		! TargetInfo(ulook_beam_az2, beam_frame_to_gc, spacecraft, instrument,
			&tip2))
	{
		return(0);
	}

	// calculate azimuth gradient
	*df_daz = (tip2.basebandFreq - tip1.basebandFreq) / (az2 - az1);

	// calculate elevation angles
	float el1 = el - half_grad_angle;
	float el2 = el + half_grad_angle;

	// convert to look angles
	Vector3 ulook_beam_el1;
	ulook_beam_el1.AzimuthElevationSet(1.0, az, el1);

	Vector3 ulook_beam_el2;
	ulook_beam_el2.AzimuthElevationSet(1.0, az, el2);

	// calculate baseband frequency
	if (! TargetInfo(ulook_beam_el1, beam_frame_to_gc, spacecraft, instrument,
			&tip1) ||
		! TargetInfo(ulook_beam_el2, beam_frame_to_gc, spacecraft, instrument,
			&tip2))
	{
		return(0);
	}

	// calculate elevation gradient
	*df_del = (tip2.basebandFreq - tip1.basebandFreq) / (el2 - el1);

	return(1);
}

//------------//
// TargetInfo //
//------------//
// Compute some useful numbers for the target on the earth's surface
// intercepted by a particular direciton in a beam frame.
//
// Inputs:

int
TargetInfo(
	Vector3				ulook_beam,
	CoordinateSwitch*	beam_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	TargetInfoPackage*	tip)
{
	// dereference
	OrbitState* sc_orbit_state = &(spacecraft->orbitState);

	// Compute earth intercept point and range.
	Vector3 ulook_gc = beam_frame_to_gc->Forward(ulook_beam);
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
	CoordinateSwitch*	beam_frame_to_gc,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	float*				az,
	float*				el,
	float				grad_angle,
	float*				angle)
{
	//------------------------//
	// calculate the gradient //
	//------------------------//

	float df_daz, df_del;
	if (! FreqGradient(beam_frame_to_gc, spacecraft, instrument, *az, *el,
		grad_angle, &df_daz, &df_del))
	{
		return(0);
	}

	//--------------------------------------------//
	// determine the iso-frequency gradient angle //
	//--------------------------------------------//

	*angle = atan2(df_del, df_daz);		// max grad angle
	*angle += pi / 2.0;					// iso freq angle

	return(1);
}

//-----------//
// SetPoints //
//-----------//

int
SetPoints(
	float		az_0,
	float		el_0,
	float		distance,
	float		angle,
	float		az[3],
	float		el[3])
{
	//----------------------------------------//
	// calculate two outer points on the line //
	//----------------------------------------//

	float delta_az = distance * cos(angle);
	az[0] = az_0 - delta_az;
	az[2] = az_0 + delta_az;

	float delta_el = distance * sin(angle);
	el[0] = el_0 - delta_el;
	el[2] = el_0 + delta_el;

	return(1);
}

//-----------//
// GainSlice //
//-----------//

int
GainSlice(
	Instrument*			instrument,
	float				az[3],
	float				el[3],
	double				s[3],
	double				c[3])
{
	//------------------------//
	// calculate center point //
	//------------------------//

	az[1] = (az[0] + az[2]) / 2.0;
	el[1] = (el[0] + el[2]) / 2.0;

	//-------------------------------//
	// calculate projected distances //
	//-------------------------------//

	s[0] = 0.0;
	float da, de;
	da = (az[1] - az[0]);
	de = (el[1] - el[0]);
	s[1] = sqrt(da*da + de*de);
	da = (az[2] - az[0]);
	de = (el[2] - el[0]);
	s[2] = sqrt(da*da + de*de);

	//---------------------------------//
	// get gain for those three points //
	//---------------------------------//

	double gain[3];
	int idx = instrument->antenna.currentBeamIdx;
	gain[0] = instrument->antenna.beam[idx].GetPowerGain(az[0], el[0]);
	gain[1] = instrument->antenna.beam[idx].GetPowerGain(az[1], el[1]);
	gain[2] = instrument->antenna.beam[idx].GetPowerGain(az[2], el[2]);

	//-----------------//
	// fit a quadratic //
	//-----------------//

	if (! polcoe(s, gain, 2, c))
		return(0);

	return(1);
}
