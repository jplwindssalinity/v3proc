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
#include "Beam.h"
#include "GenericGeom.h"
#include "Constants.h"
#include "Matrix3.h"

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

