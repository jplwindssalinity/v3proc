//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef SIGMA0_H
#define SIGMA0_H

static const char rcs_id_sigma0_h[] =
	"@(#) $Id$";

#include "Spacecraft.h"
#include "Instrument.h"
#include "Meas.h"

//======================================================================
// Functions
//		radar_X sigma0_to_Pr Pr_to_sigma0
//======================================================================

//=======================================================================
// Function
//		radar_X
//
// This function computes the combined factor in the radar equation called X.
// X contains the power, geometric, gain, loss, and constant factors that
// relate the received signal power with the mean cross-section of the target.
// The K-factor correction term is not included.
// ie., Pr(signal) = K*X*sigma0.
//=======================================================================

int radar_X(CoordinateSwitch* gc_to_antenna, Spacecraft* spacecraft,
		Instrument* instrument, Meas* meas, double* X);


//======================================================================
// Function
//		sigma0_to_Pr
//
// DESCRIPTION
//		The receive_power function computes the power received for a
//		given instrument state and average sigma0.
//======================================================================

int sigma0_to_Pr(CoordinateSwitch* gc_to_antenna, Spacecraft* spacecraft,
		Instrument* instrument, Meas* meas, float Kfactor, float sigma0,
		float* Pr);


//=========================================================================
// Function
//		Pr_to_sigma0
//
// The Pr_to_sigma0 function computes sigma0 from a signal+noise and noise power
// measurement for a given instrument state.
//
//=========================================================================

int Pr_to_sigma0(CoordinateSwitch* gc_to_antenna, Spacecraft* spacecraft,
		Instrument* instrument, Meas* meas, float Kfactor, float Pr,
		float* sigma0);


#endif
