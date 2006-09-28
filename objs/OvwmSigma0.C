//==============================================================//
// Copyright (C) 1997-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//
// This file contains functions useful for sigma0 and power calculations
//

static const char rcs_id_sigma0_c[] =
    "@(#) $Id$";

#include "CoordinateSwitch.h"
#include "Spacecraft.h"
#include "Meas.h"
#include "OvwmSigma0.h"
#include "Ovwm.h"
#include "Distributions.h"
#include "Misc.h"

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
//	ovwm = pointer to current Ovwm object
//	meas = pointer to current measurement (sigma0, cell center, area etc.)
//	gc_to_antenna = pointer to a CoordinateSwitch from geocentric coordinates
//		to the antenna frame for the prevailing geometry.
//	X = pointer to return variable
//
//

int
radar_X(
    CoordinateSwitch*  gc_to_antenna,
    Spacecraft*        spacecraft,
    Ovwm*             ovwm,
    Meas*              meas,
    double*            X)
{
	double lambda = speed_light_kps / ovwm->ses.txFrequency;
	double A3db = 1.0;
	Vector3 rlook = meas->centroid - spacecraft->orbitState.rsat;
	double R = rlook.Magnitude();
	double roundTripTime = 2.0 * R / speed_light_kps;

    Beam* beam = ovwm->GetCurrentBeam();
	Vector3 rlook_antenna = gc_to_antenna->Forward(rlook);
	double r, theta, phi;
	rlook_antenna.SphericalGet(&r,&theta,&phi);
	float GatGar;

	if(! beam->GetPowerGainProduct(theta, phi, roundTripTime,
        ovwm->sas.antenna.spinRate, &GatGar))
    {
        return(0);
    }

	*X = ovwm->ses.transmitPower * ovwm->ses.rxGainEcho * GatGar *
		A3db * lambda*lambda / (64*pi*pi*pi * R*R*R*R * ovwm->systemLoss);
	return(1);
}

// Same as above, but takes a value of PtGr
int
radar_X_PtGr(
    CoordinateSwitch*  gc_to_antenna,
    Spacecraft*        spacecraft,
    Ovwm*             ovwm,
    Meas*              meas,
    float              PtGr,
    double*            X)
{
	double lambda = speed_light_kps / ovwm->ses.txFrequency;
	double A3db = 1.0;
	Vector3 rlook = meas->centroid - spacecraft->orbitState.rsat;
	double R = rlook.Magnitude();
	double roundTripTime = 2.0 * R / speed_light_kps;

    Beam* beam = ovwm->GetCurrentBeam();
	Vector3 rlook_antenna = gc_to_antenna->Forward(rlook);
	double r, theta, phi;
	rlook_antenna.SphericalGet(&r,&theta,&phi);
	float GatGar;
	if( ! beam->GetPowerGainProduct(theta, phi, roundTripTime,
        ovwm->sas.antenna.spinRate, &GatGar))
    {
        return(0);
    }

	*X = PtGr * GatGar * A3db * lambda*lambda /
        (64*pi*pi*pi * R*R*R*R * ovwm->systemLoss);

	return(1);
}

//------------------------------------------//
// Compute Xcal defined in IOM-3347-98-019. //
//------------------------------------------//

int
radar_Xcal(
    Ovwm*   ovwm,
    float    Es_cal,
    double*  Xcal)
{
    double L13 = ovwm->ses.receivePathLoss;
    double L21 = ovwm->ses.transmitPathLoss;
    double L23 = ovwm->ses.loopbackLoss;
    double Lcalop = ovwm->ses.loopbackLossRatio;
    double delta = ovwm->ses.calibrationBias;
	double lambda = speed_light_kps / ovwm->ses.txFrequency;
    Beam* beam = ovwm->GetCurrentBeam();

	*Xcal = beam->peakGain * beam->peakGain * lambda*lambda /
        (64*pi*pi*pi) * (L23*Lcalop/L13/L21) / delta * Es_cal;

	return(1);
}

//------------------------------------------------------//
// Compute true loopback signal energy using true PtGr. //
//------------------------------------------------------//

double
true_Es_cal(
    Ovwm*  ovwm)
{
    double Es_cal = ovwm->ses.transmitPower * ovwm->ses.rxGainEcho *
                 ovwm->ses.transmitPathLoss / ovwm->ses.calibrationBias /
                 ovwm->ses.loopbackLoss / ovwm->ses.loopbackLossRatio *
                 ovwm->ses.txPulseWidth;

    return(Es_cal);
}

