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
#include "Constants.h"

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

void Matrix3::Rowset(Vector3 r1, Vector3 r2, Vector3 r3)

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

void Matrix3::Identity()

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

void Matrix3::Inverse()

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

void Matrix3::Show(char *name)

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
// Dot product of two vectors.
//

double Vector3::operator%(Vector3 v2)

{

return(_v[0]*v2._v[0] + _v[1]*v2._v[1] + _v[2]*v2._v[2]);

}

//
// Scale the vector to have the specifed magnitude.
//

void Vector3::Scale(double r)

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

double Vector3::Magnitude()

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

void Vector3::Show(char *name)

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

//-----------------------//
// Vector3::SphericalSet //
//-----------------------//

//
// This method sets the (rectangular) elements of a Vector3 object given the
// corresonding elements in a spherical coordinate system.
// The standard definition of spherical coordinates is used:
//   r is the length (magnitude) of the vector
//   theta is the angle away from the no. 3 axis (z-axis)
//   phi is the angle away from the no. 1 axis (x-axis) in the 1-2 (x-y) plane.
//

int
Vector3::SphericalSet(
	double	r,
	double	theta,
	double	phi)
{
	_v[0] = r*sin(theta)*cos(phi);
	_v[1] = r*sin(theta)*sin(phi);
	_v[2] = r*cos(theta);
	return(1);
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

void
Vector3::Set(double x1, double x2, double x3)

{

_v[0] = x1;
_v[1] = x2;
_v[2] = x3;

}

//--------------//
// Vector3::Get //
//--------------//

int
Vector3::Get(
	int			index,
	double*		value)
{
	if (index < 0 || index > 2)
		return(0);
	*value = _v[index];
	return(1);
}

//---------------//
// Vector3::Zero //
//---------------//

void
Vector3::Zero()
{
	_v[0] = 0.0;
	_v[1] = 0.0;
	_v[2] = 0.0;
	return;
}

//
// EarthPosition
//

//
// Initialize with a complete set of 3 user-specified elements.
// The meaning of the 3 elements is specified by the type argument at the end.
// Note that altitudes must be specified in km!
//

EarthPosition::EarthPosition(double x1, double x2, double x3,
                             earthposition_typeE etype)

{

if (etype == GEODETIC)
  {	// convert geodetic latitude to geocentric latitude
  x2 = atan(tan(x2)*(1-eccentricity_earth*eccentricity_earth));
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
  // x1 is the altitude above the ellipsoidal earth's surface. (km)
  // x2 is the geocentric latitude of the surface location (radians).
  // x3 is the east longitude of the surface location (radians).

  double sx2 = sin(x2);
  double cx2 = cos(x2);
  double sx3 = sin(x3);
  double cx3 = cos(x3);

  // Radius of earth at desired location.
  double flat = 1.0 - sqrt(1.0-eccentricity_earth*eccentricity_earth);
  double radius = r1_earth*(1.0 - flat*sx2*sx2);

  // Form sea-level position vector
  _v[0] = radius*cx2*cx3;
  _v[1] = radius*cx2*sx3;
  _v[2] = radius*sx2;

  // Form vector normal to the surface of the ellipsoidal earth at the desired
  // location, with length equal to the desired altitude, and add it to the
  // sealevel position vector to get the final position vector.
  Vector3 normal(_v[0]/(r1_earth*r1_earth),
                 _v[1]/(r1_earth*r1_earth),
                 _v[2]/(r2_earth*r2_earth));
  normal.Scale(x1);

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
// Initialize with the elements of a Vector3 object.
// The elements are assumed to be rectangular coordinates.
//

EarthPosition::EarthPosition(Vector3 v)

{

v.Get(0,&_v[0]);
v.Get(1,&_v[1]);
v.Get(2,&_v[2]);

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
// Compute the distance along the surface of the earth between the position
// specified in the calling object and the position specified by the argument.
// This function assumes that both positions are actually on the surface
// (ie., altitude = 0), but does not check.  If one is not on the surface,
// then the result is nonsense.
// Right now, this routine uses a simple spherical approximation of the earth.
//

double EarthPosition::surface_distance(EarthPosition r)

{

double mag = this->Magnitude();
//double mag = sqrt(_v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2]);
double theta = acos((*this % r) / (mag * r.Magnitude()));
return(mag * theta);

}

//
// Convert the rectangular vector stored in this object into the
// corresponding altitude above the surface, latitude, and longitude.
// Units are km and radians.
//
// Inputs:
//  etype = GEOCENTRIC or GEODETIC - the type of latitude to return
//
// Return Value:
//  A Vector3 object containing the altitude, latitude, and E. longitude
//  (in that order) of the point contained in the calling EarthPosition object.
//

Vector3 EarthPosition::get_alt_lat_lon(earthposition_typeE etype)

{

double lat;
double elon;
double alt;

double flat = 1.0 - sqrt(1.0-eccentricity_earth*eccentricity_earth);
double f1 = 1 - flat;
double f2 = flat*(2.0 - flat);
double rho = sqrt(_v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2]);
double r = sqrt(_v[0]*_v[0] + _v[1]*_v[1]);

if (rho == 0.0)
  {	// center of the earth
  Vector3 result(-r2_earth,0,0);
  return(result);
  }

// Compute east longitude
if (_v[0] == 0.0)
  {	// on the 90 -- 270 great circle
  if (_v[1] > 0.0) elon = pi/2; else elon = 3*pi/2;
  }
else
  {
  elon = atan2(_v[1],_v[0]);
  if (elon < 0.0) elon += 2*pi;
  }

// Trial values to start the interation
lat = asin(_v[2]/rho);
double sinlat = sin(lat);
double coslat = cos(lat);
alt = rho - r1_earth*(1.0 - flat*sinlat*sinlat);

// Iterate to solution (or maximum numer of interations)
double g0,g1,g2;
double dr,dz,dalt,dlat;
double tol = 1.0e-14;
int maxiter = 10;
int i;
for (i=1; i <= maxiter; i++)
  {
  sinlat = sin(lat);
  coslat = cos(lat);
  g0 = r1_earth/sqrt(1.0 - f2*sinlat*sinlat);
  g1 = g0 + alt;
  g2 = g0*(1-flat)*(1-flat) + alt;
  dr = r - g1*coslat;
  dz = _v[2] - g2*sinlat;
  dalt = dr*coslat + dz*sinlat;
  dlat = (dz*coslat - dr*sinlat) / (r1_earth + alt + dalt);
  lat += dlat;
  alt += dalt;
  if ((dlat < tol) && (fabs(dalt)/(r1_earth + alt) < tol)) break;
  }

if (i >= maxiter)
  {
  printf("Error: EarthPosition::get_alt_lat_lon\n");
  printf("  Did not converge to a solution for the surface point\n");
  exit(-1);
  }

if (etype == GEOCENTRIC)
  {	// convert geodetic latitude to geocentric latitude
  lat = atan(tan(lat)*(1-eccentricity_earth*eccentricity_earth));
  }

Vector3 result(alt,lat,elon);
return(result);

}
