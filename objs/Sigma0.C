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
	double lambda = speed_light / instrument->baseTransmitFreq;
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
// sigma0_to_Pr
//
// The receive_power function computes the power received for a
// given instrument state and average sigma0.
//
// Inputs:
//  spacecraft = pointer to current spacecraft object
//  instrument = pointer to current instrument object
//	meas = pointer to current measurement (sigma0, cell center, area etc.)
//  Kfactor = Radar equation correction factor for this cell.
//  gc_to_antenna = pointer to a CoordinateSwitch from geocentric coordinates
//		to the antenna frame for the prevailing geometry.
//  Pr = pointer to return variable
//
//

int
sigma0_to_Pr(
	CoordinateSwitch*	gc_to_antenna,
	Spacecraft*			spacecraft,
	Instrument*			instrument,
	Meas*				meas,
	float				Kfactor,
	float				sigma0,
	float*				Pr)
{
	double X;
	radar_X(gc_to_antenna, spacecraft, instrument, meas, &X);
	*Pr = (float)(Kfactor*X*sigma0);
	return(1);
}

//
// Pr_to_sigma0
//
// The Pr_to_sigma0 function computes sigma0 from a signal+noise and noise power
// measurement for a given instrument state.
//
// Inputs:
//  spacecraft = pointer to current spacecraft object
//  instrument = pointer to current instrument object
//	meas = pointer to current measurement (sigma0, cell center, area etc.)
//  Kfactor = Radar equation correction factor for this cell.
//  gc_to_antenna = pointer to a CoordinateSwitch from geocentric coordinates
//		to the antenna frame for the prevailing geometry.
//  Pr = the received signal power (no noise).
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
	float				Pr,
	float*				sigma0)
{
	double X;
	radar_X(gc_to_antenna, spacecraft, instrument, meas, &X);
	*sigma0 = (float)(Pr / (X*Kfactor));
	return(1);
}
