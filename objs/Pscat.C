//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_pscat_c[] =
    "@(#) $Id$";

#include "Pscat.h"

//============//
// PscatEvent //
//============//

PscatEvent::PscatEvent()
:   eventTime(0.0), eventId(NONE), beamIdx(0)
{
    return;
}

PscatEvent::~PscatEvent()
{
    return;
}

//=======//
// Pscat //
//=======//

Pscat::Pscat()
{
    return;
}

Pscat::~Pscat()
{
    return;
}

//-----------//
// MeasToEsn //
//-----------//

// This method converts a sigma0 measurement (of a particular type)
// to signal + noise energy measurements.

int
Pscat::MeasToEsn(Meas* meas,
  Meas* prev_meas,
  float X,
  float sigma0,
  int sim_kpc_flag,
  float* Esn,
  float* Es,
  float* En,
  float* var_Esn)

{

  SesBeamInfo* ses_beam_info = GetCurrentSesBeamInfo();
  double Tp = ses.txPulseWidth;
  double Tg = ses_beam_info->rxGateWidth;
  double Bs = meas->bandwidth;

  //------------------------------------------------------------------------//
  // Signal (ie., echo) energy referenced to the point just before the
  // I-Q detection occurs (ie., including the receiver gain and system loss).
  // X has units of energy because Xcal has units of Pt * Tp.
  //------------------------------------------------------------------------//
 
  *Es = X*sigma0;
 
  //------------------------------------------------------------------------//
  // Noise power spectral densities referenced the same way as the signal.
  //------------------------------------------------------------------------//

  double N0_echo = bK * systemTemperature * ses.rxGainEcho /
    ses.receivePathLoss;

  //------------------------------------------------------------------------//
  // Noise energy within one slice referenced like the signal energy.
  //------------------------------------------------------------------------//

  double En1_slice = N0_echo * Bs * Tp;       // noise with signal
  double En2_slice = N0_echo * Bs * (Tg-Tp);  // noise without signal
  *En = En1_slice + En2_slice;

  //------------------------------------------------------------------------//
  // Signal + Noise Energy within one slice referenced like the signal energy.
  //------------------------------------------------------------------------//

  *Esn = (float)(*Es + *En);

  if (sim_kpc_flag == 0)
  {
      *var_Esn = 0.0;
      return(1);
  }
*var_Esn = 0.0;

/*
  //------------------------------------------------------------------------//
  // Estimate the variance of the slice signal + noise energy measurements.
  // For copolarized measurements,
  // the variance is simply the sum of the variance when the signal
  // (and noise) are present together and the variance when only noise
  // is present.  These variances come from radiometer theory, ie.,
  // the reciprocal of the time bandwidth product is the normalized variance.
  // The variance of the power is derived from the variance of the energy.
  // For correlation measurements,
  // the variance is a little more complicated as described in M. Spencer's
  // draft memo.  The true SNR for co-pol and cross-pol measurements is
  // required.  The variance from the noise only portion is added just like
  // the co-pol case.
  //------------------------------------------------------------------------//

  if (meas->measType == Meas::VV_MEAS_TYPE ||
      meas->measType == Meas::HH_MEAS_TYPE)
  {  // co-pol measurement
    if (Tg != Tp)
    {
      *var_Esn = (Es_slice + En1_slice)*(Es_slice + En1_slice) / (Bs*Tp) +
                  En2_slice*En2_slice / (Bs*(Tg - Tp));
    }
    else
    {  // perfect gate width, so no noise only portion
      *var_Esn = (Es_slice + En1_slice)*(Es_slice + En1_slice) / (Bs*Tp);
    }
  }
  else if (prev_meas != NULL)
  { // correlation measurement (ie., sigma0 = sigma_vvhv)
    float sig_vv = prev_meas->sigma0;
    float sig_hv = 0.0; // huh?  we need a more complete Meas type.
    float snr_vv = 1.0;
    float snr_hv = 1.0;
    double var_sigvvhv = (sigma0*sigma0 +
      sig_vv * sig_hv * (1.0/(1.0+snr_vv))*(1.0/(1.0+snr_hv))) / (2*Bs*Tp);
    if (Tg != Tp)
    {
      // since Es=X^2*sigma0, var(Es) = X^2*var(sigma0)
      *var_Esn = X*X*var_sigvvhv + En2_slice*En2_slice / (Bs*(Tg - Tp));
    }
    else
    {  // perfect gate width, so no noise only portion
      *var_Esn = X*X*var_sigvvhv;
    }
  }
  else
  {
    fprintf(stderr,"Error, Pscat::MeasToEsn needs a valid prev_meas for correlation measurements\n");
    exit(1);
  }

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

  Gaussian rv(*var_Esn,0.0);
  *Esn += rv.GetNumber();

*/
  return(1);
}

