//==============================================================//
// Copyright (C) 1997-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef KP_H
#define KP_H

static const char rcs_id_kp_h[] =
    "@(#) $Id$";

#include "Kpm.h"
#include "Kpr.h"
#include "AttenMap.h"


//======================================================================
// CLASSES
//    Kp
//======================================================================

//======================================================================
// CLASS
//    Kp
//
// DESCRIPTION
//    The Kp object holds estimates of Kpc, Kpm, instrument Kpr,
//    and spacecraft Kpr.
//======================================================================

class Kp
{
public:

    //--------------//
    // construction //
    //--------------//

    Kp();
    Kp(float kpc_val, float kpm_val, float kpri_val, float kprs_val);
    ~Kp();

    //--------------//
    // accessing Kp //
    //--------------//

    int  GetKpc2(Meas* meas, double sigma_0, double* kpc2);
    int  GetKpm2(Meas::MeasTypeE meas_type, float speed, double* kpm2);
    int  GetKpri2(double* kpri2);
    int  GetKprs2(Meas* meas, double* kprs2);
    int  GetKp2(Meas* meas, double sigma_0, Meas::MeasTypeE meas_type,
             float speed, double* kp2);

    //---------------------//
    // accessing variances //
    //---------------------//

    int  GetVpc(Meas* meas, double sigma_0, double* vpc);
    int  GetVpc(Meas* meas, double sigma0_corr, double sigma0_copol,
             double sigma0_xpol, double* vpc);
    int  GetVp(Meas* meas, double sigma_0, Meas::MeasTypeE meas_type,
             float speed, double* vp);

    //-----------//
    // variables //
    //-----------//

    Kpm       kpm;
    Kpri      kpri;
    Kprs      kprs;
    AttenMap  attenmap;
    float     kpc2Constant;
    float     kpm2Constant;
    float     kpri2Constant;
    float     kprs2Constant;
    int       useConstantValues;
    int       useAttenMap;
};

#endif
