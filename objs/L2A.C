//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l2a_c[] =
	"@(#) $Id$";

#include <memory.h>
#include "L2A.h"


//=====//
// L2A //
//=====//

L2A::L2A()
:	_status(OK), _headerRead(0),_headerWritten(0)
{
	return;
}

L2A::~L2A()
{
	return;
}

//------------------//
// L2A::WriteHeader //
//------------------//

int
L2A::WriteHeader()
{
	if (_outputFp == NULL)
		return(0);

	if (! header.Write(_outputFp))
		return(0);

	_headerWritten = 1;
	return(1);
}

//------------------//
// L2A::WriteHeader //
//------------------//

int
L2A::WriteHeaderAscii()
{
	if (_outputFp == NULL)
		return(0);

	if (! header.WriteAscii(_outputFp))
		return(0);

	_headerWritten = 1;
	return(1);
}

//-----------------//
// L2A::ReadHeader //
//-----------------//

int
L2A::ReadHeader()
{
	if (_inputFp == NULL)
		return(0);

	if (! header.Read(_inputFp))
		return(0);

	_headerRead = 1;
	return(1);
}

//------------------//
// L2A::ReadDataRec //
//------------------//

int
L2A::ReadDataRec()
{
	if (_inputFp == NULL) return(0);

	if (! _headerRead)
	{
		if (! header.Read(_inputFp))
			return(0);
		_headerRead = 1;
	}
	if (! frame.Read(_inputFp))
		return(0);

	return(1);
}

//-------------------//
// L2A::WriteDataRec //
//-------------------//

int
L2A::WriteDataRec()
{
	if (_outputFp == NULL) return(0);

	if (! _headerWritten)
	{
		if (! header.Write(_outputFp))
			return(0);
		_headerWritten = 1;
	}

	if (! frame.Write(_outputFp))
		return(0);

	return(1);
}

//------------------------//
// L2A::WriteDataRecAscii //
//------------------------//

int
L2A::WriteDataRecAscii()
{
	if (_outputFp == NULL) return(0);

	if (! _headerWritten)
	{
		if (! header.WriteAscii(_outputFp))
			return(0);
		_headerWritten = 1;
	}

	if (! frame.WriteAscii(_outputFp))
		return(0);

	return(1);
}

//--------------------//
// L2A::ReadGSDataRec //
//--------------------//

int
L2A::ReadGSDataRec()
{
	if (_inputFp == NULL) return(0);

	if (! frame.ReadGS(_inputFp))
		return(0);

	return(1);
}
