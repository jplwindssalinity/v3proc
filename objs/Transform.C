//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_transform_c[] =
    "@(#) $Id$";

#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include "Transform.h"

//===========//
// Transform //
//===========//

Transform::Transform()
{
    _rotmat.Identity(3);
    _origin.Zero(3);
    return;
}

Transform::Transform(
    const Vect&  x_axis,
    const Vect&  y_axis,
    const Vect&  z_axis,
    const Vect&  origin)
:   _rotmat(3, 3), _origin(3)
{
    SetAxes(x_axis, y_axis, z_axis);
    SetOrigin(origin);
    return;
}

//-----------------------//
// Transform::WriteAscii //
//-----------------------//

int
Transform::WriteAscii(
    FILE*  ofp)
{
    _rotmat.WriteAscii(ofp);
    _origin.WriteAscii(ofp);
    return(1);
}

//----------------------//
// Transform::SetOrigin //
//----------------------//

int
Transform::SetOrigin(
    const Vect&  origin)
{
    return(_origin.Set(origin));
}

//--------------------//
// Transform::SetAxes //
//--------------------//

void
Transform::SetAxes(
    const Vect&  x_axis,
    const Vect&  y_axis,
    const Vect&  z_axis)
{
    Vect x_unit(x_axis, 1.0);
    Vect y_unit(y_axis, 1.0);
    Vect z_unit(z_axis, 1.0);

    for (int i = 0; i < 3; i++) {
        _rotmat.FastSet(0, i, x_unit.FastGet(i));
        _rotmat.FastSet(1, i, y_unit.FastGet(i));
        _rotmat.FastSet(2, i, z_unit.FastGet(i));
    }
    return;
}

//-------------------//
// Transform::Rotate //
//-------------------//

int
Transform::Rotate(
    int     axis_index,
    double  radians)
{
    // create the rotational matrix
    Mat rot(3, 3, 0.0);

    // define the indices (relative to the axis index)
    int one_xy = axis_index;
    int sx_c1x_c1y_nsy = (axis_index + 1) % 3;
    int sy_c2x_c2y_nsx = (axis_index + 2) % 3;

    // one
    rot.FastSet(one_xy, one_xy, 1.0);

    // sine
    double sinval = sin(radians);
    rot.FastSet(sx_c1x_c1y_nsy, sy_c2x_c2y_nsx, sinval);

    // cosine
    double cosval = cos(radians);
    rot.FastSet(sx_c1x_c1y_nsy, sx_c1x_c1y_nsy, cosval);
    rot.FastSet(sy_c2x_c2y_nsx, sy_c2x_c2y_nsx, cosval);

    // minus sine
    rot.FastSet(sy_c2x_c2y_nsx, sx_c1x_c1y_nsy, -sinval);

    // update the transform by premultiplying
    Mat product;
    if (! product.Product(rot, _rotmat)) {
        return(0);
    }
    _rotmat.Set(product);
    return(1);
}

//--------------------//
// Transform::Inverse //
//--------------------//
// Sets this transform to be the inverse of the passed transform.

int
Transform::Inverse(
    const Transform&  trans)
{
    if (! _rotmat.Inverse(trans._rotmat)) {
        return(0);
    }
    Mat neg_rotmat;
    neg_rotmat.Product(-1.0, trans._rotmat);
    if (! _origin.Product(neg_rotmat, trans._origin)) {
        return(0);
    }
    return(1);
}

//------------------//
// Transform::Apply //
//------------------//
// Applies the given Transform to this transform. Since these
// are represented as matrix multiplications, this is a premultiply.

int
Transform::Apply(
    const Transform&  trans)
{
    // remember the rotational matrix
    Mat A(_rotmat);

    // form the product
    if (! _rotmat.Product(trans._rotmat, A)) {
        return(0);
    }

    // get the inverse of the rotational matrix
    Mat Ainv;
    if (! Ainv.Inverse(A)) {
        return(0);
    }

    // correct the origin
    Vect shift;
    if (! shift.Product(Ainv, trans._origin)) {
        return(0);
    }
    if (! _origin.Add(shift)) {
        return(0);
    }

    return(1);
}

//------------------//
// Transform::Apply //
//------------------//

int
Transform::Apply(
    const Vect&  v_in,
    Vect*        v_out)
{
    Vect tmp;
    tmp.Difference(v_in, _origin);
    v_out->Product(_rotmat, tmp);
    return(1);
}

//------------------------//
// Transform::GetEuler213 //
//------------------------//

int
Transform::GetEuler213(
    double  att[3])
{
    // roll
    att[0] = -asin(_rotmat.FastGet(2, 0));

    // pitch
    att[1] = asin(_rotmat.FastGet(2, 1) / cos(att[0]));

    // yaw
    att[2] = asin(_rotmat.FastGet(1, 0) / cos(att[0]));

    return(1);
}
