//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef MATRIX3_H
#define MATRIX3_H

static const char rcs_id_matrix3_h[] =
	"@(#) $Id$";

enum matrixtypeE {GENERAL, IDENTITY};

//======================================================================
// CLASSES
//		Matrix3
//		Vector3
//======================================================================

class Matrix3;
class Vector3;

//======================================================================
// CLASS
//		Matrix3
//
// DESCRIPTION
//		The Matrix3 object contains a 3 by 3 matrix of doubles
//		and methods to perform basic matrix operations.
//======================================================================

class Matrix3
{
friend Vector3;
public:

//--------------//
// construction //
//--------------//

Matrix3(double x11, double x12, double x13,
double x21, double x22, double x23,
double x31, double x32, double x33);
Matrix3(double init);
Matrix3(matrixtypeE mtype);
Matrix3();
~Matrix3();


//-------------------------//
// Matrix/Vector operators //
//-------------------------//

Matrix3 operator+(Matrix3 m2);
Matrix3 operator-(Matrix3 m2);
Matrix3 operator-();
Matrix3 operator*(Matrix3 m2);
Vector3 operator*(Vector3 v2);

//
// Other matrix methods
//

void rowset(Vector3 r1, Vector3 r2, Vector3 r3);
void identity();
void inverse();
void show();

private:

//-----------//
// variables //
//-----------//

double _m[3][3];
};

//======================================================================
// CLASS
//		Vector3
//
// DESCRIPTION
//		The Vector3 object contains a 3 element array of doubles
//		and methods to perform basic matrix/vector operations.
//======================================================================

class Vector3
{
friend Matrix3;
public:

//--------------//
// construction //
//--------------//

Vector3(double x1, double x2, double x3);
Vector3(double init);
Vector3();
~Vector3();


//-------------------------//
// Vector/Matrix operators //
//-------------------------//

Vector3 operator+(Vector3 m2);
Vector3 operator-(Vector3 m2);
Vector3 operator-();
Vector3 operator*(Vector3 m2); // element by element multiply
Vector3 operator&(Vector3 m2); // cross product

//
// Other access methods
//

void scale(double r);	// set magnitude
double get(int i);	// extract one element
void show();

	int		Set(int index, double value);

private:

//-----------//
// variables //
//-----------//

double _v[3];
};
#endif
