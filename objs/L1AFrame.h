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

#define L10_FRAME_HEADER_SIZE	69

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

	int		Allocate(int number_of_beams, int antenna_cycles_per_frame,
				int slices_per_spot);
	int		Deallocate();

	//-------------------//
	// data manipulation //
	//-------------------//

	int		Pack(char* buffer);
	int		Unpack(char* buffer);

	//-------------------//
	// product variables //
	//-------------------//

	double			time;
	unsigned int	instrumentTicks;
	unsigned int	orbitTicks;
	unsigned char	priOfOrbitTickChange;

	//-----------------//
	// S/C information //
	//-----------------//

	float			gcAltitude;
	float			gcLongitude;
	float			gcLatitude;
	float			gcX;
	float			gcY;
	float			gcZ;
	float			velX;
	float			velY;
	float			velZ;

	Attitude		attitude;

	float			ptgr;

	// antenna position
	unsigned short*		antennaPosition;

	// science measurements
	float*				science;
	float*				spotNoise;

	//-------------------------//
	// informational variables //
	//-------------------------//

	int		antennaCyclesPerFrame;
	int		spotsPerFrame;
	int		slicesPerSpot;
	int		slicesPerFrame;
};

#endif
