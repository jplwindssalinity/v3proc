//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef INSTRUMENTGEOM_H
#define INSTRUMENTGEOM_H

static const char rcs_id_instrumentgeom_h[] =
	"@(#) $Id$";

//======================================================================
// DESCRIPTION
//		Higher level geometry functions.  Typically instrument related.
//======================================================================

CoordinateSwitch BeamFrameToGC(OrbitState* orbit_state, Attitude* attitude,
					Antenna* antenna, Beam* beam);

int FindSliceOutline(float freq_1, float freq_2, float freq_tol,
	Outline* outline);

#endif
