//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef CONSTANTS_H
#define CONSTANTS_H

static const char rcs_id_constants_h[] =
	"@(#) $Id$";

#include <math.h>

//======================================================================
// DESCRIPTION
//		A bunch of "universal" constants.
//======================================================================

static const double pi = M_PI;
static const double two_pi = 2.0 * M_PI;
static const double dtr = 1.745329252e-2;
static const double rtd = 5.729577951e1;
static const double rpm_to_radps = M_PI / 30.0;
static const double speed_light = 2.99792458e8;
static const double speed_light_kps = 2.99792458e5;

// conversion factors
#define KHZ_PER_MS_TO_HZ_PER_S	1.0E6
#define KHZ_TO_HZ				1.0E3
#define GHZ_TO_HZ				1.0E9
#define US_TO_S					1.0E-6
#define MS_TO_S					1.0E-3

//
// Some fixed constants describing the Earth.
//

// semi-major/minor axes (r1_earth, r2_earth) are in km.
static const double r1_earth = 6378.1363;
static const double r2_earth = 6356.751600562937;
static const double eccentricity_earth = 8.1819221455523210E-02;
// sidereal rotation rate (rad/sec)
static const double w_earth = pi/180*4.1780746e-3;

#endif
