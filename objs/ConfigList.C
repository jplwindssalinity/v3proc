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
:	_keyword(NULL), _value(NULL)
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
:	_errorFp(stderr), _logFlag(EXIT)
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
			fprintf(_errorFp, "Error opening config file.\n");
			fprintf(_errorFp, "  Config File: %s\n", filename);
			exit(1);
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
				fprintf(_errorFp, "Error reading line from config file\n");
				fprintf(_errorFp, "  Config File: %s\n", filename);
				fprintf(_errorFp, "  Line Number: %d\n", line_number);
				exit(1);
			}
		}
		num_read = sscanf(line, " %s %s", keyword, value);
		switch (num_read)
		{
		case 1:	
			if (isalnum(keyword[0]))
			{
				// looks like a valid keyword, but no corresponding value
				fprintf(_errorFp, "Missing value for keyword\n");
				fprintf(_errorFp, "  Config File: %s\n", filename);
				fprintf(_errorFp, "  Line Number: %d\n", line_number);
				fprintf(_errorFp, "         Line: %s\n", line);
				exit(1);
			}
			break;
		case 2:
			if (isalnum(keyword[0]))
			{
				if (strcmp(keyword, INCLUDE_FILE_KEYWORD) == 0)
				{
					if (! Read(value))
					{
						fprintf(_errorFp, "Error reading included file\n");
						fprintf(_errorFp, "   Config File: %s\n", filename);
						fprintf(_errorFp, "  Include File: %s\n", value);
						exit(1);
					}
				}
				else
				{
					StompOrAppend(keyword, value);
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

	//--------------------------------------//
	// determine the maximum keyword length //
	//--------------------------------------//

	int max_length = 0;
	for (StringPair* pair = GetHead(); pair != NULL; pair = GetNext())
	{
		int length = strlen(pair->GetKeyword());
		if (length > max_length)
			max_length = length;
	}

	max_length += 4;	// leave a 4 character gap

	for (StringPair* pair = GetHead(); pair != NULL; pair = GetNext())
	{
		char* keyword = pair->GetKeyword();
		char* value = pair->GetValue();
		if (! keyword || ! value)
			continue;
		fprintf(ofp, "%-*s%s\n", max_length, keyword, value);
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
	StringPair* pair = _Find(keyword);
	if (pair)
		return(pair);

	switch (_logFlag)
	{
	case EXIT:
		fprintf(_errorFp, "ERROR: missing keyword\n");
		fprintf(_errorFp, "  Keyword: %s\n", keyword);
		exit(1);
		break;
	case WARN:
		fprintf(_errorFp, "Warning: missing keyword\n");
		fprintf(_errorFp, "  Keyword: %s\n", keyword);
		break;
	default:
		break;
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

//---------------------//
// ConfigList::GetChar //
//---------------------//
// sets the value to the retrieved char
// returns 1 on success, 0 on failure

int
ConfigList::GetChar(
	const char*		keyword,
	char*			value)
{
	char* string = Get(keyword);
	if (! string)
		return(0);

	char tmp;
	if (sscanf(string, "%c", &tmp) != 1)
	{
		fprintf(_errorFp, "Error converting value to char\n");
		fprintf(_errorFp, "  Keyword: %s\n", keyword);
		fprintf(_errorFp, "    Value: %s\n", string);
		exit(1);
	}

	*value = tmp;
	return(1);
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
		fprintf(_errorFp, "Error converting value to int\n");
		fprintf(_errorFp, "  Keyword: %s\n", keyword);
		fprintf(_errorFp, "    Value: %s\n", string);
		exit(1);
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
		fprintf(_errorFp, "Error converting value to double\n");
		fprintf(_errorFp, "  Keyword: %s\n", keyword);
		fprintf(_errorFp, "    Value: %s\n", string);
		exit(1);
	}

	*value = tmp;
	return(1);
}

//----------------------//
// ConfigList::GetFloat //
//----------------------//
// sets the value to the retrieved float
// returns 1 on success, 0 on failure

int
ConfigList::GetFloat(
	const char*		keyword,
	float*			value)
{
	char* string = Get(keyword);
	if (! string)
		return(0);

	float tmp;
	if (sscanf(string, "%g", &tmp) != 1)
	{
		fprintf(_errorFp, "Error converting value to float\n");
		fprintf(_errorFp, "  Keyword: %s\n", keyword);
		fprintf(_errorFp, "    Value: %s\n", string);
		exit(1);
	}

	*value = tmp;
	return(1);
}

//---------------------------//
// ConfigList::StompOrAppend //
//---------------------------//

int
ConfigList::StompOrAppend(
	const char*		keyword,
	const char*		value)
{
	StringPair* pair = _Find(keyword);
	if (pair)
	{
		if (! pair->SetValue(value))
			return(0);
	}
	else
	{
		StringPair* new_pair = new StringPair(keyword, value);
		if (new_pair == NULL)
			return(0);

		if (! Append(new_pair))
			return(0);
	}
	return(1);
}

//-------------------//
// ConfigList::_Find //
//-------------------//

StringPair*
ConfigList::_Find(
	const char*		keyword)
{
	for (StringPair* pair = GetHead(); pair; pair = GetNext())
	{
		if (strcmp(pair->GetKeyword(), keyword) == 0)
			return(pair);
	}
	return(NULL);
}
