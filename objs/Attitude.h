//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef ATTITUDE_H
#define ATTITUDE_H

static const char rcs_id_attitude_h[] =
	"@(#) $Id$";


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

	int			Set(double roll, double pitch, double yaw, int order_1,
					int order_2, int order_3);

	double		GetRoll() { return(_roll); };
	double		GetPitch() { return(_pitch); };
	double		GetYaw() { return(_yaw); };
	int*		GetOrderIndicies() { return(_order); };

protected:

	//-----------//
	// variables //
	//-----------//

	double		_roll;
	double		_pitch;
	double		_yaw;
	int			_order[3];
};

#endif
