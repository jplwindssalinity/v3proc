/*!
  \file L2BCconstants.h
  \author Ernesto Rodriguez
  \brief Contains the constants (mainly keywords) used in the L2BC code.

  Edited 6/3/12 by A. H. Chau to use qs_l2b_{revnum}_v3_{date}.nc file,
	instead of QS_S2B{revnum}.{date}_S3.L2BC.nc file.

*/

#ifndef _ER_L2BCCONSTANTS_H_
#define _ER_L2BCCONSTANTS_H_

// NETCDF Keyword definitions for unfiltered files

#define L2BC_WVC_QUALITY_FLAG_KW "flags"
#define L2BC_WIND_DIR_KW "retrieved_wind_direction"
#define L2BC_MODEL_WIND_DIR_KW "nudge_wind_direction"
#define L2BC_WIND_SPEED_KW "retrieved_wind_speed"
#define L2BC_MODEL_WIND_SPEED_KW "nudge_wind_speed"
#define L2BC_WVC_EXTENDED_FLAG_KW "eflags"

// NETCDF Keyword definitions for filtered files

#define L2BC_FILTERED_LAT_KW "lat"
#define L2BC_FILTERED_LON_KW "lon"
#define L2BC_FILTERED_WIND_SPEED_KW "wind_speed"
#define L2BC_FILTERED_WIND_DIR_KW "wind_direction"
#define L2BC_FILTERED_WVC_QUALITY_FLAG_KW "eflags"
#define L2BC_FILTERED_STRESS_KW "stress"
#define L2BC_FILTERED_STRESS_DIVERGENCE_KW "stress_divergence"
#define L2BC_FILTERED_STRESS_CURL_KW "stress_curl"
#define L2BC_FILTERED_WIND_DIVERGENCE_KW "wind_divergence"
#define L2BC_FILTERED_WIND_CURL_KW "wind_curl"

// These are constants for the NetCDF file

#define L2BC_FLOAT_FILL_VALUE -9999.

#endif