//
// PtGr_to_Esn
//
// This function computes the loopback cal pulse energy received for a
// given instrument state and PtGr (obtained from the instrument).
// The received energy is the sum of the looped back signal energy and
// the noise energy that falls within the appropriate bandwidth.
// The result could be fuzzed by Kpc which is where Kpri really comes from,
// but this is NOT done right now.  Instead, Kpri noise is applied directly
// to signal energy, and the noise subtraction for cal pulses works perfectly
// with simulated data.
//
// Inputs:
//  PtGr = true transmit power reciever gain product to use.
//  ptgrNoise = noise process to use when applying Kpri variance.
//	ovwm = pointer to current Ovwm object
//  sim_kpri_flag = 1 when Kpri variance should be applied, 0 otherwise.
//	Esn_echo_cal = pointer to signal+noise energy in the echo channel.
//	Esn_noise_cal = pointer to signal+noise energy in the noise channel.
//

int
PtGr_to_Esn(
    TimeCorrelatedGaussian*  ptgrNoise,
    Ovwm*       ovwm,
    int          sim_kpri_flag,
    float*       Esn_echo_cal,
    float*       Esn_noise_cal)
{

    double Tg = ovwm->GetRxGateWidth();
    double Bn = ovwm->ses.noiseBandwidth;
    double Be = ovwm->ses.chirpBandwidth;
    double beta = ovwm->ses.rxGainNoise / ovwm->ses.rxGainEcho;
    double alpha = Bn/Be;
    double L13 = ovwm->ses.receivePathLoss;

    //cout << Tg << endl;
    //cout << Bn << endl;
    //cout << Be << endl;
    //cout << beta << endl;
    //cout << alpha << endl;
    //cout << L13 << endl;

    //------------------------------------------------------------------------//
    // Signal (ie., echo) energy referenced to the point just before the
    // I-Q detection occurs (ie., including the receiver gain and system loss).
    // L21 boosts Pt up to the level at which it goes through the loopback.
    //------------------------------------------------------------------------//

    double Es_cal = true_Es_cal(ovwm);

    //cout << Es_cal << endl;

    //------------------------------------------------------------------------//
    // Add Kpri noise if requested.  This should properly be Kpc style noise
    // added to the signal + noise measurement as done in sigma0_to_Esn, but
    // we are controlling this variance source separately right now.
    //------------------------------------------------------------------------//

    if (sim_kpri_flag)
        Es_cal *= (1 + ptgrNoise->GetNumber(ovwm->cds.time));

    //cout << Es_cal << endl;

    //------------------------------------------------------------------------//
    // Noise power spectral density referenced the same way as the signal.
    //------------------------------------------------------------------------//

    double N0_echo = bK * ovwm->systemTemperature *
    ovwm->ses.rxGainEcho / L13;

    //------------------------------------------------------------------------//
    // Noise energy within echo channel referenced like the signal energy.
    //------------------------------------------------------------------------//

    double En_cal = N0_echo * Be * Tg;

    //cout << N0_echo << endl;
    //cout << En_cal << endl;

    //------------------------------------------------------------------------//
    // Signal + Noise Energy within echo channel referenced as above.
    // Noise channel measurements to get perfect reversal.
    //------------------------------------------------------------------------//

    *Esn_echo_cal = (float)(Es_cal + En_cal);
    *Esn_noise_cal = (float)(beta*Es_cal + alpha*beta*En_cal);

    //cout << *Esn_echo_cal << endl;
    //cout << *Esn_noise_cal << endl;
    //exit(1);

    return(1);
}

//
// make_load_measurements
//
// This function computes the load cal pulse energy received for a
// given instrument state.
// The received energy is
// the noise energy that falls within the appropriate bandwidth.
// The result could be fuzzed by Kpc which is where Kpri really comes from,
// but this is NOT done right now.  Instead, Kpri noise is applied directly
// to the loopback signal energy.  The load measurements recover alpha
// perfectly right now.
//
// Inputs:
//	ovwm = pointer to current Ovwm object
//	En_echo_load = pointer to load noise energy in the echo channel.
//	En_noise_load = pointer to load noise energy in the noise channel.
//

