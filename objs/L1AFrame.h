//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L10FRAME_H
#define L10FRAME_H

static const char rcs_id_l10frame_h[] =
	"@(#) $Id$";


//======================================================================
// CLASSES
//		L10Frame
//======================================================================

#define L10_FRAME_TOP			44
#define PULSES_PER_L10_FRAME	10
#define L10_FRAME_SIZE			(L10_FRAME_TOP+PULSES_PER_L10_FRAME*8)

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

	float		antennaPosition[PULSES_PER_L10_FRAME];

	// sigma-0's

	float		sigma0[PULSES_PER_L10_FRAME];
};

#endif
