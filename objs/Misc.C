//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_misc_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "Misc.h"
#include "Constants.h"
#include "Matrix.h"

#define  DEBUG_DOWNHILL 0

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

//------------------//
// downhill_simplex //
//------------------//

// Adapted from the Numerical Recipes routine called amoeba.
// This function searches for the minimum of a multi-dimensional function
// by moving a "simplex" in the multidimensional space in a systematic way.
//
// Inputs:
//  p = Initial guess for the simplex.  If the function to be minimized has
//      N unknowns and M known constants, then p has N+1 rows and N+M columns.
//      The first N columns of p are the coordinates of the initial simplex.
//      It is recommended that a best guess in N-dimensional space be put
//      in the first row (N columns) and a characteristic deflection away
//      from this point be put in the following rows.  The last M columns
//      carry any constant information needed by the function to be minimized,
//      and should be the same in all rows.
//  ndim = the number of unknowns.
//  totdim = the number of unknowns plus the number of constant values needed.
//  ftol = relative accuracy in the function value that marks termination.
//  funk = pointer to the function to be minimized.  The function takes two
//         arguments.  The first is a zero-offset array of doubles with N+M
//         elements.  The first N are trial values for the unknowns, the last
//         M are known constants.  The second argument is an arbitrary pointer
//         which the function can use to receive and send back more complex
//         information that is not used by this search. 
//  xtol = another termination criteria.
//
// Using a nonzero atol value is not guaranteed to reach the
// peak for "misbehaving" functions

int
downhill_simplex(
	double** p,
	int ndim,
	int totdim,
	double ftol,
	double (*funk)(double*,void*),
	void* ptr,
        double xtol=0.0)
{
	int i,j;
	if(DEBUG_DOWNHILL && ndim==1){
	  printf("\nDownhill Simplex: ndim=%d totdim=%d ftol=%g xtol=%g\n Initial p values:\n",
		                                    ndim,totdim,ftol,xtol);
	  for(i=0;i<ndim+1;i++){
	    for(j=0;j<totdim;j++){
	      printf("%g ",p[i][j]);
	    }
	    printf("\n");
	  }
	  printf("\n");
	}
	
	int nfunk = 0;
	double ysave;

	double* y = (double*)malloc(sizeof(double)*(ndim+1));
	if (y == NULL)
	{
		printf("Error allocating memory in downhill_simplex\n");
		return(0);
	}

	for (i=0; i < ndim+1; i++)
	{
		y[i] = (*funk)(&(p[i][0]),ptr);
	}

	double* psum = (double*)malloc(sizeof(double)*totdim);
	if (psum == NULL)
	{
		printf("Error allocating memory in downhill_simplex\n");
		return(0);
	}

	for (j=0; j < ndim; j++)
	{
		psum[j] = 0.0;
		for (i=0; i < ndim+1; i++)
		{
			psum[j] += p[i][j];
		}
	}
	for (j=ndim; j < totdim; j++)
	{	// transfer the remaining fixed parameters unchanged.
		psum[j] = p[0][j];
	}

	//
	// Main Search Loop
	//


	while (1)
	{
		int inhi;
		int ilo = 0;
		int ihi = y[0]>y[1] ? (inhi=1,0) : (inhi=0,1);
		for (i=0;i<ndim+1;i++)
		  {
		    if (y[i] <= y[ilo]) ilo=i;
		    if (y[i] > y[ihi])
		      {
			inhi=ihi;
			ihi=i;
		      }
		    else if (y[i] > y[inhi] && i != ihi) inhi=i;
		  }

		double rtol=2.0*fabs(y[ihi]-y[ilo])/(fabs(y[ihi])+fabs(y[ilo]));
		double ptol=0.0;
		for(j=0;j<ndim;j++){
		   double pmax=p[0][j];
                   double pmin=p[0][j];
		   for(i=1;i<ndim+1;i++){
		     if(p[i][j]>pmax) pmax=p[i][j];
		     if(p[i][j]<pmin) pmin=p[i][j];
		   }
		   double tmp=pmax-pmin;
		   if (ptol <tmp) ptol=tmp;
		}
		if(DEBUG_DOWNHILL && ndim==1){
		  printf("rtol=%g ptol=%g\nNew p values:\n",rtol,ptol);
		  for(i=0;i<ndim+1;i++){
		    for(j=0;j<totdim;j++){
		      printf("%g ",p[i][j]);
		    }
		    printf("\n");
		  }
		  printf("New y values: ");
		  for(i=0;i<ndim+1;i++) printf("%g ",y[i]);
		  printf("\n\n");		  
		}
		if (rtol < ftol || ptol< xtol)
		{
			double tmp;
			tmp = y[0];
			y[0] = y[ilo];
			y[ilo] = tmp;
			for (i=0;i<ndim;i++)
			{
				tmp = p[0][i];
				p[0][i] = p[ilo][i];
				p[ilo][i] = tmp;
			}
			break;
		}

		if (nfunk >= 1000)
		{
			printf("Error: too many function calls in downhill_simplex\n");
			return(0);
		}
		nfunk += 2;

		double ytry=amotry(p,y,psum,ndim,totdim,funk,ptr,ihi,-1.0);
		if (ytry <= y[ilo])
		{
			ytry=amotry(p,y,psum,ndim,totdim,funk,ptr,ihi,2.0);
		}
		else if (ytry >= y[inhi])
		{
			ysave=y[ihi];
			ytry=amotry(p,y,psum,ndim,totdim,funk,ptr,ihi,0.5);
			if (ytry >= ysave)
			{
				for (i=0;i<ndim+1;i++)
				{
					if (i != ilo)
					{
						for (j=0;j<ndim;j++)
						{
							p[i][j]=psum[j]=0.5*(p[i][j]+p[ilo][j]);
						}
						y[i]=(*funk)(psum,ptr);
					}
				}
				nfunk += ndim;
				for (j=0; j < ndim; j++)
				{
					psum[j] = 0.0;
					for (i=0; i < ndim+1; i++)
					{
						psum[j] += p[i][j];
					}
				}
			}
		}
		else
		{
			--(nfunk);
		}
	}

free(y);
free(psum);
return(1);

}

