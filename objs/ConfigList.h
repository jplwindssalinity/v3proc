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
//		It represents a list of configuration file entries.
//======================================================================

#define INSERT_FILE_KEYWORD		"INSERT_FILE"

class ConfigList : public List<StringPair>
{
public:

	//-------//
	// enums //
	//-------//

	enum StatusE { OK, ERROR_OPENING_FILE, ERROR_READING_CONFIG_ENTRY,
		ERROR_WRITING_CONFIG_FILE, MISSING_KEYWORD, ERROR_CONVERTING_VALUE,
		ERROR_SETTING_BADLINE };

	//--------------//
	// construction //
	//--------------//

	ConfigList();
	ConfigList(const char* filename);
	~ConfigList();

	//---------------------------//
	// status and error handling //
	//---------------------------//

	StatusE			GetStatus() { return (_status); };
	char*			GetBadLine() { return (_badLine); };
	void			ReportErrors(int flag = 1);

	//--------------//
	// input/output //
	//--------------//

	StatusE			Read(const char* filename = NULL);
	StatusE			Write(const char* filename);

	//----------------//
	// searching list //
	//----------------//

	StringPair*		Find(const char* keyword);
	char*			Get(const char* keyword);

	//---------------------//
	// interpreting values //
	//---------------------//

	float			ReturnFloat(const char* keyword, float default_value = 0.0);
	int				ReturnInt(const char* keyword, int default_value = 0);
	const char*		ReturnString(const char* keyword,
						const char* default_value = NULL);
	int				GetDouble(const char* keyword, double* value);

protected:

	//---------------------------//
	// status and error handling //
	//---------------------------//

	StatusE			_SetBadLine(const char* bad_line);

	//-----------//
	// variables //
	//-----------//

	StatusE			_status;
	char			_reportErrors;
	char*			_badLine;
};

#endif
