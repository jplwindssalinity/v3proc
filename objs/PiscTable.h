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

	int		GetNearestValue(PolE pol, double inc, double spd, double chi,
				double* value);
	int		GetInterpolatedValue(PolE pol, double inc, double spd, double chi,
				double* value);

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
	int		_IncToIndex(double inc);
	int		_SpdToIndex(double spd);
	int		_ChiToIndex(double chi);

	double	_IncToRealIndex(double inc);
	double	_SpdToRealIndex(double spd);
	double	_ChiToRealIndex(double chi);

	int		_ClipPolIndex(int pol_idx);
	int		_ClipIncIndex(int inc_idx);
	int		_ClipSpdIndex(int spd_idx);
	int		_ClipChiIndex(int chi_idx);

	double	_IndexToSpd(int spd_idx);
	double	_IndexToChi(int chi_idx);

	//-----------//
	// variables //
	//-----------//

	int		_polCount;		// the number of "polarizations"

	int		_incCount;		// the number of incidence angles
	double	_incMin;		// the minimum incidence angle
	double	_incMax;		// the maximum incidence angle
	double	_incStep;		// the incidence angle step size

	int		_spdCount;		// the number of wind speeds
	double	_spdMin;		// the minimum wind speed
	double	_spdMax;		// the maximum wind speed
	double	_spdStep;		// the wind speed step size

	int		_chiCount;		// the number of relative azimuth angles
	double	_chiStep;		// the relative azimuth angle step size

	double****	_value;		// the array of values
};

#endif
