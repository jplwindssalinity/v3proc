//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

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

static const char rcs_id_misc_h[] =
	"@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include <stdio.h>

//-----------//
// CONSTANTS //
//-----------//

#define VCTR_HEADER		"vctr"
#define OTLN_HEADER		"otln"

//--------//
// MACROS //
//--------//

#define ANGDIF(A,B)		(fabs(pi-fabs(pi-fabs((A)-(B)))))
#define MIN(A,B)		((A)<(B)?(A):(B))
#define MAX(A,B)		((A)>(B)?(A):(B))

//-----------//
// FUNCTIONS //
//-----------//

const char*		no_path(const char* string);
void			usage(const char* argv0, const char* option_array[],
					const int exit_value);
int				look_up(const char* string, const char* table[],
					const int count = -1);
FILE*			fopen_or_exit(const char* filename, const char* type,
					const char* command, const char* description,
					const int exit_value);
char			get_bits(char byte, int position, int bit_count);
int				substitute_string(const char* string, const char* find,
					const char* replace, char* result);

int				downhill_simplex(double** p, int ndim, int totdim, double ftol,
					double (*funk)(double*, void*), void* ptr);
double			amotry(double** p, double* y, double* psum, int ndim,
					int totdim, double (*funk)(double*, void*), void* ptr,
					int ihi, double fac);

float			median(const float* array, int num_elements);
float			mean(const float* array, int num_elements);
void			sort_increasing(float* array, int num_elements);

int				rel_to_abs_idx(int rel_idx, int array_size, int* abs_idx);
int				abs_to_rel_idx(int abs_idx, int array_size, int* rel_idx);

float			quantize(float value, float resolution);

#endif
