//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L17FRAME_H
#define L17FRAME_H

static const char rcs_id_l17frame_h[] =
	"@(#) $Id$";

#include "Meas.h"


//======================================================================
// CLASSES
//		L17Header
//		L17Frame
//======================================================================

//======================================================================
// CLASS
//		L17Header
//
// DESCRIPTION
//		The L17Header object contains the contents of a Level 1.7
//		header.
//======================================================================

class L17Header
{
public:

	//--------------//
	// construction //
	//--------------//

	L17Header();
	~L17Header();

	//--------------//
	// input/output //
	//--------------//

	int		Read(FILE* fp);
	int		Write(FILE* fp);

	//-----------//
	// variables //
	//-----------//

	float	crossTrackResolution;		// km
	float	alongTrackResolution;		// km
	int		crossTrackBins;
	int		alongTrackBins;
	int		zeroIndex;		// cti of bin for 0 km cross track distance
	double	startTime;		// zero point of along track axis
};

//======================================================================
// CLASS
//		L17Frame
//
// DESCRIPTION
//		The L17Frame object contains the contents of a Level 1.7 frame
//		as a structure.
//======================================================================

class L17Frame
{
public:

	//--------------//
	// construction //
	//--------------//

	L17Frame();
	~L17Frame();

	//--------------//
	// input/output //
	//--------------//

	int		Read(FILE* fp);
	int		Write(FILE* fp);

	//-----------//
	// variables //
	//-----------//

	unsigned int	rev;
	int				ati;
	unsigned char	cti;
	MeasList		measList;	// a list of measurements from a single frame
};

#endif
