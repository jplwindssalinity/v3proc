//==============================================================//
// Copyright (C) 1997-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_matrix3_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Matrix3.h"
#include "Constants.h"

//=========//
// Matrix3 //
//=========//

Matrix3::Matrix3()
{
    return;
}

//
// Initialize with a complete set of 9 user-specified elements.
//

Matrix3::Matrix3(
    double  x11,
    double  x12,
    double  x13,
    double  x21,
    double  x22,
    double  x23,
    double  x31,
    double  x32,
    double  x33)
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
    return;
}

//
// Initialize all 9 elements with a single value.
//

Matrix3::Matrix3(
    double  init)
{
    int i, j;
    for (i=0; i < 3; i++)
    {
        for (j=0; j < 3; j++)
        {
            _m[i][j] = init;
        }
    }
    return;
}

//
// Initialize with a special structure indicated by the enum argument.
//

Matrix3::Matrix3(matrixtypeE mtype)
{
    if (mtype == IDENTITY)
    {
        // fill matrix with 3x3 identity matrix
        int i,j;
        for (i=0; i < 3; i++)
        {
            for (j=0; j < 3; j++)
            {
                _m[i][j] = 0.0;
                if (i == j)
                    _m[i][j] = 1.0;
            }
        }
    }
    else if (mtype == GENERAL)
    {
        return;
    }
    else
    {
        printf("Error: Matrix3 object received unrecognized enum = %d\n",
            mtype);
        exit(1);
    }
    return;
}

//
// Operator + to add two 3x3 matrices together, giving a 3x3 matrix.
//

Matrix3 Matrix3::operator+(Matrix3 m2)
{
    Matrix3 result;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            result._m[i][j] = _m[i][j] + m2._m[i][j];
        }
    }
    return(result);
}

//
// Operator - to subtract two 3x3 matrices, giving a 3x3 matrix.
//

Matrix3
Matrix3::operator-(
    Matrix3  m2)
{
    Matrix3 result;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            result._m[i][j] = _m[i][j] - m2._m[i][j];
        }
    }
    return(result);
}

//
// Operator unary - to negate a 3x3 matrix, giving a 3x3 matrix.
//

Matrix3
Matrix3::operator-()
{
    Matrix3 result;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            result._m[i][j] = - _m[i][j];
        }
    }
   return(result);
}

//------------//
// operator * //
//------------//
// multiply two 3x3 matrices together, giving a 3x3 matrix.

Matrix3
Matrix3::operator*(
    Matrix3  m2)
{
    int i, j, k;
    Matrix3 result(0.0);

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            for (k = 0; k < 3; k++)
            {
                result._m[i][j] += _m[i][k] * m2._m[k][j];
            }
        }
    }
    return(result);
}

//
// Operator * to multiply a 3-vector by a 3x3 matrix, giving a 3-vector.
//

Vector3
Matrix3::operator*(
    Vector3  v2)
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

void
Matrix3::Rowset(
    Vector3  r1,
    Vector3  r2,
    Vector3  r3)
{
    int i;
    for (i=0; i < 3; i++)
    {
        _m[0][i] = r1._v[i];
        _m[1][i] = r2._v[i];
        _m[2][i] = r3._v[i];
    }
    return;
}

//-------------------//
// Matrix3::Identity //
//-------------------//
// Method identity sets the calling Matrix3 object to a 3x3 identity matrix.

void
Matrix3::Identity()
{
    int i, j;
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            _m[i][j] = 0.0;
            if (i == j)
                _m[i][j] = 1.0;
        }
    }
    return;
}

//------------------//
// Matrix3::Inverse //
//------------------//
// Method inverse sets the calling Matrix3 object to its inverse.
// The gaussj routine from Numerical Recipes is adapted for 3x3 matrices here.

int
Matrix3::Inverse()
{

#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}

int n = 3;
int indxc[3],indxr[3],ipiv[3];
int i, j, k, l, ll;
int icol = 0;
int irow = 0;
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
                }
                else if (ipiv[k] > 1)
                {
                    fprintf(stderr,
                    "Error: Matrix3 object tried to invert singular matrix\n");
                    return(0);
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
        fprintf(stderr,
            "Error: Matrix3 object tried to invert singular matrix\n");
        return(0);
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
    return(1);
}

//---------------//
// Matrix3::Show //
//---------------//

