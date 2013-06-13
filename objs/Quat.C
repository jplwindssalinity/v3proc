//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_quat_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Quat.h"
#include "List.h"
#include "List.C"
#include "Vect.h"
#include "Mat.h"

//======//
// Quat //
//======//

Quat::Quat()
:   x(0.0), y(0.0), z(0.0), w(0.0)
{
    return;
}

// setting all four components
Quat::Quat(
    double  x,
    double  y,
    double  z,
    double  w)
{
    Set(x, y, z, w);
    return;
}

// specify an axis and rotation angle
Quat::Quat(
    const Vect&   axis,
    const double  angle)
{
    SetUsingAxisAndRotation(axis, angle);
    return;
}

//-------------//
// Quat::~Quat //
//-------------//

Quat::~Quat()
{
    return;
}

//-----------//
// Quat::Set //
//-----------//

void
Quat::Set(
    double  new_x,
    double  new_y,
    double  new_z,
    double  new_w)
{
    x = new_x;
    y = new_y;
    z = new_z;
    w = new_w;
    return;
}

//-----------------//
// Quat::Normalize //
//-----------------//

void
Quat::Normalize()
{
    double norm = Norm();
    if (norm == 0.0)
    {
        w = 1.0;
        x = y = z = 0.0;
    }
    else
    {
        x /= norm;
        y /= norm;
        z /= norm;
        w /= norm;
    }
    return;
}

//--------------//
// Quat::Invert //
//--------------//

void
Quat::Invert(const Quat& quat)
{
    Set(-(quat.x), -(quat.y), -(quat.z), quat.w);
    return;
}

void
Quat::Invert()
{
    x = -x;
    y = -y;
    z = -z;
    return;
}

//------------//
// Quat::Norm //
//------------//
// return the norm of the quaternion (the root sum of the component squares)

double
Quat::Norm() const
{
    return(sqrt(x*x + y*y + z*z + w*w));
}

//-------------------------------//
// Quat::SetUsingAxisAndRotation //
//-------------------------------//

void
Quat::SetUsingAxisAndRotation(
    const Vect&   axis,
    const double  angle)
{
    Vect axiscopy(axis);
    axiscopy.SetMagnitude(1.0);

    double half_angle = angle / 2.0;
    double sint = sin(half_angle);

    x = axiscopy.FastGet(0) * sint;
    y = axiscopy.FastGet(1) * sint;
    z = axiscopy.FastGet(2) * sint;
    w = cos(half_angle);

    Normalize();

    return;
}

//--------------//
// Quat::RotMat //
//--------------//
// the quat better be a unit quat!

int
Quat::RotMat(
    Mat&  matrix)
{
    if (! matrix._Allocate(3, 3)) {
        return(0);
    }

    matrix.FastSet(0, 0, 1.0 - 2.0 * (y*y + z*z));
    matrix.FastSet(0, 1, 2.0 * (x*y - w*z));
    matrix.FastSet(0, 2, 2.0 * (x*z + w*y));

    matrix.FastSet(1, 0, 2.0 * (x*y + w*z));
    matrix.FastSet(1, 1, 1.0 - 2.0 * (x*x + z*z));
    matrix.FastSet(1, 2, 2.0 * (y*z - w*x));

    matrix.FastSet(2, 0, 2.0 * (x*z - w*y));
    matrix.FastSet(2, 1, 2.0 * (y*z + w*x));
    matrix.FastSet(2, 2, 1.0 - 2.0 * (x*x + y*y));

    return(1);
}

//-----------------------//
// Quat::ApplyRotationTo //
//-----------------------//
// converts the quat to a rotation matrix and applies it to the
// passed vector, returning the result

int
Quat::ApplyRotationTo(
    const Vect&  input_vector,
    Vect*        output_vector)
{
    Mat m(3, 3);
    if (! RotMat(m)) {
        return(0);
    }
    if (! output_vector->Product(m, input_vector)) {
        return(0);
    }
    return(1);
}

