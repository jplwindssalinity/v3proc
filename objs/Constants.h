//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

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
static const double speed_light_kps = 2.99792458e5;
// Boltzman Constant in J/K = W/Hz/K
static const double bK = 1.3805e-23;

// conversion factors
#define KHZ_PER_MS_TO_HZ_PER_S	1.0E6
#define KHZ_TO_HZ				1.0E3
#define GHZ_TO_HZ				1.0E9
#define US_TO_S					1.0E-6
#define MS_TO_S					1.0E-3
#define S_TO_MS					1000.0

//
// Some fixed constants describing the Earth.
//

// semi-major/minor axes (r1_earth, r2_earth) are in km.
static const double r1_earth = 6378.1363;
static const double r2_earth = 6356.751600562937;
static const double eccentricity_earth = 8.1819221455523210E-02;

// premultiply
static const double r1_earth_2 = r1_earth * r1_earth;
static const double e2 = eccentricity_earth * eccentricity_earth;

// earth flatness
static const double flat = 1.0 - sqrt(1.0 - e2);

// sidereal rotation rate (rad/sec)
static const double w_earth = M_PI / 180 * 4.1780746e-3;


//----------------------//
// more Earth constants //
//----------------------//

static const double xmu = 3.986032e5;	// earth mass (GM) km3/sec2
static const double rj2 = 1.08260e-3;	// earth gravitational harmonic
static const double rm = 6.3781778e3;	// equatorial radius of earth
static const double wa_deg = 4.1780746e-3;	// earth rotation rate
static const double wa = wa_deg * dtr;	// rot. rate in radians

//---------------------------------//
// golden section search constants //
//---------------------------------//

static const double golden_c = (3.0 - sqrt(5.0)) / 2.0;
static const double golden_r = 1.0 - golden_c;

#endif
