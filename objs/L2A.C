//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_l17_c[] =
	"@(#) $Id$";

#include <memory.h>
#include "L17.h"


//=====//
// L17 //
//=====//

L17::L17()
:	_status(OK), _headerTransferred(0)
{
	return;
}

L17::~L17()
{
	return;
}

//------------------//
// L17::WriteHeader //
//------------------//

int
L17::WriteHeader()
{
	if (_fp == NULL)
		return(0);

	if (! header.Write(_fp))
		return(0);

	_headerTransferred = 1;
	return(1);
}

//-----------------//
// L17::ReadHeader //
//-----------------//

int
L17::ReadHeader()
{
	if (_fp == NULL)
		return(0);

	if (! header.Read(_fp))
		return(0);

	_headerTransferred = 1;
	return(1);
}

//------------------//
// L17::ReadDataRec //
//------------------//

int
L17::ReadDataRec()
{
	if (_fp == NULL) return(0);

	if (! _headerTransferred)
	{
		if (! header.Read(_fp))
			return(0);
		_headerTransferred = 1;
	}
	if (! frame.Read(_fp))
		return(0);

	return(1);
}

//-------------------//
// L17::WriteDataRec //
//-------------------//

int
L17::WriteDataRec()
{
	if (_fp == NULL) return(0);

	if (! _headerTransferred)
	{
		if (! header.Write(_fp))
			return(0);
		_headerTransferred = 1;
	}

	if (! frame.Write(_fp))
        return(0);

	return(1);
}
