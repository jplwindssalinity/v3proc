//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_earthposition_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "EarthPosition.h"
#include "Constants.h"
#include "LonLat.h"
#include "CoordinateSwitch.h"
#include "Matrix3.h"

//===============//
// EarthPosition //
//===============//

//
// Default constructor, no initialization
//

EarthPosition::EarthPosition()
{
	return;
}

//
// Initialize with the elements of a Vector3 object.
// The elements are assumed to be rectangular coordinates.
//

EarthPosition::EarthPosition(
	Vector3		v)
{
	v.Get(0,&_v[0]);
	v.Get(1,&_v[1]);
	v.Get(2,&_v[2]);
	return;
}

//
// Default destructor, no action
//

EarthPosition::~EarthPosition()
{
	return;
}

//----------------------------//
// EarthPosition::SetPosition //
//----------------------------//
// Sets the earth position by rectangular coordinates

int
EarthPosition::SetPosition(
	double		x,
	double		y,
	double		z)
{
	_v[0] = x;
	_v[1] = y;
	_v[2] = z;
	return(1);
}

//-------------------------------//
// EarthPosition::SetAltLonGCLat //
//-------------------------------//
// Sets the earth position by altitude, longitude, and geocentric latitude
// altitude = altitude above the ellipsoidal earth surface (km)
// longitude = east longitude (radians)
// gc_latitude = geocentric latitude (radians)

int
EarthPosition::SetAltLonGCLat(
	double		altitude,
	double		longitude,
	double		gc_latitude)
{
	double slat = sin(gc_latitude);
	double clat = cos(gc_latitude);
	double slon = sin(longitude);
	double clon = cos(longitude);

	// Radius of earth at desired location.
	double radius = r1_earth*(1.0 - flat*slat*slat);

	// Form sea-level position vector
	_v[0] = radius*clat*clon;
	_v[1] = radius*clat*slon;
	_v[2] = radius*slat;

	// Form vector normal to the surface of the ellipsoidal earth at the desired
	// location, with length equal to the desired altitude, and add it to the
	// sealevel position vector to get the final position vector.
	Vector3 normal(_v[0]/(r1_earth*r1_earth), _v[1]/(r1_earth*r1_earth),
		_v[2]/(r2_earth*r2_earth));
	normal.Scale(altitude);

	_v[0] += normal.get(0);
	_v[1] += normal.get(1);
	_v[2] += normal.get(2);

	return(1);
}

//-------------------------------//
// EarthPosition::SetAltLonGDLat //
//-------------------------------//
// Sets the earth position by altitude, longitude, and geodetic latitude
// altitude = altitude above the ellipsoidal earth surface (km)
// longitude = east longitude (radians)
// gd_latitude = geodetic latitude (radians)

int
EarthPosition::SetAltLonGDLat(
	double		altitude,
	double		longitude,
	double		gd_latitude)
{
	// convert geodetic latitude to geocentric latitude
	double gc_latitude = atan(tan(gd_latitude) *
		(1-eccentricity_earth*eccentricity_earth));

	return(SetAltLonGCLat(altitude, longitude, gc_latitude));
}

//-------------------------------//
// EarthPosition::GetAltLonGCLat //
//-------------------------------//
// Returns the earth position as altitude, longitude, and geocentric latitude
// altitude = altitude above the ellipsoidal earth surface (km)
// longitude = east longitude (radians)
// gd_latitude = geocentric latitude (radians)

int
EarthPosition::GetAltLonGCLat(
	double*		altitude,
	double*		longitude,
	double*		gc_latitude)
{
	// get the geodetic information
	double gd_latitude;
	if (! GetAltLonGDLat(altitude, longitude, &gd_latitude))
		return(0);

	// convert geodetic latitude to geocentric latitude
	*gc_latitude = atan(tan(gd_latitude) *
		(1-eccentricity_earth*eccentricity_earth));

	return(1);
}

//-------------------------------//
// EarthPosition::GetAltLonGDLat //
//-------------------------------//
// Returns the earth position as altitude, longitude, and geodetic latitude
// altitude = altitude above the ellipsoidal earth surface (km)
// longitude = east longitude (radians)
// gc_latitude = geodetic latitude (radians)

