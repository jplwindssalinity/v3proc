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
	float*				Psn)
{

	//----------------------------------------------------------------------//
	// Compute the radar parameter X which includes gain, loss, and geometry
	// factors in the received power.  This is the true value.  Processing
	// uses a modified value that includes fuzzing by Kpr.
	//----------------------------------------------------------------------//

	double X;
	radar_X(gc_to_antenna, spacecraft, instrument, meas, &X);

	//------------------------------------------------------------------------//
	// Signal (ie., echo) power referenced to the point just before the
	// I-Q detection occurs (ie., including the receiver gain and system loss).
	//------------------------------------------------------------------------//

	double Ps_slice = Kfactor*X*sigma0;

	//------------------------------------------------------------------------//
	// Constant noise power within one slice referenced to the antenna terminal
	//------------------------------------------------------------------------//

	double Pn_slice = bK * meas->bandwidth * instrument->systemTemperature;

	//------------------------------------------------------------------------//
	// Multiply by receiver gain and system loss to reference noise power
	// at the same place as the signal power.
	//------------------------------------------------------------------------//

	Pn_slice *= instrument->receiverGain / instrument->systemLoss;

	if (instrument->useKpc == 0)
	{
		*Psn = (float)(Ps_slice + Pn_slice);
		return(1);
	}

	//------------------------------------------------------------------------//
	// Estimate Kpc using the true value of snr (known here!) and the
	// approximate equations in Mike Spencer's Kpc memos.
	//------------------------------------------------------------------------//

	Beam* beam = instrument->antenna.GetCurrentBeam();

	double snr = Ps_slice/Pn_slice;
	double A = 1.0 / (meas->bandwidth * beam->pulseWidth);
	double B = 2.0 / (meas->bandwidth * beam->receiverGateWidth);
	double C = B/2.0 * (1.0 + meas->bandwidth/instrument->noiseBandwidth);
	float Kpc = A + B/snr + C/snr/snr;

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
	//------------------------------------------------------------------------//

	Gaussian rv(Kpc*Kpc,1.0);
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
	float*				Pn)
{
	// Constant noise power within the noise bandwidth excluding the signal
	// bandwidth. The signal bandwidth includes all the slices.  The noise
	// power within the slices is added in below when the Psn's for the slices
	// are added.
	*Pn = bK * (instrument->noiseBandwidth -
		instrument->GetTotalSignalBandwidth()) * instrument->systemTemperature;

	// Multiply by receiver gain and system loss to reference noise power
	// at the same place as the signal power.
	*Pn *= instrument->receiverGain / instrument->systemLoss;

	// Sum the signal powers within the measurement spot.
	// This procedure effectively puts all of the variance (ie., Kpc effect)
	// into the slice signal powers.  The processing routine Pr_to_sigma0
	// will obtain the exact constant noise power used by the simulator
	// when processing simulated data.  The resulting slice signal powers
	// will then have the correct variance (Kpc).  Real data is different
	// because some of the variance in sigma0 comes from uncertainty in
	// estimating the noise in a slice.  The end result is the same.
	// Sigma0 (and Ps) will have a variance equal to Kpc^2.
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
//	gc_to_antenna = pointer to a CoordinateSwitch from geocentric coordinates
//		to the antenna frame for the prevailing geometry.
//	spacecraft = pointer to current spacecraft object
//	instrument = pointer to current instrument object
//	meas = pointer to current measurement (for radar_X: cell center, area etc.)
//	Kfactor = Radar equation correction factor for this cell.
//	Psn = the received slice power.
//	sumPsn = the sum of all the slice powers for this spot.
//	Pn = the noise bandwidth measured power.
//	sigma0 = pointer to return variable
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
	float				PtGr,
	float*				sigma0,
	double*				X,
	float*				Kpc)
{
	// Compute radar parameter using telemetry values etc that may have been
	// fuzzed by Kpr (in the simulator, or by the actual instrument).
	radar_X_PtGr(gc_to_antenna, spacecraft, instrument, meas, PtGr, X);

	// Note that the rho-factor is assumed to be 1.0. ie., we assume that
	// all of the signal power falls in the slices.

	double Bn = instrument->noiseBandwidth;
	double Bs = meas->bandwidth;
	double Be = instrument->GetTotalSignalBandwidth();

	// Estimate the noise in the slice. (exact for simulated data)
	double Pn_slice = Bs/Bn*Pn - Bs/Bn*(sumPsn - Be/Bn*Pn)/(1-Be/Bn);

	// Subtract out slice noise, leaving the signal power fuzzed by Kpc.
	double Ps_slice = Psn - Pn_slice;

	// The resulting sigma0 should have a variance equal to Kpc^2+Kpr^2.
	// Kpc comes from Ps_slice.
	// Kpr comes from 1/X
	*sigma0 = (float)(Ps_slice / (*X*Kfactor));

	if (instrument->useKpc == 0)
	{
		*Kpc = 0.0;
	}
	else
	{
		//------------------------------------------------------------------//
		// Estimate Kpc using the estimated value of snr and the
		// approximate equations in Mike Spencer's Kpc memos.
		//------------------------------------------------------------------//

		Beam* beam = instrument->antenna.GetCurrentBeam();
		double snr = Ps_slice/Pn_slice;
		double A = 1.0 / (meas->bandwidth * beam->pulseWidth);
		double B = 2.0 / (meas->bandwidth * beam->receiverGateWidth);
		double C = B/2.0 * (1.0 + meas->bandwidth/instrument->noiseBandwidth);
		*Kpc = A + B/snr + C/snr/snr;
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
	{{6.3824e-01, 5.6835e-01, 4.9845e-01, 4.2856e-01, 3.5867e-01, 2.8877e-01,
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
// Combine measurments into one composite sigma0 and Kpc.
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
	float sum_X2Kpc2 = 0.0;
	output_meas->bandwidth = 0.0;
	int N = 0;

	//
	// Using X in place of Ps when compositing Kpc assumes that sigma0 is
	// uniform across the composite cell area.  This assumption is used
	// below because we sum X^2*Kpc^2 instead of Ps^2*Kpc^2.
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
		sum_X2Kpc2 += meas->XK*meas->XK * meas->estimatedKp*meas->estimatedKp;
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

	// Composite Kpc.
	output_meas->estimatedKp = sum_X2Kpc2 / (sum_XK * sum_XK);
	if (output_meas->estimatedKp == 0.0)
	{	// assume that this means that Kpc is not being used in this run.
		// Set to 1.0 so that GMF::_ObjectiveFunction works.
		output_meas->estimatedKp = 1.0;
	}

	return(1);

}
