//==============================================================//
// Copyright (C) 1997-2003, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef MATRIX3_H
#define MATRIX3_H

static const char rcs_id_matrix3_h[] =
    "@(#) $Id$";

#include <stdio.h>

//======================================================================
// CLASSES
//    Matrix3, Vector3
//======================================================================

class Vector3;

//======================================================================
// CLASS
//    Matrix3
//
// DESCRIPTION
//    The Matrix3 object contains a 3 by 3 matrix of doubles and
//    methods to perform basic matrix operations.
//======================================================================

class Matrix3
{
    friend class Vector3;

public:

    enum matrixtypeE {GENERAL, IDENTITY};

    //--------------//
    // construction //
    //--------------//

    Matrix3();
    Matrix3(double x11, double x12, double x13,
    double x21, double x22, double x23,
    double x31, double x32, double x33);
    Matrix3(double init);
    Matrix3(matrixtypeE mtype);

    //-------------------------//
    // Matrix/Vector operators //
    //-------------------------//

    Matrix3 operator+(Matrix3 m2);
    Matrix3 operator-(const Matrix3 m2) const;
    Matrix3 operator-() const;
    Matrix3 operator*(const Matrix3 m2) const;
    Vector3 operator*(Vector3 v2);

    //
    // Other matrix methods
    //

    void Rowset(Vector3 r1, Vector3 r2, Vector3 r3);
    void Identity();
    int  Inverse();
    void Show(char *name = (char*)NULL);

protected:

    //-----------//
    // variables //
    //-----------//

    double _m[3][3];
};

//======================================================================
// CLASS
//    Vector3
//
// DESCRIPTION
//    The Vector3 object contains a 3 element array of doubles and
//    methods to perform basic matrix/vector operations.
//======================================================================

#define ZERO_VECTOR  Vector3(0.0)

class Vector3
{
    friend class Matrix3;

public:

    //--------------//
    // construction //
    //--------------//

    Vector3();
    Vector3(double x1, double x2, double x3);
    Vector3(double init);

    //-------------------------//
    // Vector/Matrix operators //
    //-------------------------//

    Vector3  operator+(Vector3 m2);
    Vector3  operator-(Vector3 m2);
    Vector3  operator-();
    Vector3  operator*(Vector3 m2);    // element by element multiply
    Vector3  operator*(double s);    // multiply by scalar
    void     operator*=(double s);
    void     operator/=(double s);
    void     operator+=(Vector3 m2);
    Vector3  operator&(Vector3 m2);    // cross product
    double   operator%(Vector3 m2);    // dot product
    Vector3  operator/(double s);    // divide by scalar
    int      operator==(Vector3 m2);

    //-----//
    // I/O //
    //-----//

    int Read(FILE* fptr);    // Binary read
    int Write(FILE* fptr);   // Binary write

    //
    // Other access methods
    //

    void    Scale(double r);        // set magnitude
    double  Magnitude();        // get vector magnitude
    void    Show(char *name = (char*)NULL);

    int     SphericalSet(double r, double theta, double phi);
    int     SphericalGet(double *r, double *theta, double *phi);
    int     AzimuthElevationSet(double r, double az, double el);
    int     AzimuthElevationGet(double *r, double *az, double *el);
    int     Set(int index, double value);
    void    Set(double x1, double x2, double x3);
    void    Get(double* x1, double* x2, double* x3);
    double  Get(int idx);
    int     Get(int idx, double* value);
    void    Zero();

protected:

//-----------//
// variables //
//-----------//

    double _v[3];
};

#endif
