//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//
// This file contains functions useful for sigma0 and power calculations
//

static const char rcs_id_sigma0_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "Sigma0.h"
#include "Spacecraft.h"
#include "Instrument.h"
#include "Meas.h"
#include "Distributions.h"
#include "Constants.h"
#include "Wind.h"

//
// radar_X
//
// This function computes the combined factor in the radar equation called X.
// X contains the power, geometric, gain, loss, and constant factors that
// relate the received signal power with the mean cross-section of the target.
// The K-factor correction term is not included.
// ie., Pr(signal) = K*X*sigma0.
//
// Inputs:
//	spacecraft = pointer to current spacecraft object
//	instrument = pointer to current instrument object
//	meas = pointer to current measurement (sigma0, cell center, area etc.)
//	gc_to_antenna = pointer to a CoordinateSwitch from geocentric coordinates
//		to the antenna frame for the prevailing geometry.
//	X = pointer to return variable
//
//

int
radar_X(
	CoordinateSwitch*	gc_to_antenna,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	Meas*				meas,
	double*				X)
{
	double lambda = speed_light_kps / instrument->transmitFreq;
	double A3db = meas->outline.Area1();
	Vector3 rlook = meas->centroid - spacecraft->orbitState.rsat;
	double R = rlook.Magnitude();
	double roundTripTime = 2.0 * R / speed_light_kps;

	int ib = instrument->antenna.currentBeamIdx;
	Vector3 rlook_antenna = gc_to_antenna->Forward(rlook);
	double r,theta,phi;
	rlook_antenna.SphericalGet(&r,&theta,&phi);
	float GatGar;
	instrument->antenna.beam[ib].GetPowerGainProduct(theta,phi,roundTripTime,
		instrument->antenna.spinRate,&GatGar);

	*X = instrument->transmitPower * instrument->echo_receiverGain * GatGar *
		 A3db * lambda*lambda /
		(64*pi*pi*pi * R*R*R*R * instrument->systemLoss);
	return(1);
}

// Same as above, but takes a value of PtGr
int
radar_X_PtGr(
	CoordinateSwitch*	gc_to_antenna,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	Meas*				meas,
	float				PtGr,
	double*				X)
{
	double lambda = speed_light_kps / instrument->transmitFreq;
	double A3db = meas->outline.Area1();
	Vector3 rlook = meas->centroid - spacecraft->orbitState.rsat;
	double R = rlook.Magnitude();
	double roundTripTime = 2.0 * R / speed_light_kps;

	int ib = instrument->antenna.currentBeamIdx;
	Vector3 rlook_antenna = gc_to_antenna->Forward(rlook);
	double r,theta,phi;
	rlook_antenna.SphericalGet(&r,&theta,&phi);
	float GatGar;
	instrument->antenna.beam[ib].GetPowerGainProduct(theta,phi,roundTripTime,
		instrument->antenna.spinRate,&GatGar);

	*X = PtGr * GatGar *
		 A3db * lambda*lambda /
		(64*pi*pi*pi * R*R*R*R * instrument->systemLoss);
	return(1);
}


//
// sigma0_to_Esn_slice
//
// This function computes the energy received for a
// given instrument state and average sigma0.
// The received energy is the sum of the signal energy the noise energy
// that falls within the appropriate bandwidth.  This assumes that the
// geometry related factors such as range and antenna gain can be replaced
// by effective values (instead of integrating over the bandwidth).
// K-factor should remove any error introduced by this assumption.
// The result is fuzzed by Kpc (if requested) and by Kpm (as supplied).
// See also the sigma0_to_Esn_Enoise function which computes the total
// signal (all slices)
// plus noise energy over a much larger noise measurement bandwidth.
//
// Inputs:
//	gc_to_antenna = pointer to a CoordinateSwitch from geocentric coordinates
//		to the antenna frame for the prevailing geometry.
//	spacecraft = pointer to current spacecraft object
//	instrument = pointer to current instrument object
//	meas = pointer to current measurement (sigma0, cell center, area etc.)
//	Kfactor = Radar equation correction factor for this cell.
//	sigma0 = true sigma0 to assume.
//	Esn_slice = pointer to signal+noise energy in a slice.
//	XK = pointer to true X * Kfactor.
//

