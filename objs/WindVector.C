//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_wind_vector_c[] =
    "@(#) $Id$";

#include <math.h>
#include "WindVector.h"

//============//
// WindVector //
//============//

WindVector::WindVector()
:   spd(0.0), dir(0.0)
{
    return;
}

WindVector::~WindVector()
{
    return;
}

//-----------------------//
// WindVector::SetSpdDir //
//-----------------------//

int
WindVector::SetSpdDir(
    float  speed,
    float  direction)
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
    float  u,
    float  v)
{
    spd = (float)hypot((double)u, (double)v);
    dir = (float)atan2((double)v, (double)u);
    return(1);
}

//-------------------//
// WindVector::GetUV //
//-------------------//

int
WindVector::GetUV(
    float*  u,
    float*  v)
{
    *u = spd * (float)cos((double)dir);
    *v = spd * (float)sin((double)dir);
    return(1);
}
