//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef PRODUCT_H
#define PRODUCT_H

static const char rcs_id_product_h[] =
	"@(#) $Id$";

#include "List.h"


//======================================================================
// CLASSES
//		ProductFile, ProductFileList, Product
//======================================================================


//======================================================================
// CLASS
//		ProductFile
//
// DESCRIPTION
//		The ProductFile class is an abstract class used as a generic
//		product file (e.g. Level 0 File).
//======================================================================

#define INVALID_FD		-1

class ProductFile
{
public:

	//--------------//
	// construction //
	//--------------//

	ProductFile();
	~ProductFile();

	int		AssignFile(const char* filename);

	//--------------//
	// manipulation //
	//--------------//

	int		OpenForOutput();
	int		Close();

	//---------//
	// getting //
	//---------//

	int		GetFd() { return(_fd); };

protected:

	//-----------//
	// variables //
	//-----------//

	char*		_filename;
	int			_fd;
};


//======================================================================
// CLASS
//		ProductFileList
//
// DESCRIPTION
//		The ProductFileList class is an abstract class used as a
//		generic product file list (e.g. Level 0 file list).
//======================================================================

class ProductFileList : public List<ProductFile>
{
public:

	//--------------//
	// construction //
	//--------------//

	ProductFileList();
	~ProductFileList();

	int		AddFile(const char* filename);
	int		AddFiles(const char* filename[], const int count);

	//-------------------//
	// file manipulation //
	//-------------------//

	int		OpenCurrentForOutput();
	int		GetCurrentFd();
	int		CloseCurrentFile();
};


//======================================================================
// CLASS
//		Product
//
// DESCRIPTION
//		The Product class is an abstract class used as a generic
//		product (e.g. Level 0).  Every product has a list of associated
//		files and a buffer for data records or packets.
//======================================================================

class Product
{
public:

	//--------------//
	// construction //
	//--------------//

	Product();
	~Product();

	int		AllocateBuffer(const int buffer_size);
	int		AddFile(const char* filename);

	//-------------------//
	// file manipulation //
	//-------------------//

	int		GotoFirstFile();
	int		OpenCurrentForOutput();
	int		CloseCurrentFile();

	//--------------//
	// input/output //
	//--------------//

	int		WriteBuffer();

protected:

	//-----------//
	// variables //
	//-----------//

	ProductFileList		_fileList;
	char*				_frame;
	int					_size;
};

#endif
