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
	Meas* meas,
	double*		kprs2)
{
	if(kprs.Empty())
	{
		*kprs2=0.0;
		return(1);
	}
        int beam_number = meas->beamIdx;
        int start_slice_rel_idx = meas->startSliceIdx;
        int num_slices_per_comp = meas->numSlices;
        float azimuth = meas->scanAngle;
	*kprs2 = kprs.Interpolate(beam_number,num_slices_per_comp,
				  start_slice_rel_idx,azimuth);
	*kprs2 *= *kprs2;
	return(1);
}

//------------//
// Kp::GetKp2 //
//------------//

int
Kp::GetKp2(
	Meas*		meas,
	double		sigma_0,
	int			pol_idx,
	float		speed,
	double*		kp2)
{
	double kpc2, kpm2, kpri2, kprs2;

	if (! GetKpc2(meas, sigma_0, &kpc2) ||
		! GetKpm2(pol_idx, speed, &kpm2) ||
		! GetKpri2(&kpri2) ||
		! GetKprs2(meas, &kprs2))
	{
		return(0);
	}

	*kp2 = kpc2 + kpm2 + kpri2 + kprs2;
	return(1);
}

//------------//
// Kp::GetVpc //
//------------//

int
Kp::GetVpc(
	Meas*		meas,
	double		sigma_0,
	double*		vpc)
{
	//--------------------------------//
	// calculate sigma-0 coefficients //
	//--------------------------------//

	double xktp = meas->XK * meas->transmitPulseWidth;
	double aa = meas->A;
	double bb = meas->B * meas->EnSlice / xktp;
	double cc = meas->C * meas->EnSlice * meas->EnSlice / (xktp * xktp);

	//--------------------//
	// calculate variance //
	//--------------------//

	*vpc = (aa * sigma_0 + bb) * sigma_0 + cc;
	return(1);
}

//-----------//
// Kp::GetVp //
//-----------//

int
Kp::GetVp(
	Meas*		meas,
	double		sigma_0,
	int			pol_idx,
	float		speed,
	double*		vp)
{
	double vpc;
	if (! GetVpc(meas, sigma_0, &vpc))
	{
		return(0);
	}

	double kpm2, kpri2, kprs2;
	if (! GetKpm2(pol_idx, speed, &kpm2) ||
		! GetKpri2(&kpri2) ||
		! GetKprs2(meas, &kprs2))
	{
		return(0);
	}

	*vp = vpc + (kpm2 + kpri2 + kprs2) * sigma_0 * sigma_0;

	return(1);
}







