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
// sigma0_to_Psn
//
// This function computes the power received for a
// given instrument state and average sigma0.
// The received power is the sum of the signal power and the noise power
// that falls within the appropriate bandwidth.  This assumes that the
// geometry related factors such as range and antenna gain can be replaced
// by effective values (instead of integrating over the bandwidth).
// K-factor should remove any error introduced by this assumption.
// The result is fuzzed by Kpc (if requested) and by Kpm (as supplied).
// See also the Pnoise function which computes the total signal (all slices)
// plus noise power over a much larger noise measurement bandwidth.
//
// Inputs:
//	gc_to_antenna = pointer to a CoordinateSwitch from geocentric coordinates
//		to the antenna frame for the prevailing geometry.
//	spacecraft = pointer to current spacecraft object
//	instrument = pointer to current instrument object
//	meas = pointer to current measurement (sigma0, cell center, area etc.)
//	Kfactor = Radar equation correction factor for this cell.
//	sigma0 = true sigma0 to assume.
//	Pr = pointer to return variable
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
	float*				Psn_slice)
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

	Beam* beam = instrument->antenna.GetCurrentBeam();
	double Tp = beam->pulseWidth;
	double Tg = beam->receiverGateWidth;
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
	// Signal + Noise POWER within one slice referenced like the signal power.
	//------------------------------------------------------------------------//

	*Psn_slice = (float)((Es_slice + En_slice) / Tg);

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
	float var_psn_slice = var_esn_slice / (Tg*Tg);

	//------------------------------------------------------------------------//
	// Fuzz the Psn value by adding a random number drawn from
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

	Gaussian rv(var_psn_slice,0.0);
	*Psn_slice += rv.GetNumber();

	// Below is the old approach based on SNR
	//------------------------------------------------------------------------//
	// Estimate Kpc using the true value of snr (known here!) and the
	// approximate equations in Mike Spencer's Kpc memos.
	//------------------------------------------------------------------------//

	//Beam* beam = instrument->antenna.GetCurrentBeam();

	//double snr = Ps_slice/Pn_slice;
	//double A = 1.0 / (meas->bandwidth * beam->pulseWidth);
	//double B = 2.0 / (meas->bandwidth * beam->receiverGateWidth);
	//double C = B/2.0 * (1.0 + meas->bandwidth/instrument->noiseBandwidth);
	//float Kpc2 = A + B/snr + C/snr/snr;
	//float var_psn_slice1 = A*Ps_slice*Ps_slice + B*Ps_slice*Pn_slice +
//		C*Pn_slice*Pn_slice;

	//------------------------------------------------------------------------//
	// Fuzz the Ps value by multiplying by a random number drawn from
	// a gaussian distribution with a variance of Kpc^2.  This includes both
	// thermal noise effects, and fading due to the random nature of the
	// surface target.
	// Kpc is applied to Ps instead of Psn because sigma0 is proportional
	// to Ps (not Psn), and Kpc is defined for sigma0.  A more correct way
	// would be to separate Kpc into a thermal effect that applies only
	// to the noise, and a fading effect that applies only to the signal.
	// However, as long as Kpc is correct, the approach implemented here
	// will give the final sigma0's the correct variance.
	// When the snr is low, the Kpc fuzzing can be large enough that
	// the processing step will estimate a negative sigma0 from the
	// fuzzed power.  This is normal, and will occur in real data also.
	// The wind retrieval has to estimate the variance using the model
	// sigma0 rather than the measured sigma0 to avoid problems computing
	// Kpc for weighting purposes.
	//------------------------------------------------------------------------//

	//Gaussian rv(Kpc2,1.0);
	//Ps_slice *= rv.GetNumber();

	// Assuming that rho = 1.0 for now.
	//double Bn = instrument->noiseBandwidth;
	//double Be = instrument->GetTotalSignalBandwidth();
	//double N0_noise = bK * instrument->systemTemperature *
	//	instrument->noise_receiverGain / instrument->systemLoss;
	//float var_psn_noise = (Bn - Be)*Tg*N0_noise*N0_noise;

//	printf("%g %g %g %g %g\n",
//		sigma0,Kpc2,var_psn_slice1,var_psn_slice,var_psn_noise);

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
//	instrument = pointer to current instrument object
//	spot = pointer to current spot (Psn)
//		Note: the measurements in this spot must ALREADY have Psn values
//			stored in the value member in the same units that this method
//			uses.
//	Pn = pointer to return variable
//
//

