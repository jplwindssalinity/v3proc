//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L00FRAME_H
#define L00FRAME_H

static const char rcs_id_l00frame_h[] =
	"@(#) $Id$";

#include "Attitude.h"


//======================================================================
// CLASSES
//		L00Frame
//======================================================================

#define L00_FRAME_HEADER_SIZE	56

//======================================================================
// CLASS
//		L00Frame
//
// DESCRIPTION
//		The L00Frame object contains the contents of a Level 0.0 frame
//		as a structure.
//======================================================================

class L00Frame
{
public:

	//--------------//
	// construction //
	//--------------//

	L00Frame();
	~L00Frame();

	int		Allocate(int spots_per_frame, int slices_per_spot);
	int		Deallocate();

	//-------------------//
	// data manipulation //
	//-------------------//

	int		Pack(char* buffer);

	//-------------------//
	// product variables //
	//-------------------//

	double		time;

	// S/C information

	float		gcAltitude;
	float		gcLongitude;
	float		gcLatitude;
	float		gcX;
	float		gcY;
	float		gcZ;
	float		velX;
	float		velY;
	float		velZ;

	Attitude	attitude;

	// antenna position
	unsigned short*		antennaPosition;

	// science measurements
	float*				science;

	//-------------------------//
	// informational variables //
	//-------------------------//

	int		spotsPerFrame;
	int		slicesPerSpot;
	int		totalSlices;
};

#endif
