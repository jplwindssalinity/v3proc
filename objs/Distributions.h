//==================================================================//
// Copyright (C) 1997, California Institute of Technology.          //
// U.S. Goverment sponsorship acknowledged                          //
//==================================================================//

//==================================================================//
// Author:  Bryan Stiles   Created 9/18/97                          //
//==================================================================//

//==================================================================//
// CLASSES							    //
//		GenericDist, Uniform, Gaussian, GTC	            //
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

//double drand48();  		// prototypes for random number generator
//void srand48(long seedval);     // required due to bug in standard header files

//==================================================================//
// CLASS 							    //
//		GenericDist					    //
// 								    //
// Description: Base class for Probability Distributions            //
//==================================================================//

class GenericDist
{
public:
	virtual float GetNumber()=0;
        virtual float GetNumber(double time);
	virtual ~GenericDist();
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

class Uniform : public GenericDist
{
public:
	Uniform();
        Uniform(float radius, float mean);
	~Uniform();
	float GetNumber();
protected:
	float _radius;
        float _mean;
};


//==================================================================//
// Class                                                            //
//         Gaussian                                                 //
// Description: Gaussian Distribution                               //
//==================================================================//

class Gaussian : public GenericDist
{
public:
	Gaussian();
	Gaussian(float variance, float mean);
	~Gaussian();
	float GetNumber();
protected:
	float _variance;
	float _mean;
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
	RandomVelocity(GenericDist* noise, float sample_period, float radius,
		    float mean);
	~RandomVelocity();
	float GetNumber(double time);
protected:
	float _sample_period;
        float _radius;
	float _mean;
        GenericDist* _noise;
	float _position;
	float _time;
	float _velocity;
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
	AttDist(GenericDist* r, GenericDist* p, GenericDist* y);
	~AttDist();
	GenericDist* roll;
	GenericDist* pitch;
	GenericDist* yaw;

};

//==================================================================//
// Function SeedFromClock					    //
// 								    //
// Description: Causes the pseudo random number generator  (drand48)//
// to be seeded by the clock.                                       //
//==================================================================//

void SeedFromClock();
#endif
