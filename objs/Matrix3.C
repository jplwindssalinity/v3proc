//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_matrix3_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "Matrix3.h"

//
// Matrix3
//

//
// Initialize with a complete set of 9 user-specified elements.
//

Matrix3::Matrix3(double x11, double x12, double x13,
                double x21, double x22, double x23,
                double x31, double x32, double x33)
{

_m[0][0] = x11;
_m[0][1] = x12;
_m[0][2] = x13;
_m[1][0] = x21;
_m[1][1] = x22;
_m[1][2] = x23;
_m[2][0] = x31;
_m[2][1] = x32;
_m[2][2] = x33;

}

//
// Initialize all 9 elements with a single value.
//

Matrix3::Matrix3(double init)

{

int i,j;
for (i=0; i < 3; i++)
for (j=0; j < 3; j++)
  {
  _m[i][j] = init;
  }

}

//
// Initialize with a special structure indicated by the enum argument.
//

Matrix3::Matrix3(matrixtypeE mtype)

{

if (mtype == IDENTITY)
  {	/* fill matrix with 3x3 identity matrix */
  int i,j;
  for (i=0; i < 3; i++)
  for (j=0; j < 3; j++)
    {
    _m[i][j] = 0;
    if (i == j) _m[i][j] = 1;
    }
  }
else if (mtype == GENERAL)
  {
  return;
  }
else
  {
  printf("Error: Matrix3 object received unrecognized enum = %d\n",mtype);
  exit(-1);
  }
}

//
// Default constructor which does no initialization.
//

Matrix3::Matrix3()
{
return;
}

//
// Destructor
//

Matrix3::~Matrix3()
{
return;
}

//
// Operator + to add two 3x3 matrices together, giving a 3x3 matrix.
//

Matrix3 Matrix3::operator+(Matrix3 m2)

{
int i,j;
Matrix3 result;

for (i=0; i < 3; i++)
for (j=0; j < 3; j++)
  {
  result._m[i][j] = _m[i][j] + m2._m[i][j];
  }

return(result);
}

//
// Operator - to subtract two 3x3 matrices, giving a 3x3 matrix.
//

Matrix3 Matrix3::operator-(Matrix3 m2)

{
int i,j;
Matrix3 result;

for (i=0; i < 3; i++)
for (j=0; j < 3; j++)
  {
  result._m[i][j] = _m[i][j] - m2._m[i][j];
  }

return(result);
}

//
// Operator unary - to negate a 3x3 matrix, giving a 3x3 matrix.
//

Matrix3 Matrix3::operator-()

{
int i,j;
Matrix3 result;

for (i=0; i < 3; i++)
for (j=0; j < 3; j++)
  {
  result._m[i][j] = - _m[i][j];
  }

return(result);
}

//
// Operator * to multiply two 3x3 matrices together, giving a 3x3 matrix.
//

Matrix3 Matrix3::operator*(Matrix3 m2)

{
int i,j,k;
Matrix3 result(0.0);

for (i=0; i < 3; i++)
for (j=0; j < 3; j++)
for (k=0; k < 3; k++)
  {
  result._m[i][j] += _m[i][k] * m2._m[k][j];
  }

return(result);
}

//
// Operator * to multiply a 3-vector by a 3x3 matrix, giving a 3-vector.
//

Vector3 Matrix3::operator*(Vector3 v2)

{
int i,k;
Vector3 result(0);

for (i=0; i < 3; i++)
for (k=0; k < 3; k++)
  {
  result._v[i] += _m[i][k] * v2._v[k];
  }

return(result);
}

//
// Method rowset will set the rows of the calling Matrix3 object with
// the contents of the specified Vector3 objects.
//

void Matrix3::rowset(Vector3 r1, Vector3 r2, Vector3 r3)

{

int i;
for (i=0; i < 3; i++)
  {
  _m[0][i] = r1._v[i];
  _m[1][i] = r2._v[i];
  _m[2][i] = r3._v[i];
  }

}

//
// Method identity sets the calling Matrix3 object to a 3x3 identity matrix.
//

void Matrix3::identity()

{

int i,j;
for (i=0; i < 3; i++)
for (j=0; j < 3; j++)
  {
  _m[i][j] = 0;
  if (i == j) _m[i][j] = 1;
  }

}

//
// Method inverse sets the calling Matrix3 object to its inverse.
// The gaussj routine from Numerical Recipes is adapted for 3x3 matrices here.
//

void Matrix3::inverse()

