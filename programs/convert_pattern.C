//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		convert_pattern
//
// SYNOPSIS
//		convert_pattern <V pol pattern> <H pol pattern>
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
//			% convert_pattern 5nl42917_Vpol.dat 5nk62922_Hpol.dat
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

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define POINTS	260000
#define HPOL	0
#define VPOL	1

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

const char* usage_array[] = { "<V pol pattern>", "<H pol pattern>",
	"<mech az>", "<mech el>", 0};

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
	if (argc != 5)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* v_pol_file = argv[clidx++];
	const char* h_pol_file = argv[clidx++];
	float mech_az = atof(argv[clidx++]) * dtr;
	float mech_el = atof(argv[clidx++]) * dtr;

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

	//----------------------------------//
	// set up coordinate transformation //
	//----------------------------------//

	Attitude mech_attitude;
	mech_attitude.Set(0.0, mech_el, mech_az, 1, 2, 3);
	CoordinateSwitch ant_to_mech(mech_attitude);
	CoordinateSwitch mech_to_ant = ant_to_mech.ReverseDirection();

	//--------------------------------------//
	// convert all points to antenna system //
	//--------------------------------------//

	for (int i = 0; i < v_pol_count; i++)
	{
		Vector3 v1;
		v1.AzimuthElevationSet(1.0, v_az[i], v_el[i]);
		Vector3 v2 = mech_to_ant.Forward(v1);
		double r, a, e;
		v2.AzimuthElevationGet(&r, &a, &e);
		v_az[i] = a;
		v_el[i] = e;
	}

	for (int i = 0; i < h_pol_count; i++)
	{
		Vector3 v1;
		v1.AzimuthElevationSet(1.0, h_az[i], h_el[i]);
		Vector3 v2 = mech_to_ant.Forward(v1);
		double r, a, e;
		v2.AzimuthElevationGet(&r, &a, &e);
		h_az[i] = a;
		h_el[i] = e;
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