//--------//
// amotry //
//--------//

double
amotry(
	double** p,
	double* y,
	double* psum,
	int ndim,
	int totdim,
	double (*funk)(double*,void*),
	void* ptr,
	int ihi,
	double fac)

{
	int j;
	double fac1,fac2,ytry,*ptry;

	ptry = (double*)malloc(sizeof(double)*totdim);
	if (ptry == NULL)
	{
		printf("Error allocating memory in amotry\n");
		exit(-1);
	}

	for (j=ndim; j < totdim; j++)
	{	// transfer the fixed parameters unchanged.
		ptry[j] = p[0][j];
	}

	fac1=(1.0-fac)/ndim;
	fac2=fac1-fac;
	for (j=0;j<ndim;j++)
	{
		ptry[j]=psum[j]*fac1-p[ihi][j]*fac2;
	}
	ytry=(*funk)(ptry,ptr);
	if (ytry < y[ihi])
	{
		y[ihi]=ytry;
		for (j=0;j<ndim;j++)
		{
			psum[j] += ptry[j]-p[ihi][j];
			p[ihi][j]=ptry[j];
		}
	}

	free(ptry);

	return(ytry);

}

float median(const float* array, int num_elements){
  // copy array
  float* buf= new float[num_elements];
  for(int c=0;c<num_elements;c++) buf[c]=array[c];

  // sort array
  sort_increasing(buf,num_elements);

  // return median value;
  float retval=buf[num_elements/2];
  delete buf;
  return(retval);
}

float mean(const float* array, int num_elements){
  float retval=0.0;
  for(int c=0;c<num_elements;c++) retval+=array[c];
  retval=retval/num_elements;
  return(retval);
}

