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

//----------------------//
// WindVector::WriteL20 //
//----------------------//

int
WindVector::WriteL20(
	FILE*	fp)
{
	if (fwrite((void *)&spd, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&dir, sizeof(float), 1, fp) != 1 ||
		fwrite((void *)&obj, sizeof(float), 1, fp) != 1)
	{
		return(0);
	}
	return(1);
}

//-----------------------//
// WindVector::SetSpdDir //
//-----------------------//

int
WindVector::SetSpdDir(
	float	speed,
	float	direction)
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
	float	u,
	float	v)
{
	spd = (float)hypot((double)u, (double)v);
	dir = (float)atan2((double)v, (double)u);
	return(1);
}