/*
//-------------------------------//
// Quat::RotationFromFixedAngles //
//-------------------------------//
// computes rotation matix from fixed angles. 
// rx = gamma, ry = beta, rz = alpha in Craig
// R = Rz(rz)*Ry(ry)*Rx(rx);

void
Quat::RotationFromFixedAngles(
    double  rx,
    double  ry,
    double  rz)
{
    Mat R(3,3);
 
    R(0,0) = cos(rz)*cos(ry);
    R(1,0) = sin(rz)*cos(ry);
    R(2,0) = -sin(ry);
 
    R(0,1) = cos(rz)*sin(ry)*sin(rx)-sin(rz)*cos(rx);
    R(1,1) = sin(rz)*sin(ry)*sin(rx)+cos(rz)*cos(rx);
    R(2,1) = cos(ry)*sin(rx);
 
    R(0,2) = cos(rz)*sin(ry)*cos(rx)+sin(rz)*sin(rx);
    R(1,2) = sin(rz)*sin(ry)*cos(rx)-cos(rz)*sin(rx);
    R(2,2) = cos(ry)*cos(rx);

    this->QuatFromRotMat(R);

    return;
}

//----------------------//
// Quat::QuatFromRotMat //
//----------------------//
// determines rotation matrix corresponding to unit quaternion

void
Quat::QuatFromRotMat(
    const Mat&  R)
{
    double tmp, T = R(0,0) + R(1,1) + R(2,2);    // trace
    double maxPivot = MAX((MAX(R(0,0), R(1,1))), (MAX(R(2,2), T)));

    if (maxPivot == R(0,0))
    {
        // x is largest
        x = sqrt(0.25 * (1.0 + 2.0 * R(0,0) - T));
        tmp = 1.0 / (4.0 * x);
        y = (R(0,1) + R(1,0)) * tmp;
        z = (R(0,2) + R(2,0)) * tmp;
        w = (R(1,2) - R(2,1)) * tmp;
    }

    if (maxPivot == R(1,1))
    {
        // y is largest
        y = sqrt(0.25 * (1.0 + 2.0 * R(1,1) - T));
        tmp = 1.0 / (4.0 * y);
        x = (R(0,1) + R(1,0)) * tmp;
        z = (R(1,2) + R(2,1)) * tmp;
        w = (R(2,0) - R(0,2)) * tmp;
    }

    if (maxPivot == R(2,2))
    {
        // z is largest
        z = sqrt(0.25 * (1.0 + 2.0 * R(2,2) - T));
        tmp = 1.0 / (4.0 * z);
        x = (R(0,2) + R(2,0)) * tmp;
        y = (R(1,2) + R(2,1)) * tmp;
        w = (R(0,1) - R(1,0)) * tmp;
    }

    if (maxPivot == T)
    {
        // w is largest
        w = sqrt(0.25 * (1.0 + T));
        tmp = 1.0 / (4.0 * w);
        x = (R(1,2) - R(2,1)) * tmp;
        y = (R(2,0) - R(0,2)) * tmp;
        z = (R(0,1) - R(1,0)) * tmp;
    }

    // make rotation shortest by negating q if w < 0
    if (w < 0.0)
        *this *= -1;    // q = q * (-1)

    return;
}
*/

//------------------//
// Quat::WriteAscii //
//------------------//

void
Quat::WriteAscii(
    const char*  name,
    FILE*        ofp)
{
    if (name != NULL)
        fprintf(ofp, "Quat : %s [%g, %g, %g, %gw]\n", name, x, y, z, w);
    else
        fprintf(ofp, "Quat [%g, %g, %g, %gw]\n", x, y, z, w);
}

//------------------//
// Quat::WriteAscii //
//------------------//

void
Quat::WriteAscii(
    FILE*  ofp)
{
    WriteAscii(NULL, ofp);
    return;
}


//------------------//
// Quat::operator*= //
//------------------//
/*
void
Quat::operator*=(
    double  factor)
{
    x *= factor;
    y *= factor;
    z *= factor;
    w *= factor;
    return;
}
*/
//-----------//
// operator* //
//-----------//

Quat
operator*(
    const Quat  q,
    const Quat  p)
{
    Quat qp;
    qp.w = q.w*p.w - q.x*p.x - q.y*p.y - q.z*p.z;
    qp.x = q.w*p.x + q.x*p.w + q.y*p.z - q.z*p.y;
    qp.y = q.w*p.y + q.y*p.w + q.z*p.x - q.x*p.z;
    qp.z = q.w*p.z + q.z*p.w + q.x*p.y - q.y*p.x;
    return(qp);
}


