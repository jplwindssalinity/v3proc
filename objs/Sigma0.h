//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef SIGMA0_H
#define SIGMA0_H

static const char rcs_id_sigma0_h[] =
	"@(#) $Id$";

#include "CoordinateSwitch.h"
#include "Spacecraft.h"
#include "Distributions.h"
#include "Qscat.h"

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
        Qscat* qscat, Meas* meas, double* X);
int radar_X_PtGr(CoordinateSwitch* gc_to_antenna, Spacecraft* spacecraft,
        Qscat* qscat, Meas* meas, float PtGr, double* X);

//======================================================================
// Function
//		sigma0_to_Esn_noise
//
// DESCRIPTION
//		This function computes the signal + noise power received in
//		the noise measurement.
//======================================================================

int  sigma0_to_Esn_noise(Qscat* qscat, MeasSpot* spot, int sim_kpc_flag,
         float* Pn);

int radar_Xcal(Qscat* qscat, float Es_cal, double* Xcal);
double true_Es_cal(Qscat* qscat);    
int PtGr_to_Esn(TimeCorrelatedGaussian*  ptgrNoise, Qscat* qscat,
                int sim_kpri_flag,
                float* Esn_echo_cal, float* Esn_noise_cal);
int make_load_measurements(Qscat* qscat, float* En_echo_load,
                           float* En_noise_load);
int Er_to_Es(float beta, float Esn_slice, float Esn_echo, float Esn_noise,
             float En_echo_load, float En_noise_load, float q_slice,
             float* Es_slice, float* En_slice);
#endif
