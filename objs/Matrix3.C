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

void Matrix3::show(char *name)

{

int i;
if (name == NULL)
  {
  for (i=0; i < 3; i++)
    {
    printf("[%10g %10g %10g]\n",_m[i][0],_m[i][1],_m[i][2]);
    }
  }
else
  {
  char *str = (char *)malloc(strlen(name)+1);
  if (str == NULL)
    {
    printf("Error: couldn't allocate memory in Matrix3::show\n");
    exit(-1);
    }
  // fill temporary string with spaces (equal to name string in length)
  for (i=0; i < strlen(name); i++)
    {
    str[i] = ' ';
    }
  str[i] = '\0';
  for (i=0; i < 3; i++)
    {
    if (i == 0)
      {
      printf("%s = [%10g %10g %10g]\n",name,_m[i][0],_m[i][1],_m[i][2]);
      }
    else
      {
      printf("%s   [%10g %10g %10g]\n",str,_m[i][0],_m[i][1],_m[i][2]);
      }
    }
  free(str);
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
// Multiply a vector by a scalar.
//

Vector3 Vector3::operator*(double s)

{
int i;
Vector3 result;

for (i=0; i < 3; i++)
  {
  result._v[i] = _v[i] * s;
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
// Get the magnitude of the vector.
//

double Vector3::magnitude()

{
return(sqrt(_v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2]));
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

void Vector3::show(char *name)

{

if (name == NULL)
  {
  printf("(%10g, %10g, %10g)\n",_v[0],_v[1],_v[2]);
  }
else
  {
  printf("%s = (%10g, %10g, %10g)\n",name,_v[0],_v[1],_v[2]);
  }

}

//--------------//
// Vector3::Set //
//--------------//

int
Vector3::Set(
	int		index,
	double	value)
{
	if (index < 0 || index > 2)
		return(0);
	_v[index] = value;
	return(1);
}

//
// EarthPosition
//

//
// Initialize with a complete set of 3 user-specified elements.
// The meaning of the 3 elements is specified by the type argument at the end.
// Note that altitudes must be specified in meters!
//

EarthPosition::EarthPosition(double x1, double x2, double x3,
                             earthposition_typeE etype)

{

if (etype == GEODETIC)
  {	// convert geodetic latitude to geocentric latitude
  x2 = atan(tan(x2)*(1-ECCENTRICITY_EARTH*ECCENTRICITY_EARTH));
  etype = GEOCENTRIC;
  }

if (etype == RECTANGULAR)
  {
  // x1,x2,x3 are the rectangular coordinates (with the same units).
  _v[0] = x1;
  _v[1] = x2;
  _v[2] = x3;
  }
else if (etype == GEOCENTRIC)
  {
  // x1 is the altitude above the ellipsoidal earth's surface. (meters)
  // x2 is the geocentric latitude of the surface location (radians).
  // x3 is the east longitude of the surface location (radians).

  double sx2 = sin(x2);
  double cx2 = cos(x2);
  double sx3 = sin(x3);
  double cx3 = cos(x3);

  // Radius of earth at desired location.
  double flat = 1.0 - sqrt(1.0-ECCENTRICITY_EARTH*ECCENTRICITY_EARTH);
  double radius = R1_EARTH*(1.0 - flat*sx2*sx2);

  // Form sea-level position vector
  _v[0] = radius*cx2*cx3;
  _v[1] = radius*cx2*sx3;
  _v[2] = radius*sx2;

  // Form vector normal to the surface of the ellipsoidal earth at the desired
  // location, with length equal to the desired altitude, and add it to the
  // sealevel position vector to get the final position vector.
  Vector3 normal(_v[0]/(R1_EARTH*R1_EARTH),
                 _v[1]/(R1_EARTH*R1_EARTH),
                 _v[2]/(R2_EARTH*R2_EARTH));
  normal.scale(x1);

  _v[0] += normal.get(0);
  _v[1] += normal.get(1);
  _v[2] += normal.get(2);
  }
else
  {
  printf("Error: Invalid type = %d received by EarthPosition object\n",etype);
  exit(-1);
  }

}

//
// Initialize with a 2 user-specified elements.
// The altitude is assumed to be zero (ie., a position on the surface)
// The latitude type is specified by the type argument at the end.
//

EarthPosition::EarthPosition(double lat, double lon,
                             earthposition_typeE etype)

{

// call general constructor with zero altitude
EarthPosition::EarthPosition(0,lat,lon,etype);

}

//
// Default constructor, no initialization
//

EarthPosition::EarthPosition()

{
return;
}

//
// Default destructor, no action
//

EarthPosition::~EarthPosition()

{
return;
}

//
// Assignment operator to allow a Vector3 object to be assigned to a
// EarthPosition object straight across (assumes that the Vector3 object
// is a geocentric position vector).
//

void EarthPosition::operator=(Vector3 vec)

{

// transfer data straight across
_v[0] = vec.get(0);
_v[1] = vec.get(1);
_v[2] = vec.get(2);

}

//
// Other access methods
//

//
// Convert the rectangular vector stored in this object into the
// corresponding altitude above the surface, latitude, and longitude.
// Currently, only surface points (altitude = 0) are handled.
//

Vector3 EarthPosition::get_alt_lat_lon(earthposition_typeE etype)

{
double lat = 0;
double elon = 0;

double sx2 = sin(_v[1]);

// Radius of earth at desired location.
double flat = 1.0 - sqrt(1.0-ECCENTRICITY_EARTH*ECCENTRICITY_EARTH);
double radius = R1_EARTH*(1 - flat*sx2*sx2);
double mag = sqrt(_v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2]);

if (fabs(radius - mag) > 0.001)
  {	// more than 1 mm off the surface is considered off the surface
  printf("Error: get_alt_lat_lon\n");
  printf("       Off surface position vectors are not handled yet\n");
  exit(-1);
  }

if ((etype == GEOCENTRIC) || (etype == GEODETIC))
  {
  lat = asin(_v[2]/mag);
  double coslat = cos(lat);
  if (coslat == 0.0)
    {	// at one of the poles, so longitude is not defined: just use zero.
    elon = 0.0;
    }
  else
    {
    elon = acos(_v[0]/mag/coslat);
    }
  }
else
  {
  printf("Error: type must be GEOCENTRIC or GEODETIC for get_alt_lat_lon\n");
  exit(-1);
  }

if (etype == GEODETIC)
  {	// convert geocentric latitude to geodetic latitude
  lat = atan(tan(lat)/(1-ECCENTRICITY_EARTH*ECCENTRICITY_EARTH));
  }

Vector3 result(0,lat,elon);
return(result);

}
