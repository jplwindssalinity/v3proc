//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef ARRAY_H
#define ARRAY_H

static const char rcs_id_array_h[] =
	"@(#) $Id$";

#include <stdarg.h>

//======================================================================
// FUNCTIONS
//		make_array
//		free_array
//		dim_setup
//		dim_free
//
// See Array.C for function descriptions.
//======================================================================

void* make_array(int type_size, int ndims, ...);
void free_array(void* ptr, int ndims, ...);
void* dim_setup(int level, int ndims, int dimsize[], int type_size);
void dim_free(void* ptr, int level, int ndims, int dimsize[]);

#endif