void sort_increasing(float* array, int num_elements){
  int idx=0;
  while(idx<num_elements-1){
    if(array[idx]<=array[idx+1]) idx++;
    else{
      float tmp=array[idx+1];
      array[idx+1]=array[idx];
      array[idx]=tmp;
      idx=0;
    }
  }
}

//===============================================================//
// Insertion sort into ascending order.
// Also creates an index array (keyed off the first) if requested.
//===============================================================//

void insertion_sort(int N, float* a, int** indx)
{
	float aa;
	int ii;

	if (a == NULL)
	{
		printf("Error: insertion_sort passed null key array\n");
		exit(-1);
	}

	if (indx == NULL)
	{
		for (int j = 2; j < N; j++)
		{
			aa = a[j];
			int i = j - 1;
			while (i > 0 && a[i] > aa)
			{
				a[i+1] = a[i];
				i--;
			}
			a[i+1] = aa;
		}
	}
	else
	{
		*indx = (int*)malloc(N*sizeof(int));
		if (*indx == NULL)
		{
			printf("Error allocating memory in insertion_sort\n");
			exit(-1);
		}
		for (int i = 0; i < N; i++) (*indx)[i] = i;

		for (int j = 2; j < N; j++)
		{
			aa = a[j];
			ii = (*indx)[j];
			int i = j - 1;
			while (i > 0 && a[i] > aa)
			{
				a[i+1] = a[i];
				(*indx)[i+1] = (*indx)[i];
				i--;
			}
			a[i+1] = aa;
			(*indx)[i+1] = ii;
		}
	}
}

int rel_to_abs_idx(int rel_idx, int array_size, int* abs_idx){

  //===================================================//
  // ODD ARRAY SIZE CASE                               //
  //===================================================//
  if(array_size%2==1)
    *abs_idx=rel_idx + array_size/2;

  //======================================================//
  // EVEN ARRAY SIZE, NEGATIVE RELATIVE INDEX CASE        //
  //======================================================//
  else if(rel_idx < 0)
    *abs_idx=rel_idx + array_size/2;

  //======================================================//
  // EVEN ARRAY SIZE, POSITIVE RELATIVE INDEX CASE        //
  //======================================================//
  else if(rel_idx > 0)
    *abs_idx=rel_idx + array_size/2 -1;

  //===============//
  // ERROR         //
  //===============//
  else return(0);
  return(1);
}

int abs_to_rel_idx(int abs_idx, int array_size, int* rel_idx){

  //===================================================//
  // ODD ARRAY SIZE CASE                               //
  //===================================================//
  if(array_size%2==1) *rel_idx=abs_idx-array_size/2;

  //======================================================//
  // EVEN ARRAY SIZE, NEGATIVE RELATIVE INDEX CASE        //
  //======================================================//
  else if(abs_idx<array_size/2) *rel_idx=abs_idx-array_size/2;

  //======================================================//
  // EVEN ARRAY SIZE, POSITIVE RELATIVE INDEX CASE        //
  //======================================================//
  else *rel_idx=abs_idx-array_size/2 + 1;
  return(1);
}

//----------//
// quantize //
//----------//
// quantizes the value to the given resolution

float
quantize(
	float	value,
	float	resolution)
{
	double idx = floor(value / resolution + 0.5);
	float q_value = idx * resolution;
	return (q_value);
}

//-----------------//
// wrap_angle_near //
//-----------------//
// returns the ambiguous value of angle closest to target
// thus wrap_angle_near(0.01, 6.3) returns 6.2932

float
wrap_angle_near(
	float		angle,
	float		target)
{
	// very simple and stupid algorithm
	while (angle - target > pi)
		angle -= two_pi;

	while (angle - target < -pi)
		angle += two_pi;

	return(angle);
}

//------------//
// angle_diff //
//------------//

//
// Computes the positive difference between two angles with the result
// always less than pi.
//

