//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef EARTHPOSITION_H
#define EARTHPOSITION_H

static const char rcs_id_earthposition_h[] =
	"@(#) $Id$";

#include <stdio.h>
#include "Matrix3.h"
class CoordinateSwitch;

//======================================================================
// CLASSES
//		EarthPosition
//======================================================================

//======================================================================
// CLASS
//		EarthPosition
//
// DESCRIPTION
//		The EarthPosition object is derived from the Vector3 object.
//		It contains a 3-element vector which specifies a position in
//		a coordinate system fixed to the earth's surface
//		(ie., rotating), with its origin at the center of the earth.
//		In addition to the inherited vector methods, other methods
//		for handling geodetic and geocentric latitude, longitude,
//		and altitude are provided.
//		The position vector is always stored as three rectangular
//		coordinates, however, different construction and access
//		methods allow the use of latitude, longitude, and altitude.
//======================================================================

class EarthPosition : public Vector3
{
public:

//
// construction methods
//

EarthPosition();
EarthPosition(Vector3 v);
~EarthPosition();

	//---------------------//
	// setting and getting //
	//---------------------//

	int		SetPosition(double x, double y, double z);
	int		SetAltLonGCLat(double altitude, double longitude,
				double gc_latitude);
	int		SetAltLonGDLat(double altitude, double longitude,
				double gd_latitude);
	int		GetAltLonGCLat(double* altitude, double* longitude,
				double* gc_latitude);
	int		GetAltLonGDLat(double* altitude, double* longitude,
				double* gd_latitude);
	int		SurfaceSet();


//
// I/O
//

int	ReadLonLat(FILE* fp);
int WriteLonLat(FILE* fp);

//
// Operators
//

void operator=(Vector3 vec);	// assign Vector3 to EarthPosition

//
// Other access methods
//

double surface_distance(EarthPosition r);
EarthPosition Nadir();
Vector3 Normal();

CoordinateSwitch SurfaceCoordinateSystem();
double IncidenceAngle(Vector3 rlook);

};

#endif
