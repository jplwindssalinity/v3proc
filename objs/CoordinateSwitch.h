//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef COORDINATESWITCH_H
#define COORDINATESWITCH_H

#include "Matrix3.h"

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
//              how to switch from one coordinate system to another along
//              with member functions that switch a vector from one
//		coordinate system to the other.
//======================================================================

class CoordinateSwitch
{
public:

//
// construction
//

CoordinateSwitch(Vector3 x2, Vector3 y2, Vector3 z2);
CoordinateSwitch(Vector3 att,
                 int order1, int order2, int order3);
~CoordinateSwitch();


//
// methods
//

Vector3 forward(Vector3 r);
Vector3 backward(Vector3 r);
void show(char *name = NULL);

private:

//
// variables
//

Matrix3 _trans;
};

#endif
