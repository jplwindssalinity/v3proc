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
//	 GTC							    //
// Description: Gaussian with Time Constant                         //
// X(t) is a sequence of samples chosen from a gaussian             //
// distribution. The sequence Y(t) of samples from the              //
// GTC distribution depends on X and discrete time t in the         //
// following manner: Y(t)=(1-tau)(Y(t-1)) + (tau)X(t).              //
// Tau is a time constant between 0 and 1. Y(0)=0 inital condition  //
//==================================================================//

class GTC : public Gaussian
{
public:
	GTC();
	GTC(float variance, float mean, float tau);
	~GTC();
	float GetNumber();
protected:
	float _tau;
        float _prev;
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