int
EarthPosition::GetAltLonGDLat(
	double*		altitude,
	double*		longitude,
	double*		gd_latitude)
{
	double f2 = flat*(2.0 - flat);
	double rho = sqrt(_v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2]);
	double r = sqrt(_v[0]*_v[0] + _v[1]*_v[1]);

	if (rho == 0.0)
	{
		// center of the earth
		*altitude = -r2_earth;
		*longitude = 0.0;
		*gd_latitude = 0.0;
		return(1);
	}

	// Compute east longitude
	if (_v[0] == 0.0)
	{
		// on the 90 -- 270 great circle
		if (_v[1] > 0.0)
			*longitude = pi/2;
		else
			*longitude = 3*pi/2;
	}
	else
	{
		*longitude = atan2(_v[1],_v[0]);
		if (*longitude < 0.0)
			*longitude += two_pi;
	}

	// Trial values to start the iteration
	*gd_latitude = asin(_v[2]/rho);
	double sinlat = sin(*gd_latitude);
	double coslat = cos(*gd_latitude);
	*altitude = rho - r1_earth*(1.0 - flat*sinlat*sinlat);

	// Iterate to solution (or maximum numer of interations)
	double g0,g1,g2;
	double dr,dz,dalt,dlat;
	double tol = 1.0e-14;
	int maxiter = 10;
	int i;
	for (i=1; i <= maxiter; i++)
	{
		sinlat = sin(*gd_latitude);
		coslat = cos(*gd_latitude);
		g0 = r1_earth/sqrt(1.0 - f2*sinlat*sinlat);
		g1 = g0 + *altitude;
		g2 = g0*(1-flat)*(1-flat) + *altitude;
		dr = r - g1*coslat;
		dz = _v[2] - g2*sinlat;
		dalt = dr*coslat + dz*sinlat;
		dlat = (dz*coslat - dr*sinlat) / (r1_earth + *altitude + dalt);
		*gd_latitude += dlat;
		*altitude += dalt;
		if ((dlat < tol) && (fabs(dalt)/(r1_earth + *altitude) < tol))
			break;
	}

	if (i >= maxiter)
	{
		printf("Error: EarthPosition::GetAltLonGDLat\n");
		printf("  Did not converge to a solution for the surface point\n");
		return(0);
	}

	return(1);
}

//
// ReadLonLat
//
// Reads in a LonLat object from a file (using LonLat::Read()), and converts
// to an EarthPosition vector. (km)
//

int
EarthPosition::ReadLonLat(FILE* fp)
{
	LonLat lon_lat;
	if (lon_lat.Read(fp) == 0) return(0);
	SetAltLonGDLat(0.0, lon_lat.longitude, lon_lat.latitude);
	return(1);
}

//
// WriteLonLat
//
// Writes out an EarthPosition object to a file as a LonLat object
// (using LonLat::Write()).
//

int
EarthPosition::WriteLonLat(FILE* fp)
{
	LonLat lon_lat(*this);
	if (lon_lat.Write(fp) == 0) return(0);
	return(1);
}

//
// operator=
//
// Assignment operator to allow a Vector3 object to be assigned to a
// EarthPosition object straight across (assumes that the Vector3 object
// is a geocentric position vector).
//

void EarthPosition::operator=(Vector3 vec)

{

// transfer data straight across
_v[0] = vec.get(0);
_v[1] = vec.get(1);
_v[2] = vec.get(2);
	return;
}

//
// Other access methods
//

//
// Compute the distance along the surface of the earth between the position
// specified in the calling object and the position specified by the argument.
// This function assumes that both positions are actually on the surface
// (ie., altitude = 0), but does not check.  If one is not on the surface,
// then the result is nonsense.
// Right now, this routine uses a simple spherical approximation of the earth.
//

double EarthPosition::surface_distance(EarthPosition r)

{

double mag = Magnitude();
//double mag = sqrt(_v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2]);
double theta = acos((*this % r) / (mag * r.Magnitude()));
return(mag * theta);

}

//
// Nadir
//
// Compute the nadir position on the earth's surface directly below (above)
// this objects position.
//

EarthPosition
EarthPosition::Nadir()

{
	double alt, lon, gc_lat;
	GetAltLonGCLat(&alt, &lon, &gc_lat);
	EarthPosition result;
	result.SetAltLonGCLat(0.0, lon, gc_lat);
	return(result);
}

//
// Normal
//
// Form a unit vector normal to the earth's surface at the location of
// this EarthPosition.
//

Vector3
EarthPosition::Normal()

{

Vector3 normal(_v[0]/(r1_earth*r1_earth),
               _v[1]/(r1_earth*r1_earth),
               _v[2]/(r2_earth*r2_earth));
normal.Scale(1.0);
return(normal);

}

//
// SurfaceCoordinateSystem
//
// Form a CoordinateSwitch object that defines a local surface coordinate
// system at the location specified by this EarthPosition.  The local
// frame is defined with respect to the geocentric frame.
// The local frame axes are:
//    x - perpendicular to the local normal and directed to true north
//    y - perpendicular to the local normal and directed to true east
//    z - the local normal
//

CoordinateSwitch
EarthPosition::SurfaceCoordinateSystem()
{

//
// Form a vector pointing east by crossing a vector pointing
// along the earth's rotation axis with the vector pointing from
// the earths center to the surface position.
// The resulting vector is eastward from
// the location on the surface.  The local normal could be used
// in place of the position vector because it lies in the
// same plane defined by the earth's rotation axis and the position
// vector.
//

Vector3 urot(0,0,1);
Vector3 xlocal = urot & (*this);
xlocal.Scale(1.0);
Vector3 zlocal = Normal();
Vector3 ylocal = zlocal & xlocal;
CoordinateSwitch cs(xlocal,ylocal,zlocal);
return(cs);

}

//
// IncidenceAngle
//
// Compute the incidence angle of a vector (in the geocentric frame) at
// this EarthPosition.
// The angle is returned in radians.
//

double
EarthPosition::IncidenceAngle(
	Vector3		vector)
{
	Vector3 normal = Normal();
	return(acos(vector % -normal));
}
