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
//		antenna_look
//		earth_intercept
//======================================================================

//======================================================================
// Function
//		antenna_look
//
// DESCRIPTION
//		The antenna_look function computes a unit vector in the
//		antenna frame that is pointed at a particular ground target.
//======================================================================

Vector3 antenna_look(EarthPosition rsat, Vector3 vsat, EarthPosition rground,
		     Vector3 sc_att, Vector3 ant_att);

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
			      Vector3 rlook_ant);
#endif
