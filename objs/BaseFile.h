//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef BASEFILE_H
#define BASEFILE_H

static const char rcs_id_basefile_h[] =
	"@(#) $Id$";

#include <stdio.h>


//======================================================================
// CLASSES
//		BaseFile
//======================================================================


//======================================================================
// CLASS
//		BaseFile
//
// DESCRIPTION
//		The BaseFile object is used to easily handle opens, closes,
//		reads, and writes for a single stream.
//======================================================================

class BaseFile
{
public:

	//--------------//
	// construction //
	//--------------//

	BaseFile();
	~BaseFile();

	//---------------------//
	// setting and getting //
	//---------------------//

	int		SetFilename(const char* filename);
	char*	GetFilename()	{ return(_filename); };
	FILE*	GetFp() { return(_fp); };

	//--------------//
	// input/output //
	//--------------//

	int		OpenForReading();
	int		OpenForReading(const char* filename);
	int		OpenForWriting();
	int		OpenForWriting(const char* filename);

	int		Read(char* buffer, size_t bytes);
	int		Write(char* buffer, size_t bytes);

	int		Close();

	//--------//
	// status //
	//--------//

	int		EndOfFile()		{ return(feof(_fp)); };
	int		Error()			{ return(ferror(_fp)); };

protected:

	//-----------//
	// variables //
	//-----------//

	char*	_filename;
	FILE*	_fp;
};

#endif
