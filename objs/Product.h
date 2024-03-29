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

	//------//
	// enum //
	//------//

	enum StatusE { OK, ERROR_ALLOCATING_FILENAME, ERROR_MISSING_FILE,
		ERROR_REOPENING_FILE, ERROR_OPENING_INPUT_FILE,
		ERROR_OPENING_OUTPUT_FILE, ERROR_CLOSING_FILE };

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
	int		OpenForInput();
	int		Close();

	//---------//
	// getting //
	//---------//

	int		GetFd() { return(_fd); };
	int		GetStatus() { return(_status); };

protected:

	//-----------//
	// variables //
	//-----------//

	char*		_filename;
	int			_fd;

	StatusE		_status;
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

	//------//
	// enum //
	//------//

	enum StatusE { OK, ERROR_CREATING_PRODUCT_FILE, ERROR_ASSIGNING_FILENAME,
		ERROR_APPENDING_PRODUCT_FILE, ERROR_NO_CURRENT_PRODUCT_FILE,
		ERROR_OPENING_OUTPUT_FILE, ERROR_OPENING_INPUT_FILE,
		ERROR_CLOSING_PRODUCT_FILE };

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
	int		OpenCurrentForInput();
	int		GetCurrentFd();
	int		CloseCurrentFile();

	//---------//
	// getting //
	//---------//

	int		GetStatus() { return(_status); };

protected:

	//-----------//
	// variables //
	//-----------//

	StatusE		_status;
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

	//------//
	// enum //
	//------//

	enum StatusE { OK, ERROR_UNKNOWN, ERROR_ALLOCATING_FRAME, ERROR_NO_FILES,
		ERROR_ADDING_FILE, ERROR_OPENING_OUTPUT_FILE, ERROR_OPENING_INPUT_FILE,
		ERROR_CLOSING_CURRENT_FILE, ERROR_GETTING_CURRENT_FD,
		ERROR_WRITING_FRAME, ERROR_READING_FRAME, ERROR_NO_MORE_DATA,
		ERROR_PACKING_FRAME, ERROR_UNPACKING_FRAME };

	//--------------//
	// construction //
	//--------------//

	Product();
	~Product();

	int		AllocateFrame(const int buffer_size);
	int		AddFile(const char* filename);

	//-------------------//
	// file manipulation //
	//-------------------//

	int		GotoFirstFile();
	int		OpenCurrentForOutput();
	int		OpenCurrentForInput();
	int		CloseCurrentFile();

	//--------------//
	// input/output //
	//--------------//

	int		WriteFrame();
	int		ReadFrame();
	int		WriteDataRec();
	int		ReadDataRec();

	//-------------------//
	// data manipulation //
	//-------------------//

	virtual int		PackFrame() = 0;
	virtual int		UnpackFrame() = 0;

	//---------//
	// getting //
	//---------//

	int		GetStatus() { return(_status); };

protected:

	//-----------//
	// variables //
	//-----------//

	ProductFileList		_fileList;
	char*				_frame;
	int					_size;

	StatusE				_status;
};

#endif
