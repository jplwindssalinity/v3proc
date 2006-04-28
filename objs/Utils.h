//=============================================================================
// Utils.h
//
// 
//=============================================================================

#ifndef Utils_H
#define Utils_H

static const char rcs_id_utils_h[] =
  "@(#) $Id$";

#include <stdio.h>
#include <string>
#include <sstream>
#include "Matrix3.h"


using std::string;
using std::ostringstream;


// Macros
#define max(A,B)      ((A)>(B)?(A):(B))
#define min(A,B)      ((A)<(B)?(A):(B))


double SIGN_C(const double& a, const double& b);


//SH's method of heading, reast,rnorth
void geo_hdg(const double& r_a, const double& r_e2,const double& r_lati, const double& r_loni,
	     const double& r_latf, const double& r_lonf, double&  r_geohdg);

double  reast(const double& r_a, const double& r_e2,const double& r_lat);
double rnorth(const double& r_a, const double& r_e2,const double& r_lat);
double radius_along(const double& r_a, const double& r_e2,const double& r_hdg, const double& r_lat);
double radius_cross(const double& r_a, const double& r_e2,const double& r_hdg, const double& r_lat);

//SH's method of transformation between xyz and llh
void xyz_to_llh(const double& r_a, const double& r_e2, const Vector3& r_v, Vector3& r_llh);
void llh_to_xyz(const double& r_a, const double& r_e2, const Vector3& r_llh, Vector3& r_v);


//transformation between unit vector and (azi, elev)
void xyz_to_azi_elev(const Vector3& look, double& azi, double& elev);
void azi_elev_to_xyz(const double& azi, const double& elev, Vector3& u);


//compute area defined by three position vectors
void surface_area(const double& r_a, const Vector3& r_v1, const Vector3& r_v2, 
		  const Vector3& r_v3, double& area);
//round double to nearest integer
int round_double(const double& x);
#endif 




