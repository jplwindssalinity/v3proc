//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_windvector_c[] =
	"@(#) $Id$";

#include <math.h>
#include "WindVector.h"


//============//
// WindVector //
//============//

WindVector::WindVector()
:	spd(0.0), dir(0.0)
{
	return;
}

WindVector::~WindVector()
{
	return;
}

//------------------------//
// WindVector::operator== //
//------------------------//

int
WindVector::operator==(
	WindVector	wv)
{
	if (spd == wv.spd && dir == wv.dir)
		return(1);
	else
		return(0);
}

//-----------------------//
// WindVector::SetSpdDir //
//-----------------------//

int
WindVector::SetSpdDir(
	double	speed,
	double	direction)
{
	spd = speed;
	dir = direction;
	return(1);
}

//-------------------//
// WindVector::SetUV //
//-------------------//

int
WindVector::SetUV(
	double	u,
	double	v)
{
	spd = hypot(u, v);
	dir = atan2(v, u);
	return(1);
}
