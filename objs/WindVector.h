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

	//---------//
	// setting //
	//---------//

	int		SetSpdDir(double speed, double direction);
	int		SetUV(double u, double v);

	//-----------//
	// variables //
	//-----------//

	double		spd;
	double		dir;		// ccw from east
};

#endif
