//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef GMF_H
#define GMF_H

static const char rcs_id_gmf_h[] =
	"@(#) $Id$";

#include "PiscTable.h"
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

class GMF : public PiscTable
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

	int		ReadOldStyle(const char* filename);

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
				double spd_step, double phi_step);
	int		RefineSolutions(MeasurementList* measurement_list, WVC* wvc,
				double initial_spd_step, double initial_phi_step,
				double final_spd_step, double final_phi_step);

	int		ModCurves(FILE* ofp, MeasurementList* measurement_list,
				double spd_step, double phi_step);

protected:

	//----------------//
	// wind retrieval //
	//----------------//

	double	_ObjectiveFunction(MeasurementList* measurement_list, double u,
				double phi);
	int		_FindSolutionCurve(MeasurementList* measurement_list, double dspd,
				double dphi, int phi_count, int* best_spd_idx,
				double* best_obj);
};

#endif
