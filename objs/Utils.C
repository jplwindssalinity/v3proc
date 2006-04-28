//----------------------------------------------------------------------------
// Utils.cpp
//
// This file contains utility function definitions.
//----------------------------------------------------------------------------



//---------------
// Other includes
//---------------

#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <math.h>
#include "Utils.h"
#include "Constants.h"

using std::cout;
using std::endl;

//SIGN_C: fron fortran SIGN function
double SIGN_C(const double& a, const double& b) 
{
  double return_value;
  if(b >=0.0) return_value=fabs(a);
  else return_value = -fabs(a);
  return(return_value);
}

//--------------------------------------------
//This is copied from S.H.'s fortran program
//
//  geo_hdg.f
//  DEMGen
//
//  Created by Ali Safaeinili on 8/24/05.
//  Copyright 2005 __MyCompanyName__. All rights reserved.
//
//
//
//
//	subroutine geo_hdg(r_a,r_e2,r_lati,r_loni,r_latf,r_lonf,r_geohdg)
//
//
//
//	FILE NAME: geo_hdg.f
//
//     DATE WRITTEN:12/02/93 
//
//     PROGRAMMER:Scott Hensley
//
// 	FUNCTIONAL DESCRIPTION: This routine computes the heading along a geodesic
//     for either an ellipitical or spherical earth given the initial latitude
//     and longitude and the final latitude and longitude. 
//
//     ROUTINES CALLED:none
//  
//     NOTES: These results are based on the memo
//
//        "Summary of Mocomp Reference Line Determination Study" , IOM 3346-93-163
//
//      and the paper
//
//        "A Rigourous Non-iterative Procedure for Rapid Inverse Solution of Very
//         Long Geodesics" by E. M. Sadano, Bulletine Geodesique 1958
//
//     ALL ANGLES ARE ASSUMED TO BE IN RADIANS//   
//
//     UPDATE LOG:
//
//--------------------------------------------------------------------------------------

