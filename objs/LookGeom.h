//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef LOOKGEOM_H
#define LOOKGEOM_H

#include "CoordinateSwitch.h"

static const char rcs_id_lookgeom_h[] =
	"@(#) $Id$";

//======================================================================
// Functions
//		velocity_frame
//		antenna_look
//		earth_intercept
//======================================================================

//
// enum attitude_referenceE is used to specify how the spacecraft attitude
// is defined with respect to the geocentric frame.  There are two
// possibilities:
//   GEOCENTRIC - s/c velocity frame z-axis points toward the center of the
//                earth.
//   GEODETIC - s/c velocity frame z-axis points straight down to the surface,
//              intercepting perpendicular to the surface (local normal).
//

enum attitude_referenceE {GEOCENTRIC, GEODETIC};

//======================================================================
// Function
//		velocity_frame
//
// DESCRIPTION
//		The velocity_frame function computes the axial unit vectors that
//		define the s/c velocity frame (or local coordinate system) from
//		the s/c position and velocity.
//======================================================================

void velocity_frame(EarthPosition rsat, Vector3 vsat,
                    attitude_referenceE attref,
					Vector3 *xscvel_geo,
					Vector3 *yscvel_geo,
					Vector3 *zscvel_geo);

//======================================================================
// Function
//		antenna_look
//
// DESCRIPTION
//		The antenna_look function computes a unit vector in the
//		antenna frame that is pointed at a particular ground target.
//======================================================================

Vector3 antenna_look(EarthPosition rsat, Vector3 vsat, EarthPosition rground,
		     Vector3 sc_att, Vector3 ant_att, attitude_referenceE attref);

//======================================================================
// Function
//		earth_intercept
//
// DESCRIPTION
//		The earth_intercept function computes a position vector for
//		the intercept point on the earth's surface for a particular
//		look vector (specified in the antenna frame).
//======================================================================

EarthPosition earth_intercept(EarthPosition rsat, Vector3 vsat,
	              Vector3 sc_att, Vector3 ant_att,
			      Vector3 rlook_ant, attitude_referenceE attref);
#endif
