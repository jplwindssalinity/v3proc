//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef INSTRUMENTGEOM_H
#define INSTRUMENTGEOM_H

static const char rcs_id_instrumentgeom_h[] =
	"@(#) $Id$";

//======================================================================
// DESCRIPTION
//		Higher level geometry functions.  Typically instrument related.
//======================================================================

CoordinateSwitch BeamFrameToGC(OrbitState* orbit_state, Attitude* attitude,
					Antenna* antenna, Beam* beam);

int FindSliceOutline(float freq_1, float freq_2, float freq_tol,
	Outline* outline);

int
PositionParams(
	Vector3 ulook_beam,
	OrbitState*	sc_orbit_state,
	CoordinateSwitch* beam_frame_to_gc,
	double chirp_rate,
	double chirp_start_b,
	double chirp_start_m,
	double nominal_xmit_frequency,
	double fdopcom,
	double pulse_width,
	double receive_gate_delay,
	double system_delay,
	EarthPosition* rspot,
	double* fdop,
	double* frange,
	double* fbase);

#endif
