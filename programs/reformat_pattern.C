//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		reformat_pattern
//
// SYNOPSIS
//		reformat_pattern <V pol pattern> <H pol pattern>
//
// DESCRIPTION
//		Reads in the SeaWinds antenna patterns and generates
//		simulation patterns.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<V pol pattern>		The pattern file for V-pol
//		<H pol pattern>		The pattern file for H-pol
//
// EXAMPLES
//		An example of a command line is:
//			% reformat_pattern 5nl42917_Vpol.dat 5nk62922_Hpol.dat
//
// ENVIRONMENT
//		Not environment dependent.
//
// EXIT STATUS
//		The following exit values are returned:
//		0	Program executed successfully
//		>0	Program had an error
//
// NOTES
//		None.
//
// AUTHOR
//		James N. Huddleston
//		hudd@acid.jpl.nasa.gov
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
	"@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <stdlib.h>
#include "Misc.h"
#include "Constants.h"
#include "Attitude.h"
#include "CoordinateSwitch.h"
#include "Matrix3.h"
#include "Beam.h"
#include "Array.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define POINTS	260000
#define HPOL	0
#define VPOL	1
#define G0V		40.91
#define G0H		39.27

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int read_pattern(const char* filename, float* el, float* az, float* gain,
		int pol, int* number_read);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<V pol pattern>", "<H pol pattern>", 0};

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
	int		argc,
	char*	argv[])
{
	//------------------------//
	// parse the command line //
	//------------------------//

	const char* command = no_path(argv[0]);
	if (argc != 3)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* v_pol_file = argv[clidx++];
	const char* h_pol_file = argv[clidx++];

	//----------------------//
	// read in the patterns //
	//----------------------//

	float v_el[POINTS], v_az[POINTS], v_gain[POINTS];
	int v_pol_count;
	if (! read_pattern(v_pol_file, v_el, v_az, v_gain, VPOL, &v_pol_count))
	{
		fprintf(stderr, "%s: error reading file %s\n", command, v_pol_file);
		exit(1);
	}

	float h_el[POINTS], h_az[POINTS], h_gain[POINTS];
	int h_pol_count;
	if (! read_pattern(h_pol_file, h_el, h_az, h_gain, HPOL, &h_pol_count))
	{
		fprintf(stderr, "%s: error reading file %s\n", command, h_pol_file);
		exit(1);
	}

	//--------------//
	// locate peaks //
	//--------------//

	float max_v_gain = -99.0;
	float max_v_az = 0.0;
	float max_v_el = 0.0;
	for (int i = 0; i < v_pol_count; i++)
	{
		if (v_gain[i] > max_v_gain)
		{
			max_v_gain = v_gain[i];
			max_v_az = v_az[i];
			max_v_el = v_el[i];
		}
	}

	float max_h_gain = -99.0;
	float max_h_az = 0.0;
	float max_h_el = 0.0;
	for (int i = 0; i < h_pol_count; i++)
	{
		if (h_gain[i] > max_h_gain)
		{
			max_h_gain = h_gain[i];
			max_h_az = h_az[i];
			max_h_el = h_el[i];
		}
	}

	printf("%g %g %g\n", max_v_az * rtd, max_v_el * rtd, max_v_gain);
	printf("%g %g %g\n", max_h_az * rtd, max_h_el * rtd, max_h_gain);

	// Parameters defining the structure of the gain measurements.
	int Nxm = 647;
	int Nym = 401;
	double xm_spacing = 0.031 * dtr;
	double ym_spacing = 0.05 * dtr;
	int ixm_zero = 324;
	int iym_zero = 201;

	//
	// Setup the parameters and arrays needed for the beam patterns.
	// The width's and spacings are specified for azimuth and elevation
	// angles as defined for the SeaWinds-1B simulator (see Matrix3.C).
	// For now, the two beam patterns have the same parameters, and
	// match the parameters of the measured gains.  Regridding in elevation
	// may be needed to ensure that everthing is uniformly spaced.
	// For now, we assume that the measurements are uniformly spaced.
	// (They nearly are.)
	//

	double x_spacing = xm_spacing;
	double y_spacing = ym_spacing;
	int Nx = Nxm;
	int Ny = Nym;
	int ix_zero = ixm_zero;
	int iy_zero = iym_zero;

	float** power_gainv = (float**)make_array(sizeof(float),2,Nx,Ny);
	if (power_gainv == NULL)
	{
		printf("Error allocating v-pol pattern array\n");
		exit(-1);
	}
	float** power_gainh = (float**)make_array(sizeof(float),2,Nx,Ny);
	if (power_gainh == NULL)
	{
		printf("Error allocating h-pol pattern array\n");
		exit(-1);
	}

	//
	// Transfer pattern data from 1-D storage to 2-D storage.
	// Also convert from dB to real units and apply the peak pattern values.
	//

    for (int i=0; i < Nx; i++)
    for (int j=0; j < Ny; j++)
    {
		power_gainv[i][j] = pow(10.0, (v_gain[i*Ny + j]+G0V)/10.0);
		power_gainh[i][j] = pow(10.0, (h_gain[i*Ny + j]+G0H)/10.0);
	}

	//
	// Make beam objects and write them out.
	//

	Beam beamv;
	beamv.SetBeamPattern(Nx,Ny,ix_zero,iy_zero,x_spacing,y_spacing,
								max_v_el,max_v_az,power_gainv);
	beamv.WriteBeamPattern("beam2.pat");

	Beam beamh;
	beamh.SetBeamPattern(Nx,Ny,ix_zero,iy_zero,x_spacing,y_spacing,
								max_h_el,max_h_az,power_gainh);
	beamh.WriteBeamPattern("beam1.pat");
	
	return (0);

}

//--------------//
// read_pattern //
//--------------//

int
read_pattern(
	const char*		filename,
	float*			el,
	float*			az,
	float*			gain,
	int				pol,
	int*			number_read)
{
	printf("Reading %s...\n", filename);
	FILE* fp = fopen(filename, "r");
	if (fp == NULL)
		return(0);

	char line[1024];

	// skip first two lines
	fgets(line, 1024, fp);
	fgets(line, 1024, fp);

	float elev, azim, efield[2];

	int idx = 0;
	while (fgets(line, 1024, fp) == line)
	{
		if (sscanf(line, " %g %g %g %g", &elev, &azim, &(efield[0]),
			&(efield[1])) != 4)
		{
			break;
		}
		el[idx] = elev * dtr;
		az[idx] = azim * dtr;
		gain[idx] = efield[pol];
		idx++;
	}

	*number_read = idx;

	int retval;
	if (feof(fp))
		retval = 1;
	else
		retval = 0;

	fclose(fp);

	printf("  Done.\n");

	return(retval);
}
