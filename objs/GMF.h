//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef GMF_H
#define GMF_H

static const char rcs_id_gmf_h[] =
	"@(#) $Id$";

#include "PiscTable.h"
#include "Wind.h"
#include "Meas.h"
#include "Constants.h"
#include "Kp.h"


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

#define DEFAULT_SPD_TOL			0.1
#define DEFAULT_SEP_ANGLE		5.0*dtr
#define DEFAULT_SMOOTH_ANGLE	10.0*dtr
#define DEFAULT_MAX_SOLUTIONS	4
#define DEFAULT_PHI_COUNT		360

#define MINIMUM_WVC_MEASUREMENTS	4
#define MINIMUM_AZIMUTH_DIVERSITY	20.0*dtr

class GMF : public PiscTable
{
public:

	//--------------//
	// construction //
	//--------------//

	GMF();
	~GMF();

	//-----------------//
	// setting/getting //
	//-----------------//

	int		SetPhiCount(int phi_count);

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
	int		WriteSolutionCurves(FILE* ofp, MeasList* meas_list, Kp* kp);
	int		WritePdf(FILE* ofp, MeasList* meas_list, Kp* kp);
	int		WriteObjectiveCurve(FILE* ofp);
	int		AppendSolutions(FILE* ofp, WVC* wvc);

	//----------------//
	// wind retrieval //
	//----------------//

	int		CheckRetrieveCriteria(MeasList* meas_list);
	int		RetrieveWinds(MeasList* meas_list, Kp* kp, WVC* wvc);
	int		RetrieveManyWinds(MeasList* meas_list, Kp* kp, WVC* wvc);
	int             RetrieveWindsWithPeakSplitting(MeasList* meas_list,
                           Kp* kp, WVC* wvc, float one_peak_width, float
			   two_peak_separation_threshold, 
			   float threshold, int max_num_ambigs);
	int		SolutionCurve(MeasList* meas_list, Kp* kp);
	int		Smooth();
	int		FindMaxima(WVC* wvc);
	int		FindMany(WVC* wvc);

	//-------------------//
	// GS wind retrieval //
	//-------------------//

	int		GSRetrieveWinds(MeasList* meas_list, Kp* kp, WVC* wvc);
	int		Calculate_Init_Wind_Solutions(MeasList* meas_list, Kp* kp,
				WVC* wvc);
	int		Optimize_Wind_Solutions(MeasList* meas_list, Kp* kp,
				WVC* wvc);

	//-------//
	// flags //
	//-------//

	int		retrieveUsingKpcFlag;
	int		retrieveUsingKpmFlag;
	int		retrieveUsingKpriFlag;
	int		retrieveUsingKprsFlag;
	int		retrieveUsingLogVar;

protected:

	//----------------//
	// wind retrieval //
	//----------------//

	float	_ObjectiveFunction(MeasList* meas_list, float u, float phi,
				Kp* kp);
	int     _ObjectiveToProbability(float scale, int radius);

	//-----------//
	// variables //
	//-----------//

	int		_phiCount;		// number of angles for wind retrieval
	float	_phiStepSize;	// step size of angles
	float	_spdTol;		// speed tolerance for golden section search
	float	_sepAngle;		// minimum angle between solutions
	float	_smoothAngle;	// widest angle of smoothing obj
	int		_maxSolutions;	// the maximum number of solutions

	float*	_bestSpd;		// array to hold best speed for each direction
	float*	_bestObj;		// array to hold best objective for each direction
	float*	_copyObj;		// storage for a copy of obj
};

#endif







