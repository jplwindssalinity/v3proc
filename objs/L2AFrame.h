//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef L2AFRAME_H
#define L2AFRAME_H

static const char rcs_id_l2aframe_h[] =
	"@(#) $Id$";

#include "Meas.h"


//======================================================================
// CLASSES
//		L2AHeader
//		L2AFrame
//======================================================================

//======================================================================
// CLASS
//		L2AHeader
//
// DESCRIPTION
//		The L2AHeader object contains the contents of a Level 2A
//		header.
//======================================================================

class L2AHeader
{
public:

	//--------------//
	// construction //
	//--------------//

	L2AHeader();
	~L2AHeader();

	//--------------//
	// input/output //
	//--------------//

	int		Read(FILE* fp);
	int		Write(FILE* fp);
	int		WriteAscii(FILE* fp);

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
//		L2AFrame
//
// DESCRIPTION
//		The L2AFrame object contains the contents of a Level 2A frame
//		as a structure.
//======================================================================

class L2AFrame
{
public:

	//--------------//
	// construction //
	//--------------//

	L2AFrame();
	~L2AFrame();

	//--------------//
	// input/output //
	//--------------//

	int		Read(FILE* fp);
	int		Write(FILE* fp);
	int		WriteAscii(FILE* fp);

	int		ReadGS(FILE* fp);

	//-----------//
	// variables //
	//-----------//

	unsigned int	rev;
	int				ati;
	unsigned char	cti;
	MeasList		measList;	// a list of measurements from a single frame
};

#endif
