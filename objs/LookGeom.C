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
#include "Constants.h"


//
// velocity_frame
//
// This function determines the spacecraft velocity frame (also called
// the local coordinate system) given the s/c position and velocity.
// The attitude uses a GEOCENTRIC reference meaning that the z-axis intercepts
// the center of the earth.
//
// Inputs:
//  rsat = geocentric position of the spacecraft (km)
//  vsat = geocentric inertial velocity of the spacecraft (km/s)
//  xscvel_geo = pointer to a Vector3 object to hold the x-axis unit vector of
//               the velocity coordinate frame represented in the geocentric
//               frame.
//  yscvel_geo = same thing for the y-axis.
//  zscvel_geo = same thing for the z-axis.
//
// Action:
//  The Vector3 objects pointed to by x,y,zscvel_geo are filled with the
//  corresponding unit vectors.
//

void velocity_frame(EarthPosition rsat, Vector3 vsat,
					Vector3 *xscvel_geo,
					Vector3 *yscvel_geo,
					Vector3 *zscvel_geo)

{

// Geocentric definition of the z-axis
*zscvel_geo = -rsat;
// y-axis is perpendicular to the orbit plane
*yscvel_geo = *zscvel_geo & vsat;
// x-axis close to, but not the same as the velocity vector
*xscvel_geo = *yscvel_geo & *zscvel_geo;

}

//
// velocity_frame_geodetic
//
// This function determines the spacecraft velocity frame (also called
// the local coordinate system) given the s/c position and velocity.
// The attitude uses a GEODETIC reference meaning that the z-axis intercepts
// the earth's surface perpendicular to the local tangent plane.
//
// Inputs:
//  rsat = geocentric position of the spacecraft (km)
//  vsat = geocentric inertial velocity of the spacecraft (km/s)
//  xscvel_geo = pointer to a Vector3 object to hold the x-axis unit vector of
//               the velocity coordinate frame represented in the geocentric
//               frame.
//  yscvel_geo = same thing for the y-axis.
//  zscvel_geo = same thing for the z-axis.
//
// Action:
//  The Vector3 objects pointed to by x,y,zscvel_geo are filled with the
//  corresponding unit vectors.
//

void velocity_frame_geodetic(EarthPosition rsat, Vector3 vsat,
							 Vector3 *xscvel_geo,
							 Vector3 *yscvel_geo,
							 Vector3 *zscvel_geo)

{

// Geodetic definition of the z-axis
Vector3 rsat_geodetic = rsat.get_alt_lat_lon(EarthPosition::GEODETIC);
double nadir_lat = rsat_geodetic.get(1);
double nadir_lon = rsat_geodetic.get(2);
EarthPosition rnadir(nadir_lat,nadir_lon,EarthPosition::GEODETIC);
*zscvel_geo = rnadir - rsat;

// y-axis is perpendicular to the orbit plane
*yscvel_geo = *zscvel_geo & vsat;
// x-axis close to, but not the same as the velocity vector
*xscvel_geo = *yscvel_geo & *zscvel_geo;

}

//
// beam_look
//
// This function computes the look direction in the beam coordinate system
// for a given s/c state and ground target.
// Spacecraft attitude is assumed to use a GEOCENTRIC reference.
//
// Inputs:
//   rsat = s/c position vector (rotating geocentric frame)
//   vsat = s/c intertial velocity (rotating geocentric frame)
//   rground = ground target position vector (rotating geocentric frame)
//             Note that rsat and rground must have the same units (eg., km).
//   sc_att = s/c body attitude (roll,pitch,yaw) with respect to the
//            frame defined by the orbit plane (just like NSCAT)
//   ant_att = antenna attitude (roll,pitch,yaw) with respect to the
//             s/c body frame
//   beam_att = beam attitude (roll,pitch,yaw) with respect to the
//             antenna frame
//        All angle inputs are in radians.
//
// Return Value:
//   A unit vector in the beam coordinate system pointed at the
//   ground target.
//

Vector3 beam_look(EarthPosition rsat, Vector3 vsat, EarthPosition rground,
		     Attitude sc_att, Attitude ant_att, Attitude beam_att)