float angle_diff(float ang1, float ang2)

{
	while (ang1 < 0) ang1 += two_pi;
	while (ang1 > two_pi) ang1 -= two_pi;

	while (ang2 < 0) ang2 += two_pi;
	while (ang2 > two_pi) ang2 -= two_pi;

	float result;
	if (ang1 > ang2)
	{
		result = ang1 - ang2;
	}
	else
	{
		result = ang2 - ang1;
	}

	if (result > pi) result = two_pi - result;

	return(result);

}

//--------------------//
// set_character_time //
//--------------------//

// This function sets a date/time string of the form: yyyy-mm-ddThh:mm:ss.dddZ
// which corresponds to the input time in secs past a specifed epoch time.
// The epoch date/time string is also supplied by the user.
// The string pointers are assumed to point to already allocated memory
// (24 bytes long).
//
// time = time in seconds since an arbitrary zero point.
// epoch_time = another time in seconds with the same zero point as time.
// epoch_time_str = pointer to date/time string that corresponds to epoch_time
// time_str = pointer to space for the date/time string that is determined
//            to correspond to time (given the epoch correspondence).

int set_character_time(double time, double epoch_time, char* epoch_time_str,
                       char* time_str)

{
  double etime = asc2sec(epoch_time_str);
  sec2asc(etime + time - epoch_time, time_str);
  // Set trailing 3 spaces, overwriting null terminator!
  time_str[21] = ' ';
  time_str[22] = ' ';
  time_str[23] = ' ';
  return(1);
}

/*
 * asc2sec()
 *
 * Translation from asc2sec.F
 *
 *
 *    SUBROUTINE asc2sec( asctime, SEC )
 *
 *  Input
 *
 *  asctime  is a character string containing a left justified time in th
 *           ASCII Time Code :
 *
 *              YYYY-DDDThh:mm:ss.ddd
 *
 *           where
 *
 *              YYYY     is the year
 *              DDD      is the day of year
 *              hh       is the hour
 *              mm       is the minute
 *              ss       is the seconds
 *              .ddd     is the fractional seconds
 *
 *
 *  Output
 *
 *  SEC      is the seconds past the reference epoch 
 *
 */

double asc2sec(char *asctime)

{
int year,doy,hour,minute;
int y,m,d,c,ya,date2j;
float second;
double sec,jd,jdref;

if (strlen(asctime) < 16 || strlen(asctime) > 24)
{
  fprintf(stderr,"Error: asc2sec received an invalid time string = %s\n",
          asctime);
  exit(1);
}

if (sscanf(asctime,"%4d-%3dT%2d:%2d:%f",&year,&doy,&hour,&minute,&second) != 5)
  {
  fprintf(stderr,"asc2sec: Error reading elements of date string %s\n",asctime);
  exit(0);
  }

/*
 *     compute the double precision Julian date at the start of
 *     the current day.
 *
 */

y = year - 1;
m = 10;
d = doy;

c = y/100;
ya = y - 100*c;
date2j = (146097*c)/4 + (1461*ya)/4 + (153*m+2)/5 + d + 1721119;
jd = date2j - 0.5;

/*
 * year,month,day: 1995,1,1
 * Julian date = 2449718.500000
 *
 * year,month,day: 1996,1,1
 * Julian date = 2450083.500000
 *
 * year,month,day: 1997,1,1
 * Julian date = 2450449.500000
 *
 * year,month,day: 1998,1,1
 * Julian date = 2450814.500000
 *
 */

jdref = 2450083.5;

sec = (jd - jdref)*86400.0 + hour*3600.0 + minute*60.0 + second;

return(sec);

}

