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

	//--------------//
	// input/output //
	//--------------//

	int		WriteL20(FILE* fp);

	//-----------//
	// operators //
	//-----------//

	int		operator==(WindVector wv);

	//---------//
	// setting //
	//---------//

	int		SetSpdDir(float speed, float direction);
	int		SetUV(float u, float v);

	//-----------//
	// variables //
	//-----------//

	float	spd;
	float	dir;	// ccw from east
	float	obj;	// objective function value
};

#endif
