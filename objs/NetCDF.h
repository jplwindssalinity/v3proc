/*********************************************************************
 *
 * ORIGINAL AUTHOR: Thomas Werne
 * COMPANY: Jet Propulsion Laboratory
 * VERSION:
 *
 * File Name:     NetCDF.h
 * Creation Date: 15 Apr 2012
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

#ifndef NETCDF_H
#define NETCDF_H

#include <netcdf.h>

#define RAD_TO_DEG(x) ((x)*180.0f/M_PI)
#define DEG_TO_RAD(x) ((x)*M_PI/180.0f)

#define FREE(x) \
do {            \
    free(x);    \
    x = NULL;   \
} while(0)

#define MALLOC(v, n) (v) = (typeof (v))malloc((n)*sizeof(*(v)))


// A helper function for mapping from C -> NetCDF variable types
template<typename T> nc_type NetCDF_Type();

// Forward declaration of template specialization
template <> nc_type NetCDF_Type<unsigned char>();
template <> nc_type NetCDF_Type<signed char>();
template <> nc_type NetCDF_Type<char>();
template <> nc_type NetCDF_Type<short>();
template <> nc_type NetCDF_Type<int>();
template <> nc_type NetCDF_Type<float>();
template <> nc_type NetCDF_Type<double>();

// Really belongs in a different header
typedef struct {
    float lambda_0;
    float inclination;
    float rev_period;
    int xt_steps;
    double at_res;
    double xt_res;
} latlon_config;

void bin_to_latlon(int at_ind, int ct_ind,
        const latlon_config *config, float *lat, float *lon);

#endif

