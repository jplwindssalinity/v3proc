//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_pscat_c[] =
    "@(#) $Id$";

#include "Pscat.h"
#include "PMeas.h"
#include "Misc.h"
#include "Distributions.h"

//============//
// PscatEvent //
//============//

const char* pscat_event_map[] = { "None", "VV", "HH", "VVHV", "HHVH",
    "LOOP", "LOAD" };

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

//------------//
// MakeSlices //
//------------//

// This method creates the measurement list (of PMeas's) for a spot
// and sets the PMeas indices (with one slice per PMeas).

int
Pscat::MakeSlices(
    MeasSpot*  meas_spot)
{
    meas_spot->FreeContents();
    int total_slices = ses.GetTotalSliceCount();

    for (int slice_idx = 0; slice_idx < total_slices; slice_idx++)
    {
        PMeas* pmeas = new PMeas();
        // We assume that the slices are sequential.
        abs_to_rel_idx(slice_idx,total_slices,&(pmeas->startSliceIdx));
        pmeas->numSlices = 1;
        meas_spot->Append(pmeas);
    }

    return(1);
}

//------------//
// PMeasToEsn //
//------------//

// This method converts a sigma0 measurement (of a particular type)
// to signal + noise energy measurements.

int
Pscat::PMeasToEsn(
    PMeas*  meas,
    PMeas*  meas1,
    PMeas*  meas2,
    float   X,
    float   sigma0,
    int     sim_kpc_flag,
    float*  Esn,
    float*  Es,
    float*  En,
    float*  var_Esn)
{
    SesBeamInfo* ses_beam_info = GetCurrentSesBeamInfo();
    double Tp = ses.txPulseWidth;
    double Tg = ses_beam_info->rxGateWidth;
    double Bs = meas->bandwidth;

    //--------------------------------------------------------------------//
    // Signal (ie., echo) energy referenced to the point just before the
    // I-Q detection occurs (ie., including the receiver gain and system loss).
    // X has units of energy because Xcal has units of Pt * Tp.
    //--------------------------------------------------------------------//

    *Es = X * sigma0;

    //-------------------------------------------//
    // use measurement type to decide what to do //
    //-------------------------------------------//

    double N0_echo, En1, En2;
    switch (meas->measType)
    {
    case Meas::VV_MEAS_TYPE:
    case Meas::HH_MEAS_TYPE:
        N0_echo = bK * systemTemperature * ses.rxGainEcho /
            ses.receivePathLoss;
        En1 = N0_echo * Bs * Tp;       // noise with signal
        En2 = N0_echo * Bs * (Tg-Tp);  // noise without signal
        *En = En1 + En2;
        *Esn = (float)(*Es + *En);
        break;
    case Meas::VH_MEAS_TYPE:
    case Meas::HV_MEAS_TYPE:
        // these don't get processed, so who cares?
        break;
    case Meas::VV_HV_CORR_MEAS_TYPE:
    case Meas::HH_VH_CORR_MEAS_TYPE:
        *Esn = *Es;
        break;
    default:
        return(0);
        break;
    }

    if (sim_kpc_flag == 0)
    {
        *var_Esn = 0.0;
        return(1);
    }

    //--------------------------------------------------------------------//
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
    //--------------------------------------------------------------------//

    if (meas->measType == Meas::VV_MEAS_TYPE ||
        meas->measType == Meas::HH_MEAS_TYPE ||
        meas->measType == Meas::VH_MEAS_TYPE ||
        meas->measType == Meas::HV_MEAS_TYPE)
    {
        // co-pol or cross-pol measurement
        if (Tg > Tp)
        {
            *var_Esn = (*Es + En1)*(*Es + En1) / (Bs*Tp) +
                En2*En2 / (Bs*(Tg - Tp));
        }
        else
        {
            // receive gate is filled, so no noise only portion
            *var_Esn = (*Es + En1)*(*Es + En1) / (Bs*Tg);
        }
    }
    else if (meas1 != NULL && meas2 != NULL)
    {
        // correlation measurement (ie., sigma0 = sigma_vvhv or sigma_hhvh)
        double var_corr = (sigma0*sigma0 + meas1->Sigma0 * meas2->Sigma0 *
            (1.0 + 1.0/meas1->Snr)*(1.0 + 1.0/meas2->Snr)) / (2*Bs*Tp);
        if (Tg > Tp)
        {
            // since Es=X^2*sigma0, var(Es) = X^2*var(sigma0)
            *var_Esn = X*X*var_corr + En2*En2 / (Bs*(Tg - Tp));
        }
        else
        {
            // receive gate is filled, so no noise only portion
            *var_Esn = X*X*var_corr;
        }
    }
    else
    {
        fprintf(stderr, "Error, Pscat::PMeasToEsn needs valid copol and xpol measurements to process correlation measurements\n");
        exit(1);
    }

    //--------------------------------------------------------------------//
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
    //--------------------------------------------------------------------//

    Gaussian rv(*var_Esn,0.0);
    *Esn += rv.GetNumber();

    return(1);
}
