//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//
 
static const char rcs_id_configlist_c[] =
	"@(#) $Id$";

#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include "ConfigList.h"
 
//============//
// StringPair //
//============//

StringPair::StringPair()
:	_keyword(NULL), _value(NULL)
{
	return;
}

StringPair::StringPair(
	const char*		keyword,
	const char*		value)
{
	Set(keyword, value);
	return;
}

StringPair::~StringPair()
{
	free(_keyword);
	free(_value);
	return;
}

//-----------------//
// StringPair::Set //
//-----------------//
// Sets the keyword and the value simultaneously.
// Returns 1 on success, 0 on error.

int
StringPair::Set(
	const char*		keyword,
	const char*		value)
{
	if (! SetKeyword(keyword))
		return(0);
	if (! SetValue(value))
		return(0);
	return(1);
}

//------------------------//
// StringPair::SetKeyword //
//------------------------//
// Sets the keyword.
// Returns 1 on success, 0 on error.

int
StringPair::SetKeyword(
	const char*		keyword)
{
	free(_keyword);
	_keyword = NULL;

	if (keyword == NULL)
		return(1);

	_keyword = strdup(keyword);
	if (_keyword == NULL)
		return(0);

	return(1);
}

//----------------------//
// StringPair::SetValue //
//----------------------//
// Sets the value.
// Returns 1 on success, 0 on error.

int
StringPair::SetValue(
	const char*		value)
{
	free(_value);
	_value = NULL;

	if (value == NULL)
		return(1);

	_value = strdup(value);
	if (_value == NULL)
		return(0);

	return(1);
}


//============//
// ConfigList //
//============//

ConfigList::ConfigList()
:	_status(OK), _reportErrors(0), _badLine(0)
{
	return;
}

ConfigList::ConfigList(
	const char*		filename)
:	_status(OK), _reportErrors(0), _badLine(0)
{
	Read(filename);
	return;
}

ConfigList::~ConfigList()
{
	free(_badLine);
	return;
}

//--------------------------//
// ConfigList::ReportErrors //
//--------------------------//

void
ConfigList::ReportErrors(
	int		flag)
{
	if (flag)
		_reportErrors = 1;
	else
		_reportErrors = 0;
	return;
}

//------------------//
// ConfigList::Read //
//------------------//

#define LINE_SIZE		1024

ConfigList::StatusE
ConfigList::Read(
	const char*		filename)
{
	//---------------------------------------------//
	// open the file or standard input for reading //
	//---------------------------------------------//

	FILE* ifp = stdin;
	if (filename)
	{
		ifp = fopen(filename, "r");
		if (ifp == NULL)
		{
			if (_reportErrors)
				fprintf(stderr, "Error reading file: %s\n", filename);
			return(_status = ERROR_OPENING_FILE);
		}
	}

	char line[LINE_SIZE], keyword[LINE_SIZE], value[LINE_SIZE];
	int num_read = 0;
	do
	{
		char* ptr = fgets(line, LINE_SIZE, ifp);
		if (ptr != line)
		{
			if (feof(ifp))	// EOF
				break;
			else			// error
				return (_status = ERROR_READING_CONFIG_ENTRY);
		}
		num_read = sscanf(line, " %s %s", keyword, value);
		switch (num_read)
		{
		case 1:	
			if (isalnum(keyword[0]))
			{	// looks like a valid keyword, but there is no value
				_SetBadLine(line);
				return (_status = ERROR_READING_CONFIG_ENTRY);
			}
			break;
		case 2:
			if (isalnum(keyword[0]))
			{
				if (strcmp(keyword, INSERT_FILE_KEYWORD) == 0)
					Read(value);
				else
				{
					StringPair* pair = new StringPair(keyword, value);
					Append(pair);
				}
			}
		}
	} while (1);

	// don't need the file anymore
	if (filename)
		fclose(ifp);

	return (_status);
}

//-------------------//
// ConfigList::Write //
//-------------------//

ConfigList::StatusE
ConfigList::Write(
	const char*		filename)
{
	//===========================================//
	// open the file for writing, write the whole//
	// list into the file.                       //
	//===========================================//
	FILE* ofp = fopen(filename, "w");
	if (ofp == NULL)
		return (_status = ERROR_OPENING_FILE);

	int num_chars = 0;
	for (StringPair* pair = GetHead(); pair != NULL; pair = GetNext())
	{
		if ((num_chars = fprintf(ofp, "%s %s\n",
				pair->GetKeyword(), pair->GetValue())) < 0)
			return (_status = ERROR_WRITING_CONFIG_FILE);
	}
	fclose(ofp);

	return (_status);
}

//------------------//
// ConfigList::Find //
//------------------//

StringPair*
ConfigList::Find(
	const char*		keyword)
{
	for (StringPair* pair = GetHead(); pair; pair = GetNext())
	{
		if (strcmp(pair->GetKeyword(), keyword) == 0)
			return(pair);
	}
	return(0);
}

//-----------------//
// ConfigList::Get //
//-----------------//

char*
ConfigList::Get(
const char*		keyword)
{
	StringPair* pair = Find(keyword);
	if (pair)
		return(pair->GetValue());
	else
		return(0);
}

//-------------------------//
// ConfigList::ReturnFloat //
//-------------------------//
// returns the float corresponding to the keyword
// returns the default value if the float does not exist

float
ConfigList::ReturnFloat(
	const char*		keyword,
	float			default_value)
{
	char* string = Get(keyword);
	float value = default_value;
	if (string == 0)
	{
		_status = MISSING_KEYWORD;
		if (_reportErrors)
			fprintf(stderr, "Missing Keyword: %s\n", keyword);
	}
	else if (sscanf(string, "%f", &value) != 1)
	{
		_status = ERROR_CONVERTING_VALUE;
		if (_reportErrors)
			fprintf(stderr, "Error converting value for Keyword: %s\n",
				keyword);
	}
	else
		_status = OK;

	return (value);
}

//-----------------------//
// ConfigList::ReturnInt //
//-----------------------//
// returns the int corresponding to the keyword
// returns the default value if the int does not exist

int
ConfigList::ReturnInt(
	const char*		keyword,
	int				default_value)
{
	char* string = Get(keyword);
	int value = default_value;
	if (string == 0)
	{
		_status = MISSING_KEYWORD;
		if (_reportErrors)
			fprintf(stderr, "Missing Keyword: %s\n", keyword);
	}
	else if (sscanf(string, "%i", &value) != 1)
	{
		_status = ERROR_CONVERTING_VALUE;
		if (_reportErrors)
			fprintf(stderr, "Error converting value for Keyword: %s\n",
				keyword);
	}
	else
		_status = OK;

	return (value);
}

//--------------------------//
// ConfigList::ReturnString //
//--------------------------//
// returns the string corresponding to the keyword
// returns the default value if the string does not exist

const char*
ConfigList::ReturnString(
	const char*		keyword,
	const char*		default_value)
{
	char* string = Get(keyword);
	const char* value = default_value;
	if (string == 0)
	{
		_status = MISSING_KEYWORD;
		if (_reportErrors)
			fprintf(stderr, "Missing Keyword: %s\n", keyword);
	}
	else
		_status = OK;

	return (value);
}

//-------------------------//
// ConfigList::_SetBadLine //
//-------------------------//

ConfigList::StatusE
ConfigList::_SetBadLine(
	const char*		bad_line)
{
	free(_badLine);
	_badLine = strdup(bad_line);
	if (_badLine == NULL && bad_line != NULL)
		return(_status = ERROR_SETTING_BADLINE);
	return(_status);
}
