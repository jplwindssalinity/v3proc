//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef MATRIX3_H
#define MATRIX3_H

static const char rcs_id_matrix3_h[] =
	"@(#) $Id$";

//======================================================================
// CLASSES
//		Matrix3
//		Vector3
//		EarthPosition
//======================================================================

class Matrix3;
class Vector3;
class EarthPosition;

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

enum matrixtypeE {GENERAL, IDENTITY};

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
void show(char *name = NULL);

protected:

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
Vector3 operator*(double s);   // multiply by scalar
Vector3 operator&(Vector3 m2); // cross product

//
// Other access methods
//

void scale(double r);		// set magnitude
double Vector3::Magnitude();	// get vector magnitude
double get(int i);		// extract one element
void show(char *name = NULL);

	int		SphericalSet(double r, double theta, double phi);
	int		Set(int index, double value);
	int		Get(int index, double* value);

protected:

//-----------//
// variables //
//-----------//

double _v[3];
};

//======================================================================
// CLASS
//		EarthPosition
//
// DESCRIPTION
//		The EarthPosition object is derived from the Vector3 object.
//		It contains a 3-element vector which specifies a position in
//		a coordinate system fixed to the earth's surface
//		(ie., rotating), with its origin at the center of the earth.
//		In addition to the inherited vector methods, other methods
//		for handling geodetic and geocentric latitude, longitude,
//		and altitude are provided.
//		The position vector is always stored as three rectangular
//		coordinates, however, different construction and access
//		methods allow the use of latitude, longitude, and altitude.
//======================================================================

class EarthPosition : public Vector3
{
public:

enum earthposition_typeE {RECTANGULAR, GEOCENTRIC, GEODETIC};

//
// Additional construction methods
//

EarthPosition(double x1, double x2, double x3, earthposition_typeE etype);
EarthPosition(double x1, double x2, earthposition_typeE etype);
EarthPosition(Vector3 v);
EarthPosition();
~EarthPosition();

//
// Operators
//

void operator=(Vector3 vec);	// assign Vector3 to EarthPosition

//
// Other access methods
//

// lat,lon access
Vector3 Get_alt_lat_lon(earthposition_typeE etype);
//double get_alt();	// extract the height above the earth's surface
//double get_dlat();	// extract the geodetic latitude
//double get_clat();	// extract the geocentric latitude
//double get_elon();	// extract the east longitude
//Vector3 normal(EarthPosition rground);	// get unit surface normal vector
//void show_latlon();	// print as dlat,elon,alt triplet

};
#endif