int
Pnoise(
	Instrument*			instrument,
	MeasSpot*			spot,
	float*				Psn_noise)
{
	//------------------------------------------------------------------------//
	// Noise power spectral densities referenced the same way as the signal.
	//------------------------------------------------------------------------//

	double N0_echo = bK * instrument->systemTemperature *
		instrument->echo_receiverGain / instrument->systemLoss;
	double N0_noise = bK * instrument->systemTemperature *
		instrument->noise_receiverGain / instrument->systemLoss;

	//------------------------------------------------------------------------//
	// Useful quantities.
	//------------------------------------------------------------------------//

	Beam* beam = instrument->antenna.GetCurrentBeam();
	double Tg = beam->receiverGateWidth;
	double Bn = instrument->noiseBandwidth;
	double Be = instrument->GetTotalSignalBandwidth();
	double beta = instrument->noise_receiverGain/instrument->echo_receiverGain;

	//------------------------------------------------------------------------//
	// Start with the noise contribution to the noise energy measurement.
	// This is simply the noise spectral density (using the noise channel gain)
	// multiplied by the noise bandwidth and the receiver gate width.
	//------------------------------------------------------------------------//

	double Esn_noise = N0_noise*Bn*Tg;

	//------------------------------------------------------------------------//
	// Add in the signal energies within the measurement spot.
	// These signal energies will include Kpc variance (if selected) that
	// incorporates both signal variation due to fading, and thermal noise
	// variation from the receiver front end.
	//------------------------------------------------------------------------//

	double En_slice,Es_slice;
	Meas* meas = spot->GetHead();
	while (meas != NULL)
	{
		En_slice = N0_echo * meas->bandwidth * Tg;
		Es_slice = meas->value*Tg - En_slice;
		Esn_noise += Es_slice*beta;
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
	Esn_noise += rv.GetNumber();

	*Psn_noise = Esn_noise / Tg;

	return(1);
}

//
// Pr_to_sigma0
//
// The Pr_to_sigma0 function estimates sigma0 from two signal+noise
// measurements. One is the slice measurement Psn value.  The other is
// the noise channel measurement which includes all of the slice powers.
// See sigma0_to_Psn and Pnoise above.
// Various outputs are put in the Meas object passed in.
//
// Inputs:
//	gc_to_antenna = pointer to a CoordinateSwitch from geocentric coordinates
//		to the antenna frame for the prevailing geometry.
//	spacecraft = pointer to current spacecraft object
//	instrument = pointer to current instrument object
//	meas = pointer to current measurement (for radar_X: cell center, area etc.)
//	Kfactor = Radar equation correction factor for this cell.
//	Psn = the received slice power.
//	sumPsn = the sum of all the slice powers for this spot.
//	Pn = the noise bandwidth measured power.
//	PtGr = power gain product to use (includes any Kpr fuzzing).
//

int
Pr_to_sigma0(
	CoordinateSwitch*	gc_to_antenna,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	Meas*				meas,
	float				Kfactor,
	float				Psn_slice,
	float				Psn_echo,
	float				Psn_noise,
	float				PtGr)
{
	// Compute radar parameter using telemetry values etc that may have been
	// fuzzed by Kpr (in the simulator, or by the actual instrument).
	double X;
	radar_X_PtGr(gc_to_antenna, spacecraft, instrument, meas, PtGr, &X);
	meas->XK = X*Kfactor;

	// Note that the rho-factor is assumed to be 1.0. ie., we assume that
	// all of the signal power falls in the slices.

	double Bn = instrument->noiseBandwidth;
	double Bs = meas->bandwidth;
	double Be = instrument->GetTotalSignalBandwidth();
	double beta = instrument->noise_receiverGain/instrument->echo_receiverGain;
	double alpha = Bn/Be*beta;
	double rho = 1.0;

	// Estimate the noise in the slice. (exact for simulated data)
	meas->Pn_slice = Bs/Be*(rho/beta*Psn_noise-Psn_echo)/(alpha*rho/beta-1.0);

	// Subtract out slice noise, leaving the signal power fuzzed by Kpc.
	double Ps_slice = Psn_slice - meas->Pn_slice;

	// The resulting sigma0 should have a variance equal to Kpc^2+Kpr^2.
	// Kpc comes from Ps_slice.
	// Kpr comes from 1/X
	meas->value = (float)(Ps_slice / meas->XK);

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

		Beam* beam = instrument->antenna.GetCurrentBeam();
		meas->A = 1.0 / (Bs * beam->pulseWidth);
		meas->B = 2.0 / (Bs * beam->receiverGateWidth);
		meas->C = meas->B/2.0 * (1.0 + Bs/Bn);
	}

	return(1);
}

//
// GetKpm
//
// Return the Kpm value appropriate for the current instrument state
// (ie., beam) and wind vector.
// This function uses the wind speed value (rounded to the nearest integer)
// to look up the Kpm value in a table (one for each beam).
//
// Inputs:
//	instrument = pointer to current instrument object.
//	wv = pointer to wind vector to use.
//
// Return Value:
//	The value of Kpm to use.
//

float
GetKpm(
	Instrument* instrument,
	WindVector* wv)

