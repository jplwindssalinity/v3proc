//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef WIND_H
#define WIND_H

static const char rcs_id_wind_h[] =
	"@(#) $Id$";

#include <stdio.h>
#include "List.h"


//======================================================================
// CLASSES
//		WindVector, WindVectorPlus, WVC, WindField, WindSwath
//======================================================================

//======================================================================
// CLASS
//		WindVector
//
// DESCRIPTION
//		The WindVector object represents a wind vector (speed and
//		direction).  The wind direction is counter-clockwise from East.
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

	int		SetSpdDir(float speed, float direction);
	int		SetUV(float u, float v);

	//-----------//
	// variables //
	//-----------//

	float		spd;
	float		dir;
};

//======================================================================
// CLASS
//		WindVectorPlus
//
// DESCRIPTION
//		The WindVectorPlus object is subclassed from a WindVector and
//		contains additional information generated by the wind
//		retrieval processing.
//======================================================================

class WindVectorPlus : public WindVector
{
public:

	//--------------//
	// construction //
	//--------------//

	WindVectorPlus();
	~WindVectorPlus();

	//--------------//
	// input/output //
	//--------------//

	int		WriteL20(FILE* fp);

	//-----------//
	// variables //
	//-----------//

	float		obj;		// the objective function value
};

//======================================================================
// CLASS
//		WVC
//
// DESCRIPTION
//		The WVC object represents a wind vector cell.  It contains
//		a list of ambiguous solution WindVectorPlus.
//======================================================================

class WVC
{
public:

	//--------------//
	// construction //
	//--------------//

	WVC();
	~WVC();

	//--------------//
	// input/output //
	//--------------//

	int		WriteAmbigsAscii(FILE* ofp);
	int		WriteL20(FILE* fp);

	//--------------//
	// manipulation //
	//--------------//

	int		RemoveDuplicates();
	int		SortByObj();

	//---------//
	// freeing //
	//---------//

	void	FreeContents();

	//-----------//
	// variables //
	//-----------//

	char					selectedIdx;
	List<WindVectorPlus>	ambiguities;
};

//======================================================================
// CLASS
//		WindField
//
// DESCRIPTION
//		The WindField object hold a non-ambiguous wind field.
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

	WindVector*		NearestWindVector(float longitude, float latitude);

protected:

	//--------------//
	// construction //
	//--------------//
 
	int		_Allocate();
	int		_Deallocate();

	//-----------//
	// variables //
	//-----------//

	int			_lonCount;
	float		_lonMin;
	float		_lonMax;
	float		_lonStep;

	int			_latCount;
	float		_latMin;
	float		_latMax;
	float		_latStep;

	WindVector***	_field;
};

//======================================================================
// CLASS
//		WindSwath
//
// DESCRIPTION
//		The WindSwath object hold an ambiguous wind field gridded in
//		along track and cross track.
//======================================================================

class WindSwath
{
public:

	//--------------//
	// construction //
	//--------------//

	WindSwath();
	~WindSwath();

	int		Allocate(int cross_track_size, int along_track_size);

	//---------//
	// freeing //
	//---------//

	int		DeleteWVCs();

	//--------------//
	// input/output //
	//--------------//

	int		WriteL20(FILE* fp);

	//-----------//
	// variables //
	//-----------//

	WVC***		swath;

protected:

	//--------------//
	// construction //
	//--------------//
 
	int		_Allocate();
	int		_Deallocate();

	//-----------//
	// variables //
	//-----------//

	int			_crossTrackSize;
	int			_alongTrackSize;
};

#endif
