//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef MEAS_H
#define MEAS_H

static const char rcs_id_meas_h[] =
	"@(#) $Id$";

#include "Beam.h"
#include "LonLat.h"
#include "Ephemeris.h"
#include "Attitude.h"
#include "List.h"
#include "List.C"


//======================================================================
// CLASSES
//		Meas, MeasList, MeasSpot, MeasSpotList
//======================================================================

//======================================================================
// CLASS
//		Meas
//
// DESCRIPTION
//		The Meas object is a general purpose object for holding and
//		manipulating sigma-0 and brightness temperature measurements.
//======================================================================

class Meas
{
public:

	//--------------//
	// construction //
	//--------------//

	Meas();
	~Meas();

	//-----------//
	// variables //
	//-----------//

	float		value;
	Outline		outline;
	LonLat		center;

	PolE		pol;
	float		eastAzimuth;		// azimuth angle ccw from east
	float		scAzimuth;			// azimuth angle ccw from s/c velocity
	float		incidenceAngle;
	float		estimatedKp;
};

//======================================================================
// CLASS
//		MeasList
//
// DESCRIPTION
//		The MeasList object is a list of Meas objects.  Used for
//		gridding.
//======================================================================

class MeasList : public List<Meas>
{
public:

	//--------------//
	// construction //
	//--------------//

	MeasList();
	~MeasList();
};

//======================================================================
// CLASS
//		MeasSpot
//
// DESCRIPTION
//		The MeasSpot object contains information about a spot.  It
//		contains spacecraft information as well as a list of Meas for
//		each slice.
//======================================================================

class MeasSpot
{
public:

	//--------------//
	// construction //
	//--------------//

	MeasSpot();
	~MeasSpot();

	//-----------//
	// variables //
	//-----------//

	double		time;
	OrbitState	scEphemeris;
	Attitude	scAttitude;
	MeasList	slices;
};

//======================================================================
// CLASS
//		MeasSpotList
//
// DESCRIPTION
//		The MeasSpotList object contains a list of MeasSpot.  It is
//		used for storing and manipulating a set of measurements.
//======================================================================

class MeasSpotList : public List<MeasSpot>
{
public:

	//--------------//
	// construction //
	//--------------//

	MeasSpotList();
	~MeasSpotList();
};

#endif
