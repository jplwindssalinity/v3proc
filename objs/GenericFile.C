//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_genericfile_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "GenericFile.h"

//=============//
// GenericFile //
//=============//

GenericFile::GenericFile()
:	_filename(NULL), _fp(NULL)
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

int
GenericFile::OpenForInput()
{
	if (_filename == NULL)
		return(0);

	_fp = fopen(_filename, "r");
	if (_fp == NULL)
		return(0);

	return(1);
}

//----------------------------//
// GenericFile::OpenForOutput //
//----------------------------//

int
GenericFile::OpenForOutput()
{
	if (_filename == NULL)
		return(0);

	_fp = fopen(_filename, "w");
	if (_fp == NULL)
		return(0);

	return(1);
}

//-------------------//
// GenericFile::Read //
//-------------------//

int
GenericFile::Read(
	char*		buffer,
	size_t		bytes)
{
	if (fread(buffer, bytes, 1, _fp) != bytes)
		return(0);
	return(1);
}

//--------------------//
// GenericFile::Write //
//--------------------//

int
GenericFile::Write(
	char*		buffer,
	size_t		bytes)
{
	if (fwrite(buffer, bytes, 1, _fp) != bytes)
		return(0);
	return(1);
}

//--------------------//
// GenericFile::Close //
//--------------------//

int
GenericFile::Close()
{
	if (_fp != NULL)
	{
		fclose(_fp);
		_fp = NULL;
	}
	return(1);
}
