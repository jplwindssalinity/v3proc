//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_vect_c[] =
    "@(#) $Id$";

#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "Vect.h"
#include "Mat.h"

//======//
// Vect //
//======//

Vect::Vect()
:   _mSize(0), _v(NULL)
{
    return;
}

Vect::Vect(
    double  v0,
    double  v1,
    double  v2)
:   _mSize(0), _v(NULL)
{
    if (! _Allocate(3)) {
        return;
    }
    _v[0] = v0;
    _v[1] = v1;
    _v[2] = v2;
    return;
}

// Creates a Vect of the specified size.
Vect::Vect(
    int  size)
:   _mSize(0), _v(NULL)
{
    _Allocate(size);
    return;
}

Vect::Vect(
    int      size,
    double*  elements)
:   _mSize(0), _v(NULL)
{
    if (! _Allocate(size)) {
        return;
    }
    for (int i = 0; i < size; i++) {
        _v[i] = elements[i];
    }
    return;
}

// Creates a Vect and scales it to have the target
// magnitude. This is a nice way to generate unit vectors.
Vect::Vect(
    const Vect&  vect,
    double       magnitude)
:   _mSize(0), _v(NULL)
{
    Set(vect);
    SetMagnitude(magnitude);
    return;
}

Vect::~Vect()
{
    if (_v != NULL) {
        free(_v);
    }
    return;
}

//------------------//
// Vect::WriteAscii //
//------------------//

int
Vect::WriteAscii(
    FILE*  ofp)
const
{
    if (_mSize == 3) {
        fprintf(ofp, "%g %g %g\n", _v[0], _v[1], _v[2]);
    } else {
        fprintf(ofp, "Size = %d\n", _mSize);
        for (int i = 0; i < _mSize; i++)
        {
            fprintf(ofp, " %9g", _v[i]);
        }
        fprintf(ofp, "\n");
    }
    return(1);
}

//-----------//
// Vect::Set //
//-----------//

int
Vect::Set(
    double  v0,
    double  v1,
    double  v2)
{
    if (! _Allocate(3)) {
        return(0);
    }
    _v[0] = v0;
    _v[1] = v1;
    _v[2] = v2;
    return(1);
}

//-----------//
// Vect::Set //
//-----------//

int
Vect::Set(
    const Vect&  vect)
{
    int size = vect._mSize;
    if (! _Allocate(size)) {
        return(0);
    }
    for (int i = 0; i < _mSize; i++) {
        _v[i] = vect._v[i];
    }
    return(1);
}

//------------//
// Vect::Zero //
//------------//

int
Vect::Zero(
    int  size)
{
    if (! _Allocate(size)) {
        return(0);
    }
    Fill(0.0);
    return(1);
}

//------------//
// Vect::Fill //
//------------//

void
Vect::Fill(
    double  value)
{
    for (int i = 0; i < _mSize; i++) {
        _v[i] = value;
    }
    return;
}

//-----------------//
// Vect::Magnitude //
//-----------------//

double
Vect::Magnitude() const
{
    double mag = 0.0;
    for (int i = 0; i < _mSize; i++) {
        mag += (_v[i] * _v[i]);
    }
    return(sqrt(mag));
}

//----------------//
// Vect::Multiply //
//----------------//

void
Vect::Multiply(
    double  factor)
{
    for (int i = 0; i < _mSize; i++) {
        _v[i] *= factor;
    }
    return;
}

//--------------------//
// Vect::SetMagnitude //
//--------------------//

int
Vect::SetMagnitude(
    double  magnitude)
{
    double current_magnitude = Magnitude();
    if (current_magnitude == 0.0) {
        return(0);
    }
    double scale_factor = magnitude / current_magnitude;
    Multiply(scale_factor);
    return(1);
}

//-----------//
// Vect::Add //
//-----------//

int
Vect::Add(
    const Vect&  a)
{
    if (a._mSize != _mSize) {
        return(0);
    }
    for (int i = 0; i < _mSize; i++) {
        _v[i] += a._v[i];
    }
    return(1);
}

//------------------//
// Vect::Difference //
//------------------//
// Sets this Vect to be a - b.

int
Vect::Difference(
    const Vect&  a,
    const Vect&  b)
{
    if (a._mSize != b._mSize) {
        return(0);
    }
    if (! _Allocate(a._mSize)) {
        return(0);
    }
    for (int i = 0; i < _mSize; i++) {
        _v[i] = a._v[i] - b._v[i];
    }
    return(1);
}

//-------------//
// Vect::Cross //
//-------------//
// Sets this Vect to be a x b.
// For now, a and b have to have size = 3.

int
Vect::Cross(
    const Vect&  a,
    const Vect&  b)
{
    if (a._mSize != 3 || b._mSize != 3) {
        return(0);
    }
    if (! _Allocate(a._mSize)) {
        return(0);
    }
    _v[0] = a._v[1]*b._v[2] - a._v[2]*b._v[1];
    _v[1] = a._v[2]*b._v[0] - a._v[0]*b._v[2];
    _v[2] = a._v[0]*b._v[1] - a._v[1]*b._v[0];
    return(1);
}

//-----------//
// Vect::Dot //
//-----------//
// Returns the dot product of this Vect and b.

double
Vect::Dot(
    const Vect&  b)
{
    if (_mSize != b._mSize) {
        fprintf(stderr, "Vect::Dot: Vect dimension mismatch (%d != %d)\n",
            _mSize, b._mSize);
        exit(1);
    }
    double sum = 0.0;
    for (int i = 0; i < _mSize; i++) {
        sum += _v[i] * b._v[i];
    }
    return sum;
}

//---------------//
// Vect::Product //
//---------------//

int
Vect::Product(
    const Mat&   mat,
    const Vect&  vect)
{
    if (mat._nSize != vect._mSize) {
        return(0);
    }

    if (! _Allocate(mat._mSize)) {
        return(0);
    }

    double** am = mat._m;
    double* bv = vect._v;

    for (int i = 0; i < _mSize; i++) {
        _v[i] = 0.0;
        for (int j = 0; j < mat._nSize; j++) {
            _v[i] += am[i][j] * bv[j];
        }
    }
    return(1);
}

//-----------------//
// Vect::Decompose //
//-----------------//
// Decomposes the given point into an s_coef and a t_coef s.t.
//   point = A + s_coef * AB + t_coef * AC
// For details, see...
// http://geometryalgorithms.com/Archive/algorithm_0105/algorithm_0105.htm

int
Vect::Decompose(
    const Vect&  a,
    const Vect&  b,
    const Vect&  c,
    double*      s_coef,
    double*      t_coef) const
{
    Vect u, v, w;
    u.Difference(b, a);
    v.Difference(c, a);
    w.Difference(*this, a);

    double uv = u.Dot(v);
    double uu = u.Dot(u);
    double vv = v.Dot(v);
    double wv = w.Dot(v);
    double wu = w.Dot(u);

    double denom = uv * uv - uu * vv;
    if (denom == 0.0) {
        return(0);
    }

    *s_coef = (uv * wv - vv * wu) / denom;
    *t_coef = (uv * wu - uu * wv) / denom;
    return(1);
}

//-----------------//
// Vect::_Allocate //
//-----------------//

int
Vect::_Allocate(
    int  size)
{
    if (size == _mSize) {
        return(1);    // already have the space
    }
    _v = (double *)realloc(_v, size * sizeof(double));
    if (_v == NULL) {
        _mSize = 0;
        return(0);
    } else {
        _mSize = size;
        return(1);
    }
}