{

// Spacecraft velocity frame unit vectors (in geocentric frame).
Vector3 xscvel_geo;
Vector3 yscvel_geo;
Vector3 zscvel_geo;
velocity_frame(rsat,vsat,&xscvel_geo,&yscvel_geo,&zscvel_geo);

// Coordinate transformation from geocentric to s/c velocity
// Note that no translation is used because we are dealing only with
// directions.
CoordinateSwitch geo_to_scvel(xscvel_geo,yscvel_geo,zscvel_geo);
//geo_to_scvel.Show("beam_look: geo_to_scvel");

// Coordinate transformation from s/c velocity to s/c body
CoordinateSwitch scvel_to_scbody(sc_att);
//scvel_to_scbody.Show("beam_look: scvel_to_scbody");

// Coordinate transformation from s/c body to antenna frame
CoordinateSwitch scbody_to_ant(ant_att);
//scbody_to_ant.Show("beam_look: scbody_to_ant");

// Coordinate transformation from antenna frame to beam frame
CoordinateSwitch ant_to_beam(beam_att);

// rlook is a vector from the s/c to the ground target (in geocentric frame)
Vector3 rlook = rground - rsat;
//rlook.Show("beam_look: rlook");

// Apply coordinate transformations to put rlook in the beam frame.

Vector3 rlook_scvel = geo_to_scvel.Forward(rlook);
Vector3 rlook_scbody = scvel_to_scbody.Forward(rlook_scvel);
Vector3 rlook_ant = scbody_to_ant.Forward(rlook_scbody);
Vector3 rlook_beam = ant_to_beam.Forward(rlook_ant);
//rlook_scvel.Show("beam_look: rlook_scvel");
//rlook_scbody.Show("beam_look: rlook_scbody");
//rlook_ant.Show("beam_look: rlook_ant");

rlook_beam.Scale(1.0);
//rlook_ant.Show("beam_look: rlook_ant");
return(rlook_beam);

}

//
// earth_intercept
//
// This function computes the ground intercept point for a given s/c
// and antenna state.
// The algorithm is the same as that used by the NSCAT ATB routine Locate.
// Spacecraft attitude is assumed to use a GEOCENTRIC reference.
//
// Inputs:
//   rsat = s/c position vector (rotating geocentric frame)
//   vsat = s/c intertial velocity (rotating geocentric frame)
//   sc_att = s/c body attitude (roll,pitch,yaw) with respect to the
//            frame defined by the orbit plane (just like NSCAT)
//   ant_att = antenna attitude (roll,pitch,yaw) with respect to the
//             s/c body frame
//   beam_att = beam attitude (roll,pitch,yaw) with respect to the
//             antenna frame
//   rlook_ant = unit vector in antenna frame pointed in the desired
//               look direction.
//        All angle inputs are in radians.
//
// Return Value:
//   The position vector of the ground intercept point in geocentric
//   coordinates (km).
//

EarthPosition earth_intercept(EarthPosition rsat, Vector3 vsat,
	              Attitude sc_att, Attitude ant_att, Attitude beam_att,
			      Vector3 rlook_beam)

