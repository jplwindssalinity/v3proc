//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_earthfield_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include "LonLat.h"
#include "Constants.h"
#include "Array.h"
#include "EarthField.h"

//============//
// EarthField //
//============//

EarthField::EarthField()
:   field(NULL), _lonCount(0), _lonMin(0.0), _lonMax(0.0), _lonStep(0.0),
    _latCount(0), _latMin(0.0), _latMax(0.0), _latStep(0.0)
{
	return;
}

EarthField::~EarthField()
{
	if (field != NULL)
	{
    	Deallocate();
	}
	return;
}

//------------------------------//
// EarthField::NearestElement
//------------------------------//

int
EarthField::NearestElement(
	LonLat		lon_lat,
	float*		element)
{
	if (! field)
	{
		printf("Error: attempted to access non-existent EarthField\n");
		exit(-1);
	}

	// put longitude in range (hopefully)
	int wrap_factor = (int)ceil((_lonMin - lon_lat.longitude) / two_pi);
	float lon = lon_lat.longitude + (float)wrap_factor * two_pi;

	// convert to longitude index
	int lon_idx = (int)((lon - _lonMin) / _lonStep + 0.5);
	if (lon_idx < 0 || lon_idx >= _lonCount)
	{
		printf("EarthField::InterpolatedElement: lon = %g out of range\n",
				lon_lat.longitude);
		return(0);
	}

	// convert to latitude index
	int lat_idx = (int)((lon_lat.latitude - _latMin) / _latStep + 0.5);
	if (lat_idx < 0 || lat_idx >= _latCount)
	{
		printf("EarthField::InterpolatedElement: lat = %g out of range\n",
				lon_lat.latitude);
		return(0);
	}

	*element = field[lon_idx][lat_idx];

	return(1);
}

//-----------------------------------//
// EarthField::InterpolatedElement
//-----------------------------------//

int
EarthField::InterpolatedElement(
	LonLat		lon_lat,
	float*		element)
{
	if (! field)
	{
		printf("Error: attempted to access non-existent EarthField\n");
		exit(-1);
	}

	// put longitude in range (hopefully)
	int wrap_factor = (int)ceil((_lonMin - lon_lat.longitude) / two_pi);
	float lon = lon_lat.longitude + (float)wrap_factor * two_pi;

	// find lower longitude index
	int lon_idx_1 = (int)((lon - _lonMin) / _lonStep);
	if (lon_idx_1 < 0 || lon_idx_1 >= _lonCount)
	{
		printf("EarthField::InterpolatedElement: longitude = %g out of range\n",
				lon_lat.longitude);
		return(0);
	}
	float lon_1 = _lonMin + lon_idx_1 * _lonStep;

	// find upper longitude index
	int lon_idx_2 = lon_idx_1 + 1;		// try the quick easy way
	if (lon_idx_2 >= _lonCount)
	{
		// must do the long hard way
		lon = lon_lat.longitude + _lonStep;
		wrap_factor = (int)ceil((_lonMin - lon) / two_pi);
		lon += (float)wrap_factor * two_pi;

		lon_idx_2 = (int)((lon - _lonMin) / _lonStep);
		if (lon_idx_2 >= _lonCount)
		{
			printf("EarthField::InterpolatedElement: lon = %g out of range\n",
					lon_lat.longitude);
			return(0);
		}
	}
	if (lon_idx_2 < 0)
	{
		printf("EarthField::InterpolatedElement: lon = %g out of range\n",
				lon_lat.longitude);
		return(0);
	}
	float lon_2 = _lonMin + lon_idx_2 * _lonStep;

	// find lower latitude index
	int lat_idx_1 = (int)((lon_lat.latitude - _latMin) / _latStep);
	if (lat_idx_1 < 0 || lat_idx_1 >= _latCount)
	{
		printf("EarthField::InterpolatedElement: lat = %g out of range\n",
				lon_lat.latitude);
		return(0);
	}
	float lat_1 = _latMin + lat_idx_1 * _latStep;

	// find upper latitude index
	int lat_idx_2 = lat_idx_1 + 1;
	if (lat_idx_2 >= _latCount)
	{
		printf("EarthField::InterpolatedElement: lat = %g out of range\n",
				lon_lat.latitude);
		return(0);
	}
	float lat_2 = _latMin + lat_idx_2 * _latStep;

	float p;
	if (lon_idx_1 == lon_idx_2)
		p = 1.0;
	else
		p = (lon - lon_1) / (lon_2 - lon_1);
	float pn = 1.0 - p;

	float q;
	if (lat_idx_1 == lat_idx_2)
		q = 1.0;
	else
		q = (lon_lat.latitude - lat_1) / (lat_2 - lat_1);
	float qn = 1.0 - q;

	float e1 = field[lon_idx_1][lat_idx_1];
	float e2 = field[lon_idx_2][lat_idx_1];
	float e3 = field[lon_idx_1][lat_idx_2];
	float e4 = field[lon_idx_2][lat_idx_2];

	*element = pn * qn * e1 + p * qn * e2 + p * q * e3 + pn * q * e4;
	return(1);
}

