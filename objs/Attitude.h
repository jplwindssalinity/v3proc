//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef ATTITUDE_H
#define ATTITUDE_H

static const char rcs_id_attitude_h[] =
	"@(#) $Id$";

#include <stdio.h>


//======================================================================
// CLASSES
//		Attitude
//======================================================================

//======================================================================
// CLASS
//		Attitude
//
// DESCRIPTION
//		The Attitude object contains orientation information.
//======================================================================

class Attitude
{
public:

	//--------------//
	// construction //
	//--------------//

	Attitude();
	~Attitude();

	//---------------------//
	// setting and getting //
	//---------------------//

	int		SetOrder(int order_1, int order_2, int order_3);
	int		SetRPY(float roll, float pitch, float yaw);
	int		Set(float roll, float pitch, float yaw, int order_1, int order_2,
				int order_3);

	float	GetRoll() { return(_roll); };
	void	SetRoll(float roll) { _roll = roll; };
	float	GetPitch() { return(_pitch); };
	void	SetPitch(float pitch) { _pitch = pitch; };
	float	GetYaw() { return(_yaw); };
	void	SetYaw(float yaw) { _yaw = yaw; };
	unsigned char*	GetOrder() { return(_order); };

	//--------------//
	// input/output //
	//--------------//

	int		Write(FILE* fp);
	int		WriteAscii(FILE* fp);
	int		Read(FILE* fp);

protected:

	//-----------//
	// variables //
	//-----------//

	float			_roll;		// radians
	float			_pitch;		// radians
	float			_yaw;		// radians
	unsigned char	_order[3];
};

#endif
