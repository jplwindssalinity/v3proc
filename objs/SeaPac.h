//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef SEAPAC_H
#define SEAPAC_H

static const char rcs_id_seapac_h[] =
	"@(#) $Id$";

int  ijbin(double orb_smaj_axis, double orb_eccen, double orb_inclination,
         double long_asc_node, double arg_lat, double nodal_period,
         double meas_lon, double meas_lat, int* ati, int* cti);

int  compute_orbit_elements(double x_pos, double y_pos, double z_pos,
         double x_vel, double y_vel, double z_vel, double* nodal_period,
         double* arg_lat, double* long_asc_node, double* orb_inclination,
         double* orb_smaj_axis, double* orb_eccen);

#endif
