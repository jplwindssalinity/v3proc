//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_kp_c[] =
    "@(#) $Id$";

#include "Kp.h"


//====//
// Kp //
//====//

Kp::Kp()
:   kpc2Constant(0.0), kpm2Constant(0.0), kpri2Constant(0.0),
    kprs2Constant(0.0), useConstantValues(0)
{
    return;
}

Kp::Kp(
    float  kpc_val,
    float  kpm_val,
    float  kpri_val,
    float  kprs_val)
{
    useConstantValues = 1;
    kpc2Constant = kpc_val*kpc_val;
    kpm2Constant = kpm_val*kpm_val;
    kpri2Constant = kpri_val*kpri_val;
    kprs2Constant = kprs_val*kprs_val;
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
        // Constant Value case        //
        //----------------------------//
        if(useConstantValues){
	  *kpc2=kpc2Constant;
          return(1);
	}

	//----------------------------//
	// check for division by zero //
	//----------------------------//

	if (meas->EnSlice == 0)
		return(0);

	//-----------------------------//
	// calculate and check the SNR //
	//-----------------------------//

	double snr = sigma_0 * meas->XK * meas->txPulseWidth /
		meas->EnSlice;
	if (snr <= 0.0)
	{
		fprintf(stderr, "Kp::GetKpc2: SNR <= 0.0\n");
		fprintf(stderr, "  s0 = %g, XK = %g, Tp = %g, EnSlice = %g\n",
			sigma_0, meas->XK, meas->txPulseWidth, meas->EnSlice);
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
    Meas::MeasTypeE  meas_type,
    float            speed,
    double*          kpm2)
{
    //---------------------//
    // constant value case //
    //---------------------//

    if (useConstantValues)
    {
        *kpm2 = kpm2Constant;
        return(1);
    }

    double kpm_value;
    if (! kpm.GetKpm(meas_type, speed, &kpm_value))
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
        //----------------------------//
        // Constant Value case        //
        //----------------------------//
        if(useConstantValues){
	  *kpri2=kpri2Constant;
          return(1);
	}

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
        //----------------------------//
        // Constant Value case        //
        //----------------------------//
        if(useConstantValues){
	  *kprs2=kprs2Constant;
          return(1);
	}

	if(kprs.Empty())
	{
		*kprs2=0.005;
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
    Meas*            meas,
    double           sigma_0,
    Meas::MeasTypeE  meas_type,
    float            speed,
    double*          kp2)
{
    double kpc2, kpm2, kpri2, kprs2;

    if (! GetKpc2(meas, sigma_0, &kpc2) ||
        ! GetKpm2(meas_type, speed, &kpm2) ||
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

  //----------------------------//
  // Constant Value case        //
  //----------------------------//

  if(useConstantValues)
  {
    *vpc=kpc2Constant*sigma_0*sigma_0;
    return(1);
  }

  //--------------------------------//
  // calculate sigma-0 coefficients //
  //--------------------------------//

  double sigma0_over_snr = meas->EnSlice / meas->XK / meas->txPulseWidth;
  double aa = meas->A;
  double bb = meas->B * sigma0_over_snr;
  double cc = meas->C * sigma0_over_snr * sigma0_over_snr;

  //--------------------//
  // calculate variance //
  //--------------------//

  *vpc = (aa * sigma_0 + bb) * sigma_0 + cc;
  return(1);

}

//------------//
// Kp::GetVpc //
//------------//

// This is the polarimetric version overloaded onto the same name.

int
Kp::GetVpc(
	Meas*		meas,
	double		sigma0_corr,
	double		sigma0_copol,
	double		sigma0_xpol,
	double*		vpc)
{

  //-------------------------------//
  // Redirect copol and xpol cases //
  //-------------------------------//

  if (meas->measType == Meas::VV_MEAS_TYPE ||
      meas->measType == Meas::HH_MEAS_TYPE)
  {
    return(GetVpc(meas,sigma0_copol,vpc));
  }
  else if (meas->measType == Meas::VH_MEAS_TYPE ||
      meas->measType == Meas::HV_MEAS_TYPE)
  {
    return(GetVpc(meas,sigma0_xpol,vpc));
  }
  
  //----------------------------//
  // Constant Value case        //
  //----------------------------//

  if(useConstantValues)
  {
    *vpc=kpc2Constant*sigma0_corr*sigma0_corr;
    return(1);
  }

  //--------------------//
  // calculate variance //
  //--------------------//

  double sigma0_over_snr = meas->EnSlice / meas->XK / meas->txPulseWidth;
  *vpc = 0.5 / meas->txPulseWidth / meas->bandwidth *
         (sigma0_corr*sigma0_corr + sigma0_copol*sigma0_xpol +
           (sigma0_copol + sigma0_xpol) * sigma0_over_snr +
           sigma0_over_snr*sigma0_over_snr
         );

  return(1);

}

//-----------//
// Kp::GetVp //
//-----------//

int
Kp::GetVp(
    Meas*            meas,
    double           sigma_0,
    Meas::MeasTypeE  meas_type,
    float            speed,
    double*          vp)
{
    double vpc;
    double kpm2, kpri2, kprs2;
    if (! GetVpc(meas, sigma_0, &vpc) ||
        ! GetKpm2(meas_type, speed, &kpm2) ||
        ! GetKpri2(&kpri2) ||
        ! GetKprs2(meas, &kprs2))
    {
        return(0);
    }

    *vp = vpc + (kpm2 + kpri2 + kprs2) * sigma_0 * sigma_0;

    return(1);
}
