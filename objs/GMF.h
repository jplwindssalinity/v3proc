//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef GMF_H
#define GMF_H

static const char rcs_id_gmf_h[] =
	"@(#) $Id$";

//======================================================================
// CLASSES
//		GMF
//======================================================================

//======================================================================
// CLASS
//		GMF
//
// DESCRIPTION
//		The GMF object contains a geophysical model function in
//		tabular format and allows easy access.
//======================================================================

class GMF
{
public:

	//--------------//
	// construction //
	//--------------//

	GMF();
	~GMF();

	//--------------//
	// input/output //
	//--------------//

	int		Read(const char* filename);
	int		ReadOldStyle(const char* filename);
	int		ReadHeader(int fd);
	int		ReadTable(int fd);

	//--------//
	// access //
	//--------//

	// directions dir and chi are w.r.t. north
	int		GetNearestSigma0(int pol, double inc, double spd, double chi,
				double* sigma_0);
	int		GetInterSigma0(int pol, double inc, double spd, double chi,
				double* sigma_0);

	//---------//
	// analyze //
	//---------//

	int		GetCoefs(int pol, double inc, double spd, double* A0, double* A1,
				double* A1_phase, double* A2, double* A2_phase);

protected:

	int		_Allocate();

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
	double	_chiMin;		// the minimum relative azimuth angle
	double	_chiMax;		// the maximum relative azimuth angle
	double	_chiStep;		// the relative azimuth angle step size

	double**** _sigma0;		// the array of sigma-0 values
};

#endif