//----------------------//
// EarthField::Allocate
//----------------------//

int
EarthField::Allocate()
{
	if (field != NULL)
		return(0);

	field = (float **)make_array(sizeof(float), 2, _lonCount, _latCount);

	if (field == NULL)
	{
		printf("Error allocating space for an EarthField array\n");
		return(0);
	}

	for (int i = 0; i < _lonCount; i++)
	{
		for (int j = 0; j < _latCount; j++)
		{
			field[i][j] = 0.0;
		}
	}

	return(1);
}

//------------------------//
// EarthField::Deallocate
//------------------------//

int
EarthField::Deallocate()
{
	if (field == NULL)
		return(1);

	free_array((void *)field, 2, _lonCount, _latCount);

	field = NULL;
	return(1);
}

//----------------------------//
// EarthField::GetDimensions
//----------------------------//

int
EarthField::GetDimensions(int* Nlon, int* Nlat)
{
	if (!field)
	{
		*Nlon = 0;
		*Nlat = 0;
		return(1);
	}

	*Nlon = _lonCount;
	*Nlat = _latCount;
	return(1);

}

//----------------------------//
// EarthField::Setup
//----------------------------//

int
EarthField::Setup(double lonmin,
	double lonmax,
	double lonstep,
	double latmin,
	double latmax,
	double latstep)

{
	//---------------------------------//
	// Replace any pre-existing field
	//---------------------------------//

	if (field)
	{	// replace exising field
		Deallocate();
	}

	//---------------------------------//
	// Check for valid inputs
	//---------------------------------//

	if (lonmin < 0.0 || lonmin > lonmax || lonmax > two_pi || lonstep < 0.0)
	{
		printf("Error: Invalid longitude setup in EarthField\n");
		exit(-1);
	}

	if (latmin < -pi/2.0 || latmin > latmax || latmax > pi/2.0 || latstep < 0.0)
	{
		printf("Error: Invalid latitude setup in EarthField\n");
		exit(-1);
	}

	//----------------------------------------//
	// Compute array sizes and store limits. 
	//----------------------------------------//

	_lonCount = (int)((lonmax - lonmin)/lonstep);
	_lonMin = lonmin;
	_lonMax = lonmin + _lonCount*lonstep;
	_lonStep = lonstep;

	_latCount = (int)((latmax - latmin)/latstep);
	_latMin = latmin;
	_latMax = latmin + _latCount*latstep;
	_latStep = latstep;

	return(1);

}

//------------------------//
// EarthField::Read
//------------------------//

