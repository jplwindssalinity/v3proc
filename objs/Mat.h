//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef MAT_H
#define MAT_H

static const char rcs_id_mat_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include "Vect.h"

//======================================================================
// CLASS
//    Mat
//
// DESCRIPTION
//    A Matrix.
//======================================================================

class Mat
{

    friend class Vect;
public:

    Mat();
    Mat(const Mat& mat);
    Mat(int m_size, int n_size);
    Mat(int m_size, int n_size, double fill);

    int  WriteAscii(FILE* ofp = stdout);

    //------------//
    // initialize //
    //------------//

    int   Set(const Mat& mat);
    void  Fill(double fill_value);
    int   Identity(int size);
    void  Identity();

    //--------//
    // access //
    //--------//

    double  FastGet(int m_idx, int n_idx)
                { return(_m[m_idx][n_idx]); };
    void    FastSet(int m_idx, int n_idx, double value)
                { _m[m_idx][n_idx] = value; };

    //------------//
    // operations //
    //------------//

    int  Inverse(const Mat& a);
    int  Transpose(const Mat& a);
    int  Product(double factor, const Mat& a);
    int  Product(const Mat& a, const Mat& b);
    int  Premultiply(const Mat& a);
    int  SVD(Mat* u, Vect* w, Mat* v) const;

protected:

    int   _Allocate(const Mat& a);
    int   _Allocate(int m_size, int n_size);
    void  _Free();

    int       _mSize;
    int       _nSize;
    double**  _m;
};

#endif
