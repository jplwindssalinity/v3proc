//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_earthposition_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "Constants.h"
#include "EarthPosition.h"
#include "Matrix3.h"
#include "CoordinateSwitch.h"

//
// EarthPosition
//

//
// Initialize with a complete set of 3 user-specified elements.
// The meaning of the 3 elements is specified by the type argument at the end.
// Note that altitudes must be specified in km!
//

EarthPosition::EarthPosition(double x1, double x2, double x3,
                             earthposition_typeE etype)

{
EarthPosition::SetPosition(x1,x2,x3,etype);
return;
}

//
// Initialize with a 2 user-specified elements.
// The altitude is assumed to be zero (ie., a position on the surface)
// The latitude type is specified by the type argument at the end.
//

EarthPosition::EarthPosition(double lat, double lon,
                             earthposition_typeE etype)

{
// call SetPosition with zero altitude
EarthPosition::SetPosition(0.0,lat,lon,etype);
return;
}

//
// Initialize with the elements of a Vector3 object.
// The elements are assumed to be rectangular coordinates.
//

EarthPosition::EarthPosition(Vector3 v)

{

v.Get(0,&_v[0]);
v.Get(1,&_v[1]);
v.Get(2,&_v[2]);
	return;
}

//
// Default constructor, no initialization
//

EarthPosition::EarthPosition()

{
return;
}

//
// Default destructor, no action
//

EarthPosition::~EarthPosition()

{
return;
}

//
// SetPosition
//
// Initialize with a complete set of 3 user-specified elements.
// The meaning of the 3 elements is specified by the type argument at the end.
// Note that altitudes must be specified in km!
//

int
EarthPosition::SetPosition(double x1, double x2, double x3,
                             earthposition_typeE etype)

{

if (etype == GEODETIC)
  {	// convert geodetic latitude to geocentric latitude
  x2 = atan(tan(x2)*(1-eccentricity_earth*eccentricity_earth));
  etype = GEOCENTRIC;
  }

if (etype == RECTANGULAR)
  {
  // x1,x2,x3 are the rectangular coordinates (with the same units).
  _v[0] = x1;
  _v[1] = x2;
  _v[2] = x3;
  }
else if (etype == GEOCENTRIC)
  {
  // x1 is the altitude above the ellipsoidal earth's surface. (km)
  // x2 is the geocentric latitude of the surface location (radians).
  // x3 is the east longitude of the surface location (radians).

  double sx2 = sin(x2);
  double cx2 = cos(x2);
  double sx3 = sin(x3);
  double cx3 = cos(x3);

  // Radius of earth at desired location.
  double flat = 1.0 - sqrt(1.0-eccentricity_earth*eccentricity_earth);
  double radius = r1_earth*(1.0 - flat*sx2*sx2);

  // Form sea-level position vector
  _v[0] = radius*cx2*cx3;
  _v[1] = radius*cx2*sx3;
  _v[2] = radius*sx2;

  // Form vector normal to the surface of the ellipsoidal earth at the desired
  // location, with length equal to the desired altitude, and add it to the
  // sealevel position vector to get the final position vector.
  Vector3 normal(_v[0]/(r1_earth*r1_earth),
                 _v[1]/(r1_earth*r1_earth),
                 _v[2]/(r2_earth*r2_earth));
  normal.Scale(x1);

  _v[0] += normal.get(0);
  _v[1] += normal.get(1);
  _v[2] += normal.get(2);
  }
else
  {
  printf("Error: Invalid type = %d received by EarthPosition object\n",etype);
  exit(-1);
  }

return(1);

}

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
// Convert the rectangular vector stored in this object into the
// corresponding altitude above the surface, latitude, and longitude.
// Units are km and radians.
//
// Inputs:
//  etype = GEOCENTRIC or GEODETIC - the type of latitude to return
//  alt,lat,elon = pointers to put the altitude (km), latitude (rads),
//		and east longitude (rads) in.
//

int
EarthPosition::GetAltLatLon(earthposition_typeE etype,
double* alt, double* lat, double* elon)

{

double flat = 1.0 - sqrt(1.0-eccentricity_earth*eccentricity_earth);
// double f1 = 1 - flat;
double f2 = flat*(2.0 - flat);
double rho = sqrt(_v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2]);
double r = sqrt(_v[0]*_v[0] + _v[1]*_v[1]);

if (rho == 0.0)
  {	// center of the earth
  *alt = -r2_earth;
  *lat = 0.0;
  *elon = 0.0;
  return(1);
  }

// Compute east longitude
if (_v[0] == 0.0)
  {	// on the 90 -- 270 great circle
  if (_v[1] > 0.0) *elon = pi/2; else *elon = 3*pi/2;
  }
else
  {
  *elon = atan2(_v[1],_v[0]);
  if (*elon < 0.0) *elon += 2*pi;
  }

// Trial values to start the interation
*lat = asin(_v[2]/rho);
double sinlat = sin(*lat);
double coslat = cos(*lat);
*alt = rho - r1_earth*(1.0 - flat*sinlat*sinlat);

// Iterate to solution (or maximum numer of interations)
double g0,g1,g2;
double dr,dz,dalt,dlat;
double tol = 1.0e-14;
int maxiter = 10;
int i;
for (i=1; i <= maxiter; i++)
  {
  sinlat = sin(*lat);
  coslat = cos(*lat);
  g0 = r1_earth/sqrt(1.0 - f2*sinlat*sinlat);
  g1 = g0 + *alt;
  g2 = g0*(1-flat)*(1-flat) + *alt;
  dr = r - g1*coslat;
  dz = _v[2] - g2*sinlat;
  dalt = dr*coslat + dz*sinlat;
  dlat = (dz*coslat - dr*sinlat) / (r1_earth + *alt + dalt);
  *lat += dlat;
  *alt += dalt;
  if ((dlat < tol) && (fabs(dalt)/(r1_earth + *alt) < tol)) break;
  }

if (i >= maxiter)
  {
  printf("Error: EarthPosition::GetAltLatLon\n");
  printf("  Did not converge to a solution for the surface point\n");
  return(0);
  }

if (etype == GEOCENTRIC)
  {	// convert geodetic latitude to geocentric latitude
  *lat = atan(tan(*lat)*(1-eccentricity_earth*eccentricity_earth));
  }

return(1);

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

double alt,lat,lon;
GetAltLatLon(EarthPosition::GEOCENTRIC,&alt,&lat,&lon);
EarthPosition result(lat,lon,EarthPosition::GEOCENTRIC);
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
