//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef GMF_H
#define GMF_H

static const char rcs_id_gmf_h[] =
	"@(#) $Id$";

#include "Measurement.h"
#include "WVC.h"


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
//		tabular format.  It contains methods to access the model
//		function as well as methods for performing wind retrieval.
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

	//--------//
	// access //
	//--------//

	int		GetNearestValue(PolE pol, double inc, double spd, double chi,
				double* value);
	int		GetInterpolatedValue(PolE pol, double inc, double spd, double chi,
				double* value);

	//---------//
	// analyze //
	//---------//

	int		GetCoefs(PolE pol, double inc, double spd, double* A0, double* A1,
				double* A1_phase, double* A2, double* A2_phase, double* A3,
				double* A3_phase, double* A4, double* A4_phase);

	//----------------//
	// wind retrieval //
	//----------------//

	int		FindSolutions(MeasurementList* measurement_list, WVC* wvc,
				double initial_spd, double spd_step, double phi_step);

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

	//----------------//
	// wind retrieval //
	//----------------//

	double	_ObjectiveFunction(MeasurementList* measurement_list, double u,
				double phi);

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

	double****	_value;		// the array of model function values
};

#endif
