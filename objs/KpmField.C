//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_kpmfield_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include "KpmField.h"
#include "Constants.h"
#include "Beam.h"

//==========//
// KpmField //
//==========//

KpmField::KpmField()
{
	_gaussianRv.SetMean(0.0);
	_gaussianRv.SetVariance(1.0);
	_corrLength = 0.0;
	return;
}

KpmField::~KpmField()
{
	return;
}

//----------------------//
// KpmField::Build
//----------------------//

// corr_length should be in km.

int
KpmField::Build(float corr_length)
{

	_corrLength = corr_length;
	if (_corrLength < 0.0)
	{
		printf("Error: KpmField received a negative correlation length\n");
		exit(-1);
	}
	else if (_corrLength == 0.0)
	{	// With no correlation, on the fly gaussian rv's are supplied.
		corr.Deallocate();
		uncorr.Deallocate();
		return(1);
	}

	//--------------------------------------------//
    // Configure field sizes.
	// This will destroy any pre-existing fields.
	//--------------------------------------------//

	// Set step sizes to a fraction of a correlation length at the equator.
	float lonlat_step = corr_length/STEPS_PER_CORRLENGTH / r1_earth;

	corr.Setup(0.0,two_pi,lonlat_step,-pi/2.0,pi/2.0,lonlat_step);
	uncorr.Setup(0.0,two_pi,lonlat_step,-pi/2.0,pi/2.0,lonlat_step);

	//-----------------------------//
    // Allocate fields
	//-----------------------------//

	if (!corr.Allocate() || !uncorr.Allocate())
	{
		printf("Error allocating fields in KpmField::Build\n");
		return(0);
	}

	//-----------------------------//
    // Fill uncorrelated field
	//-----------------------------//

	int Nlon,Nlat;
	int i,j;
	uncorr.GetDimensions(&Nlon,&Nlat);
	printf("Field sizes: %d by %d\n",Nlon,Nlat);

	for (i=0; i < Nlon; i++)
	for (j=0; j < Nlat; j++)
	{
		uncorr.field[i][j] = _gaussianRv.GetNumber();
	}

	//----------------------------------//
    // Compute correlated field.
	// Uses a brute force convolution.
	//----------------------------------//

	double denom = _corrLength*_corrLength/2.0;
	double lat_rad_to_km = (r1_earth + r2_earth)/2.0;
	double latitude,lon_rad_to_km;

	int ip,jp,ipmin,ipmax,jpmin,jpmax;
	for (i=0; i < Nlon; i++)
	{
		printf("corr field: lon = %g\n",i*lonlat_step*rtd);
		for (j=0; j < Nlat; j++)
		{
			latitude = j*lonlat_step;
			lon_rad_to_km = lat_rad_to_km * cos(latitude);
			ipmin = i - STEPS_PER_CORRLENGTH*N_CORRLENGTHS_INTEGRATE;
			ipmax = i + STEPS_PER_CORRLENGTH*N_CORRLENGTHS_INTEGRATE;
			jpmin = j - STEPS_PER_CORRLENGTH*N_CORRLENGTHS_INTEGRATE;
			jpmax = j + STEPS_PER_CORRLENGTH*N_CORRLENGTHS_INTEGRATE;

			if (ipmin < 0) ipmin = 0;
			if (ipmax > Nlon) ipmax = Nlon;
			if (jpmin < 0) jpmin = 0;
			if (jpmax > Nlat) jpmax = Nlat;

			for (ip=ipmin; ip < ipmax; ip++)
			for (jp=jpmin; jp < jpmax; jp++)
			{
				// Crude but fast distance calculation. (short distances only)
				// This method will suffer from larger errors near the poles.
				double lon_step = lonlat_step*(ip - i)*lon_rad_to_km;
				double lat_step = lonlat_step*(jp - j)*lat_rad_to_km;
				double r2 = lon_step*lon_step + lat_step*lat_step;
				// Convolution sum.
				corr.field[i][j] += exp(-r2 / denom) * uncorr.field[ip][jp];
			}
		}
	}

	//-----------------------------------------------//
    // Ditch uncorrelated field to save memory.
	//-----------------------------------------------//

	uncorr.Deallocate();

	//-----------------------------------------------//
    // Scale correlated field to have unit variance.
	//-----------------------------------------------//

	float var = corr.GetVariance();
	corr.Scale(1.0/sqrt(var));

	return(1);

}

