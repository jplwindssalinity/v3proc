//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef L2ATOL2B_H
#define L2ATOL2B_H

static const char rcs_id_l2atol2b_h[] =
	"@(#) $Id$";

#include "L2A.h"
#include "L2B.h"
#include "GMF.h"

#define DESIRED_SOLUTIONS		4


//======================================================================
// CLASSES
//		L2AToL2B
//======================================================================

//======================================================================
// CLASS
//		L2AToL2B
//
// DESCRIPTION
//		The L2AToL2B object is used to convert between Level 2A data
//		and Level 2B data.  It performs wind retrieval.
//======================================================================

class L2AToL2B
{
public:

    //------//
    // enum //
    //------//

    enum WindRetrievalMethodE { GS, GS_FIXED, H1, H2, H3, S1, S2, S3,
				S4, POLAR_SPECIAL, CHEAT };

    //--------------//
    // construction //
    //--------------//
    
    L2AToL2B();
    ~L2AToL2B();
    
    //---------//
    // setting //
    //---------//

    int    SetWindRetrievalMethod(const char* wr_method);
    
    //------------//
    // conversion //
    //------------//
    
    int		ConvertAndWrite(L2A* l2a, GMF* gmf, Kp* kp, L2B* l2b);
    int		Flush(L2B* l2b);
    
    //------------------------------------------//
    // Routine for outputting the Nudge Field   //
    // wind vector                              //
    //------------------------------------------//
	
    int             Cheat(MeasList* meas_list, WVC* wvc);

    //-----------//
    // debugging //
    //-----------//

    int		WriteSolutionCurves(L2A* l2a, GMF* gmf, Kp* kp,
				    const char* output_file);

    //----------------------//
    // processing variables //
    //----------------------//
    
    int  medianFilterWindowSize;
    int  medianFilterMaxPasses;
    int  maxRankForNudging;

    //-------//
    // flags //
    //-------//

    int                   useManyAmbiguities;
    int                   useAmbiguityWeights;
    int                   useNudging;
    int                   smartNudgeFlag;
    WindRetrievalMethodE  wrMethod;
    int                   useNudgeThreshold;
    int                   useNMF;
    int                   useRandomInit;
    
    //-----------------------------------------//
    // Parameters for Peak Splitting Algorithm //
    //-----------------------------------------//
    
    float	onePeakWidth;
    float	twoPeakSep;
    float	probThreshold;
    
    //-------------------//
    // nudging variables //
    //-------------------//
    
    WindField	nudgeField;
    WindVectorField nudgeVctrField;
    float nudgeThresholds[2];

    //------------------------------------------------//
    // Auxiliary Variables For Hurricane Processing   //
    //------------------------------------------------//
    int        useHurricaneNudgeField;
    WindField  hurricaneField;
    float hurricaneRadius;  // km 
    EarthPosition hurricaneCenter; 
};

#endif
