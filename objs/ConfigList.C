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
{
	return;
}

ConfigList::~ConfigList()
{
	return;
}

//------------------//
// ConfigList::Read //
//------------------//

#define LINE_SIZE		1024

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
			return(0);
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
				return(0);
		}
		num_read = sscanf(line, " %s %s", keyword, value);
		switch (num_read)
		{
		case 1:	
			if (isalnum(keyword[0]))
				return(0);	// looks like a valid keyword, but no value
			break;
		case 2:
			if (isalnum(keyword[0]))
			{
				if (strcmp(keyword, INSERT_FILE_KEYWORD) == 0)
				{
					if (! Read(value))
						return(0);
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
		return(0);

	*value = tmp;
	return(1);
}
