//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef KPMFIELD_H
#define KPMFIELD_H

static const char rcs_id_kpmgrid_h[] =
	"@(#) $Id$";

#include "LonLat.h"
#include "Distributions.h"
#include "EarthField.h"

#define STEPS_PER_CORRLENGTH	5
#define N_CORRLENGTHS_INTEGRATE	5

//======================================================================
// CLASSES
//		KpmField
//======================================================================

//======================================================================
// CLASS
//		KpmField
//
// DESCRIPTION
//		The KpmField object manages a global grid of spatially correlated
//		deviations which are used to represent model function errors
//		when relating wind speed and direction to sigma0.
//======================================================================

class KpmField
{
public:

	//--------------//
	// construction //
	//--------------//

	KpmField();
	~KpmField();
	int Build(float corr_length);

	//--------------//
	// access
	//--------------//

	float GetRV(int polarization, float wspd, LonLat lon_lat);
	float GetKpm(int polarization, float wspd);

    //-----------//
    // variables //
    //-----------//

	// The two fields of random values, one correlated, the other not.
	EarthField corr;
	EarthField uncorr;

protected:

	// Supplies gaussian random values with unit variance and zero mean.
	Gaussian _gaussianRv;

	// Spatial correlation length (km) of this field.
	float _corrLength;

};

#endif
