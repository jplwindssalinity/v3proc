//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_attitude_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include "Attitude.h"
#include "Constants.h"


//==========//
// Attitude //
//==========//

Attitude::Attitude()
:	_roll(0.0), _pitch(0.0), _yaw(0.0)
{
	for (int i = 0; i < 3; i++)
	{
		_order[i] = (unsigned char)(i + 1);
	}
	return;
}

Attitude::~Attitude()
{
	return;
}

//--------------------//
// Attitude::SetOrder //
//--------------------//

int
Attitude::SetOrder(
	int		order_1,
	int		order_2,
	int		order_3)
{
	if (order_1 < 1 || order_1 > 3 ||
		order_2 < 1 || order_2 > 3 ||
		order_3 < 1 || order_3 > 3 ||
		order_1 == order_2 ||
		order_1 == order_3 ||
		order_2 == order_3)
	{
		return(0);
	}
	_order[0] = (unsigned char)order_1;
	_order[1] = (unsigned char)order_2;
	_order[2] = (unsigned char)order_3;
	return(1);
}

//------------------//
// Attitude::SetRPY //
//------------------//

int
Attitude::SetRPY(
	float	roll,
	float	pitch,
	float	yaw)
{
	_roll = roll;
	_pitch = pitch;
	_yaw = yaw;
	return(1);
}

//------------------//
// Attitude::GetRPY //
//------------------//

int
Attitude::GetRPY(
    float*  roll,
    float*  pitch,
    float*  yaw)
{
    *roll = _roll;
    *pitch = _pitch;
    *yaw = _yaw;
    return(1);
}

//---------------//
// Attitude::Set //
//---------------//

int
Attitude::Set(
	float	roll,
	float	pitch,
	float	yaw,
	int		order_1,
	int		order_2,
	int		order_3)
{
	if (! SetRPY(roll, pitch, yaw))
		return(0);

	if (! SetOrder(order_1, order_2, order_3))
		return(0);

	return(1);
}

//-------------------//
// Attitude::GSWrite //
//-------------------//

int
Attitude::GSWrite(
	FILE*	fp,
    double  time)
{
    float zero = 0.0;

    // Write out a GS compatible attitude record. (rates are set to zero)
	if (fwrite((void *)&time, sizeof(double), 1, fp) != 1 ||
	    fwrite((void *)&_roll, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&_pitch, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&_yaw, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&zero, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&zero, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&zero, sizeof(float), 1, fp) != 1)
	{
		return(0);
	}
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

//----------------------//
// Attitude::WriteAscii //
//----------------------//

int
Attitude::WriteAscii(
	FILE*	fp)
{
  const char* rpy[] = { "roll","pitch","yaw",0};
        fprintf(fp,"########## Spacecraft Attitude Info #############\n");
        fprintf(fp,"Roll: %g Pitch: %g Yaw: %g\n",_roll*rtd,_pitch*rtd,
		_yaw*rtd);
        fprintf(fp,"Order is: %s,%s,%s\n",rpy[_order[0]-1],rpy[_order[1]-1],
		rpy[_order[2]-1]);
	return(1);
}

//----------------//
// Attitude::Read //
//----------------//

int
Attitude::Read(
	FILE*	fp)
{
	if (fread((void *)&_roll, sizeof(float), 1, fp) != 1 ||
		fread((void *)&_pitch, sizeof(float), 1, fp) != 1 ||
		fread((void *)&_yaw, sizeof(float), 1, fp) != 1 ||
		fread((void *)&_order, 3 * sizeof(unsigned char), 1, fp) != 1)
	{
		return(0);
	}
	return(1);
}