/*
 * sec2asc
 *
 * Translation from sec2asc.F
 * The return string asctime must be at least 22 bytes long (21 for the string
 * and 1 for the '\0').
 *
 *
 *    SUBROUTINE sec2asc( sec, asctime )
 *
 *  Input
 *
 *  sec      is the seconds past the reference epoch 
 *
 *
 *  Output
 *
 *  asctime  is a character string containing a left justified time in th
 *           ASCII Time Code :
 *
 *              YYYY-DDDThh:mm:ss.ddd
 *
 *           where
 *
 *              YYYY     is the year
 *              DDD      is the day of year
 *              hh       is the hour
 *              mm       is the minute
 *              ss       is the seconds
 *              .ddd     is the fractional seconds
 *
 */

int sec2asc(double sec, char *asctime)

{
int month,day,year,doy,hour,minute,sec10;
int j,jd0;
int y,m,d,c,ya,jdint,isec;
float second;
double frac,jd,jdref,jdplus,dsec;

/*
 * year,month,day: 1995,1,1
 * Julian date = 2449718.500000
 *
 * year,month,day: 1996,1,1
 * Julian date = 2450083.500000
 *
 * year,month,day: 1997,1,1
 * Julian date = 2450449.500000
 *
 * year,month,day: 1998,1,1
 * Julian date = 2450814.500000
 *
 */

if (asctime == NULL)
  {
  fprintf(stderr,"Error: sec2asc received a NULL string pointer\n");
  exit(1);
  }

/* 1996,1,1 */
jdref = 2450083.50;
frac = fmod( sec, 1.0 );
if (frac < 0.0) frac = 1 + frac;
jd = (sec - frac + 0.5) / 86400.0 + jdref;

/*
 *  Compute JDINT, the integer Julian date at noon of the current day
 *  compute DSEC, the number of seconds elapsed since the start of the
 *  current day.
 *
 */

jdplus = jd + 0.5;
jdint  = (int)jdplus;
dsec   = 86400.0 * fmod(jdplus, 1.0 );
if (dsec >= 86400.0)
  {
  jdint = jdint + 1;
  dsec  = dsec  - 86400.0;
  }
isec = (int)dsec;

/*
 *  compute the year, month, and day of the calendar date.
 */
j = jdint;

j = j - 1721119;
y = (4*j-1)/146097;
j = 4*j - 1 - 146097*y;
d = j/4;
j = (4*d+3)/1461;
d = 4*d + 3 - 1461*j;
d = (d+4)/4;
m = (5*d-3)/153;
d = 5*d - 3 - 153*m;
d = (d+5)/5;
y = 100*y + j;
if ( m < 10 )
  {
  m = m + 3;
  }
else
  {
  m = m - 9;
  y = y + 1;
  }

year   = y;
month  = m;
day    = d;

/*  Compute HOUR. */
hour   = isec/3600;
isec   = isec - 3600*hour;

/*  Compute minute. */
minute = isec/60;
isec   = isec - 60*minute;

/*  Compute sec10. */
sec10  = isec/10;
isec   = isec - 10*sec10;

/*  Compute second. */
second = isec + frac;

/*
 *      DAYOYR = DATE2J( YEAR, MONTH, DAY ) - DATE2J( YEAR, 1, 0 )
 */

y = year;
m = month;
d = day;

if ( m > 2 )
  {
  m = m - 3;
  }
else
  {
  m = m + 9;
  y = y - 1;
  }

c  = y/100;
ya = y - 100*c;
jd = (146097*c)/4 + (1461*ya)/4 + (153*m+2)/5 + d + 1721119;

m = 10;
y = year - 1;

c  = y/100;
ya = y - 100*c;
jd0 = (146097*c)/4 + (1461*ya)/4 + (153*m+2)/5 + 1721119;

doy = (int)(jd - jd0);

sprintf(asctime,"%04d-%03dT%02d:%02d:%1d%05.3f",
	year,doy,hour,minute,sec10,second);

return(1);

}

