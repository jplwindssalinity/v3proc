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

int
downhill_simplex(
	double** p,
	int ndim,
	int totdim,
	double ftol,
	double (*funk)(double*,void*),
	void* ptr)
{
	int i,j;
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
		if (rtol < ftol)
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
