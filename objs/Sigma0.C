//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

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
//  spacecraft = pointer to current spacecraft object
//  instrument = pointer to current instrument object
//	meas = pointer to current measurement (sigma0, cell center, area etc.)
//  gc_to_antenna = pointer to a CoordinateSwitch from geocentric coordinates
//		to the antenna frame for the prevailing geometry.
//  X = pointer to return variable
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
	double lambda = speed_light_kps / instrument->baseTransmitFreq;
	double A3db = meas->outline.Area();
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

	*X = instrument->transmitPower * instrument->receiverGain * GatGar *
		 A3db * lambda*lambda /
		(64*pi*pi*pi * R*R*R*R * instrument->systemLoss);
	return(1);
}


//
// sigma0_to_Psn
//
// This function computes the power received for a
// given instrument state and average sigma0.
// The received power is the sum of the signal power and the noise power
// that falls within the appropriate bandwidth.  This assumes that the
// geometry related factors such as range and antenna gain can be replaced
// by effective values (instead of integrating over the bandwidth).
// K-factor should remove any error introduced by this assumption.
// The result is fuzzed by Kpc.
// See also the Pnoise function which computes the total signal (all slices)
// plus noise power over a much larger noise measurement bandwidth.
//
// Inputs:
//  gc_to_antenna = pointer to a CoordinateSwitch from geocentric coordinates
//		to the antenna frame for the prevailing geometry.
//  spacecraft = pointer to current spacecraft object
//  instrument = pointer to current instrument object
//	meas = pointer to current measurement (sigma0, cell center, area etc.)
//  Kfactor = Radar equation correction factor for this cell.
//  sigma0 = true sigma0 to assume.
//  Pr = pointer to return variable
//
//

int
sigma0_to_Psn(
	CoordinateSwitch*	gc_to_antenna,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	Meas*				meas,
	float				Kfactor,
	float				sigma0,
	float*				Psn)
{
	double X;
	radar_X(gc_to_antenna, spacecraft, instrument, meas, &X);

	// Signal (ie., echo) power
	double Ps_slice = Kfactor*X*sigma0;

	// Constant noise power within one slice
	double Pn_slice = bK*instrument->sliceBandwidth *
		instrument->systemTemperature;

	if (instrument->useKpc == 0)
	{
		*Psn = (float)(Ps_slice + Pn_slice);
		return(1);
	}

	// Estimate Kpc using the true value of snr (known here!) and the
	// approximate equations in Mike Spencer's Kpc memos.

	double snr = Ps_slice/Pn_slice;
	double A = 1.0/(instrument->sliceBandwidth * instrument->xmitPulsewidth);
	double B = 2.0/(instrument->sliceBandwidth * instrument->receiverGateWidth);
	double C = B/2.0 *
		(1.0 + instrument->sliceBandwidth/instrument->noiseBandwidth);
	float Kpc = A + B/snr + C/snr/snr;

	// Fuzz the Ps value by multiplying by a random number drawn from
	// a gaussian distribution with a variance of Kpc.  This includes both
	// thermal noise effects, and fading due to the random nature of the 
	// surface target.
	// Kpc is applied to Ps instead of Psn because sigma0 is proportional
	// to Ps (not Psn), and Kpc is defined for sigma0.  A more correct way
	// would be to separate Kpc into a thermal effect that applies only
	// to the noise, and a fading effect that applies only to the signal.
	// However, as long as Kpc is correct, the approach implemented here
	// will give the final sigma0's the correct variance.

	Gaussian rv(Kpc,1.0);
	Ps_slice *= rv.GetNumber();

	*Psn = (float)(Ps_slice + Pn_slice);

	return(1);
}

//
// Pnoise
//
// This function computes the power returned by the noise channel measurement.
// The resulting power is mostly noise power over the noise bandwidth.
// The result also contains all of the signal (echo) power which lies within
// the much smaller echo bandwidth (contained within the noise bandwidth).
//
// Inputs:
//  instrument = pointer to current instrument object
//	spot = pointer to current spot (Psn)
//		Note: the measurements in this spot must ALREADY have Psn values
//			  stored in the value member in the same units that this method
//			  uses.
//  Pn = pointer to return variable
//
//

int
Pnoise(
	Instrument*			instrument,
	MeasSpot*			spot,
	float*				Pn)
{
	// Constant noise power within the noise bandwidth excluding the signal
	// bandwidth. The signal bandwidth includes all the slices.  The noise
	// power within the slices is added in below when the Psn's for the slices
	// are added.
	*Pn = bK*(instrument->noiseBandwidth - instrument->signalBandwidth) *
		instrument->systemTemperature;

	// Sum the signal powers within the measurement spot.
	// This procedure effectively puts all of the variance (ie., Kpc effect)
	// into the slice signal powers.  The processing routine Pr_to_sigma0
	// will obtain the exact constant noise power used by the simulator
	// when processing simulated data.  The resulting slice signal powers
	// will then have the correct variance (Kpc).  Real data is different
	// because some of the variance in sigma0 comes from uncertainty in
	// estimating the noise in a slice.  The end result is the same.
	// Sigma0 (and Ps) will have a variance equal to Kpc.
	// To be strictly correct, the simulator should separate Kpc into
	// a thermal noise contribution, and a fading contribution (which only
	// affects the signal power), and apply them separately.

	Meas* meas = spot->GetHead();
	while (meas != NULL)
	{
		*Pn += meas->value;
		meas = spot->GetNext();
	}

	return(1);
}

//
// Pr_to_sigma0
//
// The Pr_to_sigma0 function estimates sigma0 from two signal+noise
// measurements. One is the slice measurement Psn value.  The other is
// the noise channel measurement which includes all of the slice powers.
// See sigma0_to_Psn and Pnoise above.
//
// Inputs:
//  gc_to_antenna = pointer to a CoordinateSwitch from geocentric coordinates
//		to the antenna frame for the prevailing geometry.
//  spacecraft = pointer to current spacecraft object
//  instrument = pointer to current instrument object
//	meas = pointer to current measurement (for radar_X: cell center, area etc.)
//  Kfactor = Radar equation correction factor for this cell.
//  Psn = the received slice power.
//	sumPsn = the sum of all the slice powers for this spot.
//  Pn = the noise bandwidth measured power.
//  sigma0 = pointer to return variable
//
//

int
Pr_to_sigma0(
	CoordinateSwitch*	gc_to_antenna,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	Meas*				meas,
	float				Kfactor,
	float				Psn,
	float				sumPsn,
	float				Pn,
	float*				sigma0)
{
	// Compute radar parameter using telemetry values etc that may have been
	// fuzzed by Kpr (in the simulator, or by the actual instrument).
	double X;
	radar_X(gc_to_antenna, spacecraft, instrument, meas, &X);

	// Note that the rho-factor is assumed to be 1.0. ie., we assume that
	// all of the signal power falls in the slices.

	double Bn = instrument->noiseBandwidth;
	double Bs = instrument->sliceBandwidth;
	double Be = instrument->signalBandwidth;

	// Estimate the noise in the slice. (exact for simulated data)
	double Pn_slice = Bs/Bn*Pn - Bs/Bn*(sumPsn - Be/Bn*Pn)/(1-Be/Bn);

	// Subtract out slice noise, leaving the signal power fuzzed by Kpc.
	double Ps_slice = Psn - Pn_slice;

	// The resulting sigma0 should have a variance equal to Kpc+Kpr.
	// Kpc comes from Ps_slice.
	// Kpr comes from 1/X
	*sigma0 = (float)(Ps_slice / (X*Kfactor));

	return(1);
}