{

#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}

int n = 3;
int indxc[3],indxr[3],ipiv[3];
int i,icol,irow,j,k,l,ll;
float big,dum,pivinv,temp;

for (j=0;j<n;j++) ipiv[j]=0;
for (i=0;i<n;i++) {
	big=0.0;
	for (j=0;j<n;j++)
		if (ipiv[j] != 1)
			for (k=0;k<n;k++) {
				if (ipiv[k] == 0) {
					if (fabs(_m[j][k]) >= big) {
						big=fabs(_m[j][k]);
						irow=j;
						icol=k;
					}
				} else if (ipiv[k] > 1)
				    {
    printf("Error: Matrix3 object tried to invert singular matrix\n");
    exit(-1);
				    }
			}
	++(ipiv[icol]);
	if (irow != icol) {
		for (l=0;l<n;l++) SWAP(_m[irow][l],_m[icol][l])
	}
	indxr[i]=irow;
	indxc[i]=icol;
	if (_m[icol][icol] == 0.0)
	    {
    printf("Error: Matrix3 object tried to invert singular matrix\n");
    exit(-1);
	    }
	pivinv=1.0/_m[icol][icol];
	_m[icol][icol]=1.0;
	for (l=0;l<n;l++) _m[icol][l] *= pivinv;
	for (ll=0;ll<n;ll++)
		if (ll != icol) {
			dum=_m[ll][icol];
			_m[ll][icol]=0.0;
			for (l=0;l<n;l++) _m[ll][l] -= _m[icol][l]*dum;
		}
}
for (l=n-1;l>=0;l--) {
	if (indxr[l] != indxc[l])
		for (k=0;k<n;k++)
			SWAP(_m[k][indxr[l]],_m[k][indxc[l]]);
}

#undef SWAP

}

void Matrix3::show()

{

int i;
for (i=0; i < 3; i++)
  {
  printf("%10g %10g %10g\n",_m[i][0],_m[i][1],_m[i][2]);
  }

}

//
// Vector3
//

//
// Initialize with a complete set of 3 user-specified elements.
//

Vector3::Vector3(double x1, double x2, double x3)

{

_v[0] = x1;
_v[1] = x2;
_v[2] = x3;

}

//
// Initialize all 3 elements with a single value.
//

Vector3::Vector3(double init)

{

int i;
for (i=0; i < 3; i++)
  {
  _v[i] = init;
  }

}
	
//
// Create, but don't initialize.
//

Vector3::Vector3()

{
return;
}

Vector3::~Vector3()
{
return;
}

Vector3 Vector3::operator+(Vector3 v2)

{
int i;
Vector3 result;

for (i=0; i < 3; i++)
  {
  result._v[i] = _v[i] + v2._v[i];
  }

return(result);
}

Vector3 Vector3::operator-(Vector3 v2)

{
int i;
Vector3 result;

for (i=0; i < 3; i++)
  {
  result._v[i] = _v[i] - v2._v[i];
  }

return(result);
}

Vector3 Vector3::operator-()

{
int i;
Vector3 result;

for (i=0; i < 3; i++)
  {
  result._v[i] = - _v[i];
  }

return(result);
}

//
// Element by element multiplication of two vectors.
//

Vector3 Vector3::operator*(Vector3 v2)

{
int i;
Vector3 result;

for (i=0; i < 3; i++)
  {
  result._v[i] = _v[i] * v2._v[i];
  }

return(result);
}

//
// Cross product of two vectors.
//

Vector3 Vector3::operator&(Vector3 v2)

{
Vector3 result;

result._v[0] = _v[1]*v2._v[2] - _v[2]*v2._v[1];
result._v[1] = _v[2]*v2._v[0] - _v[0]*v2._v[2];
result._v[2] = _v[0]*v2._v[1] - _v[1]*v2._v[0];

return(result);
}

//
// Scale the vector to have the specifed magnitude.
//

void Vector3::scale(double r)

{
int i;
double mag = sqrt(_v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2]);

if (mag != 0.0)
  for (i=0; i < 3; i++)
    {
    _v[i] *= r/mag;
    }

}

//
// Extract one element.
//

double Vector3::get(int i)

{

if ((i >= 0) && (i < 3))
  {
  return(_v[i]);
  }
else
  {
  printf("Error: attempted to get out of range element from Vector3 object\n");
  exit(-1);
  }

}

void Vector3::show()

{

printf("%10g %10g %10g\n",_v[0],_v[1],_v[2]);

}
