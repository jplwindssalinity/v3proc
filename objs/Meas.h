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


//======================================================================
// CLASSES
//		Meas, MeasList, MeasSpot, MeasSpotList, OffsetList, OffsetListList
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

	//--------------//
	// input/output //
	//--------------//

	int		Write(FILE* fp);
	int		Read(FILE* fp);

	int		WriteAscii(FILE* fp);

	//---------//
	// freeing //
	//---------//

	void	FreeContents();

	//-----------//
	// variables //
	//-----------//

	float				value;
	float				XK;
	float				bandwidth;
	Outline				outline;
	EarthPosition		centroid;

	PolE		pol;
	float		eastAzimuth;		// azimuth angle ccw from east
	float		incidenceAngle;
	float		estimatedKp;

	//------------------------//
	// not to be written out! //
	//------------------------//

	long		offset;				// byte offset in file
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

	//--------------//
	// input/output //
	//--------------//

	int		Write(FILE* fp);
	int		Read(FILE* fp);
	int		WriteAscii(FILE* fp);

	//------//
	// info //
	//------//

	LonLat		AverageLonLat();

	//---------//
	// freeing //
	//---------//

	void	FreeContents();
};

//======================================================================
// CLASS
//		OffsetList
//
// DESCRIPTION
//		The OffsetList object is a list of byte offsets into a file.
//		It is used to index Meas objects in a file.
//======================================================================

class OffsetList : public List<long>
{
public:

	//--------------//
	// construction //
	//--------------//

	OffsetList();
	~OffsetList();

	int		MakeMeasList(FILE* fp, MeasList* meas_list);

	//---------//
	// freeing //
	//---------//

	void	FreeContents();

	//-----------//
	// variables //
	//-----------//

	long	spotId;
};

//======================================================================
// CLASS
//		OffsetListList
//
// DESCRIPTION
//		The OffsetListList object is a list of offset lists.
//======================================================================

class OffsetListList : public List<OffsetList>
{
public:

	//--------------//
	// construction //
	//--------------//

	OffsetListList();
	~OffsetListList();

	//---------//
	// freeing //
	//---------//

	void	FreeContents();
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

class MeasSpot : public MeasList
{
public:

	//--------------//
	// construction //
	//--------------//

	MeasSpot();
	~MeasSpot();

	//--------------//
	// input/output //
	//--------------//

	int		Write(FILE* fp);
	int		Read(FILE* fp);

	//-----------//
	// variables //
	//-----------//

	double		time;
	OrbitState	scOrbitState;
	Attitude	scAttitude;
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

	//--------------//
	// input/output //
	//--------------//

	int		Write(FILE* fp);
	int		Read(FILE* fp);

	//---------//
	// freeing //
	//---------//

	void	FreeContents();
};

#endif
