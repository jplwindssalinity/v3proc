//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_basefile_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "BaseFile.h"

//==========//
// BaseFile //
//==========//

BaseFile::BaseFile()
:	_filename(NULL), _fp(NULL)
{
	return;
}

BaseFile::~BaseFile()
{
	Close();
	free(_filename);
	return;
}

//-----------------------//
// BaseFile::SetFilename //
//-----------------------//

int
BaseFile::SetFilename(
	const char*		filename)
{
	if (_filename != NULL)
		free(_filename);
	_filename = strdup(filename);
	if (_filename == NULL)
		return(0);
	return(1);
}

//--------------------------//
// BaseFile::OpenForReading //
//--------------------------//

int
BaseFile::OpenForReading()
{
	if (_filename == NULL)
		return(0);

	_fp = fopen(_filename, "r");
	if (_fp == NULL)
		return(0);

	return(1);
}

int
BaseFile::OpenForReading(
	const char*		filename)
{
	if (! SetFilename(filename))
		return(0);

	if (! OpenForReading())
		return(0);

	return(1);
}

//--------------------------//
// BaseFile::OpenForWriting //
//--------------------------//

int
BaseFile::OpenForWriting()
{
	if (_filename == NULL)
		return(0);

	_fp = fopen(_filename, "w");
	if (_fp == NULL)
		return(0);

	return(1);
}

int
BaseFile::OpenForWriting(
	const char*		filename)
{
	if (! SetFilename(filename))
		return(0);

	if (! OpenForWriting())
		return(0);

	return(1);
}

//----------------//
// BaseFile::Read //
//----------------//

int
BaseFile::Read(
	char*		buffer,
	size_t		bytes)
{
	if (fread(buffer, bytes, 1, _fp) != 1)
		return(0);
	return(1);
}

//-----------------//
// BaseFile::Write //
//-----------------//

int
BaseFile::Write(
	char*		buffer,
	size_t		bytes)
{
	if (fwrite(buffer, bytes, 1, _fp) != 1)
		return(0);
	return(1);
}

//-----------------//
// BaseFile::Close //
//-----------------//

int
BaseFile::Close()
{
	if (_fp != NULL)
	{
		fclose(_fp);
		_fp = NULL;
	}
	return(1);
}
