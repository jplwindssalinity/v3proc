//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef GENERICFILE_H
#define GENERICFILE_H

static const char rcs_id_genericfile_h[] =
	"@(#) $Id$";

#include <stdio.h>


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
//		reads, and writes for a single stream.
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

	int		Read(char* buffer, size_t bytes);
	int		Write(char* buffer, size_t bytes);

	int		Close();

	//--------//
	// status //
	//--------//

	int		EndOfFile() { return(feof(_fp)); };
	int		Error() { return(ferror(_fp)); };

protected:

	//-----------//
	// variables //
	//-----------//

	char*	_filename;
	FILE*	_fp;
};

#endif
