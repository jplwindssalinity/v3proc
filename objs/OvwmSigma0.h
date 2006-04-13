//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef OVWM_SIGMA0_H
#define OVWM_SIGMA0_H

static const char rcs_id_ovwm_sigma0_h[] =
	"@(#) $Id$";

#include "CoordinateSwitch.h"
#include "Spacecraft.h"
#include "Distributions.h"
#include "Ovwm.h"

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
        Ovwm* ovwm, Meas* meas, double* X);
int radar_X_PtGr(CoordinateSwitch* gc_to_antenna, Spacecraft* spacecraft,
        Ovwm* ovwm, Meas* meas, float PtGr, double* X);

//======================================================================
// Function
//		sigma0_to_Esn_noise
//
// DESCRIPTION
//		This function computes the signal + noise power received in
//		the noise measurement.
//======================================================================

int  sigma0_to_Esn_noise(Ovwm* ovwm, MeasSpot* spot, int sim_kpc_flag,
         float* Pn);

int radar_Xcal(Ovwm* ovwm, float Es_cal, double* Xcal);
double true_Es_cal(Ovwm* ovwm);    
int PtGr_to_Esn(TimeCorrelatedGaussian*  ptgrNoise, Ovwm* ovwm,
                int sim_kpri_flag,
                float* Esn_echo_cal, float* Esn_noise_cal);
int make_load_measurements(Ovwm* ovwm, float* En_echo_load,
                           float* En_noise_load);
#endif
