//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_kp_c[] =
	"@(#) $Id$";

#include "Kp.h"


//====//
// Kp //
//====//

Kp::Kp()
{
	return;
}

Kp::~Kp()
{
	return;
}

//-------------//
// Kp::GetKpc2 //
//-------------//

int
Kp::GetKpc2(
	Meas*		meas,
	double		sigma_0,
	double*		kpc2)
{
	//----------------------------//
	// check for division by zero //
	//----------------------------//

	if (meas->EnSlice == 0)
		return(0);

	//-----------------------------//
	// calculate and check the SNR //
	//-----------------------------//

	double snr = sigma_0 * meas->XK * meas->transmitPulseWidth /
		meas->EnSlice;
	if (snr <= 0.0)
	{
		fprintf(stderr, "Kp::GetKpc2: SNR <= 0.0\n");
		fprintf(stderr, "  s0 = %g, XK = %g, Tp = %g, EnSlice = %g\n",
			sigma_0, meas->XK, meas->transmitPulseWidth, meas->EnSlice);
		return(0);
	}

	*kpc2 = meas->A + meas->B / snr + meas->C / (snr * snr);
	return(1);
}

//-------------//
// Kp::GetKpm2 //
//-------------//

int
Kp::GetKpm2(
	int			pol_idx,
	float		speed,
	double*		kpm2)
{
	double kpm_value;
	if (! kpm.GetKpm(pol_idx, speed, &kpm_value))
		return(0);

	*kpm2 = kpm_value * kpm_value;
	return(1);
}

//--------------//
// Kp::GetKpri2 //
//--------------//

int
Kp::GetKpri2(
	double*		kpri2)
{
	if (! kpri.GetKpri2(kpri2))
		return(0);

	return(1);
}

//--------------//
// Kp::GetKprs2 //
//--------------//

int
Kp::GetKprs2(
	double*		kprs2,
	int			beam_number,
	int			slice_number,
	float		azimuth)
{
	if(kprs.Empty())
	{
		*kprs2=0.0;
		return(1);
	}
	*kprs2 = kprs.Interpolate(beam_number,slice_number,azimuth);
	*kprs2 *= *kprs2;
	return(1);
}

//-----------------//
// Kp::GetTotalKp2 //
//-----------------//

int
Kp::GetTotalKp2(
	Meas*		meas,
	double		sigma_0,
	int			pol_idx,
	float		speed,
	int			beam_number,
	int			slice_number,
	float		azimuth,
	double*		kp2)
{
	double kpc2, kpm2, kpri2, kprs2;

	if (! GetKpc2(meas, sigma_0, &kpc2) ||
		! GetKpm2(pol_idx, speed, &kpm2) ||
		! GetKpri2(&kpri2) ||
		! GetKprs2(&kprs2, beam_number, slice_number, azimuth))
	{
		return(0);
	}

	*kp2 = kpc2 + kpm2 + kpri2 + kprs2;
	return(1);
}

//-----------------//
// Kp::GetVariance //
//-----------------//

int
Kp::GetVariance(
	Meas*		meas,
	double		sigma_0,
	int			pol_idx,
	float		speed,
	int			beam_number,
	int			slice_number,
	float		azimuth,
	double*		var)
{
	if(! GetTotalKp2(meas, sigma_0, pol_idx, speed, beam_number,
			slice_number, azimuth, var))
	{
		return(0);
	}
	if(*var == 0)
		*var=1;
	else
		*var*=sigma_0*sigma_0;

	return(1);
}