int
EarthField::Read(char* filename)
{
	//---------------------------------//
	// Replace any pre-existing field
	//---------------------------------//

	if (field)
	{
		Deallocate();
	}

	FILE* fp = fopen(filename,"r");
	if (fp == NULL)
	{
		return(0);
	}

	//---------------------------------//
	// Read header information
	//---------------------------------//

    if (fread((void *)&_lonCount, sizeof(int), 1, fp) != 1 ||
        fread((void *)&_lonMin, sizeof(float), 1, fp) != 1 ||
        fread((void *)&_lonMax, sizeof(float), 1, fp) != 1 ||
        fread((void *)&_lonStep, sizeof(float), 1, fp) != 1 ||
    	fread((void *)&_latCount, sizeof(int), 1, fp) != 1 ||
        fread((void *)&_latMin, sizeof(float), 1, fp) != 1 ||
        fread((void *)&_latMax, sizeof(float), 1, fp) != 1 ||
        fread((void *)&_latStep, sizeof(float), 1, fp) != 1)
    {
        return(0);
    }
 
	//------------------------------------------------------//
	// Make space for the field based on the header info.
	// Don't use Allocate to avoid unneeded zeroing.
	//------------------------------------------------------//

	field = (float **)make_array(sizeof(float), 2, _lonCount, _latCount);
	if (field == NULL)
	{
		printf("Error allocating space for an EarthField array\n");
		exit(-1);
	}

	//---------------------------------//
	// Read the field array
	//---------------------------------//

	for (int i=0; i < _lonCount; i++)
	for (int j=0; j < _latCount; j++)
	{
    	if (fread((void *)&(field[i][j]), sizeof(float), 1, fp) != 1)
		{	// Error reading.
			return(0);
		}
	}

	return(1);

}

//------------------------//
// EarthField::Write
//------------------------//

int
EarthField::Write(char* filename)
{
	if (! field)
	{
		printf("Error: Nothing to write in EarthField::Write\n");
		exit(-1);
	}

	//---------------------------------//
	// Replace any pre-existing file
	//---------------------------------//

	FILE* fp = fopen(filename,"w");
	if (fp == NULL)
	{
		printf("Error opening %s for writing in EarthField::Write\n",filename);
		exit(-1);
	}

	//---------------------------------//
	// Write header information
	//---------------------------------//

    if (fwrite((void *)&_lonCount, sizeof(int), 1, fp) != 1 ||
        fwrite((void *)&_lonMin, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&_lonMax, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&_lonStep, sizeof(float), 1, fp) != 1 ||
    	fwrite((void *)&_latCount, sizeof(int), 1, fp) != 1 ||
        fwrite((void *)&_latMin, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&_latMax, sizeof(float), 1, fp) != 1 ||
        fwrite((void *)&_latStep, sizeof(float), 1, fp) != 1)
    {
		printf("Error writing header data to %s in EarthField::Write\n",
			filename);
		exit(-1);
    }
 
	//---------------------------------//
	// Write the field array
	//---------------------------------//

	for (int i=0; i < _lonCount; i++)
	for (int j=0; j < _latCount; j++)
	{
    	if (fwrite((void *)&(field[i][j]), sizeof(float), 1, fp) != 1)
		{	// Error writing.
			printf("Error writing data to %s in EarthField::Write\n",
				filename);
			exit(-1);
		}
	}

	return(1);

}

//------------------------//
// EarthField::Scale
//------------------------//

int
EarthField::Scale(float factor)
{
	if (!field)
	{
		printf("Warning: attempted to scale an empty EarthField\n");
		return(0);
	}

	for (int i=0; i < _lonCount; i++)
	for (int j=0; j < _latCount; j++)
	{
		field[i][j] *= factor; 
	}

	return(1);

}

//------------------------//
// EarthField::GetMean
//------------------------//

double
EarthField::GetMean()
{
	if (!field)
	{
		printf("Warning: attempted to find the mean of an empty EarthField\n");
		return(0.0);
	}

	int N = _lonCount * _latCount;	// total number of elements
	double sum1 = 0.0;

	for (int i=0; i < _lonCount; i++)
	for (int j=0; j < _latCount; j++)
	{
		sum1 += field[i][j];
	}

	double mean = 1.0/N * sum1;

	return(mean);

}

//------------------------//
// EarthField::GetVariance
//------------------------//

double
EarthField::GetVariance()
{
	if (!field)
	{
		return(0.0);
	}

	int N = _lonCount * _latCount;	// total number of elements
	double mean = GetMean();
	double sum1 = 0.0;
	double sum2 = 0.0;

	for (int i=0; i < _lonCount; i++)
	for (int j=0; j < _latCount; j++)
	{
		double a = field[i][j] - mean;
		sum1 += a*a;
		sum2 += a;
	}

	double var = 1.0/(N - 1) * (sum1 - 1.0/N * sum2*sum2);

	return(var);

}

