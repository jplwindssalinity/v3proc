//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef CHECKFRAME_H
#define CHECKFRAME_H

static const char rcs_id_checkframe_h[] =
	"@(#) $Id$";

#include "Attitude.h"
#include "Matrix3.h"
#include "EarthPosition.h"
#include "Wind.h"

//======================================================================
// CLASSES
//		CheckFrame
//======================================================================

//======================================================================
// CLASS
//		CheckFrame
//
// DESCRIPTION
//		The CheckFrame object contains data used to cross check true values
//		from the simulation with derived values in the L1AToL1B processor.
//======================================================================

class CheckFrame
{
public:

	//--------------//
	// construction //
	//--------------//

	CheckFrame();
	CheckFrame(int slices_per_spot);
	~CheckFrame();

	//-------//
	// Setup //
	//-------//

	int	Allocate(int slices_per_spot);
	int	Deallocate();

	//-----//
	// I/O //
	//-----//

	int AppendRecord(FILE* fptr);

	//-----------//
	// spot data //
	//-----------//

	double			time;
	EarthPosition	rsat;
	Vector3			vsat;
	Attitude		attitude;
	float			ptgr;

	//------------//
	// slice data //
	//------------//

	float*			sigma0;
	WindVector*		wv;
	float*			XK;
	EarthPosition*	centroid;
	float*			azimuth;
	float*			incidence;

	//-------------------------//
	// informational variables //
	//-------------------------//

	int		slicesPerSpot;
};

#endif
