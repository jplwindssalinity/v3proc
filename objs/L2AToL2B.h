//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L17TOL20_H
#define L17TOL20_H

static const char rcs_id_l17tol20_h[] =
	"@(#) $Id$";

#include "L17.h"
#include "L20.h"
#include "GMF.h"

#define DESIRED_SOLUTIONS		4


//======================================================================
// CLASSES
//		L17ToL20
//======================================================================

//======================================================================
// CLASS
//		L17ToL20
//
// DESCRIPTION
//		The L17ToL20 object is used to convert between Level 1.7 data
//		and Level 2.0 data.  It performs wind retrieval.
//======================================================================

class L17ToL20
{
public:

	//--------------//
	// construction //
	//--------------//

	L17ToL20();
	~L17ToL20();

	//------------//
	// conversion //
	//------------//

	int		ConvertAndWrite(L17* l17, GMF* gmf, L20* l20);
	int		Flush(L20* l20);

	//-----------//
	// debugging //
	//-----------//

	int		WriteSolutionCurves(L17* l17, GMF* gmf, const char* output_file);

	//-----------//
	// variables //
	//-----------//

	float	phiStep;
	float	phiBuffer;
	float	phiMaxSmoothing;
	float	spdTolerance;
};

#endif
