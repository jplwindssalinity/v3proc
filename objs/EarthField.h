//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef EARTHFIELD_H
#define EARTHFIELD_H

static const char rcs_id_earthfield_h[] =
	"@(#) $Id$";

#include "LonLat.h"

//======================================================================
// CLASSES
//		EarthField
//======================================================================

//======================================================================
// CLASS
//		EarthField
//
// DESCRIPTION
//		The EarthField object manages a global grid of floating point numbers.
//		The grid is uniformly spaced in degrees of geodetic latitude and
//		east longitude (resulting in non-uniform distance spacing)
//======================================================================

class EarthField
{
public:

	//--------------//
	// construction //
	//--------------//

	EarthField();
	~EarthField();

	//--------------//
	// Allocation etc
	//--------------//

	int Allocate();
	int Deallocate();
	int Setup(double lonmin, double lonmax, double lonstep,
		double latmin, double latmax, double latstep);

	//--------------//
	// I/O
	//--------------//

	int Read(char *filename);
	int Write(char *filename);
	int	WriteAsciiCols(char *filename);

	//--------------//
	// access
	//--------------//

	int NearestElement(LonLat lon_lat, float* element);
	int InterpolatedElement(LonLat lon_lat, float* element);
	int GetDimensions(int* Nlon, int* Nlat);

	//--------------------//
	// field computations
	//--------------------//

	int Scale(float factor);
	double GetMean();
	double GetVariance();

    //-----------//
    // variables //
    //-----------//

	float**		field;

protected:

    int         _lonCount;
    float       _lonMin;
    float       _lonMax;
    float       _lonStep;

    int         _latCount;
    float       _latMin;
    float       _latMax;
    float       _latStep;

};

#endif
