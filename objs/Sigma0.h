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
#include "Wind.h"

//======================================================================
// Functions
//		radar_X, sigma0_to_Psn, Pnoise, Pr_to_sigma0, GetKpm, composite
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
//		sigma0_to_Esn_slice
//
// DESCRIPTION
//		This function computes the signal + noise power received in
//		a slice.
//======================================================================

int sigma0_to_Esn_slice(CoordinateSwitch* gc_to_antenna, Spacecraft* spacecraft,
		Instrument* instrument, Meas* meas, float Kfactor, float sigma0,
		float* Esn, float* XK);

//======================================================================
// Function
//		sigma0_to_Esn_noise
//
// DESCRIPTION
//		This function computes the signal + noise power received in
//		the noise measurement.
//======================================================================

int sigma0_to_Esn_noise(Instrument* instrument, MeasSpot* spot, float* Pn);

//=========================================================================
// Function
//		Er_to_sigma0
//
// The Er_to_sigma0 function computes sigma0 from a signal+noise and noise power
// measurement for a given instrument state.
//
//=========================================================================

int Er_to_sigma0(CoordinateSwitch* gc_to_antenna, Spacecraft* spacecraft,
		Instrument* instrument, Meas* meas, float Kfactor, float Psn,
		float sumPsn, float Pn, float PtGr);

int composite(MeasList* input_measList, Meas* output_meas);

#endif

