//==================================================================//
// Copyright (C) 1997, California Institute of Technology.          //
// U.S. Goverment sponsorship acknowledged                          //
//==================================================================//

//==================================================================//
// Author:  Bryan Stiles   Created 9/22/97                          //
//==================================================================/

//==================================================================//
// CLASSES							    //
//		Generic_Dist, Uniform, Gaussian			    //
//==================================================================//

//==================================================================//
// Functions: 					SeedFromClock	    //
//==================================================================//

static const char rcs_id_distributions_c[] =
        "@(#) $Id$";

#include"Distributions.h"

//============================//
// Generic_Dist               //
//============================//

Generic_Dist::~Generic_Dist(){
	return;
}

//============================//
// Uniform                    //
//============================//

Uniform::Uniform()
:	_radius(0.0), _mean(0.0)
{
	return;
}

Uniform::Uniform(float radius, float mean)
{
	_radius=radius;
	_mean=mean;
	return;
}

Uniform::~Uniform(){
	return;
}

//================================//
// Uniform::get_number            //
//================================//

float
Uniform::get_number()
{
	float num;
	num=(float)(2*_radius*(drand48()-0.5)+_mean);
	return(num);
}


//============================//
// Gaussian                    //
//============================//

Gaussian::Gaussian()
:	_variance(0.0), _mean(0.0)
{
	return;
}

Gaussian::Gaussian(float variance, float mean)
{
	_variance=variance;
	_mean=mean;
	return;
}

Gaussian::~Gaussian()
{
	return;
}

float
Gaussian::get_number()
{
	float num;
	double v1, v2, r, fac;
        do {
             v1=2.0*drand48()-1.0;
             v2=2.0*drand48()-1.0;
             r = v1*v1+v2*v2;
        } while (r >= 1.0 || r == 0.0);
        fac=sqrt((float) -2.0*log(r)/r);
        num=(float)v2*fac;
        num=num*sqrt(_variance) + _mean;
	return(num);
}


//==================================//
// GTC                              //
//==================================//

GTC::GTC()
:	_tau(1.0), _prev(1.0), Gaussian()
{
 	return;
}

GTC::GTC(float variance, float mean, float tau)
{
	_variance=variance;
	_mean=mean;
	_prev=0.0;
	_tau=tau;
	return;
}

GTC::~GTC(){
	return;
}

//================================//
// GTC::get_number	          //
//================================//

float GTC::get_number(){

	_prev*=(1-_tau);
	_prev+=_tau*Gaussian::get_number();	
	return(_prev);
}

//==================================//
// SeedFromClock                    //
//==================================//
void
SeedFromClock()
{
  struct timeval now;
  gettimeofday(&now,NULL);
  srand48(now.tv_sec);
}


