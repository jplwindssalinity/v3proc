//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_earth_geom_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include "EarthGeom.h"
#include "Vect.h"
#include "Transform.h"

int
gc_att_to_gd_att(
    const Vect&  sc_pos,
    const Vect&  sc_vel,
    double       gc_att[3],
    double       gd_att[3])
{
    //--------------------------//
    // geocentric to spacecraft //
    //--------------------------//
    // The spacecraft y-axis is set to approximately equal to the velocity
    // vector. Thus, a pitch about the x-axis needs a negative sign to
    // match the ground system coordinates which have the x-axis
    // approximately equal to the velocity vector.

    Transform t_sc_gc;
    t_sc_gc.Rotate(0, -gc_att[1]);   // pitch (2) about the x-axis
    t_sc_gc.Rotate(1, gc_att[0]);   // roll  (1) about the y-axis
    t_sc_gc.Rotate(2, gc_att[2]);   // yaw   (3) about the z-axis

    //------------------------//
    // inertial to geocentric //
    //------------------------//

    Vect v_center(0.0, 0.0, 0.0);
    Vect x_gc, y_gc, z_gc;
    z_gc.Difference(v_center, sc_pos);
    x_gc.Cross(sc_vel, z_gc);
    y_gc.Cross(z_gc, x_gc);
    Transform t_gc_irf(x_gc, y_gc, z_gc, sc_pos);

    //----------------------//
    // geodetic to inertial //
    //----------------------//
    // v_surface is the point on the earth s.t. the vector
    // sc_pos - v_surface is normal to the surface

    Vect v_surface;
    double alt, lon, gd_lat;
    if (! get_alt_lon_gdlat(sc_pos, &alt, &lon, &gd_lat)) {
        return(0);
    }
    if (! set_alt_lon_gdlat(0.0, lon, gd_lat, &v_surface)) {
        return(0);
    }
    Vect x_gd, y_gd, z_gd;
    z_gd.Difference(v_surface, sc_pos);
    x_gd.Cross(sc_vel, z_gd);
    y_gd.Cross(z_gd, x_gd);
    Transform t_gd_irf(x_gd, y_gd, z_gd, sc_pos);
    Transform t_irf_gd;
    if (! t_irf_gd.Inverse(t_gd_irf)) {
        return(0);
    }

    //----------------------------------//
    // calculate spacecraft to geodetic //
    //----------------------------------//

    Transform t_sc_gd(t_irf_gd);
    t_sc_gd.Apply(t_gc_irf);
    t_sc_gd.Apply(t_sc_gc);

/*
    Transform t_gd_sc;
    t_gd_sc.Inverse(t_sc_gd);
*/

    //----------------------//
    // get the euler angles //
    //----------------------//

    t_sc_gd.GetEuler213(gd_att);
    gd_att[1] = -gd_att[1];

    return(1);
}

//-------------------//
// get_alt_lon_gdlat //
//-------------------//
// vect = the input position vector (km)
// alt = altitude above the ellipsoidal earth surface (km)
// lon = east longitude (radians)
// gd_lat = geodetic latitude (radians)

int
get_alt_lon_gdlat(
    const Vect&  vect,
    double*      alt,
    double*      lon,
    double*      gd_lat)
{
    double v0 = vect.FastGet(0);
    double v1 = vect.FastGet(1);
    double v2 = vect.FastGet(2);

    double rho = sqrt(v0*v0 + v1*v1 + v2*v2);
    double r = sqrt(v0*v0 + v1*v1);

    if (rho == 0.0) {
        // center of the earth
        *alt = -r2_earth;
        *lon = 0.0;
        *gd_lat = 0.0;
        return(1);
    }

    // Compute east longitude
    if (v0 == 0.0) {
        // on the 90 -- 270 great circle
        if (v1 > 0.0) {
            *lon = pi_over_two;
        } else {
            *lon = 3.0 * pi_over_two;
        }
    } else {
        *lon = atan2(v1, v0);
        if (*lon < 0.0) {
            *lon += two_pi;
        }
    }

    // Trial values to start the iteration
    *gd_lat = asin(v2 / rho);
    double sinlat = sin(*gd_lat);
    double coslat = cos(*gd_lat);
    *alt = rho - r1_earth * (1.0 - flat*sinlat*sinlat);

    // Iterate to solution (or maximum numer of interations)
    double g0, g1, g2;
    double dr, dz, dalt, dlat;
    double tol = 1.0e-14;

    int maxiter = 10;
    int success = 0;
    for (int i = 0; i < maxiter; i++) {
        sinlat = sin(*gd_lat);
        coslat = cos(*gd_lat);
        g0 = r1_earth / sqrt(1.0 - f2 * sinlat * sinlat);
        g1 = g0 + *alt;
        g2 = g0 * (1.0 - flat) * (1.0 - flat) + *alt;
        dr = r - g1*coslat;
        dz = v2 - g2*sinlat;
        dalt = dr*coslat + dz*sinlat;
        dlat = (dz*coslat - dr*sinlat) / (r1_earth + *alt + dalt);
        *gd_lat += dlat;
        *alt += dalt;
        if ((dlat < tol) && (fabs(dalt)/(r1_earth + *alt) < tol)) {
            success = 1;
            break;
        }
    }

    if (! success) {
        return(0);
    }

    return(1);
}

//-------------------//
// set_alt_lon_gdlat //
//-------------------//
// alt = altitude above the ellipsoidal earth surface (km)
// lon = east longitude (radians)
// gd_lat = geodetic latitude (radians)
// vect = the output position vector (km)

int
set_alt_lon_gdlat(
    double  alt,
    double  lon,
    double  gd_lat,
    Vect*   vect)
{
    // convert geodetic latitude to geocentric latitude
    double gc_lat = atan(tan(gd_lat)
        * (1.0 - eccentricity_earth*eccentricity_earth));
    return(set_alt_lon_gclat(alt, lon, gc_lat, vect));
}

//-------------------//
// set_alt_lon_gclat //
//-------------------//
// alt = altitude above the ellipsoidal earth surface (km)
// lon = east longitude (radians)
// gc_lat = geocentric latitude (radians)
// vect = the output position vector (km)

int
set_alt_lon_gclat(
    double  alt,
    double  lon,
    double  gc_lat,
    Vect*   vect)
{
    double slat = sin(gc_lat);
    double clat = cos(gc_lat);
    double slon = sin(lon);
    double clon = cos(lon);

    // Radius of earth at desired location.
    double radius = r1_earth * (1.0 - flat*slat*slat);

    // Form sea-level position vector
    double v0 = radius*clat*clon;
    double v1 = radius*clat*slon;
    double v2 = radius*slat;

    // Form vector normal to the surface of the ellipsoidal earth at
    // the desired location, with length equal to the desired altitude,
    // and add it to the sealevel position vector to get the final
    // position vector.
    Vect normal(v0 / (r1_earth*r1_earth),
                v1 / (r1_earth*r1_earth),
                v2 / (r2_earth*r2_earth));
    normal.SetMagnitude(alt);

    v0 += normal.FastGet(0);
    v1 += normal.FastGet(1);
    v2 += normal.FastGet(2);

    if (! vect->Set(v0, v1, v2)) {
        return(0);
    }

    return(1);
}
