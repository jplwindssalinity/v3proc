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

//
// Some fixed constants describing the Earth.
//

// semi-major/minor axes (r1_earth, r2_earth) are in meters.
static const double r1_earth = 6378136.3;
static const double r2_earth = 6356751.600562937;
static const double eccentricity_earth = 8.1819221455523210E-02;

#endif
