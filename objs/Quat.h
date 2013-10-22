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

#include "Matrix3.h"
#include "BufferedList.h"
#include "Attitude.h"

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
    Quat(const Vector3& axis, const double angle);
    ~Quat();

    void  Set(double x, double y, double z, double w);

    //---------------//
    // manipulations //
    //---------------//

    void  Normalize();
    void  Invert(const Quat& quat);
    void  Invert();

    double  Norm() const;
    void    SetUsingAxisAndRotation(const Vector3& axis,
                const double angle);
    int     RotMat(Matrix3* mat);
    int     ApplyRotationTo(const Vector3& vector, Vector3* result);
    void    Scale(double factor);
    void    Product(const Quat& p, const Quat& q);
    void    Power( double a );
    void    GetAttitude( Attitude* attitude );
    void    GetAttitudeGS( Attitude* attitude );
    
    void QuatFromMatrix( const Matrix3& matrix );
    
    Quat operator*(Quat q2);
    void operator*=(double factor);
    
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

//protected:
    double  x;
    double  y;
    double  z;
    double  w;    // the scalar
};


// QuatRec class is Quat + time-tag and I/O
class QuatRec : public Quat
{
public:
    //--------------//
    // construction //
    //--------------//
    QuatRec();
    QuatRec( Quat quat, double t );
    ~QuatRec();
    
    int   Read(FILE* inputfile);
    int   Write(FILE* outputfile );
    
//protected:
    double time;
};

// QuatFile is to QuatRec as Ephermeris is to OrbitState
class QuatFile : public BufferedList<QuatRec>
{
public:

    //--------------//
    // construction //
    //--------------//

    QuatFile();
    QuatFile(const char* filename, unsigned int max_states );
    ~QuatFile();
    
    // Interpolation
    
    int GetQuat( double time, Quat* quat );
    
protected:

    int _GetBracketingQuatRecs( double time, QuatRec** quat1, QuatRec** quat2 );
    
    //-----------//
    // variables //
    //-----------//

};
#endif
