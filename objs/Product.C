//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_product_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "Product.h"


//=============//
// ProductFile //
//=============//

ProductFile::ProductFile()
:	_filename(NULL), _fd(INVALID_FD)
{
	return;
}

ProductFile::~ProductFile()
{
	if (_filename)
		free(_filename);
	if (_fd != INVALID_FD)
		close(_fd);
	return;
}

//-------------------------//
// ProductFile::AssignFile //
//-------------------------//

int
ProductFile::AssignFile(
	const char*		filename)
{
	if (_filename)
		free(_filename);
	if (_fd != INVALID_FD)
		close(_fd);

	_filename = strdup(filename);
	if (! _filename)
		return(0);

	return(1);
}

//----------------------------//
// ProductFile::OpenForOutput //
//----------------------------//

int
ProductFile::OpenForOutput()
{
	if (! _filename)
		return(0);

	if (_fd != INVALID_FD)
		return(0);

	_fd = creat(_filename, 0644);
	if (_fd == INVALID_FD)
		return(0);

	return(1);
}

//--------------------//
// ProductFile::Close //
//--------------------//

int
ProductFile::Close()
{
	if (_fd == INVALID_FD)
		return(1);			// already closed (?)

	if (close(_fd) != 0)
		return(0);

	return(1);
}


//=================//
// ProductFileList //
//=================//

ProductFileList::ProductFileList()
{
	return;
}

ProductFileList::~ProductFileList()
{
	return;
}

//--------------------------//
// ProductFileList::AddFile //
//--------------------------//

int
ProductFileList::AddFile(
	const char*		filename)
{
	ProductFile* new_product_file = new ProductFile();
	if (! new_product_file)
		return(0);

	if (! new_product_file->AssignFile(filename))
	{
		free(new_product_file);
		return(0);
	}

	if (! Append(new_product_file))
	{
		free(new_product_file);
		return(0);
	}

	return(1);
}

//---------------------------//
// ProductFileList::AddFiles //
//---------------------------//

int
ProductFileList::AddFiles(
	const char*		filenames[],
	const int		count)
{
	for (int i = 0; i < count; i++)
	{
		if (! AddFile(filenames[i]))
			return(0);
	}
	return(1);
}

//---------------------------------------//
// ProductFileList::OpenCurrentForOutput //
//---------------------------------------//

int
ProductFileList::OpenCurrentForOutput()
{
	ProductFile* pf = GetCurrent();
	if (! pf)
		return(0);

	if (! pf->OpenForOutput())
		return(0);

	return(1);
}

//-------------------------------//
// ProductFileList::GetCurrentFd //
//-------------------------------//

int
ProductFileList::GetCurrentFd()
{
	ProductFile* pf = GetCurrent();
	if (! pf)
		return(0);

	return(pf->GetFd());
}

//-----------------------------------//
// ProductFileList::CloseCurrentFile //
//-----------------------------------//

int
ProductFileList::CloseCurrentFile()
{
	ProductFile* pf = GetCurrent();
	if (! pf)
		return(0);

	if (! pf->Close())
		return(0);

	return(1);
}


//=========//
// Product //
//=========//

Product::Product()
:	_frame(NULL), _size(0)
{
	return;
}

Product::~Product()
{
	return;
}

//-------------------------//
// Product::AllocateBuffer //
//-------------------------//

int
Product::AllocateBuffer(
	const int	buffer_size)
{
	_frame = (char *)realloc((void *)_frame, buffer_size);
	if (! _frame)
		return(0);

	_size = buffer_size;
	return(1);
}

//------------------//
// Product::AddFile //
//------------------//

int
Product::AddFile(
	const char*		filename)
{
	return(_fileList.AddFile(filename));
}

//------------------------//
// Product::GotoFirstFile //
//------------------------//

int
Product::GotoFirstFile()
{
	if (! _fileList.GetHead())
		return(0);

	return(1);
}

//-------------------------------//
// Product::OpenCurrentForOutput //
//-------------------------------//

int
Product::OpenCurrentForOutput()
{
	return(_fileList.OpenCurrentForOutput());
}

//---------------------------//
// Product::CloseCurrentFile //
//---------------------------//

int
Product::CloseCurrentFile()
{
	return(_fileList.CloseCurrentFile());
}

//----------------------//
// Product::WriteBuffer //
//----------------------//

int
Product::WriteBuffer()
{
	int ofd = _fileList.GetCurrentFd();
	if (ofd == INVALID_FD)
		return(0);

	if (write(ofd, _frame, _size) != _size)
		return(0);

	return(1);
}