{
	// V-pol is index 0, H-pol is index 1 for the 1st dim.
	static float Kpmtable[2][36] =
	{ {6.3824e-01, 5.6835e-01, 4.9845e-01, 4.2856e-01, 3.5867e-01, 2.8877e-01,
	2.5092e-01, 2.1307e-01, 1.9431e-01, 1.7555e-01, 1.7072e-01, 1.6589e-01,
	1.6072e-01, 1.5554e-01, 1.4772e-01, 1.3990e-01, 1.2843e-01, 1.1696e-01,
	1.1656e-01, 1.1615e-01, 1.0877e-01, 1.0138e-01, 9.0447e-02, 7.9516e-02,
	8.6400e-02, 9.3285e-02, 8.4927e-02, 7.6569e-02, 7.2302e-02, 6.8036e-02,
	7.7333e-02, 8.6630e-02, 9.0959e-02, 9.5287e-02, 9.9616e-02, 1.0394e-01},
	  {4.3769e-01,  4.0107e-01, 3.6446e-01, 3.2784e-01, 2.9122e-01, 2.5461e-01,
	2.2463e-01, 1.9464e-01, 1.7066e-01, 1.4667e-01, 1.3207e-01, 1.1747e-01,
	1.0719e-01, 9.6918e-02, 9.0944e-02, 8.4969e-02, 7.7334e-02, 6.9699e-02,
	6.9107e-02, 6.8515e-02, 6.6772e-02, 6.5030e-02, 5.7429e-02, 4.9828e-02,
	4.3047e-02, 3.6266e-02, 3.0961e-02, 2.5656e-02, 2.9063e-02, 3.2471e-02,
	2.7050e-02, 2.1629e-02, 2.8697e-02, 3.5764e-02, 4.2831e-02, 4.9899e-02}};

	float Kpm;
	int ib = instrument->antenna.currentBeamIdx;
	if (wv->spd < 0)
	{
		printf("Error: GetKpm received a negative wind speed\n");
		exit(-1);
	}
	else if (wv->spd < 35)
	{
		Kpm = Kpmtable[ib][(int)(wv->spd+0.5)];
	}
	else
	{
		Kpm = Kpmtable[ib][35];
	}

	return(Kpm);
}

//
// composite
//
// Combine measurments into one composite sigma0 and Kpc coefficients.
// The input measurement list should all come from one spot, but this
// routine does not (and can not) check for this.
// The final composite measurement is put in a single measurement.
//
// Inputs:
//	input_measList = pointer to list of measurements to be composited.
//	output_meas = pointer to the Meas structure to put results in.
//

int
composite(
	MeasList*	input_measList,
	Meas*		output_meas)

{
	float sum_Ps = 0.0;
	float sum_XK = 0.0;
	Vector3 sum_centroid(0.0,0.0,0.0);
	float sum_inc_angle = 0.0;
	float sum_azi_angle = 0.0;
	float sum_X2 = 0.0;
	output_meas->bandwidth = 0.0;
	int N = 0;

	//
	// Using X in place of Ps when compositing Kpc assumes that sigma0 is
	// uniform across the composite cell area.  This assumption is used
	// below because we sum X^2 instead of Ps^2.
	// We actually use XK which subsumes the K-factor with X.
	//

	Meas* meas;
	for (meas = input_measList->GetHead();
		meas;
		meas = input_measList->GetNext())
	{
		sum_Ps += meas->value * meas->XK;
		sum_XK += meas->XK;
		sum_centroid += meas->centroid;
		sum_inc_angle += meas->incidenceAngle;
		sum_azi_angle += meas->eastAzimuth;
		sum_X2 += meas->XK*meas->XK;
		output_meas->bandwidth += meas->bandwidth;
		N++;
	}

	meas = input_measList->GetHead();

	//---------------------------------------------------------------------//
	// Form the composite measurement from appropriate combinations of the
	// elements of each slice measurement in this composite cell.
	//---------------------------------------------------------------------//

	output_meas->value = sum_Ps / sum_XK;
	output_meas->XK = sum_XK;
	output_meas->Pn_slice = meas->Pn_slice;		// same for all slices.
	output_meas->bandwidth = meas->bandwidth;	// assumed same for all slices.

	output_meas->outline.FreeContents();	// merged outlines not done yet.
	output_meas->centroid = sum_centroid / N;
	// Make sure centroid is on the surface.
	double alt,lat,lon;
	output_meas->centroid.GetAltLonGDLat(&alt,&lat,&lon);
	output_meas->centroid.SetAltLonGDLat(0.0,lat,lon);

	output_meas->pol = meas->pol;

	// Approx incidence and azimuth angles.
	// These should really be done using the satellite
	// position, but that would mean putting the sat. position in Meas.
	output_meas->incidenceAngle = sum_inc_angle / N;
	output_meas->eastAzimuth = sum_azi_angle / N;

	// Composite Kpc coefficients.
	// Here we assume that all the slices in one spot have the same values
	// of A,B,C (ie., bandwidths, and pulsewidths don't change within a spot).
	// This will only be an issue if guard slices are composited with regular
	// slices.
	output_meas->A = meas->A * sum_X2 / (sum_XK * sum_XK);
	output_meas->B = meas->B  / N;
	output_meas->C = meas->C  / N;

	return(1);
}
