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
//		sigma0_to_Esn_slice
//
// DESCRIPTION
//		This function computes the signal + noise power received in
//		a slice.
//======================================================================

int  sigma0_to_Esn_slice(CoordinateSwitch* gc_to_antenna,
         Spacecraft* spacecraft, Qscat* qscat, Meas* meas, float Kfactor,
         float sigma0, int sim_kpc_flag, float* Esn, float* XK,
         float* true_Es, float* true_En, float* var_esn_slice);

//======================================================================
// Function
//		sigma0_to_Esn_slice_given_X
//
// DESCRIPTION
//		This function computes the signal + noise power received in
//		a slice. It uses X instead of K.
//======================================================================

int  sigma0_to_Esn_slice_given_X(Qscat* qscat, Meas* meas, float X,
         float sigma0, int sim_kpc_flag, float* Esn, float* true_Es,
         float* true_En, float* var_esn_slice);

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

//=========================================================================
// Function
//		Er_to_sigma0
//
// The Er_to_sigma0 function computes sigma0 from a signal+noise and noise power
// measurement for a given instrument state.
//
//=========================================================================

int  Er_to_sigma0(CoordinateSwitch* gc_to_antenna, Spacecraft* spacecraft,
         Qscat* qscat, Meas* meas, float Kfactor, float Psn, float sumPsn,
         float Pn, float PtGr);

//=========================================================================
// Function
//		Er_to_sigma0_given_X
//
// The Er_to_sigma0_given_X function computes sigma0 from a signal+noise
// and noise power
// measurement for a given instrument state using Xfactor rather than Kfactor
//
//=========================================================================

int  Er_to_sigma0_given_X(Qscat* qscat, Meas* meas, float Xfactor, float Psn,
         float sumPsn, float Pn);

int radar_Xcal(Qscat* qscat, float Es_cal, double* Xcal);
    
int PtGr_to_Esn(float PtGr, TimeCorrelatedGaussian*  ptgrNoise, Qscat* qscat,
                int sim_kpri_flag,
                float* Esn_echo_cal, float* Esn_noise_cal);
int make_load_measurements(Qscat* qscat, float* En_echo_load,
                           float* En_noise_load);
int compute_sigma0(Qscat* qscat, Meas* meas, float Xfactor, float Esn_slice,
                   float Esn_echo, float Esn_noise, float En_echo_load,
                   float En_noise_load, float* Es_slice, float* En_slice);
int Er_to_Es(float beta, float Esn_slice, float Esn_echo, float Esn_noise,
             float En_echo_load, float En_noise_load, float q_slice,
             float* Es_slice, float* En_slice);
#endif
