//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//
 
static const char rcs_id_configlist_c[] =
	"@(#) $Id$";

#include <stdio.h>
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
:	_errorFp(stderr), _logFlag(0)
{
	return;
}

ConfigList::~ConfigList()
{
	StringPair* pair;
	GetHead();
	while ((pair=RemoveCurrent()) != NULL)
		delete pair;

	return;
}

//-----------------------//
// ConfigList::LogErrors //
//-----------------------//

void
ConfigList::LogErrors(
	FILE*			error_fp,
	int				log_flag)
{
	_errorFp = error_fp;
	_logFlag = log_flag;
	return;
}

void
ConfigList::LogErrors(
	int		log_flag)
{
	_logFlag = log_flag;
	return;
}

//------------------//
// ConfigList::Read //
//------------------//

int
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
			if (_logFlag)
			{
				fprintf(_errorFp, "Error opening config file\n");
				fprintf(_errorFp, "  Config File: %s\n", filename);
			}
			return(0);
		}
	}

	char line[CONFIG_FILE_LINE_SIZE];
	char keyword[CONFIG_FILE_LINE_SIZE];
	char value[CONFIG_FILE_LINE_SIZE];

	int num_read = 0;
	int line_number = 0;
	do
	{
		char* ptr = fgets(line, CONFIG_FILE_LINE_SIZE, ifp);
		line_number++;
		if (ptr != line)
		{
			if (feof(ifp))	// EOF
				break;
			else			// error
			{
				if (_logFlag)
				{
					fprintf(_errorFp, "Error reading line from config file\n");
					fprintf(_errorFp, "  Config File: %s\n", filename);
					fprintf(_errorFp, "  Line Number: %d\n", line_number);
				}
				return(0);
			}
		}
		num_read = sscanf(line, " %s %s", keyword, value);
		switch (num_read)
		{
		case 1:	
			if (isalnum(keyword[0]))
			{
				// looks like a valid keyword, but no corresponding value
				if (_logFlag)
				{
					fprintf(_errorFp, "Missing value for keyword\n");
					fprintf(_errorFp, "  Config File: %s\n", filename);
					fprintf(_errorFp, "  Line Number: %d\n", line_number);
					fprintf(_errorFp, "         Line: %s\n", line);
				}
				return(0);
			}
			break;
		case 2:
			if (isalnum(keyword[0]))
			{
				if (strcmp(keyword, INSERT_FILE_KEYWORD) == 0)
				{
					if (! Read(value))
					{
						if (_logFlag)
						{
							fprintf(_errorFp, "Error reading inserted file\n");
							fprintf(_errorFp, "  Config File: %s\n", filename);
							fprintf(_errorFp, "  Insert File: %s\n", value);
						}
						return(0);
					}
				}
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

	return (1);
}

//-------------------//
// ConfigList::Write //
//-------------------//

int
ConfigList::Write(
	const char*		filename)
{
	//--------------------------------------//
	// open the file or use standard output //
	//--------------------------------------//

	FILE* ofp = fopen(filename, "w");
	if (ofp == NULL)
		return (0);

	int num_chars = 0;
	for (StringPair* pair = GetHead(); pair != NULL; pair = GetNext())
	{
		if ((num_chars = fprintf(ofp, "%s %s\n",
				pair->GetKeyword(), pair->GetValue())) < 0)
		{
			return (0);
		}
	}
	fclose(ofp);

	return(1);
}

//------------------//
// ConfigList::Find //
//------------------//

StringPair*
ConfigList::Find(
	const char*		keyword)
{
	for (StringPair* pair = GetTail(); pair; pair = GetPrev())
	{
		if (strcmp(pair->GetKeyword(), keyword) == 0)
			return(pair);
	}
	if (_logFlag)
	{
		fprintf(_errorFp, "Missing keyword\n");
		fprintf(_errorFp, "  Keyword: %s\n", keyword);
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

//--------------------//
// ConfigList::GetInt //
//--------------------//
// sets the value to the retrieved int
// returns 1 on success, 0 on failure

int
ConfigList::GetInt(
	const char*		keyword,
	int*			value)
{
	char* string = Get(keyword);
	if (! string)
		return(0);

	int tmp;
	if (sscanf(string, "%d", &tmp) != 1)
	{
		if (_logFlag)
		{
			fprintf(_errorFp, "Error converting value to int\n");
			fprintf(_errorFp, "  Keyword: %s\n", keyword);
			fprintf(_errorFp, "    Value: %s\n", string);
		}
		return(0);
	}

	*value = tmp;
	return(1);
}

//-----------------------//
// ConfigList::GetDouble //
//-----------------------//
// sets the value to the retrieved double
// returns 1 on success, 0 on failure

int
ConfigList::GetDouble(
	const char*		keyword,
	double*			value)
{
	char* string = Get(keyword);
	if (! string)
		return(0);

	double tmp;
	if (sscanf(string, "%lg", &tmp) != 1)
	{
		if (_logFlag)
		{
			fprintf(_errorFp, "Error converting value to double\n");
			fprintf(_errorFp, "  Keyword: %s\n", keyword);
			fprintf(_errorFp, "    Value: %s\n", string);
		}
		return(0);
	}

	*value = tmp;
	return(1);
}
