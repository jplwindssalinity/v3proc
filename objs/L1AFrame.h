//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L10FRAME_H
#define L10FRAME_H

static const char rcs_id_l10frame_h[] =
	"@(#) $Id$";

#include "Attitude.h"


//======================================================================
// CLASSES
//		L10Frame
//======================================================================

#define L10_FRAME_TOP			56
#define SPOTS_PER_L10_FRAME		10
#define L10_FRAME_SIZE			(L10_FRAME_TOP+SPOTS_PER_L10_FRAME*6)

//======================================================================
// CLASS
//		L10Frame
//
// DESCRIPTION
//		The L10Frame object contains the contents of a Level 0.0 frame
//		as a structure.
//======================================================================

class L10Frame
{
public:

	//--------------//
	// construction //
	//--------------//

	L10Frame();
	~L10Frame();

	//-------------------//
	// data manipulation //
	//-------------------//

	int		Pack(char* buffer);
	int		Unpack(char* buffer);

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

	unsigned short	antennaPosition[SPOTS_PER_L10_FRAME];

	// sigma-0's

	float		sigma0[SPOTS_PER_L10_FRAME];
};

#endif
