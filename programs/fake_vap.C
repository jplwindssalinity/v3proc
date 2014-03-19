//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		fake_vap
//
// SYNOPSIS
//		fake_vap <spd> <dir> <outfile>
//
// DESCRIPTION
//		Generates a vap wind field
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<spd>		The wind speed in m/s
//		<dir>		The wind direction in degrees ccw from E
//		<outfile>	The output file name
//
// EXAMPLES
//		An example of a command line is:
//			% fake_vap 5.0 20.0 vap.5.20
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
#include "Wind.h"
#include "Constants.h"
/*
#include <stdlib.h>
#include <fcntl.h>
#include "List.h"
#include "BufferedList.h"
#include "ConfigList.h"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "Array.h"
#include "Meas.h"
#include "Ephemeris.h"
#include "Wind.h"
*/

//-----------//
// TEMPLATES //
//-----------//

/*
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<WindVectorPlus>;
*/

//-----------//
// CONSTANTS //
//-----------//

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<spd>", "<dir>", "<outfile>", 0};

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
	if (argc != 4)
		usage(command, usage_array, 1);

	int clidx = 1;
	float spd = atof(argv[clidx++]);
	float dir = atof(argv[clidx++]);
	const char* output_file = argv[clidx++];

	//-----------------------//
	// generate the vap data //
	//-----------------------//

	float u[VAP_LAT_DIM][VAP_LON_DIM];
	float v[VAP_LAT_DIM][VAP_LON_DIM];

	float use_u = spd * (float)cos(dir * dtr);
	float use_v = spd * (float)sin(dir * dtr);

	for (int i = 0; i < VAP_LAT_DIM; i++)
	{
		for (int j = 0; j < VAP_LON_DIM; j++)
		{
			u[i][j] = use_u;
			v[i][j] = use_v;
		}
	}

	//--------------------//
	// write the vap data //
	//--------------------//

	FILE* fp = fopen(output_file, "w");
	if (fp == NULL)
	{
		fprintf(stderr, "%s: error opening output file %s\n", command,
			output_file);
		exit(1);
	}

	int size = VAP_LON_DIM * VAP_LAT_DIM * sizeof(float);
	if (fwrite((void *)u, size, 1, fp) != 1 ||
		fwrite((void *)v, size, 1, fp) != 1)
	{
		fprintf(stderr, "%s: error writing to output file %s\n", command,
			output_file);
		exit(1);
	}

	fclose(fp);

	return (0);
}