void
Matrix3::Show(
    char*  name)
{
    int i;
    if (name == NULL)
    {
        for (i = 0; i < 3; i++)
        {
            printf("[%10g %10g %10g]\n", _m[i][0], _m[i][1], _m[i][2]);
        }
    }
    else
    {
        char *str = (char *)malloc(strlen(name) + 1);
        if (str == NULL)
        {
            fprintf(stderr, "Matrix3::Show: couldn't allocate memory\n");
            exit(1);
        }
        // fill temporary string with spaces (equal to name string in length)
        for (i = 0; i < (int)strlen(name); i++)
        {
            str[i] = ' ';
        }
        str[i] = '\0';
        for (i = 0; i < 3; i++)
        {
            if (i == 0)
            {
                printf("%s = [%10g %10g %10g]\n", name, _m[i][0], _m[i][1],
                    _m[i][2]);
            }
            else
            {
                printf("%s   [%10g %10g %10g]\n", str, _m[i][0], _m[i][1],
                    _m[i][2]);
            }
       }
       free(str);
    }
    return;
}

//=========//
// Vector3 //
//=========//

Vector3::Vector3()
{
    return;
}

//
// Initialize with a complete set of 3 user-specified elements.
//

Vector3::Vector3(
    double  x1,
    double  x2,
    double  x3)
{
    _v[0] = x1;
    _v[1] = x2;
    _v[2] = x3;
    return;
}

//
// Initialize all 3 elements with a single value.
//

Vector3::Vector3(
    double  init)
{
    for (int i = 0; i < 3; i++)
    {
        _v[i] = init;
    }
    return;
}

Vector3 Vector3::operator+(Vector3 v2)
{
    Vector3 result;
    for (int i = 0; i < 3; i++)
    {
        result._v[i] = _v[i] + v2._v[i];
    }
    return(result);
}

//------------//
// operator - //
//------------//

Vector3
Vector3::operator-(
    Vector3  v2)
{
    Vector3 result;
    for (int i = 0; i < 3; i++)
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

//--------------------//
// Vector3::operator* //
//--------------------//
// Element by element multiplication of two vectors.

Vector3
Vector3::operator*(
    Vector3  v2)
{
    int i;
    Vector3 result;

    for (i=0; i < 3; i++)
    {
        result._v[i] = _v[i] * v2._v[i];
    }

    return(result);
}

//-------------------//
// Vector3:operator* //
//-------------------//
// Scalar times a vector

Vector3
Vector3::operator*(
    double    s)
{
    Vector3 result;
    for (int i = 0; i < 3; i++)
    {
        result._v[i] = _v[i] * s;
    }
    return(result);
}

//
// Scalar multiply and assign to one vector.
//

void
Vector3::operator*=(
    double    s)
{

_v[0] *= s;
_v[1] *= s;
_v[2] *= s;

}

//
// Scalar divide and assign to one vector.
//

void
Vector3::operator/=(
    double    s)
{

_v[0] /= s;
_v[1] /= s;
_v[2] /= s;

}

//
// Add and assign one vector to another.
//

void Vector3::operator+=(Vector3 v2)

{

_v[0] += v2._v[0];
_v[1] += v2._v[1];
_v[2] += v2._v[2];
return;

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

//------------//
// operator / //
//------------//

Vector3
Vector3::operator/(
    double    s)
{
    Vector3 result;
    for (int i = 0; i < 3; i++)
        result._v[i] = _v[i] / s;
    return(result);
}

//----------------//
// operator ==    //
//----------------//

int
Vector3::operator==(Vector3 m2)
{
       for(int i = 0; i < 3; i++)
           if(m2._v[i]!= _v[i]) return(0);
       return(1);
}

//---------------//
// Vector3::Read //
//---------------//

int
Vector3::Read(FILE* fptr)
{
  for (int i=0; i < 3; i++)
    if (fread((void *)&_v[i],sizeof(double),1,fptr) != 1) return(0);

  return(1);

}

//----------------//
// Vector3::Write //
//----------------//

int
Vector3::Write(FILE* fptr)
{
  for (int i=0; i < 3; i++)
    if (fwrite((void *)&_v[i],sizeof(double),1,fptr) != 1) return(0);

  return(1);

}

//----------------//
// Vector3::Scale //
//----------------//
// Scale the vector to have the specifed magnitude.

void
Vector3::Scale(
    double  r)
{
    double mag = sqrt(_v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2]);

    if (mag == 0.0)
        return;

    for (int i = 0; i < 3; i++)
    {
        _v[i] *= (r / mag);
    }
    return;
}

//--------------------//
// Vector3::Magnitude //
//--------------------//
// Get the magnitude of the vector.

double
Vector3::Magnitude()
{
    return(sqrt(_v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2]));
}

