//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//======================================================================
// CLASSES
//    Quat
//======================================================================

#ifndef QUAT_H
#define QUAT_H

static const char rcs_id_quat_h[] =
    "@(#) $Id$";

#include "Vect.h"
#include "Mat.h"

//======================================================================
// CLASS
//    Quat
//
// DESCRIPTION
//    The Quat object represents a quaternion.
//======================================================================

class Quat
{
public:

    //--------------//
    // construction //
    //--------------//

    Quat();
    Quat(double x, double y, double z, double w);
    Quat(const Vect& axis, const double angle);
    ~Quat();

    void  Set(double x, double y, double z, double w);

    //---------------//
    // manipulations //
    //---------------//

    void  Normalize();
    void  Invert(const Quat& quat);
    void  Invert();

    double  Norm() const;
    void    SetUsingAxisAndRotation(const Vect& axis,
                const double angle);
    int     RotMat(Mat& mat);
    int     ApplyRotationTo(const Vect& vector, Vect* result);
    void    Scale(double factor);
    void    Product(const Quat& p, const Quat& q);

/*
    void RotationFromFixedAngles(double rx, double ry, double rz);
    void QuatFromRotMat(const Matrix& R);
*/

    //--------------//
    // input/output //
    //--------------//

    void  WriteAscii(const char* name, FILE* ofp = stdout);
    void  WriteAscii(FILE* ofp = stdout);

    //----------------//
    // the components //
    //----------------//

protected:
    double  x;
    double  y;
    double  z;
    double  w;    // the scalar
};

#endif
