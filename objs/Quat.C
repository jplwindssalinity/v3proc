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
#include "Matrix3.h"
#include "List.h"
#include "Attitude.h"

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
    const Vector3& axis,
    const double   angle)
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
    const Vector3& axis,
    const double   angle)
{
    Vector3 axiscopy(axis);
    axiscopy /= axiscopy.Magnitude();

    double half_angle = angle / 2.0;
    double sint = sin(half_angle);

    x = axiscopy.Get(0) * sint;
    y = axiscopy.Get(1) * sint;
    z = axiscopy.Get(2) * sint;
    w = cos(half_angle);

    Normalize();

    return;
}

//--------------//
// Quat::RotMat //
//--------------//
// the quat better be a unit quat!

int
Quat::RotMat( Matrix3* mat ) const {
    double x11 = 1.0 - 2.0 * (y*y + z*z);
    double x12 = 2.0 * (x*y - w*z);
    double x13 = 2.0 * (x*z + w*y);
    
    double x21 = 2.0 * (x*y + w*z);
    double x22 = 1.0 - 2.0 * (x*x + z*z);
    double x23 = 2.0 * (y*z - w*x);
    
    double x31 = 2.0 * (x*z - w*y);
    double x32 = 2.0 * (y*z + w*x);
    double x33 = 1.0 - 2.0 * (x*x + y*y);
    
    mat->Set( x11, x12, x13, x21, x22, x23, x31, x32, x33 );
    return(1);
}

//-----------------------//
// Quat::ApplyRotationTo //
//-----------------------//
// converts the quat to a rotation matrix and applies it to the
// passed vector, returning the result

int
Quat::ApplyRotationTo(
    const Vector3&  input_vector,
    Vector3*        output_vector) const {
    Matrix3 m;
    RotMat(&m);
    *output_vector = m * input_vector;
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
*/

void Quat::QuatFromMatrix( const Matrix3& mat ) {
    
    double R00 = mat.Get( 0, 0 );
    double R01 = mat.Get( 0, 1 );
    double R02 = mat.Get( 0, 2 );

    double R10 = mat.Get( 1, 0 );
    double R11 = mat.Get( 1, 1 );
    double R12 = mat.Get( 1, 2 );

    double R20 = mat.Get( 2, 0 );
    double R21 = mat.Get( 2, 1 );
    double R22 = mat.Get( 2, 2 );
    
    double tmp, T = R00 + R11 + R22;    // trace
    double maxPivot = fmax((fmax(R00, R11)), (fmax(R22, T)));

    if (maxPivot == R00)
    {
        // x is largest
        x = sqrt(0.25 * (1.0 + 2.0 * R00 - T));
        tmp = 1.0 / (4.0 * x);
        y = (R01 + R10) * tmp;
        z = (R02 + R20) * tmp;
        w = (R12 - R21) * tmp;
    }

    if (maxPivot == R11)
    {
        // y is largest
        y = sqrt(0.25 * (1.0 + 2.0 * R11 - T));
        tmp = 1.0 / (4.0 * y);
        x = (R01 + R10) * tmp;
        z = (R12 + R21) * tmp;
        w = (R20 - R02) * tmp;
    }

    if (maxPivot == R22)
    {
        // z is largest
        z = sqrt(0.25 * (1.0 + 2.0 * R22 - T));
        tmp = 1.0 / (4.0 * z);
        x = (R02 + R20) * tmp;
        y = (R12 + R21) * tmp;
        w = (R01 - R10) * tmp;
    }

    if (maxPivot == T)
    {
        // w is largest
        w = sqrt(0.25 * (1.0 + T));
        tmp = 1.0 / (4.0 * w);
        x = (R12 - R21) * tmp;
        y = (R20 - R02) * tmp;
        z = (R01 - R10) * tmp;
    }

    // make rotation shortest by negating q if w < 0
    if (w < 0.0)
        *this *= -1;    // q = q * (-1)

    return;
}

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

//-----------//
// operator* //
//-----------//
Quat 
Quat::operator*(Quat q2) 
{
    Quat result;
    result.w = w*q2.w - x*q2.x - y*q2.y - z*q2.z;
    result.x = w*q2.x + x*q2.w + y*q2.z - z*q2.y;
    result.y = w*q2.y + y*q2.w + z*q2.x - x*q2.z;
    result.z = w*q2.z + z*q2.w + x*q2.y - y*q2.x;
    return(result);
}

void 
Quat::Power( double a ) 
{    
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

void Quat::GetAttitude( Attitude* attitude ) const {
    float phi   = atan2( 2*(w*x+y*z), 1-2*(x*x+y*y));
    float theta = asin(  2*(w*y-z*x));
    float psi   = atan2( 2*(w*z+x*y), 1-2*(y*y+z*z));
    // Yaw, Pitch, Roll (ZYX) order of rotations
    attitude->Set( phi, theta, psi, 3, 2, 1 );
}

void Quat::GetAttitudeGS( Attitude* attitude ) const {
    
    Matrix3 rot_mat;
    RotMat(&rot_mat);
    
    double r11 = rot_mat.Get(0,0);
    double r12 = rot_mat.Get(0,1);
    double r13 = rot_mat.Get(0,2);

    double r21 = rot_mat.Get(1,0);
    double r22 = rot_mat.Get(1,1);
    double r23 = rot_mat.Get(1,2);

    double r31 = rot_mat.Get(2,0);
    double r32 = rot_mat.Get(2,1);
    double r33 = rot_mat.Get(2,2);
    
    // Two estimators for the cosine of roll angle
    double cr1 = sqrt( r21*r21 + r22*r22 );
    double cr2 = sqrt( r13*r13 + r33*r33 );
    double sr  = -r23;
    
    double roll = atan2( sr, cr1 );
    
    double pitch, yaw;
    
    // Check if degenerate (roll = +/- 90 ).
    // Three different tests are mathematically the same,
    // but may differ due to round-off.
    int degen = 0; 
    if( cr1==0 || cr2==0 || fabs(sr)==1 ) degen = 1;
    
    if(degen) {
      yaw   = 0;
      pitch = atan2( r12/sr, r11 );
    } else {
      yaw   = atan2(r21,r22);
      pitch = atan2(r13,r33);
    }
    // Force GS order of rotations: [ Y R P ]
    attitude->Set( (float)roll, (float)pitch, (float)yaw, 3, 1, 2 );
}


QuatRec::QuatRec() {
    return;
}

QuatRec::QuatRec( Quat quat, double t ) {
    w = quat.w;
    x = quat.x;
    y = quat.y;
    z = quat.z;
    time = t;
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

QuatFile::QuatFile(const char* filename,unsigned int  max_states) {
    SetInputFile(filename);
    SetMaxNodes(max_states);
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