void geo_hdg(const double& r_a, const double& r_e2,const double& r_lati, 
	     const double& r_loni,const double& r_latf, 
	     const double& r_lonf, double&  r_geohdg){


     


  //double r_a                    //semi-major axis
  //double r_e2                   //square of eccentricity
  //double r_lati                 //starting latitude
  //double r_loni                 //starting longitude
  //double r_latf                 //ending latitude
  //double r_lonf                 //ending longitude  
  
  //   	OUTPUT VARIABLES:
  // double r_geohdg
  
  //	LOCAL VARIABLES:
  double r_t1,r_t2,r_e,r_ome2,r_sqrtome2,r_b0,r_f,r_ep,r_n;
  double r_k1,r_k2,r_k3,r_k4,r_k5,r_l,r_ac,r_bc,r_phi,r_phi0;
  double r_tanbetai,r_cosbetai,r_sinbetai,r_cosphi,r_sinphi;
  double r_tanbetaf,r_cosbetaf,r_sinbetaf,r_lambda,r_coslam,r_sinlam;
  double r_ca,r_cb,r_cc,r_cd,r_ce,r_cf,r_cg,r_ch,r_ci,r_cj,r_x,r_q;
  double r_sinlati,r_coslati,r_tanlatf,r_tanlati,r_coslon,r_sinlon;
  double r_sin2phi,r_cosph0,r_sinph0,r_cosbeta0,r_cos2sig,r_cos4sig;
  double r_cotalpha12,r_cotalpha21,r_lsign ;

  //pi is declared in Constants.h
   
 
  

 
//	FUNCTION STATEMENTS: none

//  	PROCESSING STEPS:

  if(r_e2 == 0.0){  //use the simplier spherical formula
    
    r_sinlati = sin(r_lati);
    r_coslati = cos(r_lati);
    r_tanlatf = tan(r_latf);
    
    r_t1 =  r_lonf - r_loni;
    if(fabs(r_t1) > pi)
      r_t1 = -(2.0*pi - fabs(r_t1))*SIGN_C(1.0,r_t1);
    
    r_sinlon = sin(r_t1);
    r_coslon = cos(r_t1);
    r_t2 = r_coslati*r_tanlatf - r_sinlati*r_coslon;
    
    r_geohdg = atan2(r_sinlon,r_t2);
    //cout<<"r_sinlon, r_t2 "<< r_sinlon<<" "<<r_t2<<endl;
  }
  else{   // use the full ellipsoid formulation
    
    r_e = sqrt(r_e2);
    r_ome2 = 1.0 - r_e2;
    r_sqrtome2 = sqrt(r_ome2);
    r_b0 = r_a*r_sqrtome2;
    r_f = 1.0 - r_sqrtome2;
    r_ep = r_e*r_f/(r_e2-r_f);
    r_n = r_f/r_e2;
    r_k1 = (16.0*r_e2*pow(r_n,2) + pow(r_ep,2))/pow(r_ep,2)  ; 
    r_k2 = (16.0*r_e2*pow(r_n,2))/(16.0*r_e2*pow(r_n,2) + pow(r_ep,2));
    r_k3 = (16.0*r_e2*pow(r_n,2))/pow(r_ep,2);
    r_k4 = (16.0*r_n - pow(r_ep,2))/(16.0*r_e2*pow(r_n,2) + pow(r_ep,2));
    r_k5 = 16.0/(r_e2*(16.0*r_e2*pow(r_n,2) + pow(r_ep,2)));
    
    //cout<<"r_k1-5 "<<r_k1<<" "<<r_k2<<" "<<r_k3<<" "<<r_k4<<" "<<r_k5<<endl;
    
    r_tanlati = tan(r_lati);
    r_tanlatf = tan(r_latf);
    r_l  =  fabs(r_lonf-r_loni);
    r_lsign = r_lonf - r_loni;
    if(fabs(r_lsign) > pi){
      r_lsign = -(2.0*pi - r_l)*SIGN_C(1.0,-r_lsign);
    }
    r_sinlon = sin(r_l);
    r_coslon = cos(r_l);
    
    r_tanbetai = r_sqrtome2*r_tanlati;
    r_tanbetaf = r_sqrtome2*r_tanlatf;
    
    r_cosbetai = 1.0/sqrt(1.0 + pow(r_tanbetai,2));
    r_cosbetaf = 1.0/sqrt(1.0 + pow(r_tanbetaf,2));
    r_sinbetai = r_tanbetai*r_cosbetai   ;     
    r_sinbetaf = r_tanbetaf*r_cosbetaf;
    
    r_ac = r_sinbetai*r_sinbetaf  ;      
    r_bc = r_cosbetai*r_cosbetaf  ;      
    
    r_cosphi = r_ac + r_bc*r_coslon;
    r_sinphi = SIGN_C(1.0,r_sinlon)*sqrt(1.0 - min(pow(r_cosphi,2),1.0));
    r_phi = fabs(atan2(r_sinphi,r_cosphi));
    
    if(r_a*fabs(r_phi) >  1.0e-6){
      
      r_ca = (r_bc*r_sinlon)/r_sinphi;
      r_cb = pow(r_ca,2);
      r_cc = (r_cosphi*(1.0 - r_cb))/r_k1;
      r_cd = (-2.0*r_ac)/r_k1;
      r_ce = -r_ac*r_k2;
      r_cf = r_k3*r_cc;
      r_cg = pow(r_phi,2)/r_sinphi;
      
      r_x = ((r_phi*(r_k4 + r_cb) + r_sinphi*(r_cc + r_cd) + r_cg*(r_cf + r_ce))*r_ca)/r_k5;
      
      r_lambda = r_l + r_x;
      
      r_sinlam = sin(r_lambda);
      r_coslam = cos(r_lambda);
      
      r_cosph0 = r_ac + r_bc*r_coslam;
      r_sinph0 = SIGN_C(1.0,r_sinlam)*sqrt(1.0 - pow(r_cosph0,2));
      
      r_phi0 = fabs(atan2(r_sinph0,r_cosph0));
      
      r_sin2phi = 2.0*r_sinph0*r_cosph0;
      
      r_cosbeta0 = (r_bc*r_sinlam)/r_sinph0;
      r_q = 1.0 - pow(r_cosbeta0,2);
      r_cos2sig = (2.0*r_ac - r_q*r_cosph0)/r_q;
      r_cos4sig = 2.0*(pow(r_cos2sig,2) - .50);
      
      r_ch = r_b0*(1.0 + (r_q*pow(r_ep,2))/4.0 - (3.0*(pow(r_q,2))*pow(r_ep,4))/64.0);
      r_ci = r_b0*((r_q*pow(r_ep,2))/4.0 - ((pow(r_q,2))*pow(r_ep,4))/16.0);
      r_cj = (pow(r_q,2)*r_b0*pow(r_ep,4))/128.0;
      
      r_t2 = (r_tanbetaf*r_cosbetai - r_coslam*r_sinbetai);
      r_sinlon = r_sinlam*SIGN_C(1.0,r_lsign);
      
      r_cotalpha12 = (r_tanbetaf*r_cosbetai - r_coslam*r_sinbetai)/r_sinlam;
      r_cotalpha21 = (r_sinbetaf*r_coslam - r_cosbetaf*r_tanbetai)/r_sinlam;
      
      r_geohdg = atan2(r_sinlon,r_t2);
    }
    else{
      r_geohdg = 0.00;
      cout<<"Out to lunch..."<<endl;
    }
  }
}
       
       
//------------
//Eastern radius
//--------------
double  reast(const double& r_a, const double& r_e2,const double& r_lat){
  return( r_a/sqrt(1.0 - r_e2*sin(r_lat)*sin(r_lat)));

}

