//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_attitude_c[] =
	"@(#) $Id$";

#include <stdio.h>
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
	float			roll,
	float			pitch,
	float			yaw,
	unsigned char	order_1,
	unsigned char	order_2,
	unsigned char	order_3)
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

//-----------------//
// Attitude::Write //
//-----------------//

int
Attitude::Write(
	FILE*	fp)
{
	if (fwrite((void *)&_roll, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&_pitch, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&_yaw, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&_order, 3 * sizeof(unsigned char), 1, fp) != 1)
	{
		return(0);
	}
	return(1);
}
