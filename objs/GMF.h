//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef GMF_H
#define GMF_H

static const char rcs_id_gmf_h[] =
	"@(#) $Id$";

#include "PiscTable.h"
#include "Wind.h"
#include "Meas.h"


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

	int		GetCoefs(PolE pol, float inc, float spd, float* A0, float* A1,
				float* A1_phase, float* A2, float* A2_phase, float* A3,
				float* A3_phase, float* A4, float* A4_phase);

	//----------------//
	// wind retrieval //
	//----------------//

	int		FindSolutions(MeasList* meas_list, WVC* wvc,
				float spd_step, float phi_step);
	int		RefineSolutions(MeasList* meas_list, WVC* wvc,
				float initial_spd_step, float initial_phi_step,
				float final_spd_step, float final_phi_step);

	int		ModCurves(FILE* ofp, MeasList* meas_list,
				float spd_step, float phi_step);

	int		SolutionCurve(MeasList* meas_list, float spd_step, int phi_count,
				float* best_spd, float* best_obj);

protected:

	//----------------//
	// wind retrieval //
	//----------------//

	float	_ObjectiveFunction(MeasList* meas_list, float u,
				float phi);
	int		_FindSolutionCurve(MeasList* meas_list, float dspd,
				float dphi, int phi_count, float* best_spd,
				float* best_obj);
};

#endif
