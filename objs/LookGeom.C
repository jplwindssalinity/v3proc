//
// This file contains functions useful for geometric calculations
// for a particular satellite location and ground target.
//

static const char rcs_id_lookgeom_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "LookGeom.h"
#include "CoordinateSwitch.h"
#include "earth_params.h"


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

Vector3 antenna_look(EarthPosition rsat, Vector3 vsat, EarthPosition rground,
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
// The algorithm is the same as that used by the NSCAT ATB routine Locate.
//
// Inputs:
//  
//   rsat = s/c position vector (rotating geocentric frame)
//   vsat = s/c intertial velocity (rotating geocentric frame)
//   sc_att = s/c body attitude vector (roll,pitch,yaw) with respect to the
//            frame defined by the orbit plane (just like NSCAT)
//   ant_att = antenna attitude (roll,pitch,yaw) with respect to the
//             s/c body frame
//   rlook_ant = unit vector in antenna frame pointed in the desired
//               look direction.
//        All angle inputs are in radians.
//
// Return Value:
//   The position vector of the ground intercept point in geocentric
//   coordinates (km).
//

EarthPosition earth_intercept(EarthPosition rsat, Vector3 vsat,
		              Vector3 sc_att, Vector3 ant_att,
			      Vector3 rlook_ant)

{

//
// Transform rlook_ant from the antenna frame to
// the geocentric frame.
//

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

// Apply coordinate transformations to put rlook_ant in the geocentric
// frame.

Vector3 rlook_scbody = scbody_to_ant.backward(rlook_ant);
Vector3 rlook_scvel = scvel_to_scbody.backward(rlook_scbody);
Vector3 rlook_geo = geo_to_scvel.backward(rlook_scvel);

rlook_geo.scale(1.0);
rlook_geo.show("rlook_geo");

Vector3 v1 = rlook_geo * rlook_geo;
Vector3 v2 = rsat * rlook_geo;
Vector3 v3 = rsat * rsat;

double efactor = 1.0 - ECCENTRICITY_EARTH*ECCENTRICITY_EARTH;
double C1 = v1.get(0) + v1.get(1) + v1.get(2)/efactor;
double C2 = v2.get(0) + v2.get(1) + v2.get(2)/efactor;
double C3 = v3.get(0) + v3.get(1) + v3.get(2)/efactor -
            R1_EARTH*R1_EARTH;

// Quadratic solution for the slant range S.

double discriminate = 4*C2*C2 - 4*C1*C3;

if (discriminate < 0)
  {
  printf("Warning: earth_intercept did not find an intercept\n");
  EarthPosition rground(0,0,0,RECTANGULAR);
  return(rground);
  }

double righthalf = sqrt(discriminate)/(2*C1);
double s1 = -C2/C1 + righthalf;
double s2 = -C2/C1 - righthalf;
double S;
if (s1 > s2) S = s2; else S = s1;
printf("slant range = %g\n",S);
EarthPosition rground;
rground = rsat + rlook_geo*S;

return(rground);

}
