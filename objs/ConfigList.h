//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//
 
#ifndef CONFIGLIST_H
#define CONFIGLIST_H

static const char rcs_id_configlist_h[] =
	"@(#) $Id$";

#include <stdio.h>
#include "List.h"

//======================================================================
// CLASSES
//		StringPair, ConfigList
//======================================================================

//======================================================================
// CLASS
//		StringPair
//
// DESCRIPTION
//		The StringPair object contains a keyword/value pair.  It is
//		used by ConfigList to hold configuration list entries.
//		StringPair actually creates a copy of the passed strings so
//		the originals can be freed.
//======================================================================

class StringPair
{
public:

	//--------------//
	// construction //
	//--------------//

	StringPair();
	StringPair(const char* keyword, const char* value);
	~StringPair();

	//-----------------------------//
	// setting keywords and values //
	//-----------------------------//

	int		Set(const char* keyword, const char* value);
	int		SetKeyword(const char* keyword);
	int		SetValue(const char* value);

	//-----------------------------//
	// getting keywords and values //
	//-----------------------------//

	char*	GetKeyword() { return (_keyword); };
	char*	GetValue() { return (_value); };

private:

	//-----------//
	// variables //
	//-----------//

	char*	_keyword;
	char*	_value;
};


//======================================================================
// CLASS
//		ConfigList
//
// DESCRIPTION
//		The ConfigList object contains a list of StringPair objects.
//		It represents a list of configuration file entries.  The
//		ConfigList object is typically used at the program level and
//		therefore it contains error logging capabilities.
//======================================================================

#define INSERT_FILE_KEYWORD		"INSERT_FILE"
#define CONFIG_FILE_LINE_SIZE	1024

class ConfigList : public List<StringPair>
{
public:

	//--------------//
	// construction //
	//--------------//

	ConfigList();
	~ConfigList();

	//---------------//
	// error logging //
	//---------------//

	void	LogErrors(FILE* error_fp, int log_flag = 1);
	void	LogErrors(int log_flag = 1);

	//--------------//
	// input/output //
	//--------------//

	int		Read(const char* filename = NULL);
	int		Write(const char* filename = NULL);

	//----------------//
	// searching list //
	//----------------//

	StringPair*		Find(const char* keyword);
	char*			Get(const char* keyword);

	//---------------------//
	// interpreting values //
	//---------------------//

	int		GetChar(const char* keyword, char* value);
	int		GetInt(const char* keyword, int* value);
	int		GetDouble(const char* keyword, double* value);

protected:

	//-----------//
	// variables //
	//-----------//

	FILE*	_errorFp;		// for error logging
	int		_logFlag;		// for error logging (non-zero = log)
};

#endif
