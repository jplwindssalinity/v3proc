//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef VECT_H
#define VECT_H

static const char rcs_id_vect_h[] =
    "@(#) $Id$";

#include <stdio.h>

//======================================================================
// CLASS
//    Vect
//
// DESCRIPTION
//    A Vector.
//======================================================================

class Vect
{
    friend class Mat;

public:
    Vect();
    Vect(double v0, double v1, double v2);
    Vect(int size);
    Vect(int size, double* elements);
    Vect(const Vect& vect, double magnitude);

    ~Vect();

    int   WriteAscii(FILE* ofp = stdout) const;

    //----------------//
    // initialization //
    //----------------//

    int   Set(double v0, double v1, double v2);
    int   Set(const Vect& vect);
    int   Zero(int size);
    void  Fill(double value);

    //--------//
    // access //
    //--------//

    int     GetSize() const { return _mSize; };
    double  FastGet(int index) const { return(_v[index]); };

    //-------------//
    // information //
    //-------------//

    double  Magnitude() const;

    //------------//
    // operations //
    //------------//

    void  Multiply(double factor);
    int   SetMagnitude(double magnitude);
    int   Add(const Vect& a);
    int   Difference(const Vect& a, const Vect& b);
    int   Cross(const Vect& a, const Vect& b);
    int   Product(const Mat& mat, const Vect& vect);

protected:

    int  _Allocate(int size);

    int      _mSize;
    double*  _v;
};

#endif
