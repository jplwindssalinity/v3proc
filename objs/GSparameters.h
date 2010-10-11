//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef GS_PARAMETERS_H
#define GS_PARAMETERS_H

static const char rcs_id_gs_parameters_h[] =
    "@(#) $Id$";

#define WIND_START_SPEED       8.0
#define LOWER_SPEED_BOUND      0

//#define upper_speed_bound     50
#define UPPER_SPEED_BOUND      100  // This is a HACK.
#define MIN_INCIDENCE_INDEX    40
#define MAX_INCIDENCE_INDEX    59
#define LOWER_AZIMUTH_BOUND    0
#define UPPER_AZIMUTH_BOUND    90
#define WIND_MAX_SOLUTIONS     10
#define WIND_SPEED_BAND        4
#define WIND_SPEED_INTV_INIT   0.2
#define WIND_SPEED_INTV_OPTI   0.1
#define WIND_DIR_INTV_INIT     8.0
#define WIND_DIR_INTV_OPTI     1.0
#define WIND_VARIANCE_LIMIT    1.0e-9
#define MAX_DIR_SAMPLES        800

#define WIND_SPEED_DELTA       0.5
#define WIND_DIR_DELTA         5.0
#define WIND_LIKELIHOOD_DELTA  0.5

#endif
