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

	//-----------//
	// variables //
	//-----------//

	double		roll;
	double		pitch;
	double		yaw;
};

#endif