void Quat::Power( double a ) {
    
    if( w==1 ) return;
    double cosold=w;
    double sinold=sqrt(x*x+y*y+z*z);
    double angold;
    if(cosold==0) 
      angold=M_PI/2;
    else 
      angold=atan(sinold/cosold);
    double cosnew=cos(angold*a);
    double sinnew=sin(angold*a);
    w=cosnew;
    x=x*sinnew/sinold;
    y=y*sinnew/sinold;
    z=z*sinnew/sinold;
}

QuatRec::QuatRec() {
    return;
}

QuatRec::~QuatRec() {
    return;
}

int QuatRec::Read( FILE* inputfile ) {
    
    if(!inputfile) 
      return(0);

    if (fread(&time, sizeof(double), 1, inputfile) != 1)
        return(0);
    if (fread(&w, sizeof(double), 1, inputfile) != 1)
        return(0);
    if (fread(&x, sizeof(double), 1, inputfile) != 1)
        return(0);
    if (fread(&y, sizeof(double), 1, inputfile) != 1)
        return(0);
    if (fread(&z, sizeof(double), 1, inputfile) != 1)
        return(0);
    return(1);
}

int QuatRec::Write( FILE* outputfile ) {
    if (fwrite(&time, sizeof(double), 1, outputfile) != 1)
        return(0);
    if (fwrite(&w, sizeof(double), 1, outputfile) != 1)
        return(0);
    if (fwrite(&x, sizeof(double), 1, outputfile) != 1)
        return(0);
    if (fwrite(&y, sizeof(double), 1, outputfile) != 1)
        return(0);
    if (fwrite(&z, sizeof(double), 1, outputfile) != 1)
        return(0);
    return(1);
}

QuatFile::QuatFile() {
    return;
}

QuatFile::QuatFile(const char* filename) {
    SetInputFile(filename);
    SetMaxNodes(1000000);
    return;
}

QuatFile::~QuatFile() {
    CloseInputFile();
    return;
}

int QuatFile::_GetBracketingQuatRecs( double time, QuatRec** quat1, QuatRec** quat2 ) {
    // Shamelessly copied from Ephemeris object by AGF
    // make sure there is data in the list (if there is any at all)
    QuatRec* current_quat = GetCurrent();
    if (current_quat == NULL)
        current_quat = GetOrReadNext();

    if (current_quat == NULL) return(0);

    // search forward
    while (current_quat && current_quat->time < time)
        current_quat = GetOrReadNext();

    if (current_quat == NULL) return(0);

    // search backward
    while (current_quat && current_quat->time > time)
        current_quat = GetPrev();

    if (current_quat == NULL) return(0);

    QuatRec* next_quat = GetOrReadNext();

    // check
    if (next_quat == NULL) return(0);

    // set quats
    *quat1 = current_quat;
    *quat2 = next_quat;
    return(1);
}

int QuatFile::GetQuat( double time, Quat* quat ) {
    // Obtain bracketing Quats from QuatFile
    QuatRec* quat1;
    QuatRec* quat2;
    
    if( _GetBracketingQuatRecs( time, &quat1, &quat2 ) == 0 ) {
      fprintf(stderr,"Error: Can't find requested time %18.12g in QuatFile\n",time);
      return(0);
    }
    
    // Make sure they are normalized
    quat1->Normalize();
    quat2->Normalize();
    
    // multiply quat2 by quat1^-1
    Quat quat1_inv;
    quat1_inv.Set( quat1->x, quat1->y, quat1->z, quat1->w );
    quat1_inv.Invert();
    
    Quat qtrans = (*quat2) * quat1_inv;
    qtrans.Normalize();
    
    // Fix sign of quaternion if not right
    if( qtrans.w < 0 ) {
      qtrans.w *= -1; qtrans.x *= -1;
      qtrans.y *= -1; qtrans.z *= -1;
    }
    
    // Interpolation step for t in [t1,t2] interval (t-t1)/(t2-1)
    double alpha = (time-quat1->time)/(quat2->time-quat1->time);
    
    // Compute qtrans^alpha
    qtrans.Power( alpha );
    
    // Compute interpolated quaternion.
    *quat = qtrans * (*quat1);
    
    // Fix sign of quaternion if not right
    if( quat->w < 0 ) {
      quat->w *= -1; quat->x *= -1;
      quat->y *= -1; quat->z *= -1;
    }
    return(1);
}