int
make_load_measurements(
    Ovwm*       ovwm,
    float*       En_echo_load,
    float*       En_noise_load)
{
    //-------------------------------------------//
    // Compute load noise measurements to assure //
    // a perfect retrieval of alpha.             //
    //-------------------------------------------//

    double Tg = ovwm->GetRxGateWidth();
    double Bn = ovwm->ses.noiseBandwidth;
    double Be = ovwm->ses.chirpBandwidth;

    double N0_echo = bK * ovwm->systemTemperature *
        ovwm->ses.rxGainEcho / ovwm->ses.receivePathLoss;
    double N0_noise = bK * ovwm->systemTemperature *
        ovwm->ses.rxGainNoise / ovwm->ses.receivePathLoss;

    //cout << ovwm->systemTemperature << endl;
    //cout << ovwm->ses.rxGainEcho << endl;
    //cout << ovwm->ses.rxGainNoise << endl;
    //cout << ovwm->ses.receivePathLoss << endl;

    *En_echo_load = N0_echo * Be * Tg;
    *En_noise_load = N0_noise * Bn * Tg;

    //cout << *En_echo_load << endl;
    //cout << *En_noise_load << endl;
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
//	ovwm = pointer to current Ovwm object
//	spot = pointer to current spot (Psn)
//		Note: the measurements in this spot must ALREADY have Psn values
//			stored in the value member in the same units that this method
//			uses.
//	Esn_noise = pointer to return variable
//
//

int
sigma0_to_Esn_noise(
    Ovwm*       ovwm,
    MeasSpot*    spot,
    int          sim_kpc_flag,
    float*       Esn_noise)
{

    //------------------------------------------------------------------------//
    // Noise power spectral densities referenced the same way as the signal.
    //------------------------------------------------------------------------//

    double N0_noise = bK * ovwm->systemTemperature *
        ovwm->ses.rxGainNoise / ovwm->ses.receivePathLoss;

    //------------------------------------------------------------------------//
    // Useful quantities.
    //------------------------------------------------------------------------//

    /*
     * For OVWM which operating in burst mode, the Tg used for generating
     * measurements is not the whole receive window. It depends on desired
     * max range swath and number of pulses in burst.
     */

    //double Tg = ovwm->GetRxGateWidth();
    double Tg = ovwm->ses.numRangePixels*(ovwm->ses.rangeRes*2./speed_light_kps)*
                ovwm->ses.numPulses;
    //cout << "Tg: " << Tg << endl;

    double Bn = ovwm->ses.noiseBandwidth;
    double Be = ovwm->ses.chirpBandwidth;
    double beta = ovwm->ses.rxGainNoise / ovwm->ses.rxGainEcho;

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

    if (sim_kpc_flag == 0)
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

//------------------------------------------------------------------------//
// Er_to_Es
//
// The Er_to_Es function estimates the signal only energy
// in a pixel from five signal+noise measurements and assuming
// noise is evenly distributed in all pixels of the spot.
// The noise channel measurements include the echo channel but with
// a different gain. The load measurements
// are used to calibrate the bandwidth ratio of the noise filter to the
// echo filter (alpha).  The receiver gain ratio (beta) between the
// noise filter and the echo filter is a pre-launch calibrated constant.
//  
// Inputs:
//  Esn_pixel = The pixel echo channel energy (Esn_slice)
//  Esn_echo = The total echo channel energy (Esn_echo)
//  Esn_noise = The corresponding noise channel energy (Esn_noise)
//  En_echo_load = The corresponding load echo channel energy (En_echo_load)
//  En_noise_load = The corresponding load noise channel energy (En_noise_load)
//      numPixels = number of pixels in the spot
//      Es_pixel = pointer to the computed pixel signal energy.
//      En_pixel = pointer to the computed pixel noise energy.
//
//------------------------------------------------------------------------//

int
Er_to_Es(
    float   beta,
    float   Esn_pixel,
    float   Esn_echo,
    float   Esn_noise,
    float   En_echo_load,
    float   En_noise_load,
    int     numPixels,
    float*  Es_pixel,
    float*  En_pixel)
{
    // Compute alpha from the load measurements.
    if (En_echo_load == 0.0)
    {
      fprintf(stderr,"Error, echo channel load energy is 0\n");
      return(0);
    }   

    double alpha = 1.0/beta * En_noise_load/En_echo_load;

    // Estimate the noise energy in the slice. (exact when 0 variance is used)
    if (alpha == 1.0 || beta == 0.0)
    {
      fprintf(stderr,"Error, bad alpha and/or beta values\n");
      return(0);
    }

    double EnSpot = 1.0/(1.0 - alpha)*(Esn_echo - Esn_noise/beta);
    *En_pixel = EnSpot/numPixels;
    
    // Subtract out slice noise, leaving the signal power fuzzed by Kpc.
    *Es_pixel = Esn_pixel - *En_pixel;
    
    return(1);
}   

