//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef TRANSFORM_H
#define TRANSFORM_H

static const char rcs_id_transform_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include "Vect.h"
#include "Mat.h"

//======================================================================
// CLASS
//    Transform
//
// DESCRIPTION
//    A coordinate transformation.
//======================================================================

class Transform
{
public:

    Transform();
    Transform(const Vect& x_axis, const Vect& y_axis, const Vect& z_axis,
        const Vect& origin);

    int  WriteAscii(FILE* ofp = stdout);

    //----------------//
    // initialization //
    //----------------//

    int   SetOrigin(const Vect& origin);
    void  SetAxes(const Vect& x_axis, const Vect& y_axis, const Vect& z_axis);

    //------------//
    // operations //
    //------------//

    int  Rotate(int axis_index, double radians);
    int  Inverse(const Transform& trans);
    int  Apply(const Transform& trans);
    int  Apply(const Vect& v_in, Vect* v_out);
    int  GetEuler213(double att[3]);

protected:

    Mat   _rotmat;
    Vect  _origin;
};

#endif
