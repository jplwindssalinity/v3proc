//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L00FRAME_H
#define L00FRAME_H

static const char rcs_id_l00frame_h[] =
	"@(#) $Id$";


//======================================================================
// CLASSES
//		L00Frame
//======================================================================

#define MAX_L00_BUFFER_SIZE		100
#define PULSES_PER_L00_FRAME	50


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

	// antenna position

	float		antennaPosition[PULSES_PER_L00_FRAME];

	// sigma-0's

	float		sigma0[PULSES_PER_L00_FRAME];
};

#endif