//----------------
//northen radius
//-----------------
double rnorth(const double& r_a, const double& r_e2,const double& r_lat){
  
  return( (r_a*(1.0 - r_e2))/pow( 1.0 - r_e2*sin(r_lat)*sin(r_lat),1.5));
}

//---------------
//Curvature in the alongtrack direction
//----------------
double radius_along(const double& r_a, const double& r_e2,const double& r_hdg, const double& r_lat){

  double r_re = reast(r_a,r_e2,r_lat);
  double r_rn = rnorth(r_a,r_e2,r_lat);
  //cout<<"r east , r north "<< r_re<<" "<<r_rn<<endl;
  return( (r_re*r_rn)/(r_re*pow(cos(r_hdg),2) + r_rn*pow(sin(r_hdg),2)) );
}

//---------------
//Curvature in the crosstrack direction
//---------------
double radius_cross(const double& r_a, const double& r_e2,const double& r_hdg, const double& r_lat){

  double r_re = reast(r_a,r_e2,r_lat);
  double r_rn = rnorth(r_a,r_e2,r_lat);
  cout<<"r east , r north "<< r_re<<" "<<r_rn<<endl;
  return( (r_re*r_rn)/(r_re*pow(sin(r_hdg),2) + r_rn*pow(cos(r_hdg),2)) );
}


//---------------------------
//Convert xyz to llh
//----------------------------
void xyz_to_llh(const double& r_a, const double& r_e2, const Vector3& r_v, Vector3& r_llh)
{
  double r_q2 = 1.0/(1.0 - r_e2);
  double r_q = sqrt(r_q2);
  double r_q3 = r_q2 - 1.0;
  double r_b = r_a*sqrt(1.0 - r_e2);
           
  r_llh.SetY( atan2(r_v(1),r_v(0)));
           
  double r_p = sqrt(pow(r_v(0),2) + pow(r_v(1),2));
  double r_tant = (r_v(2)/r_p)*r_q;
  double r_theta = atan(r_tant);
  r_tant = (r_v(2) + r_q3*r_b*pow(sin(r_theta),3))/(r_p - r_e2*r_a*pow(cos(r_theta),3));
  r_llh.SetX(  atan(r_tant));
  double r_re = r_a/sqrt(1.0 - r_e2*pow(sin(r_llh(0)),2));
  r_llh.SetZ( r_p/cos(r_llh(0)) - r_re )    ;    
}

void llh_to_xyz(const double& r_a, const double& r_e2, const Vector3& r_llh, Vector3& r_v)
{
  double r_re = r_a/sqrt(1.0 - r_e2*pow(sin(r_llh(0)),2));

  r_v.SetX( (r_re + r_llh(2))*cos(r_llh(0))*cos(r_llh(1)));
  r_v.SetY( (r_re + r_llh(2))*cos(r_llh(0))*sin(r_llh(1)));
  r_v.SetZ( (r_re*(1.0-r_e2) + r_llh(2))*sin(r_llh(0)))   ;      
}


//----------------
//convert xyz to azi elev
//-----------------
void xyz_to_azi_elev(const Vector3& look, double& azi, double& elev)
{
  Vector3 u;
  u=look;
  u.Scale(1.0);
  elev=asin(u.GetY());
  azi=asin(u.GetX()/cos(elev));
}
void azi_elev_to_xyz(const double& azi, const double& elev, Vector3& u)
{
  u.SetX(sin(azi)*cos(elev));
  u.SetY(sin(elev));
  u.SetZ(cos(azi)*cos(elev));
}


//compute area defined by three position vectors
void surface_area(const double& r_a, const Vector3& r_v1, const Vector3& r_v2, 
		  const Vector3& r_v3, double& area) 
{

 double angle_rad12= Vector3::AngleBetween(&r_v1, &r_v2);
 double angle_rad13= Vector3::AngleBetween(&r_v1, &r_v3);
 double angle_rad23= Vector3::AngleBetween(&r_v2, &r_v3);
 double side_a= r_a*angle_rad12;
 double side_b= r_a*angle_rad13;
 double side_c= r_a*angle_rad23;
 
        
      
 double cos_angle= (side_a*side_a + side_b*side_b - side_c*side_c)/(2*side_a*side_b);
 cos_angle= acos(cos_angle);
 area= 0.5*sin(cos_angle)*side_a*side_b;
 return;
}
//------------------------
//int round(x): round double
//---------------------------
int round_double(const double& x)
  {
    double y;
    if (x >= 0.0) y = x + 0.5;
    else y = x - 0.5;
    return(int(y));
  }
