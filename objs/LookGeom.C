//
// This file contains functions useful for geometric calculations
// for a particular satellite location and ground target.
//

static const char rcs_id_lookgeom_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CoordinateSwitch.h"

//
// antenna_look
//
// This function computes the look direction in the antenna coordinate system
// for a given s/c state and ground target.
//
// Inputs:
//   rsat = s/c position vector (rotating geocentric frame)
//   vsat = s/c intertial velocity (rotating geocentric frame)
//   rground = ground target position vector (rotating geocentric frame)
//             Note that rsat and rground must have the same units (eg., km).
//   sc_att = s/c body attitude vector (roll,pitch,yaw) with respect to the
//            frame defined by the orbit plane (just like NSCAT)
//   ant_att = antenna attitude (roll,pitch,yaw) with respect to the
//             s/c body frame
//        All angle inputs are in radians.
//
// Return Value:
//   A unit vector in the antenna coordinate system pointed at the
//   ground target.
//

Vector3 antenna_look(Vector3 rsat, Vector3 vsat, Vector3 rground,
		     Vector3 sc_att, Vector3 ant_att)

{

// Spacecraft velocity frame unit vectors (in geocentric frame).
// Geocentric definition of the z-axis
Vector3 zscvel_geo = -rsat;
Vector3 yscvel_geo = zscvel_geo & vsat;
Vector3 xscvel_geo = yscvel_geo & zscvel_geo;

// Coordinate transformation from geocentric to s/c velocity
CoordinateSwitch geo_to_scvel(xscvel_geo,yscvel_geo,zscvel_geo);

// Coordinate transformation from s/c velocity to s/c body
CoordinateSwitch scvel_to_scbody(sc_att,1,2,3);

// Coordinate transformation from s/c body to antenna frame
CoordinateSwitch scbody_to_ant(ant_att,1,2,3);

// rlook is a vector from the s/c to the ground target (in geocentric frame)
Vector3 rlook = rground - rsat;

// Apply coordinate transformations to put rlook in the antenna frame.

Vector3 rlook_scvel = geo_to_scvel.forward(rlook);
Vector3 rlook_scbody = scvel_to_scbody.forward(rlook_scvel);
Vector3 rlook_ant = scbody_to_ant.forward(rlook_scbody);

rlook_ant.scale(1.0);
return(rlook_ant);

}

//
// earth_intercept
//
// This function computes the ground intercept point for a given s/c
// and antenna state.
//
// Inputs:
//  
//   rsat = s/c position vector (rotating geocentric frame)
//   vsat = s/c intertial velocity (rotating geocentric frame)
//   sc_att = s/c body attitude vector (roll,pitch,yaw) with respect to the
//            frame defined by the orbit plane (just like NSCAT)
//   ant_att = antenna attitude (roll,pitch,yaw) with respect to the
//             s/c body frame
//   antenna_look_dir = unit vector in antenna frame pointed in the desired
//                      look direction.
//        All angle inputs are in radians.
//
// Return Value:
//   The position vector of the ground intercept point in geocentric
//   coordinates (km).
//

Vector3 antenna_look(Vector3 rsat, Vector3 vsat,
		     Vector3 sc_att, Vector3 ant_att, Vector3 antenna_look_dir)

{
}
