//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef GENERICGEOM_H
#define GENERICGEOM_H

#include "EarthPosition.h"
#include "CoordinateSwitch.h"

static const char rcs_id_genericgeom_h[] =
	"@(#) $Id$";

//======================================================================
// Functions
//		velocity_frame
//		velocity_frame_geodetic
//		beam_look
//		earth_intercept
//======================================================================

//======================================================================
// Function
//		velocity_frame
//
// DESCRIPTION
//		The velocity_frame function computes the axial unit vectors that
//		define the s/c velocity frame (or local coordinate system) from
//		the s/c position and velocity.
//		A GEOCENTRIC convention for the attitude reference is used.
//======================================================================

void velocity_frame(EarthPosition rsat, Vector3 vsat,
					Vector3 *xscvel_geo,
					Vector3 *yscvel_geo,
					Vector3 *zscvel_geo);

//======================================================================
// Function
//		velocity_frame_geodetic
//
// DESCRIPTION
//		The velocity_frame_geodetic function computes the axial unit vectors
//		that define the s/c velocity frame (or local coordinate system) from
//		the s/c position and velocity.
//		A GEODETIC convention for the attitude reference is used.
//======================================================================

void velocity_frame(EarthPosition rsat, Vector3 vsat,
					Vector3 *xscvel_geo,
					Vector3 *yscvel_geo,
					Vector3 *zscvel_geo);

//======================================================================
// Function
//		beam_look
//
// DESCRIPTION
//		The beam_look function computes a unit vector in the
//		beam frame that is pointed at a particular ground target.
//		A GEOCENTRIC convention for the attitude reference is used.
//======================================================================

Vector3 beam_look(EarthPosition rsat, Vector3 vsat, EarthPosition rground,
		     Attitude sc_att, Attitude ant_att, Attitude beam_att);

//======================================================================
// Function
//		earth_intercept
//
// DESCRIPTION
//		The earth_intercept function computes a position vector for
//		the intercept point on the earth's surface for a particular
//		look vector (specified in the beam frame).
//		A GEOCENTRIC convention for the attitude reference is used.
//======================================================================

EarthPosition earth_intercept(EarthPosition rsat, Vector3 vsat,
	              Attitude sc_att, Attitude ant_att, Attitude beam_att,
			      Vector3 rlook_beam);

//======================================================================
// Function
//		earth_intercept
//
// Description
//		The earth_intercept function computes a position vector for
//		the intercept point on the earth's surface for a particular
//		look vector (specified in the earth geocentric frame).
//======================================================================

EarthPosition earth_intercept(EarthPosition rsat, Vector3 rlook_geo);

#endif
