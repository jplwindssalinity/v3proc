//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef COORDINATESWITCH_H
#define COORDINATESWITCH_H

#include "Matrix3.h"
#include "Attitude.h"

static const char rcs_id_coordinateswitch_h[] =
	"@(#) $Id$";

//======================================================================
// CLASSES
//		CoordinateSwitch
//======================================================================

//======================================================================
// CLASS
//		CoordinateSwitch
//
// DESCRIPTION
//		The CoordinateSwitch object contains information that specifies
//      how to switch from one coordinate system to another along
//      with member functions that switch a vector from one
//		coordinate system to the other.
//======================================================================

class CoordinateSwitch
{
public:

//
// construction
//

// Build from unit axial vectors
CoordinateSwitch(Vector3 o2, Vector3 x2, Vector3 y2, Vector3 z2);
// Build from a set of ordered rotations
CoordinateSwitch(Vector3 o2, Attitude att);
// Translation only
CoordinateSwitch(Vector3 o2);
~CoordinateSwitch();


//
// methods
//

Vector3 Forward(Vector3 r);
Vector3 Backward(Vector3 r);
void Show(char *name = NULL);

private:

//
// variables
//

Matrix3 _trans;
Vector3 _o2;

};

#endif
