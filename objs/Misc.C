//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_misc_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Misc.h"

//---------//
// no_path //
//---------//

const char*
no_path(
	const char*		string)
{
	const char* last_slash = strrchr(string, '/');
	if (! last_slash)
		return(string);
	return(last_slash + 1);
}

#define LINE_LENGTH		78

//-------//
// usage //
//-------//

void
usage(
	const char*		command,
	const char*		option_array[],
	const int		exit_value)
{
	fprintf(stderr, "usage: %s", command);
	int skip = 11;
	int position = 7 + strlen(command);
	for (int i = 0; option_array[i]; i++)
	{
		int length = strlen(option_array[i]);
		position += length;
		if (position > LINE_LENGTH)
		{
			fprintf(stderr, "\n%*s", skip, " ");
			position = skip + length;
		}
		fprintf(stderr, " %s", option_array[i]);
	}
	fprintf(stderr, "\n");
	exit(exit_value);
}

//---------//
// look_up //
//---------//

int
look_up(
	const char*		string,
	const char*		table[],
	const int		count)
{
	if (count != -1)
	{
		for (int i = 0; i < count; i++)
		{
			if (! strcasecmp(string, table[i]))
				return(i);
		}
		return(-1);
	}
	else
	{
		for (int i = 0; table[i]; i++)
		{
			if (! strcasecmp(string, table[i]))
				return(i);
		}
		return(-1);
	}
	return(-1);
}

//---------------//
// fopen_or_exit //
//---------------//

FILE*
fopen_or_exit(
	const char*		filename,
	const char*		type,
	const char*		command,
	const char*		description,
	const int		exit_value)
{
	FILE* fp = fopen(filename, type);
	if (fp == NULL)
	{
		fprintf(stderr, "%s: error opening %s %s\n", command, description,
			filename);
		exit(exit_value);
	}
	return(fp);
}

//----------//
// get_bits //
//----------//

char
get_bits(
	char	byte,
	int		position,
	int		bit_count)
{
	return( (byte >> (position + 1 - bit_count)) & ~(~0 << bit_count) );
}

//-------------------//
// substitute_string //
//-------------------//

int
substitute_string(
	const char*		string,
	const char*		find,
	const char*		replace,
	char*			result)
{
	char* ptr = strstr(string, find);
	if (ptr == 0)
		return(0);

	int length = strlen(find);
	sprintf(result, "%.*s%s%s", (ptr - string), string, replace,
		ptr + length);
	return(1);
}
