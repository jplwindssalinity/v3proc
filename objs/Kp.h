//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef KP_H
#define KP_H

static const char rcs_id_kp_h[] =
	"@(#) $Id$";

#include "Kpm.h"
#include "Kpr.h"


//======================================================================
// CLASSES
//		Kp
//======================================================================

//======================================================================
// CLASS
//		Kp
//
// DESCRIPTION
//		The Kp object holds estimates of Kpc, Kpm, instrument Kpr,
//		and spacecraft Kpr.
//======================================================================

class Kp
{
public:

	//--------------//
	// construction //
	//--------------//

	Kp();
	~Kp();

	//--------------//
	// accessing Kp //
	//--------------//

	int		GetKpc2(Meas* meas, double sigma_0, double* kpc2);
	int		GetKpm2(int pol_idx, float speed, double* kpm2);
	int		GetKpri2(double* kpri2);
	int		GetKprs2(double* kprs2, int beam_number, int slice_number, float azimuth);
	int		GetTotalKp2(Meas* meas, double sigma_0, int pol_idx, float speed,
				int beam_number, int slice_number, float azimuth,
				double* kp2);

	int		GetVariance(Meas* meas, double sigma_0, int pol_idx, float speed,
				int beam_number, int slice_number, float azimuth,
				double* var);

	//-----------//
	// variables //
	//-----------//

	Kpm		kpm;
	Kpri	kpri;
        Kprs	kprs;
};

#endif
