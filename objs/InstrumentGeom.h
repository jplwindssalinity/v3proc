//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef INSTRUMENTGEOM_H
#define INSTRUMENTGEOM_H

static const char rcs_id_instrumentgeom_h[] =
    "@(#) $Id$";

#include "EarthPosition.h"
#include "CoordinateSwitch.h"
#include "Ephemeris.h"
#include "Spacecraft.h"
#include "LonLat.h"
#include "Matrix3.h"
#include "Matrix3.h"
#include "Meas.h"
#include "Qscat.h"

//======================================================================
// DESCRIPTION
//    Higher level geometry functions.  Typically instrument related.
//======================================================================

CoordinateSwitch  AntennaFrameToGC(OrbitState* orbit_state,
                      Attitude* attitude, Antenna* antenna,
                      double azimuth_angle);

int  GetPeakSpatialResponse(Beam* beam, double round_trip_time,
         double azimuth_rate, double* look, double* azim,
         int ignore_range = 0);

double  NegativeSpatialResponse(double* x, void* ptr);

/*
int  RttToIdealRxDelay(Qscat* qscat, double rtt);
int  IdealCommandedDopplerForRange(Spacecraft* spacecraft, float offset);
*/

#endif
