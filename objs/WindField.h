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

	//--------------//
	// input/output //
	//--------------//

	int		ReadVap(const char* filename);

	//--------//
	// access //
	//--------//

	WindVector*		NearestWindVector(double longitude, double latitude);

protected:

	//--------------//
	// construction //
	//--------------//

	int		_Allocate();
	int		_Deallocate();

	//-----------//
	// variables //
	//-----------//

	int				_lonCount;
	double			_lonMin;
	double			_lonMax;
	double			_lonStep;

	int				_latCount;
	double			_latMin;
	double			_latMax;
	double			_latStep;

	WindVector***	_field;			// a 2-d array of pointers
};

#endif