{

//
// Transform rlook_beam from the beam frame to
// the geocentric frame.
//

// Spacecraft velocity frame unit vectors (in geocentric frame).
Vector3 xscvel_geo;
Vector3 yscvel_geo;
Vector3 zscvel_geo;
velocity_frame(rsat,vsat,&xscvel_geo,&yscvel_geo,&zscvel_geo);

// Coordinate transformation from geocentric to s/c velocity
// Note that no translation is used because we are dealing only with
// directions.
CoordinateSwitch geo_to_scvel(xscvel_geo,yscvel_geo,zscvel_geo);
//geo_to_scvel.Show("earth_intercept: geo_to_scvel");

// Coordinate transformation from s/c velocity to s/c body
CoordinateSwitch scvel_to_scbody(sc_att);
//scvel_to_scbody.Show("earth_intercept: scvel_to_scbody");

// Coordinate transformation from s/c body to antenna frame
CoordinateSwitch scbody_to_ant(ant_att);
//scbody_to_ant.Show("earth_intercept: scbody_to_ant");

// Coordinate transformation from antenna frame to beam frame
CoordinateSwitch ant_to_beam(beam_att);

// Apply coordinate transformations to put rlook_beam in the geocentric
// frame.

Vector3 rlook_ant = ant_to_beam.Backward(rlook_beam);
Vector3 rlook_scbody = scbody_to_ant.Backward(rlook_ant);
Vector3 rlook_scvel = scvel_to_scbody.Backward(rlook_scbody);
Vector3 rlook_geo = geo_to_scvel.Backward(rlook_scvel);
//rlook_scbody.Show("earth_intercept: rlook_scbody");
//rlook_scvel.Show("earth_intercept: rlook_scvel");
//rlook_geo.Show("earth_intercept: rlook_geo");

rlook_geo.Scale(1.0);
//rlook_geo.Show("earth_intercept: rlook_geo");

Vector3 v1 = rlook_geo * rlook_geo;
Vector3 v2 = rsat * rlook_geo;
Vector3 v3 = rsat * rsat;

double efactor = 1.0 - eccentricity_earth*eccentricity_earth;
double C1 = v1.get(0) + v1.get(1) + v1.get(2)/efactor;
double C2 = v2.get(0) + v2.get(1) + v2.get(2)/efactor;
double C3 = v3.get(0) + v3.get(1) + v3.get(2)/efactor -
            r1_earth*r1_earth;

// Quadratic solution for the slant range S.

double discriminate = 4*C2*C2 - 4*C1*C3;

if (discriminate < 0)
  {
  printf("Warning: earth_intercept did not find an intercept\n");
  EarthPosition rground(0,0,0,EarthPosition::RECTANGULAR);
  return(rground);
  }

double righthalf = sqrt(discriminate)/(2*C1);
double s1 = -C2/C1 + righthalf;
double s2 = -C2/C1 - righthalf;
//printf("s1 = %g, s2 = %g\n",s1,s2);
double S;
if ((s1 > 0) && (s2 > 0))
  {	// both positive, so choose the smaller one (on this side of the earth)
  if (s1 > s2) S = s2; else S = s1;
  }
else if ((s1 < 0) && (s2 < 0))
  {	// both negative, something went wrong
  printf("Error: computed negative slant range\n");
  exit(-1);
  }
else if (s1 > 0)
  {
  S = s1;	// choose the positive root
  }
else
  {
  S = s2;	// choose the positive root
  }

//printf("slant range = %g\n",S);
//Vector3 rlook_geo_new = rlook_geo*S;
//rlook_geo_new.Show("earth_intercept: rlook_geo_new");
EarthPosition rground = rsat + rlook_geo*S;
//rground.Show("earth_intercept: rground");

return(rground);

}

//-----------------//
// earth_intercept //
//-----------------//

EarthPosition
earth_intercept(
	EarthPosition	rsat,
	Vector3			rlook_geo)
{
	rlook_geo.Scale(1.0);

	Vector3 v1 = rlook_geo * rlook_geo;
	Vector3 v2 = rsat * rlook_geo;
	Vector3 v3 = rsat * rsat;

	double efactor = 1.0 - eccentricity_earth * eccentricity_earth;
	double c1 = v1.get(0) + v1.get(1) + v1.get(2) / efactor;
	double c2 = v2.get(0) + v2.get(1) + v2.get(2) / efactor;
	double c3 = v3.get(0) + v3.get(1) + v3.get(2) / efactor -
				r1_earth * r1_earth;

	//------------------------------//
	// solve for the slant range, s //
	//------------------------------//

	double discrim = 4.0 * (c2 * c2 - c1 * c3);
	if (discrim < 0.0)
	{
		fprintf(stderr,
			"ERROR: earth_intercept did not find an earth intercept!\n");
		exit(1);
	}
	double righthalf = sqrt(discrim) / (2.0 * c1);
	double s1 = -c2 / c1 + righthalf;
	double s2 = -c2 / c1 - righthalf;
	double s;
	if (s1 > 0.0 && s2 > 0.0)
	{
		// both positive, choose smaller one (this side of earth)
		s = (s1 < s2) ? s1 : s2;
	}
	else if (s1 < 0.0 && s2 < 0.0)
	{
		// both negative, error
		fprintf(stderr,
			"ERROR: earth_intercept calculated negative slant range!\n");
		exit(1);
	}
	else if (s1 > 0.0)
		s = s1;
	else
		s = s2;

	//------------------------------//
	// calculate the earth location //
	//------------------------------//

	EarthPosition rground = rsat + rlook_geo * s;
	return(rground);
}