/*
 * sec2asc
 *
 * Translation from sec2asc.F
 * Modifed to generate month and day of month instead of day of year.
 *
 *
 *    SUBROUTINE sec2asc_c( sec, asctime )
 *
 *  Input
 *
 *  sec      is the seconds past the reference epoch 
 *
 *
 *  Output
 *
 *  asctime  is a character string containing a left justified time in th
 *           ASCII Time Code :
 *
 *              YYYY-MM-DDThh:mm:ss.dddZ
 *
 *           where
 *
 *              YYYY     is the year
 *              MM       is the month
 *              DD       is the day of the month
 *              hh       is the hour
 *              mm       is the minute
 *              ss       is the seconds
 *              .ddd     is the fractional seconds
 *
 */

#include <stdio.h>
#include <math.h>

int sec2asc_month(double sec, char* asctime)

{
int month,day,year,doy,hour,minute,sec10;
int j,jd0;
int y,m,d,c,ya,jdint,isec;
float second;
double frac,jd,jdref,jdplus,dsec;

/*
 * year,month,day: 1995,1,1
 * Julian date = 2449718.500000
 *
 * year,month,day: 1996,1,1
 * Julian date = 2450083.500000
 *
 * year,month,day: 1997,1,1
 * Julian date = 2450449.500000
 *
 * year,month,day: 1998,1,1
 * Julian date = 2450814.500000
 *
 */

if (asctime == NULL)
  {
  fprintf(stderr,"Error: sec2asc_month received a NULL string pointer\n");
  exit(1);
  }

/* 1996,1,1 */
jdref = 2450083.50;
frac = fmod( sec, 1.0 );
if (frac < 0.0) frac = 1 + frac;
jd = (sec - frac + 0.5) / 86400.0 + jdref;

/*
 *  Compute JDINT, the integer Julian date at noon of the current day
 *  compute DSEC, the number of seconds elapsed since the start of the
 *  current day.
 *
 */

jdplus = jd + 0.5;
jdint  = (int)jdplus;
dsec   = 86400.0 * fmod(jdplus, 1.0 );
if (dsec >= 86400.0)
  {
  jdint = jdint + 1;
  dsec  = dsec  - 86400.0;
  }
isec = (int)dsec;

/*
 *  compute the year, month, and day of the calendar date.
 */
j = jdint;

j = j - 1721119;
y = (4*j-1)/146097;
j = 4*j - 1 - 146097*y;
d = j/4;
j = (4*d+3)/1461;
d = 4*d + 3 - 1461*j;
d = (d+4)/4;
m = (5*d-3)/153;
d = 5*d - 3 - 153*m;
d = (d+5)/5;
y = 100*y + j;
if ( m < 10 )
  {
  m = m + 3;
  }
else
  {
  m = m - 9;
  y = y + 1;
  }

year   = y;
month  = m;
day    = d;

/*  Compute HOUR. */
hour   = isec/3600;
isec   = isec - 3600*hour;

/*  Compute minute. */
minute = isec/60;
isec   = isec - 60*minute;

/*  Compute sec10. */
sec10  = isec/10;
isec   = isec - 10*sec10;

/*  Compute second. */
second = isec + frac;

/*
 *      DAYOYR = DATE2J( YEAR, MONTH, DAY ) - DATE2J( YEAR, 1, 0 )
 */

y = year;
m = month;
d = day;

if ( m > 2 )
  {
  m = m - 3;
  }
else
  {
  m = m + 9;
  y = y - 1;
  }

c  = y/100;
ya = y - 100*c;
jd = (146097*c)/4 + (1461*ya)/4 + (153*m+2)/5 + d + 1721119;

m = 10;
y = year - 1;

c  = y/100;
ya = y - 100*c;
jd0 = (146097*c)/4 + (1461*ya)/4 + (153*m+2)/5 + 1721119;

doy = (int)(jd - jd0);

sprintf(asctime,"%04d-%02d-%02dT%02d:%02d:%1d%05.3f",
	year,month,day,hour,minute,sec10,second);

return(1);
}

