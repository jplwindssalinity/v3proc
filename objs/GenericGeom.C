//==============================================================//
// Copyright (C) 1997-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//
// This file contains functions useful for geometric calculations
// for a particular satellite location and ground target.

static const char rcs_id_genericgeom_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "GenericGeom.h"
#include "CoordinateSwitch.h"
#include "Constants.h"

// this global variable is used to determine whether the pointing
// is defined to be geocentric or geodetic. geodetic is the default.

int (*g_velocity_frame)(EarthPosition rsat, Vector3 vsat,
			Vector3 *xscvel_geo, Vector3 *yscvel_geo, Vector3 *zscvel_geo, 
			double t) =
    velocity_frame_geodetic;

double g_inertial_lat=0;
double g_inertial_lon=0;
double g_inertial_sctime=0;

//---------------------------//
// velocity_frame_geocentric //
//---------------------------//
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

int
velocity_frame_geocentric(
    EarthPosition  rsat,
    Vector3        vsat,
    Vector3*       xscvel_geo,
    Vector3*       yscvel_geo,
    Vector3*       zscvel_geo,
    double t)
{
    // Geocentric definition of the z-axis
    *zscvel_geo = -rsat;
    // y-axis is perpendicular to the orbit plane
    *yscvel_geo = *zscvel_geo & vsat;
    // x-axis close to, but not the same as the velocity vector
    *xscvel_geo = *yscvel_geo & *zscvel_geo;

    return(1);
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

int
velocity_frame_geodetic(
    EarthPosition  rsat,
    Vector3        vsat,
    Vector3*       xscvel_geo,
    Vector3*       yscvel_geo,
    Vector3*       zscvel_geo,
    double t)
{
    // Geodetic definition of the z-axis
    double alt, nadir_gd_lat, nadir_lon;
    if (! rsat.GetAltLonGDLat(&alt, &nadir_lon, &nadir_gd_lat))
    {
        fprintf(stderr,
        "Error: velocity_frame_geodetic could not convert input position\n");
        exit(1);
    }
    EarthPosition rnadir;
    rnadir.SetAltLonGDLat(0.0, nadir_lon, nadir_gd_lat);
    *zscvel_geo = rnadir - rsat;

    // y-axis is perpendicular to the orbit plane
    *yscvel_geo = *zscvel_geo & vsat;
    // x-axis close to, but not the same as the velocity vector
    *xscvel_geo = *yscvel_geo & *zscvel_geo;

    return(1);
}



// Sept 16 2009, Bryan W. Stiles
// velocity_frame_inertial
//
// This function determines the spacecraft velocity frame (also called
// the local coordinate system) given the s/c position and velocity.
// The attitude uses a INERTIAL reference meaning that the z-axis points in constant
// direction in inertial space
//
// Inputs:
//  rsat = geocentric position of the spacecraft (km)
//  vsat = geocentric inertial velocity of the spacecraft (km/s)
//  xscvel_inert = pointer to a Vector3 object to hold the x-axis unit vector of
//               the velocity coordinate frame represented in the geocentric
//               frame.
//  yscvel_inert = same thing for the y-axis.
//  zscvel_inert = same thing for the z-axis.
//  t = time since epoch (epoch=g_inertial_sctime)
// Action:
//  The Vector3 objects pointed to by x,y,zscvel_geo are filled with the
//  corresponding unit vectors.
//

int
velocity_frame_inertial(
    EarthPosition  rsat,
    Vector3        vsat,
    Vector3*       xscvel_inert,
    Vector3*       yscvel_inert,
    Vector3*       zscvel_inert,
    double t)
{

    // determine longitude of inertial pointing vector at time t
    double earth_spinrate=-w_earth; // negative sign comes form the fact that
    // we are keeping track of an inertial vector in rotaing coordinates
    // rather than the other way around.                            
    double current_lon=g_inertial_lon+t*earth_spinrate;
    // Inertial definition of the z-axis
    EarthPosition rinert;
    rinert.SetAltLonGCLat(0.0, current_lon, g_inertial_lat);
    *zscvel_inert = -rinert;

    // y-axis is perpendicular to the orbit plane
    // This is a convention. The simulation treats the spacecraft as if
    // it were not rotating but the antenna is. This would give the same
    // measurements as a spinning spacecraft with an inertial spin axis.
    // This distinction of which of the two is spinning does not impact
    // the measurements, so we ignore it.

    *yscvel_inert = *zscvel_inert & vsat;
    // x-axis completes the set
    *xscvel_inert = *yscvel_inert & *zscvel_inert;

    return(1);
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
////////// THIS FUNCTION IS OBSOLETE
//  Bryan Stiles Commented it out on Sept 16 2009
/***** 

Vector3
    beam_look(
    EarthPosition  rsat,
    Vector3        vsat,
    EarthPosition  rground,
    Attitude       sc_att,
    Attitude       ant_att,
    Attitude       beam_att)
{
    // Spacecraft velocity frame unit vectors (in geocentric frame).
    Vector3 xscvel_geo;
    Vector3 yscvel_geo;
    Vector3 zscvel_geo;
    g_velocity_frame(rsat, vsat, &xscvel_geo, &yscvel_geo, &zscvel_geo,0);

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
****/

//-----------------//
// earth_intercept //
//-----------------//
//----------------------------------//
// Return Values                    //
// 0: Negative Slant Range Error    //
// 1: No Error                      //
// 2: Off the Earth Error           //
//----------------------------------//
int
earth_intercept(
    EarthPosition   rsat,
    Vector3         rlook_geo,
    EarthPosition*  intercept)
{
    rlook_geo.Scale(1.0);
//    rlook_geo.Show("earth_intercept: rlook_geo");

    Vector3 v1 = rlook_geo * rlook_geo;
    Vector3 v2 = rsat * rlook_geo;
    Vector3 v3 = rsat * rsat;

    double efactor = 1.0 - e2;
    double c1 = v1.Get(0) + v1.Get(1) + v1.Get(2) / efactor;
    double c2 = v2.Get(0) + v2.Get(1) + v2.Get(2) / efactor;
    double c3 = v3.Get(0) + v3.Get(1) + v3.Get(2) / efactor - r1_earth_2;

    //------------------------------//
    // solve for the slant range, s //
    //------------------------------//

    double discrim = 4.0 * (c2 * c2 - c1 * c3);
    if (discrim < 0.0)
    {
        fprintf(stderr,
            "ERROR: earth_intercept did not find an earth intercept!\n");
        return(2);
    }
    double righthalf = sqrt(discrim) / (2.0 * c1);
    double frac = -c2 / c1;
    double s1 = frac + righthalf;
    double s2 = frac - righthalf;

//    printf("s1 = %g, s2 = %g\n",s1,s2);
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
        return(0);
    }
    else if (s1 > 0.0)
        s = s1;
    else
        s = s2;

    //------------------------------//
    // calculate the earth location //
    //------------------------------//

    //printf("slant range = %g\n",s);
    //Vector3 rlook_geo_new = rlook_geo*s;
    //rlook_geo_new.Show("earth_intercept: rlook_geo_new");

    *intercept = rsat + rlook_geo * s;
    return(1);
}

//------//
// elem //
//------//
// Compute Keplerian elements from an EarthPosition and velocity vector
//
// Inputs:
// r = position of satellite (in rotating geocentric frame) (km)
// v = velocity of satellite (instantaneous inertail reference frame) (km/s)
//
// Orbit Elements
// *a = semi-major axis (km)
// *e = eccentricity
// *i = inclination (rad)
// *w = argument of perigee (rad)
// *RA = right ascension of (ie., longitude) of the ascending node
// *M = mean anomaly (rad)

int
elem(EarthPosition    r,
    Vector3           v,
    double*           a,
    double*           e,
    double*           i,
    double*           w,
    double*           RA,
    double*           M,
    double*           P)
{
    double R = r.Magnitude();
    double V = v.Magnitude();
    Vector3 rhat = r;
    Vector3 vhat = v;
    rhat.Scale(1.0);
    vhat.Scale(1.0);

    //---------------------------//
    // Orbit plane normal vector //
    //---------------------------//

    Vector3 Nhat = (r & v);
    Nhat.Scale(1.0);

    //-------------------//
    // Geocentric z axis //
    //-------------------//

    Vector3 zhat(0,0,1);

    //---------------------//
    // Compute inclination //
    //---------------------//

    *i = acos(Nhat % zhat);

    //-------------------------//
    // Compute right ascension //
    //-------------------------//

    *RA = atan2(-Nhat.Get(1),Nhat.Get(2));

    //------------------------------------------//
    // Compute vector toward the ascending node //
    //------------------------------------------//

    Vector3 Ohat = zhat & Nhat;

    //-------------------------//
    // Compute semi-major axis //
    //-------------------------//

    *a = 1.0 / (2.0/R - V*V/xmu);

    //----------------------//
    // Compute orbit period //
    //----------------------//

    *P = sqrt(4*pi*pi*(*a)*(*a)*(*a)/xmu);

    //--------------------------------------------//
    // Compute eccentricity and true anomaly (nu) //
    //--------------------------------------------//

    double beta = pi/2 - acos(rhat % vhat);
    double tmp = R*V*V/xmu;
    *e = sqrt((tmp-1.0)*(tmp-1.0)*cos(beta)*cos(beta) + sin(beta)*sin(beta));
    double nu = atan2(tmp*sin(beta)*cos(beta), tmp*cos(beta)*cos(beta) - 1.0);

    //-----------------------------//
    // Compute argument of perigee //
    //-----------------------------//

    *w = acos(Ohat % rhat) - nu;

    //------------------------------------------------//
    // Compute eccentric anomaly (E) and mean anomaly //
    //------------------------------------------------//

    double E = 2.0*atan(sqrt((1-(*e))/(1+(*e))) * tan(nu/2));
    *M = E - (*e)*sin(E);

    return(1);
}