int
sigma0_to_Esn_slice(
	CoordinateSwitch*	gc_to_antenna,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	Meas*				meas,
	float				Kfactor,
	float				sigma0,
	float*				Esn_slice,
	float*				XK)
{
	//------------------------//
	// Sanity check on sigma0 //
	//------------------------//

	if (fabs(sigma0) > 1.0e5)
	{
		printf("Error: sigma0_to_Psn encountered invalid sigma0 = %g\n",sigma0);
		exit(-1);
	}

	//----------------------------------------------------------------------//
	// Compute the radar parameter X which includes gain, loss, and geometry
	// factors in the received power.  This is the true value.  Processing
	// uses a modified value that includes fuzzing by Kpr.
	//----------------------------------------------------------------------//

	double X;
	radar_X(gc_to_antenna, spacecraft, instrument, meas, &X);
	*XK = X*Kfactor;

	Beam* beam = instrument->antenna.GetCurrentBeam();
	double Tp = beam->pulseWidth;
	double Tg = beam->rxGateWidth;
	double Bs = meas->bandwidth;

	//------------------------------------------------------------------------//
	// Signal (ie., echo) energy referenced to the point just before the
	// I-Q detection occurs (ie., including the receiver gain and system loss).
	//------------------------------------------------------------------------//

	double Es_slice = Kfactor*X*sigma0*Tp;

	//------------------------------------------------------------------------//
	// Noise power spectral densities referenced the same way as the signal.
	//------------------------------------------------------------------------//

	double N0_echo = bK * instrument->systemTemperature *
		instrument->echo_receiverGain / instrument->systemLoss;

	//------------------------------------------------------------------------//
	// Noise energy within one slice referenced like the signal energy.
	//------------------------------------------------------------------------//

	double En1_slice = N0_echo * Bs * Tp;		// noise with signal
	double En2_slice = N0_echo * Bs * (Tg-Tp);	// noise without signal
	double En_slice = En1_slice + En2_slice;

	//------------------------------------------------------------------------//
	// Signal + Noise Energy within one slice referenced like the signal energy.
	//------------------------------------------------------------------------//

	*Esn_slice = (float)(Es_slice + En_slice);

	if (instrument->useKpc == 0)
	{
		return(1);
	}

	//------------------------------------------------------------------------//
	// Estimate the variance of the slice signal + noise energy measurements.
	// The variance is simply the sum of the variance when the signal
	// (and noise) are present together and the variance when only noise
	// is present.  These variances come from radiometer theory, ie.,
	// the reciprocal of the time bandwidth product is the normalized variance.
	// The variance of the power is derived from the variance of the energy.
	//------------------------------------------------------------------------//

	float var_esn_slice = (Es_slice + En1_slice)*(Es_slice + En1_slice) /
		(Bs * Tp) + En2_slice*En2_slice / (Bs*(Tg - Tp));

	//------------------------------------------------------------------------//
	// Fuzz the Esn value by adding a random number drawn from
	// a gaussian distribution with the variance just computed and zero mean.
	// This includes both thermal noise effects, and fading due to the
	// random nature of the surface target.
	// When the snr is low, the Kpc fuzzing can be large enough that
	// the processing step will estimate a negative sigma0 from the
	// fuzzed power.  This is normal, and will occur in real data also.
	// The wind retrieval has to estimate the variance using the model
	// sigma0 rather than the measured sigma0 to avoid problems computing
	// Kpc for weighting purposes.
	//------------------------------------------------------------------------//

	Gaussian rv(var_esn_slice,0.0);
	*Esn_slice += rv.GetNumber();

	return(1);
}

//
// sigma0_to_Esn_noise
//
// This function computes the energy returned by the noise channel measurement.
// The resulting energy is mostly noise energy over the noise bandwidth.
// The result also contains all of the signal (echo) power which lies within
// the much smaller echo bandwidth (contained within the noise bandwidth).
//
// Inputs:
//	instrument = pointer to current instrument object
//	spot = pointer to current spot (Psn)
//		Note: the measurements in this spot must ALREADY have Psn values
//			stored in the value member in the same units that this method
//			uses.
//	Esn_noise = pointer to return variable
//
//

