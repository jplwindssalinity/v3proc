//==================================================================//
// Copyright (C) 1997, California Institute of Technology.          //
// U.S. Goverment sponsorship acknowledged                          //
//==================================================================//

//==================================================================//
// Author:  Bryan Stiles   Created 9/22/97                          //
//==================================================================/

//==================================================================//
// CLASSES							    //
//		GenericDist, Uniform, Gaussian, GTC, AttDist	    //
//==================================================================//

//==================================================================//
// Functions: 					SeedFromClock	    //
//==================================================================//

static const char rcs_id_distributions_c[] =
        "@(#) $Id$";

#include"Distributions.h"

//================================//
// GenericDist                    //
//================================//
GenericDist::~GenericDist(){
	return;
}

//===================================//
// GenericTimelessDist               //
//===================================//

GenericTimelessDist::~GenericTimelessDist(){
	return;
}

//==================================//
// GenericTimelessDist::GetNumber   //
//==================================//

float GenericTimelessDist::GetNumber(double time){
	if(time<0.0) //bogus check to keep compiler quiet
		return(0.0);
	else return(GetNumber());
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
// Uniform::GetNumber            //
//================================//

float
Uniform::GetNumber()
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
Gaussian::GetNumber()
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
// Random Velocity                  //
//==================================//

RandomVelocity::RandomVelocity()
{
	_sample_period=1.0;
	_radius=1.0;
        _mean=0.0;
	_noise= NULL;
	_position=0.0;
	_time=0.0;
	_velocity=0.0;
 	return;
}

RandomVelocity::RandomVelocity(GenericTimelessDist* noise, float sample_period,
	float radius, float mean)
{
	_noise=noise;
	_sample_period=sample_period;
   	_radius=radius;
	_mean=mean;
	_position=mean;
	_time=0.0;
	_velocity=noise->GetNumber();
	while(fabs(_position-_mean+_velocity*_sample_period) > _radius){
		_velocity=noise->GetNumber();	
	}
	return;
}

RandomVelocity::~RandomVelocity(){
	return;
}

//================================//
// RandomVelocity::GetNumber	  //
//================================//

float RandomVelocity::GetNumber(double time){
	if (time < 0.0){
	 fprintf(stderr,"Fatal Error produced by RandomVelocity::GetNumber\n");
	 fprintf(stderr,"Parameter time may not be negative.\n");
	 exit(1);
	} 
	if (time < _time){
	 fprintf(stderr,"Fatal Error produced by RandomVelocity::GetNumber\n");
	 fprintf(stderr,"Parameter time may not decrease between \n");
	 fprintf(stderr,"consecutive calls to the method. \n");
	 exit(1);
	}
	while (time >= _time + _sample_period){
	  _position+=_velocity*_sample_period;
	  _time+=_sample_period;
 	  _velocity=_noise->GetNumber();
	  while(fabs(_position-_mean+_velocity*_sample_period) > _radius){
		_velocity=_noise->GetNumber();	
	  }

	}	
	return(_position+(time-_time)*_velocity);
}


//==================================//
// AttDist                          //
//==================================//

AttDist::AttDist()
{
	return;
}

AttDist::AttDist(GenericDist * r, GenericDist* p, GenericDist* y)
{
	roll=r;
	pitch=p;
	yaw=y;
	return;
}

AttDist::~AttDist()
{
	return;
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