//---------------//
// Vector3::Show //
//---------------//

void
Vector3::Show(
    char*  name)
{
    if (name == NULL)
    {
        printf("(%10g, %10g, %10g)\n",_v[0],_v[1],_v[2]);
    }
    else
    {
        printf("%s = (%10g, %10g, %10g)\n",name,_v[0],_v[1],_v[2]);
    }
    return;
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
    double    r,
    double    theta,
    double    phi)
{
    double r_sin_theta = r * sin(theta);

    _v[0] = r_sin_theta * cos(phi);
    _v[1] = r_sin_theta * sin(phi);
    _v[2] = r * cos(theta);

    return(1);
}

//-----------------------//
// Vector3::SphericalGet //
//-----------------------//
// This method gets the spherical elements of the Vector3 object (which
// is stored in rectangular form).
// The standard definition of spherical coordinates is used:
//   r is the length (magnitude) of the vector
//   theta is the angle away from the no. 3 axis (z-axis)
//   phi is the angle away from the no. 1 axis (x-axis) in the 1-2 (x-y) plane.
//

int
Vector3::SphericalGet(
    double*  r,
    double*  theta,
    double*  phi)
{
    *r = sqrt(_v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2]);

    if (*r == 0.0)
    {
        *theta = 0.0;
        *phi = 0.0;
        return(1);
    }

    *theta = acos(_v[2] / *r);
    *phi = atan2(_v[1], _v[0]);
    return(1);
}

//------------------------------//
// Vector3::AzimuthElevationSet //
//------------------------------//

//
// This method sets the (rectangular) elements of a Vector3 object given the
// corresonding elements in a azimuth/elevation coordinate system.
// The definition of the azimuth/elevation coordinates is:
//   r is the length (magnitude) of the vector
//   az is the angle away from the z-axis (no. 3) of the projection
//     onto the yz plane.
//   el is the angle away from the z-axis (no. 3) of the projection
//     onto the xz plane.
//

int
Vector3::AzimuthElevationSet(
    double    r,
    double    az,
    double    el)
{
    _v[2] = r / sqrt(tan(az)*tan(az) + tan(el)*tan(el) + 1.0);
    _v[1] = _v[2]*tan(az);
    _v[0] = _v[2]*tan(el);
    return(1);
}

//------------------------------//
// Vector3::AzimuthElevationGet //
//------------------------------//

//
// This method gets the elements of a Vector3 object
// in a azimuth/elevation coordinate system.
// The definition of the azimuth/elevation coordinates is:
//   r is the length (magnitude) of the vector
//   az is the angle away from the z-axis (no. 3) of the projection
//     onto the yz plane.
//   el is the angle away from the z-axis (no. 3) of the projection
//     onto the xz plane.
//

int
Vector3::AzimuthElevationGet(
    double    *r,
    double    *az,
    double    *el)
{
    *r = sqrt(_v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2]);

    if (_v[2] == 0.0)
    {
        *az = pi/2.0;
        *el = pi/2.0;
        return(1);
    }

    *el = atan2(_v[0],_v[2]);
    *az = atan2(_v[1],_v[2]);
    return(1);
}

//--------------//
// Vector3::Set //
//--------------//

int
Vector3::Set(
    int        index,
    double    value)
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
    return;
}

//--------------//
// Vector3::Get //
//--------------//

double
Vector3::Get(
    int  idx)
{
    if (idx < 0 || idx > 2)
        return(0.0);
    return(_v[idx]);
}

int
Vector3::Get(
    int            idx,
    double*        value)
{
    if (idx < 0 || idx > 2)
        return(0);
    *value = _v[idx];
    return(1);
}

void
Vector3::Get(
    double*  x1,
    double*  x2,
    double*  x3)
{
    *x1 = _v[0];
    *x2 = _v[1];
    *x3 = _v[2];
    return;
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