int
sigma0_to_Esn_noise(
	Instrument*			instrument,
	MeasSpot*			spot,
	float*				Esn_noise)
{
	//------------------------------------------------------------------------//
	// Noise power spectral densities referenced the same way as the signal.
	//------------------------------------------------------------------------//

	double N0_noise = bK * instrument->systemTemperature *
		instrument->noise_receiverGain / instrument->systemLoss;

	//------------------------------------------------------------------------//
	// Useful quantities.
	//------------------------------------------------------------------------//

	Beam* beam = instrument->antenna.GetCurrentBeam();
	double Tg = beam->rxGateWidth;
	double Bn = instrument->noiseBandwidth;
	double Be = instrument->GetTotalSignalBandwidth();
	double beta = instrument->noise_receiverGain/instrument->echo_receiverGain;

	//------------------------------------------------------------------------//
	// Start with the noise contribution to the noise energy measurement
	// outside of the echo bandwidth.
	// This is simply the noise power spectral density (using the noise
	// channel gain) multiplied by the appropriate bandwidth and the
	// receiver gate width.
	//------------------------------------------------------------------------//

	*Esn_noise = N0_noise*(Bn-Be)*Tg;

	//------------------------------------------------------------------------//
	// Add in the signal + noise energies within the measurement spot.
	// These energies will include Kpc variance (if selected) that
	// accounts for both signal variation due to fading, and thermal noise
	// variation from the receiver front end.
	// The gain of the noise channel is different from the gain of the echo
	// channel, so a correction factor is applied.
	//------------------------------------------------------------------------//

	Meas* meas = spot->GetHead();
	while (meas != NULL)
	{
		*Esn_noise += meas->value*beta;
		meas = spot->GetNext();
	}

	if (instrument->useKpc == 0)
	{
		return(1);
	}

	//------------------------------------------------------------------------//
	// Compute the variance of the portion of the noise channel energy
	// measurement which falls outside of the echo bandwidth.
	//------------------------------------------------------------------------//

	float var_noise = (Bn - Be)*N0_noise*N0_noise*Tg;

	//------------------------------------------------------------------------//
	// Fuzz the Esn_noise value by adding a random number drawn from
	// a gaussian distribution with the variance just computed and zero mean.
	// This adds a small amount of additional variance on top of the variance
	// already present in the slice measurements.  The extra variance is due
	// to random variation of the thermal noise power in the noise
	// bandwidth outside of the echo bandwidth.
	// Because the noise bandwidth is much larger than the echo bandwidth,
	// this extra variance should be much smaller than the echo variance.
	//------------------------------------------------------------------------//

	Gaussian rv(var_noise,0.0);
	*Esn_noise += rv.GetNumber();

	return(1);
}

//
// Er_to_sigma0
//
// The Er_to_sigma0 function estimates sigma0 from two signal+noise
// measurements. One is the slice measurement Esn value.  The other is
// the noise channel measurement which includes all of the slice energies.
// See sigma0_to_Esn and sigma0_to_Esn_noise above.
// Various outputs are put in the Meas object passed in.
//
// Inputs:
//	gc_to_antenna = pointer to a CoordinateSwitch from geocentric coordinates
//		to the antenna frame for the prevailing geometry.
//	spacecraft = pointer to current spacecraft object
//	instrument = pointer to current instrument object
//	meas = pointer to current measurement (for radar_X: cell center, area etc.)
//	Kfactor = Radar equation correction factor for this cell.
//	Esn_slice = the received slice energy.
//	Esn_echo = the sum of all the slice energies for this spot.
//	Esn_noise = the noise channel measured energy.
//	PtGr = power gain product to use (includes any Kpr fuzzing).
//

int
Er_to_sigma0(
	CoordinateSwitch*	gc_to_antenna,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	Meas*				meas,
	float				Kfactor,
	float				Esn_slice,
	float				Esn_echo,
	float				Esn_noise,
	float				PtGr)
{
	// Compute radar parameter using telemetry values etc that may have been
	// fuzzed by Kpr (in the simulator, or by the actual instrument).
	double X;
	radar_X_PtGr(gc_to_antenna, spacecraft, instrument, meas, PtGr, &X);
	meas->XK = X*Kfactor;

	// Note that the rho-factor is assumed to be 1.0. ie., we assume that
	// all of the signal power falls in the slices.

	Beam* beam = instrument->antenna.GetCurrentBeam();
	double Tp = beam->pulseWidth;
	double Tg = beam->rxGateWidth;
	double Bn = instrument->noiseBandwidth;
	double Bs = meas->bandwidth;
	double Be = instrument->GetTotalSignalBandwidth();
	double beta = instrument->noise_receiverGain/instrument->echo_receiverGain;
	double alpha = Bn/Be*beta;
	double rho = 1.0;

	// Estimate the noise energy in the slice. (exact for simulated data)
	meas->EnSlice = Bs/Be*(rho/beta*Esn_noise-Esn_echo)/(alpha*rho/beta-1.0);

	// Subtract out slice noise, leaving the signal power fuzzed by Kpc.
	double Es_slice = Esn_slice - meas->EnSlice;

	// The resulting sigma0 should have a variance equal to Kpc^2+Kpr^2.
	// Kpc comes from Es_slice.
	// Kpr comes from 1/X
	meas->value = (float)(Es_slice / meas->XK / Tp);

	if (instrument->useKpc == 0)
	{
		meas->A = 0.0;
		meas->B = 0.0;
		meas->C = 0.0;
	}
	else
	{
		//------------------------------------------------------------------//
		// Estimate Kpc coefficients using the
		// approximate equations in Mike Spencer's Kpc memos.
		//------------------------------------------------------------------//

		meas->A = 1.0 / (Bs * Tp);
		meas->B = 2.0 / (Bs * Tg);
		meas->C = meas->B/2.0 * (1.0 + Bs/Bn);
	}

	return(1);
}
