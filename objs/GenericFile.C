//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_genericfile_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "GenericFile.h"


//=============//
// GenericFile //
//=============//

#define INVALID_FD		-1

GenericFile::GenericFile()
:	_filename(NULL), _fd(INVALID_FD)
{
	return;
}

GenericFile::~GenericFile()
{
	Close();
	if (_filename != NULL)
		free(_filename);
	return;
}

//--------------------------//
// GenericFile::SetFilename //
//--------------------------//

int
GenericFile::SetFilename(
	const char*		filename)
{
	if (_filename != NULL)
		free(_filename);
	_filename = strdup(filename);
	if (_filename == NULL)
		return(0);
	return(1);
}

//---------------------------//
// GenericFile::OpenForInput //
//---------------------------//

//----------------------------//
// GenericFile::OpenForOutput //
//----------------------------//

int
GenericFile::OpenForOutput()
{
	if (_filename == NULL)
		return(0);
	_fd = creat(_filename, 0644);
	if (_fd == -1)
	{
		_fd = INVALID_FD;
		return(0);
	}
	return(1);
}

//-------------------//
// GenericFile::Read //
//-------------------//

//--------------------//
// GenericFile::Write //
//--------------------//

int
GenericFile::Write(
	char*		buffer,
	int			bytes)
{
	if (write(_fd, buffer, bytes) != bytes)
		return(0);
	return(1);
}

//--------------------//
// GenericFile::Close //
//--------------------//

int
GenericFile::Close()
{
	if (_fd != INVALID_FD)
		close(_fd);
	return(1);
}
