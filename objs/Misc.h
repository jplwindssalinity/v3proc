//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// OBJECTS
//		Miscellaneous objects and helper functions.
//
// DESCRIPTION
//		Multipurpose helper functions, etc.
//
// AUTHOR
//		James N. Huddleston
//		hudd@acid.jpl.nasa.gov
//----------------------------------------------------------------------

#ifndef MISC_H
#define MISC_H

#include <stdio.h>

static const char rcs_id_misc_h[] =
	"@(#) $Id$";

const char*		no_path(const char* string);
void			usage(const char* argv0, const char* option_array[],
					const int exit_value);
int				look_up(const char* string, const char* table[],
					const int count = -1);
FILE*			fopen_or_exit(const char* filename, const char* type,
					const char* command, const char* description,
					const int exit_value);

#endif
