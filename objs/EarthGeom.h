//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef EARTH_GEOM_H
#define EARTH_GEOM_H

static const char rcs_id_earth_geom_h[] =
    "@(#) $Id$";

#include "Constants.h"
#include "Vect.h"

static const double f2 = flat * (2.0 - flat);

int   gc_att_to_gd_att(const Vect& sc_pos, const Vect& sc_vel,
          double gc_att[3], double gd_att[3]);
int   get_alt_lon_gdlat(const Vect& vect, double* alt, double* lon,
          double* gd_lat);
int   set_alt_lon_gdlat(double alt, double lon, double gd_lat, Vect* vect);
int   set_alt_lon_gclat(double alt, double lon, double gc_lat, Vect* vect);

#endif
