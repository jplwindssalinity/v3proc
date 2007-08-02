// Prgram to generate one-way Gaussian antenna pattern

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

float antnna(float theta, float phi, float bw1, float bw2)
{
	float alpha, beta, pattern;

	alpha= asin(sin(theta)*cos(phi));
	beta= asin(sin(theta)*sin(phi));
	pattern= exp(-2.77*alpha*alpha/(bw1*bw1) - 2.77*beta*beta/(bw2*bw2));
	return pattern;
}