//--------//
// sinfit //
//--------//
// a brilliantly derived sinusoidal regression method by B. Stiles.
// fits to y = A + B*cos(wt) + C*sin(wt)
// this equates to y = A + D*cos(wt + E) where
// B = D*cos(E) and C = -D*sin(E)
// solved for...
// D = sqrt(B*B + C*C) and E = atan2(-B / C)

int
sinfit(
    double*  azimuth,
    double*  value,
    double*  variance,
    int      count,
    double*  amplitude,
    double*  phase,
    double*  bias)
{
    //-----------------------------------//
    // set up matrix and solution vector //
    //-----------------------------------//

    double matrix_a[3][3];
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            matrix_a[i][j] = 0.0;
        }
    }

    double vector_y[3];
    for (int i = 0; i < 3; i++)
    {
        vector_y[i] = 0.0;
    }

    //------------//
    // accumulate //
    //------------//

    matrix_a[0][0] = count;
    double use_var = 1.0;
    for (int i = 0; i < count; i++)
    {
        if (variance)
            use_var = variance[i];

        double si = sin(azimuth[i]);
        double co = cos(azimuth[i]);
        double ss = si * si;
        double cc = co * co;
        double cs = co * si;
        matrix_a[0][1] += co / use_var;
        matrix_a[0][2] += si / use_var;
        matrix_a[1][0] += co / use_var;
        matrix_a[1][1] += cc / use_var;
        matrix_a[1][2] += cs / use_var;
        matrix_a[2][0] += si / use_var;
        matrix_a[2][1] += cs / use_var;
        matrix_a[2][2] += ss / use_var;

        vector_y[0] += value[i] / use_var;
        vector_y[1] += value[i] * co / use_var;
        vector_y[2] += value[i] * si / use_var;
    }

    //---------------------------------//
    // transfer into matrix and vector //
    //---------------------------------//

    Matrix A;
    A.Allocate(3, 3);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            A.SetElement(i, j, matrix_a[i][j]);
        }
    }

    Vector Y;
    Y.Allocate(3);
    for (int i = 0; i < 3; i++)
    {
        Y.SetElement(i, vector_y[i]);
    }

    //-------//
    // solve //
    //-------//

    Vector X;
    if (! A.SolveSVD(&Y, &X))
    {
        fprintf(stderr, "Error solving equations!\n");
        exit(1);
    }

    //-------------------------//
    // return the coefficients //
    //-------------------------//

    double a, b, c;
    X.GetElement(0, &a);
    X.GetElement(1, &b);
    X.GetElement(2, &c);

    *amplitude = sqrt(b*b + c*c);
    *phase = atan2(-c, b);
    *bias = a;

    return(1);
}

//----------//
// heapsort //
//----------//
// doesn't actually sort, but sets the index array for sorted access

void
heapsort(
    int      n,
    double*  data_array,
    int*     idx_array)
{
    //----------------------------//
    // initialize the index array //
    //----------------------------//

    int i, j;
    for (i = 0; i < n; i++)
    {
        idx_array[i] = i;
    }

    //------//
    // sort //
    //------//

    data_array--;
    idx_array--;

    int l = (n >> 1) + 1;
    int ir = n;
    int idx;
    double q;

    for (;;)
    {
        if (l > 1)
        {
            l--;
            idx = idx_array[l];
            q = data_array[idx];
        }
        else
        {
            idx = idx_array[ir];
            q = data_array[idx];
            idx_array[ir] = idx_array[1];
            ir--;
            if (ir == 1)
            {
                idx_array[1] = idx;
                return;
            }
        }
        i = l;
        j = l << 1;
        while (j <= ir)
        {
            if (j < ir &&
                data_array[idx_array[j]] < data_array[idx_array[j+1]])
            {
                j++;
            }
            if (q < data_array[idx_array[j]])
            {
                idx_array[i] = idx_array[j];
                j += (i=j);
            }
            else
                j = ir + 1;
        }
        idx_array[i] = idx;
    }
    return;
}
