//==============================================================//
// Copyright (C) 1997-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// OBJECTS
//    Miscellaneous objects and helper functions.
//
// DESCRIPTION
//    Multipurpose helper functions, etc.
//
// AUTHORS
//    James N. Huddleston (hudd@casket.jpl.nasa.gov)
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

#define VCTR_HEADER  "vctr"
#define OTLN_HEADER  "otln"
#define PLTR_HEADER  "pltr"

//--------//
// MACROS //
//--------//

#define ANGDIF(A,B)   (fabs(pi-fabs(pi-fabs((A)-(B)))))
#define SGN(x) ( (x) < 0 ? (-1) : (1) )
#ifndef MIN
#define MIN(A,B)      ((A)<(B)?(A):(B))
#endif

#ifndef MAX
#define MAX(A,B)      ((A)>(B)?(A):(B))
#endif

#ifndef IS_EVEN
#define IS_EVEN(A)    (A % 2 == 0 ? 1 : 0)
#endif

#define CWNTOCCWE(A)  (M_PI_2 - (A))

// Macro returns 1 if Angle A is between Start and End 0 otherwise
// Only works if all three angles are between 0 and two_pi
#define BETWEENANG(A,START,END) ((((A)>(START)) && ((A)<(END))) || (((A)>(START)) && ((END)<(START))) || (((A)<(END)) &&  ((END)<(START))))

//-----------//
// FUNCTIONS //
//-----------//

const char*  no_path(const char* string);
void         usage(const char* argv0, const char* option_array[],
                 const int exit_value);
int          look_up(const char* string, const char* table[],
                 const int count = -1);
FILE*        fopen_or_exit(const char* filename, const char* type,
                 const char* command, const char* description,
                 const int exit_value);
char         get_bits(char byte, int position, int bit_count);
int          substitute_string(const char* string, const char* find,
                 const char* replace, char* result);

float        lat_fix(float latitude);
float        lon_fix(float longitude);

int          downhill_simplex(double** p, int ndim, int totdim, double ftol,
                 double (*funk)(double*, void*), void* ptr, double xtol=0.0);
double       amotry(double** p, double* y, double* psum, int ndim, int totdim,
                 double (*funk)(double*, void*), void* ptr, int ihi,
                 double fac);

float        median(const float* array, int num_elements);
float        mean(const float* array, int num_elements);
void         sort_increasing(float* array, int num_elements);
void         insertion_sort(int N, float* a, int** indx);

int          rel_to_abs_idx(int rel_idx, int array_size, int* abs_idx);
int          abs_to_rel_idx(int abs_idx, int array_size, int* rel_idx);

float        quantize(float value, float resolution);

float        wrap_angle_near(float angle, float target);
float        angle_diff(float ang1, float ang2);

int          set_character_time(double time, double epoch_time,
                 char* epoch_time_str, char* time_str);
double       asc2sec(char *asctime);
int          sec2asc(double sec, char *asctime);
int          sec2asc_month(double sec, char* asctime);

// fit a sinusoid to data
int          sinfit(double* azimuth, double* value, double* variance,
                 int count, double* amplitude, double* phase, double* bias);

// full up spectral fit
int          specfit(double* azimuth, double* value, double* variance,
                 int sample_count, int start_term, int end_term,
                 double* amplitude, double* phase);

// sort an array
void         heapsort(int n, double* data_array, int* idx_array);

// Read fortran unformatted records
size_t       fread_f77(void* dest, size_t size, size_t nitems, FILE* stream);

// golden_section_search
int          golden_section_search(double min_x, double max_x, double x_tol,
                 double (*funk)(double x, char** arguments), char** arguments,
                 double* final_x, double* final_y);

// gaussian_fit
int     gaussian_fit(double* x, double* y, int points, double* center,
           double* variance);
double  gmse(double* c, void* ptr);

#endif
