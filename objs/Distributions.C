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

//=============================//
// RNG                         //
//=============================//

RNG::RNG(long int seed){
	SetSeed(seed);
	return;
}

RNG::RNG(){
	SetRandomSeed();
	return;
}

RNG::~RNG(){
  return;
}

void RNG::SetSeed(long int seed){
  _seed=seed;
	if (seed > 0)
		_seed=-_seed;
	else if (seed == 0)
		_seed = 1;
	_Init();
}

void RNG::SetRandomSeed(){
	SetSeed(lrand48() * lrand48());
	return;
}

double RNG::GetDouble(){
  int j=(int)(1+(97.0*_output)/RNG_M);
  _output=_tab[j];
  _seed=(RNG_IA*_seed + RNG_IC) % RNG_M;
  _tab[j]=_seed;
  return((double)(_output)/RNG_M);
}

void RNG::_Init(){
  int j;
  if((_seed=(RNG_IC - _seed) % RNG_M) < 0) _seed=-_seed;
  for(j=1;j<97;j++){
    _seed=(RNG_IA*_seed + RNG_IC) % RNG_M;
    _tab[j]=_seed;
  }
  _seed=(RNG_IA*_seed + RNG_IC) % RNG_M;
  _output=_seed;
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
	num=(float)(2*_radius*(_rng.GetDouble()-0.5)+_mean);
	return(num);
}

float Uniform::GetRadius(){
  return(_radius);
}

void Uniform::SetRadius(float r){
  _radius=r;
}

float Uniform::GetMean(){
  return(_mean);
}

void Uniform::SetMean(float m){
  _mean=m;
}

void Uniform::SetSeed(long int seed){
  _rng.SetSeed(seed);
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
             v1=2.0*_rng.GetDouble()-1.0;
             v2=2.0*_rng.GetDouble()-1.0;
             r = v1*v1+v2*v2;
        } while (r >= 1.0 || r == 0.0);
        fac=sqrt((float) -2.0*log(r)/r);
        num=(float)v2*fac;
        num=num*sqrt(_variance) + _mean;
	return(num);
}

float Gaussian::GetVariance(){
  return(_variance);
}

int Gaussian::SetVariance(float v){
  if(v < 0.0) return(0);
  _variance=v;
  return(1);
}

float Gaussian::GetMean(){
  return(_mean);
}

int Gaussian::SetMean(float m){
  _mean=m;
  return(1);
}

void Gaussian::SetSeed(long int seed){
  _rng.SetSeed(seed);
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

//==================================================================//
// Class                                                            //
//  TimeCorrelatedGaussian                                          //
//==================================================================//

TimeCorrelatedGaussian::TimeCorrelatedGaussian()
  : _previousTime(0.0), _previousOutput(0.0), _correlationLength(0.0)
{
  return;
}

TimeCorrelatedGaussian::~TimeCorrelatedGaussian(){
  return;
}

int TimeCorrelatedGaussian::Initialize(){
  _previousOutput=Uncorrelated.GetNumber();
   return(1);
}

float TimeCorrelatedGaussian::GetNumber(double time){

  /*******  BIAS ONLY CASE        *************/
  if(Uncorrelated.GetVariance()==0.0){
    return(Uncorrelated.GetMean());
  }

  /******** Error Condition ****************/
  if(time < _previousTime){
    fprintf(stderr,"TimeCorrelatedGaussian requires monotonically increasing time\n");
    exit(1);
  }
  
  /********** Uncorrelated case *************/
  if(_correlationLength == 0.0){
    return(Uncorrelated.GetNumber());
  }

  /******* Normal Mode  *******************/

  float retval=exp(-(time-_previousTime)/_correlationLength);
  retval=retval*_previousOutput+sqrt(1-retval*retval)*Uncorrelated.GetNumber();
  _previousTime=time;
  _previousOutput=retval;
  return(retval);
}

int TimeCorrelatedGaussian::SetVariance(float variance){
  if(! Uncorrelated.SetVariance(variance)) return(0);
  return(1);
}

int TimeCorrelatedGaussian::SetMean(float mean){
  if(! Uncorrelated.SetMean(mean)) return(0);
  return(1);
}

void TimeCorrelatedGaussian::SetSeed(long int seed){
  Uncorrelated.SetSeed(seed);
}

int TimeCorrelatedGaussian::SetCorrelationLength(float corrlength){
  if(corrlength < 0.0) return(0);
  _correlationLength=corrlength;
  return(1);
}

//==================================//
// AttDist                          //
//==================================//

AttDist::AttDist()
{
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





