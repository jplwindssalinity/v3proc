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
:	_filename(NULL), _fd(INVALID_FD), _status(OK)
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
	{
		_status = ERROR_ALLOCATING_FILENAME;
		return(0);
	}

	return(1);
}

//----------------------------//
// ProductFile::OpenForOutput //
//----------------------------//

int
ProductFile::OpenForOutput()
{
	if (! _filename)
	{
		_status = ERROR_MISSING_FILE;
		return(0);
	}

	if (_fd != INVALID_FD)
	{
		_status = ERROR_REOPENING_FILE;
		return(0);
	}

	_fd = creat(_filename, 0644);
	if (_fd == INVALID_FD)
	{
		_status = ERROR_OPENING_OUTPUT_FILE;
		return(0);
	}

	return(1);
}

//---------------------------//
// ProductFile::OpenForInput //
//---------------------------//

int
ProductFile::OpenForInput()
{
	if (! _filename)
	{
		_status = ERROR_MISSING_FILE;
		return(0);
	}

	if (_fd != INVALID_FD)
	{
		_status = ERROR_REOPENING_FILE;
		return(0);
	}

	_fd = open(_filename, O_RDONLY);
	if (_fd == INVALID_FD)
	{
		_status = ERROR_OPENING_INPUT_FILE;
		return(0);
	}

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
	{
		_status = ERROR_CLOSING_FILE;
		return(0);
	}

	return(1);
}


//=================//
// ProductFileList //
//=================//

ProductFileList::ProductFileList()
:	_status(OK)
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
	{
		_status = ERROR_CREATING_PRODUCT_FILE;
		return(0);
	}

	if (! new_product_file->AssignFile(filename))
	{
		free(new_product_file);
		_status = ERROR_ASSIGNING_FILENAME;
		return(0);
	}

	if (! Append(new_product_file))
	{
		free(new_product_file);
		_status = ERROR_APPENDING_PRODUCT_FILE;
		return(0);
	}

	// make the head file current
	GetHead();

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
	{
		_status = ERROR_NO_CURRENT_PRODUCT_FILE;
		return(0);
	}

	if (! pf->OpenForOutput())
	{
		_status = ERROR_OPENING_OUTPUT_FILE;
		return(0);
	}

	return(1);
}

//--------------------------------------//
// ProductFileList::OpenCurrentForInput //
//--------------------------------------//

int
ProductFileList::OpenCurrentForInput()
{
	ProductFile* pf = GetCurrent();
	if (! pf)
	{
		_status = ERROR_NO_CURRENT_PRODUCT_FILE;
		return(0);
	}

	if (! pf->OpenForInput())
	{
		_status = ERROR_OPENING_INPUT_FILE;
		return(0);
	}

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
	{
		_status = ERROR_NO_CURRENT_PRODUCT_FILE;
		return(INVALID_FD);
	}

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
	{
		_status = ERROR_NO_CURRENT_PRODUCT_FILE;
		return(0);
	}

	if (! pf->Close())
	{
		_status = ERROR_CLOSING_PRODUCT_FILE;
		return(0);
	}

	return(1);
}


//=========//
// Product //
//=========//

Product::Product()
:	_frame(NULL), _size(0), _status(OK)
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
	{
		_status = ERROR_ALLOCATING_BUFFER;
		return(0);
	}

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
	if (! _fileList.AddFile(filename))
	{
		_status = ERROR_ADDING_FILE;
		return(0);
	}
	return(1);
}

//------------------------//
// Product::GotoFirstFile //
//------------------------//

int
Product::GotoFirstFile()
{
	if (! _fileList.GetHead())
	{
		_status = ERROR_NO_FILES;
		return(0);
	}
	return(1);
}

//-------------------------------//
// Product::OpenCurrentForOutput //
//-------------------------------//

int
Product::OpenCurrentForOutput()
{
	if (! _fileList.OpenCurrentForOutput())
	{
		_status = ERROR_OPENING_OUTPUT_FILE;
		return(0);
	}
	return(1);
}

//------------------------------//
// Product::OpenCurrentForInput //
//------------------------------//

int
Product::OpenCurrentForInput()
{
	if (! _fileList.OpenCurrentForInput())
	{
		_status = ERROR_OPENING_INPUT_FILE;
		return(0);
	}
	return(1);
}

//---------------------------//
// Product::CloseCurrentFile //
//---------------------------//

int
Product::CloseCurrentFile()
{
	if (! _fileList.CloseCurrentFile())
	{
		_status = ERROR_CLOSING_CURRENT_FILE;
		return(0);
	}
	return(1);
}

//----------------------//
// Product::WriteBuffer //
//----------------------//

int
Product::WriteBuffer()
{
	int ofd = _fileList.GetCurrentFd();
	if (ofd == INVALID_FD)
	{
		_status = ERROR_GETTING_CURRENT_FD;
		return(0);
	}

	if (write(ofd, _frame, _size) != _size)
	{
		_status = ERROR_WRITING_BUFFER;
		return(0);
	}

	return(1);
}

//---------------------//
// Product::ReadBuffer //
//---------------------//

int
Product::ReadBuffer()
{
	int ofd = _fileList.GetCurrentFd();
	if (ofd == INVALID_FD)
	{
		// file not opened
		if (! OpenCurrentForInput())
			return(0);

		// try again
		return(ReadBuffer());
	}

	int bytes_read = read(ofd, _frame, _size);
	if (bytes_read != _size)
	{
		switch(bytes_read)
		{
		case 0:			// EOF
			// close this file, try next file
			if (! _fileList.CloseCurrentFile())
			{
				_status = ERROR_CLOSING_CURRENT_FILE;
				break;
			}
			if (! _fileList.GetNext())
			{
				_status = ERROR_NO_MORE_DATA;
				break;
			}
			return(ReadBuffer());
			break;
		case -1:
			_status = ERROR_READING_BUFFER;
			break;
		default:
			_status = ERROR_UNKNOWN;
			break;
		}
		return(0);
	}

	return(1);
}
