/*********************************************************************
 *
 * ORIGINAL AUTHOR: Thomas Werne
 * COMPANY: Jet Propulsion Laboratory
 * VERSION:
 *
 * File Name:     NetCDF.C
 * Creation Date: 11 Apr 2012
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 * Copyright 2009-2012, by the California Institute of Technology.
 * ALL RIGHTS RESERVED.  United States Government Sponsorship
 * acknowledged. Any commercial use must be negotiated with the Office
 * of Technology Transfer at the California Institute of Technology.
 *
 * This software may be subject to U.S. export control laws and
 * regulations.  By accepting this document, the user agrees to comply
 * with all U.S. export laws and regulations.  User has the
 * responsibility to obtain export licenses, or other export authority
 * as may be required before exporting such information to foreign
 * countries or providing access to foreign persons.
 ********************************************************************/

static const char rcs_id[] =
    "@(#) $Id$";

#include <netcdf.h>
#include <math.h>
#include <string.h>

#include "Constants.h"
#include "NetCDF.h"

using namespace std;

// Base class implementation specifics
int NC_Attribute::Write(int ncid, int varid) {
    return WriteAttr(ncid, varid);
}

// String_NC_Attribute implementation specifics
String_NC_Attribute::String_NC_Attribute(const char *name, const char *string):
    NC_Attribute(name, strlen(string)) {

    p = strdup(string);
}

String_NC_Attribute::~String_NC_Attribute() {
    FREE(p);
}

int String_NC_Attribute::WriteAttr(int ncid, int varid) {
    return nc_put_att_text(ncid, varid, name.data(), length, p);
}

// Float NC_Attribute implementation specifics

Float_NC_Attribute::Float_NC_Attribute(const char *name, const float *vals,
        size_t length): NC_Attribute(name, length) {

    p = (typeof p)malloc(length*sizeof(*p));

    memcpy(p, vals, length*sizeof(*p));
}

Float_NC_Attribute::Float_NC_Attribute(const char *name, const float val): 
    NC_Attribute(name, 1) {

    p = (typeof p)malloc(sizeof(*p));
    *p = val;
}

Float_NC_Attribute::~Float_NC_Attribute() {
    FREE(p);
}

int Float_NC_Attribute::WriteAttr(int ncid, int varid) {
    return nc_put_att_float(ncid, varid, name.data(), NC_FLOAT, length, p);
}

void bin_to_latlon(int at_ind, int ct_ind,
        const latlon_config *config, float *lat, float *lon) {

    /* Utilizes e2, r1_earth from Constants.h */
    const static double P1 = 60*1440.0f;

    const static double P2 = config->rev_period;
    const double inc = DEG_TO_RAD(config->inclination);
    const int    r_n_xt_bins = config->xt_steps;
    const double at_res = config->at_res;
    const double xt_res = config->xt_res;

    const double lambda_0 = DEG_TO_RAD(config->lambda_0);

    const double r_n_at_bins = 1624.0 * 25.0 / at_res;
    const double atrack_bin_const = two_pi/r_n_at_bins;
    const double xtrack_bin_const = xt_res/r1_earth;

    double lambda, lambda_t, lambda_pp;
    double phi, phi_pp;
    double Q, U, V, V1, V2;

    double sin_phi_pp, sin_lambda_pp;
    double sin_phi_pp2, sin_lambda_pp2;
    double sini, cosi;

    sini = sinf(inc);
    cosi = cosf(inc);

    lambda_pp = (at_ind + 0.5)*atrack_bin_const - pi_over_two;
    phi_pp = -(ct_ind - (r_n_xt_bins/2 - 0.5))*xtrack_bin_const;

    sin_phi_pp = sinf(phi_pp);
    sin_phi_pp2 = sin_phi_pp*sin_phi_pp;
    sin_lambda_pp = sinf(lambda_pp);
    sin_lambda_pp2 = sin_lambda_pp*sin_lambda_pp;
    
    Q = e2*sini*sini/(1 - e2);
    U = e2*cosi*cosi/(1 - e2);

    V1 = (1 - sin_phi_pp2/(1 - e2))*cosi*sin_lambda_pp;
    V2 = (sini*sin_phi_pp*sqrtf((1 + Q*sin_lambda_pp2)*(1 - 
                    sin_phi_pp2) - U*sin_phi_pp2));

    V = (V1 - V2)/(1 - sin_phi_pp2*(1 + U));

    lambda_t = atan2f(V, cosf(lambda_pp));
    lambda = lambda_t - (P2/P1)*lambda_pp + lambda_0;

    lambda += (lambda < 0)       ?  two_pi : 
              (lambda >= two_pi) ? -two_pi :
                                    0.0f;
    phi = atanf((tanf(lambda_pp)*cosf(lambda_t) - 
                cosf(inc)*sinf(lambda_t))/((1 - e2)*
                sinf(inc)));

    *lon = RAD_TO_DEG(lambda);
    *lat = RAD_TO_DEG(phi);
}
