//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef WINDFIELD_H
#define WINDFIELD_H

static const char rcs_id_windfield_h[] =
	"@(#) $Id$";

#include "WindVector.h"


//======================================================================
// CLASSES
//		WindField
//======================================================================

//======================================================================
// CLASS
//		WindField
//
// DESCRIPTION
//		The WindField object holds a wind field.
//======================================================================

class WindField
{
public:

	//--------------//
	// construction //
	//--------------//

	WindField();
	~WindField();

	//-----------//
	// variables //
	//-----------//

protected:

	//-----------//
	// variables //
	//-----------//

	WindVector**	field;
};

#endif
