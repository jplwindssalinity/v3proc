//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef WINDVECTOR_H
#define WINDVECTOR_H

static const char rcs_id_windvector_h[] =
	"@(#) $Id$";

//======================================================================
// CLASSES
//		WindVector
//======================================================================

//======================================================================
// CLASS
//		WindVector
//
// DESCRIPTION
//		The WindVector object hold wind vector information (namely
//		speed and direction).
//======================================================================

class WindVector
{
public:

	//--------------//
	// construction //
	//--------------//

	WindVector();
	~WindVector();

	//-----------//
	// variables //
	//-----------//

	double		speed;
	double		direction;		// ccw/cw from ??
};

#endif
