//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_attitude_c[] =
	"@(#) $Id$";

#include "Attitude.h"


//==========//
// Attitude //
//==========//

Attitude::Attitude()
:	_roll(0.0), _pitch(0.0), _yaw(0.0)
{
	for (int i = 0; i < 3; i++)
	{
		_order[i] = 0;
	}
	return;
}

Attitude::~Attitude()
{
	return;
}

//---------------//
// Attitude::Set //
//---------------//

int
Attitude::Set(
	float		roll,
	float		pitch,
	float		yaw,
	int			order_1,
	int			order_2,
	int			order_3)
{
	if (order_1 < 1 || order_1 > 3 ||
		order_2 < 1 || order_2 > 3 ||
		order_3 < 1 || order_3 > 3)
	{
		return(0);
	}
	_roll = roll;
	_pitch = pitch;
	_yaw = yaw;
	_order[0] = order_1;
	_order[1] = order_2;
	_order[2] = order_3;
	return(1);
}
