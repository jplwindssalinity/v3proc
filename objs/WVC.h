//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef WVC_H
#define WVC_H

static const char rcs_id_wvc_h[] =
	"@(#) $Id$";

#include "List.h"
#include "WindVector.h"


//======================================================================
// CLASSES
//		WVC
//======================================================================

//======================================================================
// CLASS
//		WVC
//
// DESCRIPTION
//		The WVC object represents a wind vector cell.  It contains
//		the ambiguous solution WindVectors.
//======================================================================

class WVC
{
public:

	//--------------//
	// construction //
	//--------------//

	WVC();
	~WVC();

	//-----------//
	// variables //
	//-----------//

	List<WindVector>	ambiguities;
	WindVector			truth;
};

#endif
