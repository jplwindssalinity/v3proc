//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef GENERICFILE_H
#define GENERICFILE_H

static const char rcs_id_genericfile_h[] =
	"@(#) $Id$";


//======================================================================
// CLASSES
//		GenericFile
//======================================================================


//======================================================================
// CLASS
//		GenericFile
//
// DESCRIPTION
//		The GenericFile object is used to easily handle opens, closes,
//		reads, and writes for a single binary file.
//======================================================================

class GenericFile
{
public:

	//--------------//
	// construction //
	//--------------//

	GenericFile();
	~GenericFile();

	int		SetFilename(const char* filename);

	//--------------//
	// input/output //
	//--------------//

	int		OpenForInput();
	int		OpenForOutput();

	int		Read(char* buffer, int bytes);
	int		Write(char* buffer, int bytes);

	int		Close();

protected:

	//-----------//
	// variables //
	//-----------//

	char*	_filename;
	int		_fd;
};

#endif
