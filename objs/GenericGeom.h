//==============================================================//
// Copyright (C) 1997-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef GENERICGEOM_H
#define GENERICGEOM_H

#include "EarthPosition.h"
#include "CoordinateSwitch.h"

// this was the old way of doing this and required recompilation :-(
// #define velocity_frame  velocity_frame_geodetic

static const char rcs_id_genericgeom_h[] =
    "@(#) $Id$";

//-----------------//
// global variables //
//-----------------//
// velocity frame function

extern int (*g_velocity_frame)(EarthPosition rsat, Vector3 vsat,
			       Vector3 *xscvel_geo, Vector3 *yscvel_geo, Vector3 *zscvel_geo, 
			       double t);

// coordinates for inertial spin axis if inertial pointing is used
extern double g_inertial_lat;
extern double g_inertial_lon;
extern double g_inertial_sctime;

//======================================================================
// Functions
//    velocity_frame_geocentric
//    velocity_frame_geodetic
//    velocity_frame_inertial
//    beam_look
//    earth_intercept
//======================================================================

//======================================================================
// Function
//    velocity_frame_geocentric
//
// DESCRIPTION
//    The velocity_frame_geocentric function computes the axial unit
//    vectors that define the s/c velocity frame (or local coordinate
//    system) from the s/c position and velocity.
//    A GEOCENTRIC convention for the attitude reference is used.
//    --BWS Sept 16 2009
//    Time parameter added for consistency with velocity_frame_inertial.
//    (same pointer needs to be used for all three routines)
//======================================================================

int  velocity_frame_geocentric(EarthPosition rsat, Vector3 vsat,
			       Vector3 *xscvel_geo, Vector3 *yscvel_geo, Vector3 *zscvel_geo,
			       double t);

//======================================================================
// Function
//    velocity_frame_geodetic
//
// DESCRIPTION
//    The velocity_frame_geodetic function computes the axial unit
//    vectors that define the s/c velocity frame (or local coordinate
//    system) from the s/c position and velocity.
//    A GEODETIC convention for the attitude reference is used.
//    --BWS Sept 16 2009
//    Time parameter added for consistency with velocity_frame_inertial.
//    (same pointer needs to be used for all three routines)
//======================================================================

int  velocity_frame_geodetic(EarthPosition rsat, Vector3 vsat,
			     Vector3 *xscvel_geo, Vector3 *yscvel_geo, Vector3 *zscvel_geo, 
			     double t);


//======================================================================
// Function
//    velocity_frame_inertial
//
// DESCRIPTION
//    The velocity_frame_inertial function computes the axial unit
//    vectors that define the s/c velocity frame (or local coordinate
//    system) from the s/c position and velocity and the time since epoch.
//    The case in which the coordinate system z-axis is oriented along an inertial
//    vector is used. Routine added Sept 16 2009 by Bryan Stiles
//======================================================================

int  velocity_frame_inertial(EarthPosition rsat, Vector3 vsat,
			     Vector3 *xscvel_inert, Vector3 *yscvel_inert, Vector3 *zscvel_inert,
			     double t);

//======================================================================
// Function
//   beam_look
//
// DESCRIPTION
//   The beam_look function computes a unit vector in the
//   beam frame that is pointed at a particular ground target.
//   A GEOCENTRIC convention for the attitude reference is used.
//======================================================================

// Obsolete commented out on Sept 16 2009 by Bryan Stiles
// Vector3 beam_look(EarthPosition rsat, Vector3 vsat, EarthPosition rground,
//     Attitude sc_att, Attitude ant_att, Attitude beam_att);

//======================================================================
// Function
//    earth_intercept
//
// Description
//    The earth_intercept function computes a position vector for
//    the intercept point on the earth's surface for a particular
//    look vector (specified in the earth geocentric frame).
//======================================================================

int  earth_intercept(EarthPosition rsat, Vector3 rlook_geo,
         EarthPosition* intercept);

int  elem(EarthPosition r, Vector3 v, double* a, double* e, double* i,
         double* w, double* RA, double* M, double* P);

#endif
