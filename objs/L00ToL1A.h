//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L00TOL10_H
#define L00TOL10_H

static const char rcs_id_l00tol10_h[] =
	"@(#) $Id$";

#include "L00.h"
#include "L10.h"


//======================================================================
// CLASSES
//		L00ToL10
//======================================================================

//======================================================================
// CLASS
//		L00ToL10
//
// DESCRIPTION
//		The L00ToL10 object is used to convert between Level 0.0 data
//		and Level 1.0 data.  It performs all of the engineering unit
//		conversions.
//======================================================================

class L00ToL10
{
public:

	//--------------//
	// construction //
	//--------------//

	L00ToL10();
	~L00ToL10();

	//------------//
	// conversion //
	//------------//

	int		Convert(L00* l00, L10* l10);
};

#endif
