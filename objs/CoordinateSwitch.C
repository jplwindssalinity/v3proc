//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_coordinateswitch_c[] =
	"@(#) $Id$";

#include <math.h>
#include "CoordinateSwitch.h"

//==================//
// CoordinateSwitch //
//==================//

CoordinateSwitch::CoordinateSwitch()
{
	_trans.Identity();
	_o2.Zero();
	return;
}

// Initialize with the unit coordinate vectors for frame 2 represented
// in frame 1.  These are the rows of the coordinate transformation
// matrix that changes a vector represented in frame 1 into the same
// vector represented in frame 2.

CoordinateSwitch::CoordinateSwitch(
	Vector3		x2,
	Vector3		y2,
	Vector3		z2)
{
	SetAxes(x2, y2, z2);
	_o2.Zero();
	return;
}

// The origin of frame 2 represented in frame 1 is stored in _o2.

CoordinateSwitch::CoordinateSwitch(
	Vector3		o2)
{
	_trans.Identity();
	SetOrigin(o2);
	return;
}

CoordinateSwitch::CoordinateSwitch(
	Vector3		o2,
	Vector3		x2,
	Vector3		y2,
	Vector3		z2)
{
	SetOrigin(o2);
	SetAxes(x2, y2, z2);
	return;
}

// Initialize by forming the coordinate transformation matrix from a set
// of ordered rotations about the coordinate axes of frame 1.
// The coordinate transformation matrix changes a vector represented
// in frame 1 into the same vector represented in frame 2.
// The ordered rotations are stored in an Attitude object.
// Roll, pitch, and yaw are rotations about the x-axis, y-axis, and z-axis
// respectively (also called axes 1, 2, and 3).
// The names of the axes and angles are arbitrary -- what really matters is the
// order that they are applied in.
// The order of definition for the rotations is specified by the _order
// member of the Attitude object.
// If the rotation about the 1-axis (ie., roll)
// is the first to be applied, then order[0] = 1, if the second to be applied
// then order[0] = 2, and so forth.
// The origin of frame 2 represented in frame 1 is stored in _o2.

CoordinateSwitch::CoordinateSwitch(
	Attitude	att)
{
	_o2.Zero();
	SetRotation(att);
	return;
}

CoordinateSwitch::CoordinateSwitch(
	Vector3		o2,
	Attitude	att)
{
	SetOrigin(o2);
	SetRotation(att);
	return;
}

CoordinateSwitch::~CoordinateSwitch()
{
	return;
}

//---------------------------//
// CoordinateSwitch::SetAxes //
//---------------------------//

void
CoordinateSwitch::SetAxes(
	Vector3		x2,
	Vector3		y2,
	Vector3		z2)
{
	x2.Scale(1.0);
	y2.Scale(1.0);
	z2.Scale(1.0);
	_trans.Rowset(x2, y2, z2);
	return;
}

//-----------------------------//
// CoordinateSwitch::SetOrigin //
//-----------------------------//

void
CoordinateSwitch::SetOrigin(
	Vector3		o2)
{
	_o2 = o2;
	return;
}

//-------------------------------//
// CoordinateSwitch::SetRotation //
//-------------------------------//

void
CoordinateSwitch::SetRotation(
	Attitude	att)
{
	double roll = att.GetRoll();
	double pitch = att.GetPitch();
	double yaw = att.GetYaw();

	double cr = cos(roll);
	double sr = sin(roll);
	double cp = cos(pitch);
	double sp = sin(pitch);
	double cy = cos(yaw);
	double sy = sin(yaw);

	Matrix3 rollmatrix(1,0,0,0,cr,sr,0,-sr,cr);
	Matrix3 pitchmatrix(cp,0,-sp,0,1,0,sp,0,cp);
	Matrix3 yawmatrix(cy,sy,0,-sy,cy,0,0,0,1);
	unsigned char *order = att.GetOrderIndices();

	_trans.Identity();

	for (unsigned char i = 1; i <= 3; i++)
	{	// Apply rotation matrices in specifed order
		if (order[0] == i) _trans = rollmatrix * _trans;
		if (order[1] == i) _trans = pitchmatrix * _trans;
		if (order[2] == i) _trans = yawmatrix * _trans;
	}
	return;
}

//--------------------------//
// CoordinateSwitch::Append //
//--------------------------//
// This method updates the coordinate switch object so that it
// can perform its original transformation followed by the passed
// transformation.

int
CoordinateSwitch::Append(
	CoordinateSwitch*	next)
{
	Matrix3 A = _trans;
	Vector3 a = _o2;
	_trans = next->_trans * A;
	A.Inverse();
	_o2 = a + A * next->_o2;
	return(1);
}

//------------------------------------//
// CoordinateSwitch::ReverseDirection //
//------------------------------------//
// This method returns a coordinate transform object which performs
// the coordinate switch in the opposite direction.

CoordinateSwitch
CoordinateSwitch::ReverseDirection()
{
	CoordinateSwitch x;

	x._trans = _trans;
	x._trans.Inverse();
	x._o2 = -_trans * _o2;
	return(x);
}

//
// The method Forward converts a vector represented in frame 1, into
// the same vector represented in frame 2.
// Vectors are stored as Vector3 objects.
//
// Inputs:
//  r = pointer to a 3-element array holding the vector in frame 1.
//
// Return Value:
//  The same vector, but represented in frame 2.
//

Vector3 CoordinateSwitch::Forward(Vector3 r)
{
Vector3 new_r = _trans * (r - _o2);
return(new_r);
}

//
// The method Backward converts a vector represented in frame 2, into
// the same vector represented in frame 1.
// Vectors are stored as Vector3 objects.
//
// Inputs:
//  r = a Vector3 object (or derived object) holding the vector in frame 1.
//
// Return Value:
//  the input vector represented in frame 2.
//

Vector3 CoordinateSwitch::Backward(Vector3 r)
{
Matrix3 rev_trans = _trans;
rev_trans.Inverse();
Vector3 new_r = rev_trans * r + _o2;
return(new_r);
}

//
// Show the transformation matrix and translation vector that define
// this coordinate transformation.
//

void CoordinateSwitch::Show(char *name)
{
_trans.Show(name);
_o2.Show(name);
}
