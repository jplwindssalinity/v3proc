//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef KPM_H
#define KPM_H

static const char rcs_id_kpm_h[] =
	"@(#) $Id$";

#include "Index.h"
#include "EarthField.h"
#include "Distributions.h"

//======================================================================
// CLASSES
//		Kpm, KpmField
//======================================================================

//======================================================================
// CLASS
//		Kpm
//
// DESCRIPTION
//		The Kpm object provides values of Kpm as needed.
//======================================================================

class Kpm
{
public:

	//--------------//
	// construction //
	//--------------//

	Kpm();
	~Kpm();

	//--------------//
	// input/output //
	//--------------//

	int		ReadTable(const char* filename);
	int		WriteTable(const char* filename);

	//---------//
	// getting //
	//---------//

	int		GetKpm(int pol_idx, float speed, double* kpm);

	//-----------//
	// variables //
	//-----------//

protected:

	//--------------//
	// construction //
	//--------------//

	int		_Allocate();
	int		_Deallocate();

	//--------------//
	// input/output //
	//--------------//

	int		_ReadHeader(FILE* fp);
	int		_WriteHeader(FILE* fp);
	int		_ReadTable(FILE* fp);
	int		_WriteTable(FILE* fp);

	//-----------//
	// variables //
	//-----------//

	int			_polCount;
	Index		_speedIdx;
	float**		_table;
};


//======================================================================
// CLASS
//		KpmField
//
// DESCRIPTION
//		The KpmField object manages a global grid of spatially correlated
//		deviations which are used to represent model function errors
//		when relating wind speed and direction to sigma0.
//======================================================================

#define STEPS_PER_CORRLENGTH	5
#define N_CORRLENGTHS_INTEGRATE	5

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

	float GetRV(Kpm* kpm, int polarization, float wspd, LonLat lon_lat);

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