//-------------------//
// KpmField::GetKpm
//-------------------//

//
// Return the Kpm value appropriate for the given polarization and wind speed.
// This function uses the wind speed value (rounded to the nearest integer)
// to look up the Kpm value in a table (one each for V and H pol).
//
// Inputs:
//  pol = 0 for V-pol, = 1 for H-pol.
//  spd = wind speed in m/s.
//
// Return Value:
//	The value of Kpm to use (ie., normalized standard deviation of sigma0).
//

float
KpmField::GetKpm(
	int pol,
	float spd)

{
	// V-pol is index 0, H-pol is index 1 for the 1st dim.
	static float Kpmtable[2][36] =
	{ {6.3824e-01, 5.6835e-01, 4.9845e-01, 4.2856e-01, 3.5867e-01, 2.8877e-01,
	2.5092e-01, 2.1307e-01, 1.9431e-01, 1.7555e-01, 1.7072e-01, 1.6589e-01,
	1.6072e-01, 1.5554e-01, 1.4772e-01, 1.3990e-01, 1.2843e-01, 1.1696e-01,
	1.1656e-01, 1.1615e-01, 1.0877e-01, 1.0138e-01, 9.0447e-02, 7.9516e-02,
	8.6400e-02, 9.3285e-02, 8.4927e-02, 7.6569e-02, 7.2302e-02, 6.8036e-02,
	7.7333e-02, 8.6630e-02, 9.0959e-02, 9.5287e-02, 9.9616e-02, 1.0394e-01},
	  {4.3769e-01,  4.0107e-01, 3.6446e-01, 3.2784e-01, 2.9122e-01, 2.5461e-01,
	2.2463e-01, 1.9464e-01, 1.7066e-01, 1.4667e-01, 1.3207e-01, 1.1747e-01,
	1.0719e-01, 9.6918e-02, 9.0944e-02, 8.4969e-02, 7.7334e-02, 6.9699e-02,
	6.9107e-02, 6.8515e-02, 6.6772e-02, 6.5030e-02, 5.7429e-02, 4.9828e-02,
	4.3047e-02, 3.6266e-02, 3.0961e-02, 2.5656e-02, 2.9063e-02, 3.2471e-02,
	2.7050e-02, 2.1629e-02, 2.8697e-02, 3.5764e-02, 4.2831e-02, 4.9899e-02}};

	float Kpm;
	if (spd < 0)
	{
		printf("Error: KpmField::GetKpm received a negative wind speed\n");
		exit(-1);
	}
	else if (spd < 35.0)
	{
		Kpm = Kpmtable[pol][(int)(spd+0.5)];
	}
	else
	{
		Kpm = Kpmtable[pol][35];
	}

	return(Kpm);
}

float
KpmField::GetRV(int polarization, float wspd, LonLat lon_lat)
{
	float RV;
	float rv1;

	float Kpm = GetKpm(polarization-V_POL,wspd);

	if (! corr.field)
	{	// no spatial correlation, so just draw a gaussian random number
		rv1 = _gaussianRv.GetNumber();
	}
	else
	{
		if (corr.InterpolatedElement(lon_lat,&rv1) == 0)
		{
			printf("Error getting correlated rv in KpmField::GetRV\n");
			exit(-1);
		}
	}

	// Scale for unit mean, variance = Kpm^2.
	RV = rv1*Kpm + 1.0;

	if (RV < 0.0)
	{
		RV = 0.0;	// Do not allow negative sigma0's.
	}

	return(RV);

}

