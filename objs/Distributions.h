//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Goverment sponsorship acknowledged					//
//==========================================================//

//==================================================================//
// Author:  Bryan Stiles   Created 9/18/97                          //
//==================================================================//

//==================================================================//
// CLASSES							    //
//		GenericDist, GenericTimelessDist, Uniform,          //
//              Gaussian, RandomVelocity, AttDist	            //
//==================================================================//

//==================================================================//
// Functions: 					SeedFromClock	    //
//==================================================================//


#ifndef DISTRIBS_H
#define DISTRIBS_H

static const char rcs_id_distributions_h[] =
        "@(#) $Id$";

#include<stdio.h>
#include<time.h>
#include<math.h>
#include<sys/time.h>
#include<stdlib.h>


//==================================================================//
// CLASS							    //
//		GenericDist                                        //
// Description: Base class for Probability Distributions            //
//==================================================================//

class GenericDist
{
public:
	virtual float GetNumber(double time)=0;
	virtual ~GenericDist();
};

//==================================================================//
// CLASS 							    //
//		GenericTimelessDist			            //
// 								    //
// Description: Base class for Probability Distributions which      //
// are independent of time.					    //
//==================================================================//

class GenericTimelessDist: public GenericDist
{
public:
	virtual float GetNumber() = 0;

// GetNumber may be called with a time parameter, but it is ignored  //
    float GetNumber(double time); 
	virtual ~GenericTimelessDist();
};

//==================================================================//
// CLASS							    //
//            RNG						    //
//								    //
// Description: Basic Pseudo Random Number Generator                //
// contains parameters:						    //
//         _seed                                                    //
//                      					    //
// contains member function, GetDouble() return number on uniform   //
// [0,1) range                                                      //
// SetSeed(long int) SetRandomSeed() Uses lrand48 to set the seed   //
//                                                                  //
//==================================================================//

#define RNG_M 714025
#define RNG_IA 1366
#define RNG_IC 150889

class RNG
{
public:

	RNG(long int seed);
	RNG();
	~RNG();
	void	SetSeed(long int);
	void	SetRandomSeed();
	double	GetDouble();
 
protected:

	void		_Init();
	long int	_seed;
	long int	_output;
	long int	_tab[98];
};

//==================================================================//
// CLASS							    //
//            Uniform						    //
//								    //
// Description: Uniform Distribution                                //
// contains parameters:						    //
//         _radius:=  half the width of the distribution            //
//         _mean						    //
// contains member function, GetNumber() which returns a random    //
// number extracted from the distribution.                          //
//==================================================================//

class Uniform : public GenericTimelessDist
{
public:
	Uniform();
        Uniform(float radius, float mean);
	~Uniform();
	float GetNumber();

	float GetRadius();
	void SetRadius(float r);
	float GetMean();
        void SetMean(float m);
        void SetSeed(long int seed);

protected:
	float _radius;
        float _mean;
        RNG _rng;
};


//==================================================================//
// Class                                                            //
//         Gaussian                                                 //
// Description: Gaussian Distribution                               //
//==================================================================//

class Gaussian : public GenericTimelessDist
{
public:
	Gaussian();
	Gaussian(float variance, float mean);
	~Gaussian();
	float GetNumber();	

	float GetVariance();
	int SetVariance(float v);
	float GetMean();
        int SetMean(float m);
        void SetSeed(long int seed);

protected:
	float _variance;
	float _mean;
        RNG _rng;
};

//==================================================================//
// Class 							    //
//	 RandomVelocity						    //
// Description:		Velocity varies in a random manner          //
//  with position kept within bounds (_radius)		            //
//==================================================================//

class RandomVelocity : public GenericDist
{
public:
	RandomVelocity();
	RandomVelocity(GenericTimelessDist* noise, float sample_period, 
		       float radius, float mean);
	~RandomVelocity();
	float GetNumber(double time);
protected:
	float _sample_period;
        float _radius;
	float _mean;
        GenericTimelessDist* _noise;
	float _position;
	float _time;
	float _velocity;
};

//==================================================================//
// Class                                                            //
//  TimeCorrelatedGaussian                                          //
//==================================================================//

class TimeCorrelatedGaussian : public GenericDist
{
public:

TimeCorrelatedGaussian();
~TimeCorrelatedGaussian();
int Initialize();
float GetNumber(double time);
int SetVariance(float variance);
int SetMean(float mean);
void SetSeed(long int seed);
int SetCorrelationLength(float corrlength);

Gaussian Uncorrelated;

protected:
double _previousTime;
float _previousOutput;
float _correlationLength;
};

//==================================================================//
// Class							    //
//	AttDist							    //
// Description: Contains three pointers to random distributions for //
// roll, pitch and yaw.						    //
//==================================================================//

class AttDist
{
public:
	AttDist();
	~AttDist();
	TimeCorrelatedGaussian roll;
	TimeCorrelatedGaussian pitch;
	TimeCorrelatedGaussian yaw;

};

//==================================================================//
// Function SeedFromClock					    //
// 								    //
// Description: Causes the pseudo random number generator  (drand48)//
// to be seeded by the clock.                                       //
//==================================================================//

void SeedFromClock();
#endif

