//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.2   06 Apr 1998 16:26:40   sally
// merged with SVT
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================
 
#ifndef CONFIGLIST_H
#define CONFIGLIST_H

static const char rcs_id_configlist_h[] =
	"@(#) $Header$";

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

#define INCLUDE_FILE_KEYWORD	"INCLUDE_FILE"
#define CONFIG_FILE_LINE_SIZE	1024

class ConfigList : public List<StringPair>
{
public:

	//--------------//
	// construction //
	//--------------//

	ConfigList();
	~ConfigList();

	//---------//
	// logging //
	//---------//

	void	SetLogFile(FILE* error_fp) { _errorFp = error_fp; };
	void	ExitForMissingKeywords() { _logFlag = EXIT; };
	void	WarnForMissingKeywords() { _logFlag = WARN; };
	void	DoNothingForMissingKeywords() { _logFlag = NOTHING; };

	//--------------//
	// input/output //
	//--------------//

	int		Read(const char* filename = NULL);
	int		Write(const char* filename);
	int		Write(FILE* fp);

	//----------------//
	// searching list //
	//----------------//

	StringPair*		Find(const char* keyword);
	char*			Get(const char* keyword);

	//---------------------//
	// interpreting values //
	//---------------------//

	int		SetInt(const char* keyword, const int value);

	int		GetChar(const char* keyword, char* value);
	int		GetInt(const char* keyword, int* value);
	int		GetUnsignedInt(const char* keyword, unsigned int* value);
	int		GetDouble(const char* keyword, double* value);
	int		GetFloat(const char* keyword, float* value);

	//----------------//
	// adding to list //
	//----------------//

	int		StompOrAppend(const char* keyword, const char* value);

protected:

	enum LogE { EXIT, WARN, NOTHING };

	//---------//
	// methods //
	//---------//

	StringPair*		_Find(const char* keyword);

	//-----------//
	// variables //
	//-----------//

	FILE*	_errorFp;		// for error logging
	LogE	_logFlag;		// what to do
};

#endif
