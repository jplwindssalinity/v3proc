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

#define L00_FRAME_HEADER_SIZE	69

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

	int		Allocate(int number_of_beams, int antenna_cycles_per_frame,
				int slices_per_spot);
	int		Deallocate();

	//-------------------//
	// data manipulation //
	//-------------------//

	int		Pack(char* buffer);
	int		Unpack(char* buffer);

	//--------------//
	// frame header //
	//--------------//

	double			time;
	unsigned int	instrumentTicks;
	unsigned int	orbitTicks;
    unsigned char   orbitStep;
	unsigned char	priOfOrbitStepChange;
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
    unsigned short  calPosition;

	//------------//
	// frame data //
	//------------//

    float*              loopbackSlices;
    float               loopbackNoise;
    float*              loadSlices;
    float               loadNoise;
	unsigned short*		antennaPosition;
	float*				science;
	float*				spotNoise;

	//-------------------------//
	// informational variables //
	//-------------------------//

	int		spotsPerFrame;
	int		slicesPerSpot;
	int		slicesPerFrame;
};

#endif
