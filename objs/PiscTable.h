//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef PISCTABLE_H
#define PISCTABLE_H

static const char rcs_id_pisctable_h[] =
	"@(#) $Id$";

#include "Beam.h"


//======================================================================
// CLASSES
//		PiscTable
//======================================================================

//======================================================================
// CLASS
//		PiscTable
//
// DESCRIPTION
//		The PiscTable object is a base class for GMF and Kp.  It
//		contains a table of values (model function or Kp) and some
//		access and I/O functions.
//
// NOTES
//		Pisc is an acronym for Polarization, Incidence angle, Speed,
//		and Chi (relative wind direction)
//======================================================================

class PiscTable
{
public:

	//--------------//
	// construction //
	//--------------//

	PiscTable();
	~PiscTable();

	//--------------//
	// input/output //
	//--------------//

	int		Read(const char* filename);
	int		Write(const char* filename);

	//--------//
	// access //
	//--------//

	int		GetInterpolatedValue(PolE pol, float inc, float spd,
				float chi, float* value);
	int		GetNearestValue(PolE pol, float inc, float spd, float chi,
				float* value);

protected:

	//--------------//
	// construction //
	//--------------//

	int		_Allocate();
	int		_Deallocate();

	//--------------//
	// input/output //
	//--------------//

	int		_ReadHeader(int fd);
	int		_ReadTable(int fd);
	int		_WriteHeader(int fd);
	int		_WriteTable(int fd);

	//--------//
	// access //
	//--------//

	int		_PolToIndex(PolE pol);
	int		_IncToIndex(float inc);
	int		_SpdToIndex(float spd);
	int		_ChiToIndex(float chi);

	int		_ClipPolIndex(int pol_idx);
	int		_ClipIncIndex(int inc_idx);
	int		_ClipSpdIndex(int spd_idx);
	int		_ClipChiIndex(int chi_idx);

	float	_IndexToSpd(int spd_idx);
	float	_IndexToChi(int chi_idx);

	//-----------//
	// variables //
	//-----------//

	int		_polCount;		// the number of "polarizations"

	int		_incCount;		// the number of incidence angles
	float	_incMin;		// the minimum incidence angle
	float	_incMax;		// the maximum incidence angle
	float	_incStep;		// the incidence angle step size

	int		_spdCount;		// the number of wind speeds
	float	_spdMin;		// the minimum wind speed
	float	_spdMax;		// the maximum wind speed
	float	_spdStep;		// the wind speed step size

	int		_chiCount;		// the number of relative azimuth angles
	float	_chiStep;		// the relative azimuth angle step size

	float****	_value;		// the array of values
};

#endif
