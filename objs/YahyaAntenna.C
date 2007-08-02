//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_yahyaantenna_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include "YahyaAntenna.h"
#include "Constants.h"


//==========//
// YahyaAntenna //
//==========//

YahyaAntenna::YahyaAntenna()
:	_alam(0.0), _diam(0.0), _flen(0.0), _bw1(0.0), _bw2(0.0), 
        _theta0(0.0), _disp(0.0), _blockage(0.0)
{
	return;
}

YahyaAntenna::~YahyaAntenna()
{
	return;
}



